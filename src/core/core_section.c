// Add the .text, .data, .bss sections. Sets .text as the current section if unset.
// internal void
// Sections_Table__add_basic(Sections_Table *sections_table)
// {
//         U32 location = 0;
//         Section *text                    = Sections_Table__get_or_default(sections_table, section_name_text, location);
//                  text->elf.type          = ELF_Section_Header_Type__Program_Data;
//                  text->elf.flags         = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR;
//                  // TODO(configuration): depends on extensions.
//                  text->elf.alignment     = 4;
//
//         Section *data                    = Sections_Table__get_or_default(sections_table, section_name_data, location);
//                  data->elf.type          = ELF_Section_Header_Type__Program_Data;
//                  data->elf.flags         = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE;
//                  // TODO(configuration): depends on extensions.
//                  data->elf.alignment     = 8;
//
//         Section *bss                     = Sections_Table__get_or_default(sections_table, section_name_bss, location);
//                  bss->elf.type           = ELF_Section_Header_Type__No_Data;
//                  bss->elf.flags          = ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE;
//                  // TODO(configuration): depends on extensions.
//                  data->elf.alignment     = 8;
//
//         if (!symbols_table->section_current)
//         {
//                 symbols_table->section_current = text;
//         }
//
//         return;
// }



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

                fixup->fragment            = last;
                fixup->offset              = last->data_size - encoding_size;
                fixup->fragment_write_size = encoding_size;
        }

        memory_copy(data, (U8 *)&encoding, encoding_size);
        return;
}
