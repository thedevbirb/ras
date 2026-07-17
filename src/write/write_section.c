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


        // TODO(location): it doesn't make much sense?
        U32 location = section->fragments.last->location;

        Fragments__align(&section->fragments, location, alignment);
        Fragment__wane(section->fragments.last);

        return;
}

// Compute the total size of the instructions needed to relax the jump.
internal U8
jump_instructions_total_size(Relax_Info_Jump jump, Fragment *fragment, Section *section)
{
        U8 size = 0;
        if (fragment->relax_state == Relax_State__Jump)
        {
                // NOTE: assume jumps are in range; the linker will catch any that aren't.
                size = jump.unconditional_is ? 4 : 8;
                Symbol_Ref *symbol = fragment->relax_info.jump.symbol;
                B32 symbol_defined_is = symbol->section->index != 0;
                // TODO(weak)
                B32 symbol_weak_is = 0;
                B32 section_same_is = symbol_defined_is && symbol->section->index == section->index;
                B32 size_can_be_computed = symbol_defined_is && !symbol_weak_is && section_same_is;
                if (size_can_be_computed)
                {
                        // S64 offset = 0;
                        // if (symbol->section->index == ELF_Section_Index__Absolute)
                        // {
                        //
                        // }
                }
        }

        return size;
}

internal void
Section__relax(Section *section, Arena *arena, Diagnostic_List *diagnostics)
{
        // First pass to compute address estimate
        U64 address = 0;
        Fragment *current = section->fragments.first;

        for (;;)
        {
                current->object_file_offset = address;
                address += current->data_size;

                Relax_Info relax_info = current->relax_info;

                switch (current->relax_state)
                {
                // TODO(low): should this be a diagnostic instead?
                case Relax_State__None: { assert_always_m(0 && "expected finished fragment"); } break;
                case Relax_State__Fill:
                {
                        // Add the repeated pattern: repeat times size.
                        // Non-constant repeat will be evaluated later.
                        Expression *repeat_expression = relax_info.fill_expression;
                        U64 repeat = repeat_expression && repeat_expression->evaluation == Expression_Kind__Constant
                                   ? repeat_expression->integer_value : 0;
                        address += repeat * current->data_variable_size;

                } break;
                case Relax_State__Align:
                {
                        U32 boundary = relax_info.alignment.boundary;
                        assert_always_m(pow_2_is_m(boundary) || !boundary);

                        U64 address_aligned = align_pow_2_m(address, boundary);
                        U64 growth          = address_aligned - address;
                        U8 pattern_size     = current->data_variable_size;

                        if (growth % pattern_size != 0)
                        {
                                // The padding added should be a multiple of the size of the align pattern.
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->message    = String8__format
                                (
                                        arena,
                                        "alignment padding of size %d is not a multiple of alignment pattern size %d", growth, pattern_size
                                );
                                diagnostic->location  = current->location;
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        address += growth;
                } break;
                case Relax_State__Jump:
                {
                        Symbol_Ref *symbol = relax_info.jump.symbol;
                        if (symbol)
                        {
                                S64 result = Symbol_Ref__resolve(symbol, arena, diagnostics, 1);
                                (void)result;
                        }
                } break;
                }

                if (!current->next)
                {
                        break;
                }

                current = current->next;
        }
}
