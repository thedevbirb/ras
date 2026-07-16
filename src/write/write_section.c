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
        Alignment alignment =
        {
                .boundary = section->elf.alignment,
        };

        B32 code_section  = (section->elf.flags & ELF_Section_Header_Flags__EXECINSTR) != 0;
        B32 table_section = (section->elf.flags & ELF_Section_Header_Flags__MERGE)     != 0
                         || (section->elf.flags & ELF_Section_Header_Flags__STRINGS)   != 0;

        if (code_section)
        {
                // NOTE: we leave pattern zero here, will be replaced later since otherwise it'd be invalid.
                alignment.pattern_size = 4;
        }

        if (table_section)
        {
                // We take the highest power of two divisor as best alignment boundary.
                U8 trailing_zeroes                = count_trailing_zeros(section->elf.entry_size);
                U8 trailing_zeroes_capped         = min_m(31, trailing_zeroes);
                U32 alignment_boundary_entry_size = (1UL << trailing_zeroes_capped);
                alignment.boundary                = max_m(alignment_boundary_entry_size, alignment.boundary);
        }


        // TODO(locations): it doesn't make much sense?
        U32 location = section->fragments.last->location;

        Fragments__align(&section->fragments, location, alignment);
        Fragment__wane(section->fragments.last);

        return;
}

// internal void
// Section__relax(Section *section, Arena *arena, Diagnostic_List *diagnostics)
// {
//         // First pass to compute address estimate
//         U64 address = 0;
//
//         for (;;)
//         {
//                 Fragment *current = section->fragments.first;
//                           current->object_file_offset = address;
//
//                 address += current->size_fixed;
//                 Relax_flags flags = current->relax_flags;
//
//                 switch (flags & Relax_Flags__variants_mask)
//                 {
//                 // TODO(low): should this be a diagnostic instead.
//                 case Relax_Flags__None: { assert_always_m(0 && "expected finished section"); } break;
//                 case Relax_Flags__Fill
//                 {
//                         if (flags & Relax_Flags__Constant_Expression)
//                         {
//                                 address += current->relax_state_info.constant * current->size_variable;
//                         }
//                         // If non-constant expression-sized, will be checked later.
//
//                         // Add the repeated pattern: repeat times size
//                 } break;
//                 case Relax_State__Align: {} // fallthrough
//                 case Relax_State__Align_Code:
//                 {
//                         U32 alignment = (U32)current->expression_constant;
//                         U64 growth    = alignment_distance(address, alignment);
//
//                         U8 pattern_size = current->size_variable;
//                         if (growth % pattern_size != 0)
//                         {
//                                 // The padding added should be a multiple of the size of the align pattern.
//                                 Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
//                                 diagnostic->message    = String8__format
//                                 (
//                                         arena,
//                                         "alignment padding of size %d is not a multiple of alignment pattern size %d", growth, pattern_size
//                                 );
//                                 diagnostic->location  = current->location;
//                                 SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
//                         }
//
//                         address  += growth;
//                 } break;
//                 case Relax_State__Machine:
//                 {
//                         // TODO: estimate branch size;
//                 } break;
//                 }
//
//                 if (!current->next)
//                 {
//                         break;
//                 }
//
//                 current = current->next;
//         }
// }
