Sections_Trie *
Sections_Trie__get(Sections_Trie *trie, U64 hash, String8 name)
{
        Sections_Trie *result = 0;
        Sections_Trie *trie_current = trie;
        U64 hash_shifted = hash;
        for (;;)
        {
                B32 trie_current_zero = trie_current == 0;
                B32 found = !trie_current_zero && String8__match_exact(trie_current->section.name, name);
                if (found)
                {
                        result = trie_current;
                }

                B32 break_should = trie_current_zero || found;
                if (break_should)
                {
                        break;
                }

                trie_current = trie_current->children[(hash_shifted >> 62)];
                hash_shifted = hash_shifted << 2;
        }

        return result;
}

Sections_Trie *
Sections_Trie__get_or_default(Sections_Trie **root, Arena *arena, U64 hash, String8 name)
{
        B32 initialized = 0;
        B32 match = 0;

        Sections_Trie **trie_current = root;
        U64 hash_shifted = hash;
        for (;;)
        {
                if (*trie_current == 0)
                {
                        Sections_Trie *trie_new = Arena__push_struct_m(arena, Sections_Trie);
                        memory_zero_array(trie_new->children);
                        *trie_current = trie_new;
                        initialized = 1;
                }

                if (!initialized && String8__match_exact((*trie_current)->section.name, name))
                {
                        match = 1;
                }

                B32 break_should = initialized || match;
                if (break_should)
                {
                        break;
                }

                trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
                hash_shifted = hash_shifted << 2;
        }

        return *trie_current;
}

// TODO(medium): should add all boilerplate containing special section attributes, see https://gabi.xinuos.com/v42/elf/03-sheader.html#special-sections.
// We just need a struct containing type and attributes and a String8 table.

internal Section *
Sections_Table__get(Sections_Table *sections_table, String8 name)
{
        U64 hash = FNV_hash_U64(name);
        Sections_Trie *trie = Sections_Trie__get(sections_table->root, hash, name);

        Section *result = trie ? &trie->section : 0;
        return result;
}

// Return the section or create one with appropriate defaults, meaning:
//
// 1. Set the provided name.
// 2. Set the section index field.
// 3. Add an empty fragment to it.
internal Section *
Sections_Table__get_or_default(Sections_Table *sections_table, String8 name, U32 location)
{
        U64 hash = FNV_hash_U64(name);
        Sections_Trie *trie = Sections_Trie__get_or_default(&sections_table->root, sections_table->arena, hash, name);

        Section zero = {0};
        B32 zero_is = memory_match_struct(&trie->section, &zero);
        if (zero_is)
        {
                sections_table->count += 1;
                DLL_push_front_m(sections_table->first, sections_table->last, &trie->section);

                // TODO(low): support configuration for it.
                Arena *arena = Arena__allocate_m();
                trie->section = (Section)
                {
                        .name     = String8__duplicate(sections_table->arena, name),
                        .location = location,
                };
                Fragment  *fragment         = Arena__push_struct_m(arena, Fragment);
                Fragments *fragments        = &trie->section.fragments;
                           fragments->arena = arena;
                SLL_queue_push_m(fragments->first, fragments->last, fragment);
        }

        return &trie->section;
}

// Add the undefined, absolute and common sections.
internal void
Sections_Table__add_internal(Sections_Table *sections_table)
{
        Section *first = sections_table->first;
        Section *last  = sections_table->last;

        U32 location = 0;
        Section *undefined       = Sections_Table__get_or_default(sections_table, section_name_undefined,  location);

        Section *absolute        = Sections_Table__get_or_default(sections_table, section_name_absolute, location);
                 absolute->index = ELF_Section_Index__Absolute;

        Section *common          = Sections_Table__get_or_default(sections_table, section_name_common, location);
                 common->index   = ELF_Section_Index__Common;

        sections_table->undefined = undefined;
        sections_table->absolute  = absolute;
        sections_table->common    = common;

        // Restore previous first/last
        sections_table->first = first;
        sections_table->last  = last;

        return;
}

// Add the .text, .data, .bss sections. Sets .text as the current section if unset.
internal void
Sections_Table__add_basic(Sections_Table *sections_table)
{
        U32 location = 0;
        Section *text                    = Sections_Table__get_or_default(sections_table, section_name_text, location);
                 text->elf.type          = ELF_Section_Header_Type__Program_Data;
                 text->elf.flags         = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR;
                 // TODO(configuration): depends on extensions.
                 text->elf.alignment     = 4;

        Section *data                    = Sections_Table__get_or_default(sections_table, section_name_data, location);
                 data->elf.type          = ELF_Section_Header_Type__Program_Data;
                 data->elf.flags         = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE;
                 // TODO(configuration): depends on extensions.
                 data->elf.alignment     = 8;

        Section *bss                     = Sections_Table__get_or_default(sections_table, section_name_bss, location);
                 bss->elf.type           = ELF_Section_Header_Type__No_Data;
                 bss->elf.flags          = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE;
                 // TODO(configuration): depends on extensions.
                 data->elf.alignment     = 8;

        if (!sections_table->current)
        {
                sections_table->current = text;
        }

        return;
}



// Add a fixed size instruction into a fragment. If there is a fixup associated to this function (fixup != 0),
// track the information of where this instruction has been placed.
internal void
Section__add_instruction_fixed
(
        Section *section,
        Fixup   *fixup,

        U32      encoding,
        U8       encoding_size,
        U32      location
)
{
        U8 *data = Fragments__push
        (
                 &section->fragments,
                 location,
                 encoding_size
        );

        // Track its precise location within the fragment. Important to do it _after_ we've written it
        // since it might have landed into another fragment because of low capacity of the previous.
        if (fixup)
        {
                Fragment *last = section->fragments.last;
                U8 *fragment_write_area = last->data + (last->data_size - encoding_size);

                fixup->fragment            = last;
                fixup->fragment_write_area = fragment_write_area;
                fixup->fragment_write_size = encoding_size;
        }

        memory_copy(data, (U8 *)&encoding, encoding_size);
        return;
}
