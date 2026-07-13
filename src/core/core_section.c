internal B32
Section__zero_is(Section *s)
{
        B32 result = memory_match_struct(s, &Section__zero);
        return result;
}

Sections_Trie *
Sections_Trie_Chunk_List__push(Sections_Trie_Chunk_List *chunks, Arena *arena, U64 capacity)
{
        if (chunks->last == 0 || chunks->last->count >= chunks->last->capacity)
        {
                Sections_Trie_Chunk *chunk_new = Arena__push_struct_m(arena, Sections_Trie_Chunk);
                chunk_new->nodes = Arena__push_array_m(arena, Sections_Trie, capacity);
                chunk_new->capacity = capacity;

                SLL_queue_push_m(chunks->first, chunks->last, chunk_new);
                chunks->count += 1;
        }

        Sections_Trie_Chunk *chunk_last = chunks->last;
        Sections_Trie *result = &chunk_last->nodes[chunk_last->count];
        chunk_last->count += 1;

        return result;
}

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
Sections_Trie__get_or_default(Sections_Trie **root, Arena *arena, Sections_Trie_Chunk_List *chunks, U64 hash, String8 name)
{
        B32 initialized = 0;
        B32 match = 0;

        Sections_Trie **trie_current = root;
        U64 hash_shifted = hash;
        for (;;)
        {
                if (*trie_current == 0)
                {
                        Sections_Trie *trie_new = Sections_Trie_Chunk_List__push(chunks, arena, Sections_Trie_Chunk__capacity_default);
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


internal Sections_Table *
Sections_Table__default(void)
{
        Arena *arena = Arena__allocate_m();
        Sections_Table *sections_table = Arena__push_struct_m(arena, Sections_Table);
        sections_table->arena  = arena;
        sections_table->chunks = Arena__push_struct_m(arena, Sections_Trie_Chunk_List);
        return sections_table;
}

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
        Sections_Trie *trie = Sections_Trie__get_or_default(&sections_table->root, sections_table->arena, sections_table->chunks, hash, name);

        B32 zero_is = Section__zero_is(&trie->section);
        if (zero_is)
        {
                Arena *arena = Arena__allocate_m();
                trie->section = (Section)
                {
                        .arena    = arena,
                        .name     = name,
                        .index    = sections_table->index_next,
                        .location = location,
                };
                sections_table->index_next += 1;
                Fragment *fragment = Arena__push_struct_m(arena, Fragment);
                Fragment_List *fragment_list = &trie->section.fragment_list;
                SLL_queue_push_m(fragment_list->first, fragment_list->last, fragment);
        }


        return &trie->section;
}


internal void
Sections_Table__add_common(Sections_Table *sections_table)
{
        String8 nil     = String8__literal("");
        String8 text_n  = String8__literal(".text");
        String8 data_n  = String8__literal(".data");
        String8 rodata_n = String8__literal(".rodata");
        String8 bss_n   = String8__literal(".bss");

        Sections_Table__get_or_default(sections_table, nil,  0);

        Section *text                  = Sections_Table__get_or_default(sections_table, text_n, 0);
                 text->elf.type        = ELF_Section_Header_Type__Program_Data;
                 text->elf.flags       = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR;
                 // TODO(compressed): this should probably be 2?
                 text->elf.alignment   = 4;

        Section *data                  = Sections_Table__get_or_default(sections_table, data_n, 0);
                 data->elf.type        = ELF_Section_Header_Type__Program_Data;
                 data->elf.flags       = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE;
                 data->elf.alignment   = 8;

        Section *rodata                = Sections_Table__get_or_default(sections_table, rodata_n, 0);
                 rodata->elf.type      = ELF_Section_Header_Type__Program_Data;
                 rodata->elf.flags     = ELF_Section_Header_Flags__ALLOC;
                 rodata->elf.alignment = 8;

        Section *bss                   = Sections_Table__get_or_default(sections_table, bss_n, 0);
                 bss->elf.type         = ELF_Section_Header_Type__No_Data;
                 bss->elf.flags        = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE;
                 bss->elf.alignment    = 8;

        return;
}

// Sections_Trie *
// sections_trie_push(Arena *arena, Sections_Trie_Chunk_List *chunks, Sections_Trie **trie_ptr, U64 hash, Section *value)
// {
//      B32 initialized = 0;
//      B32 match = 0;
//
//      Sections_Trie **trie_current = trie_ptr;
//      U64 hash_shifted = hash;
//      for (;;)
//      {
//              if (*trie_current == 0)
//              {
//                      Sections_Trie *trie_new = sections_trie_chunk_list_push(arena, chunks, Sections_Trie_Chunk__capacity_default);
//                      trie_new->section = *value;
//                      memory_zero_array(trie_new->children);
//                      *trie_current = trie_new;
//                      initialized = 1;
//              }
//
//              if (!initialized && (*trie_current)->key && String8__match_exact(*(*trie_current)->key, value->key))
//              {
//                      match = 1;
//              }
//
//              B32 break_should = initialized || match;
//              if (break_should)
//              {
//                      break;
//              }
//
//              trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
//              hash_shifted = hash_shifted << 2;
//      }
//
//      return *trie_current;
// }

internal void
Section__add_instruction_relaxed
(
        Section         *section,
        U32              encoding,
        U8               encoding_size,
        U32              location,
        U8               worst_case_size,
        U8               best_case_size,
        Expression_Node *expression_node,
        U32              subtype
)
{
        U8 *data = Fragment_List__variable
        (
                &section->fragment_list,
                section->arena,
                location,
                worst_case_size,
                best_case_size,
                expression_node,
                0,
                subtype,
                Relax_State__Machine
        );

        memory_copy(data, (U8 *)&encoding, encoding_size);
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
        U8 *data = Fragment_List__fixed
        (
                 &section->fragment_list,
                 section->arena,
                 location,
                 encoding_size
        );

        // Track its precise location within the fragment. Important to do it _after_ we've written it
        // since it might have landed into another fragment because of low capacity of the previous.
        if (fixup)
        {
                Fragment *last = section->fragment_list.last;
                U32 encoding_offset = last->size_fixed - encoding_size;

                fixup->fragment        = last;
                fixup->encoding_offset = encoding_offset;
        }

        memory_copy(data, (U8 *)&encoding, encoding_size);
        return;
}

// Finish the given section ensuring the last fragment in it has the tail `[alignment fragment][zero-fill fragment]`.
// This is done for two main reasons:
// 1. Ensure all sections have a consistent ending layout which can be relied upon.
// 2. Add final alignment to the sections that might need it. For an example, a code section should end up with a proper
//    alignment of NOPs to make execution and disassembly safe, while for table sections (`SEC_MERGE | SEC_STRINGS`)
//    it's good to ensure alignment so that there is no entry of invalid byte size and other tools (like a linker) don't
//    read over because they're assuming a certain size and less bytes have been written.
internal void
Section__finish(Section *section)
{
        // TODO(low): this is ELF specific but anyway.
        U32 alignment = 0;

        B32 code_section  = (section->elf.flags & ELF_Section_Header_Flags__EXECINSTR) != 0;
        B32 table_section = (section->elf.flags & ELF_Section_Header_Flags__MERGE)     != 0
                         || (section->elf.flags & ELF_Section_Header_Flags__STRINGS)   != 0;

        if (code_section)
        {
                // We write a power of two, not the exponent.
                // TODO(compressed): make this 2
                alignment = 4;
        }

        if (table_section)
        {
                // We take the highest power of two divisor as best alignment effort. If it's not a power of two, and we can't
                // align properly, we will warn later.
                U8 trailing_zeroes = count_trailing_zeros(section->elf.entry_size);
                U8 trailing_zeroes_capped = min_m(31, trailing_zeroes);
                U32 alignment_entry_size = (1UL << trailing_zeroes_capped);
                alignment = max_m(alignment_entry_size, alignment);
        }

        U32 location = section->fragment_list.last->location;

        if (code_section)
        {
                Fragment_List__align_code
                (
                        &section->fragment_list,
                        section->arena,
                        location,
                        alignment,
                        alignment
                );
        }
        else
        {
                Fragment_List__align
                (
                        &section->fragment_list,
                        section->arena,
                        location,
                        alignment,
                        0,
                        alignment
                );
        }

        Fragment_List__fill(&section->fragment_list, section->arena, location, 0, 0, 0, 0);
        return;
}
