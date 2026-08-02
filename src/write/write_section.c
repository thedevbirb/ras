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

// Compute the total size of the instructions needed to relax a jump fragment.
internal U8
Fragment__jump_instructions_total_size(Fragment *fragment, Section *section)
{
        U8 size = 0;
        if (fragment->relax_state == Relax_State__Jump)
        {
                // NOTE: assume jumps are in range; the linker will catch any that aren't.
                // For branches, assume worst size and then fix it.
                size = fragment->relax_info.jump.unconditional_is ? 4 : 8;
                Symbol_Ref *symbol_target_jump = fragment->relax_info.jump.expression->symbol;
                B32 symbol_defined_is = symbol_target_jump->section != &Section__undefined;
                // TODO(weak)
                B32 symbol_weak_is = 0;
                B32 section_same_is = symbol_defined_is && symbol_target_jump->section == section;
                B32 size_can_be_computed = symbol_defined_is && !symbol_weak_is && section_same_is;
                if (size_can_be_computed)
                {
                        S64 jump_target_offset = symbol_target_jump->value;
                        // The branch instruction is placed as the last data in the fragment
                        S64 distance = jump_target_offset - (fragment->object_file_offset + fragment->data_size);

                        // TODO(compressed, check-gas): compressed range
                        //
                        // Check that `distance` fits a signed `RISCV_BRANCH_REACH`, i.e.
                        // `[RISCV_BRANCH_REACH/2, RISCV_BRANCH_REACH/2)`
                        // if (compressed && range compressed blah blah)
                        B32 within_branch_range_is = (S64)(distance + RISCV_BRANCH_REACH/2) < (S64)RISCV_BRANCH_REACH;
                        if (within_branch_range_is)
                        {
                                size = 4;
                        }
                        // else if (!unconditional && compressed) then this is 6.
                }
        }

        return size;
}

internal B32
Section__relax(Section *section, Arena *arena, Diagnostics *diagnostics)
{
        // First pass to compute address estimate
        {
        U64 address = 0;
        for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
        {
                fragment->object_file_offset = address;
                address += fragment->data_size;

                switch (fragment->relax_state)
                {
                // TODO(low): should this be a diagnostic instead?
                case Relax_State__None: { assert_always_m(0 && "expected finished fragment"); } break;
                case Relax_State__Fill:
                {
                        // Add the repeated pattern: repeat times size.
                        // Non-constant repeat will be evaluated later.
                        Expression *repeat_expression = fragment->relax_info.fill_expression;
                        U64 repeat = repeat_expression && repeat_expression->evaluation == Expression_Kind__Constant
                                   ? repeat_expression->integer_value : 0;
                        address += repeat * fragment->data_variable_size;

                } break;
                case Relax_State__Align:
                {
                        // TODO(medium): `|| 1` not sure if it's a patch or not. I don't know yet whether a zero
                        // boundary is something we should silently convert to 1 (a no-op) or error.
                        U32 boundary = fragment->relax_info.alignment.boundary || 1;
                        assert_always_m(pow_2_is_m(boundary) || !boundary);

                        U64 address_aligned = align_pow_2_m(address, boundary);
                        U64 growth          = address_aligned - address;
                        U8 pattern_size     = fragment->data_variable_size;

                        if (growth > fragment->relax_info.alignment.write_size_max)
                        {
                                // Explicitly give up as alignment, as request by the user.
                                growth = 0;
                        }

                        if (growth % pattern_size != 0)
                        {
                                // The padding added should be a multiple of the size of the align pattern.
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message    = String8__format
                                (
                                        arena,
                                        "alignment padding of size %d is not a multiple of alignment pattern size %d", growth, pattern_size
                                );
                                diagnostic->location  = fragment->location;
                        }

                        address += growth;
                } break;
                case Relax_State__Jump:
                {
                        // TODO(medium): not super super clear here if there is no symbol, for example `j 6`.
                        Symbol_Ref *symbol = fragment->relax_info.jump.expression->symbol;
                        if (symbol)
                        {
                                Symbol_Ref__resolve(symbol, diagnostics, Resolve_Level__Traverse);
                                U8 size = Fragment__jump_instructions_total_size(fragment, section);
                                fragment->data_variable_size = size;
                                address += size;

                        }
                } break;
                }
        }
        }

        // Start of the actual relaxation algorithm

        U32 index          = 0;
        U32 iterations_max = 0;
        S64 stretch        = 0;
        B32 stretched      = 0;

        // To avoid an infinite loop, I follow GNU as heuristic of making this step at most O^2 of the fragments.
        iterations_max = section->fragments.count * section->fragments.count;
        if (iterations_max < section->fragments.count)
        {
                // Overflow detected
                iterations_max = section->fragments.count;
        }

        B32 error = 0;

        for (;;)
        {
                // Cumulative across inner iterations
                stretch   = 0;
                stretched = 0;

                for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
                {
                        if (!fragment)
                        {
                                break;
                        }

                        // TODO(medium) flip relax marker? still not clear the utility.
                        S64 growth = 0;
                        U64 offset_was = fragment->object_file_offset;
                        U64 offset     = fragment->object_file_offset += stretch;

                        switch (fragment->relax_state)
                        {
                        case Relax_State__Fill:
                        {
                                Expression *expression = fragment->relax_info.fill_expression;
                                if (expression)
                                {
                                        // Time to resolve the expression fully
                                        Symbol_Ref symbol_expression = { .expression = expression };
                                        Symbol_Ref__resolve(&symbol_expression, diagnostics, Resolve_Level__Traverse);
                                        if (expression->evaluation != Expression_Kind__Constant)
                                        {
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message    = String8__literal("filling directive doesn't resolve to constant expression");
                                                diagnostic->location   = expression->location;
                                                diagnostic->ranges[0]  = expression->location_range;

                                                // TODO(unsure) Prevent this error from being repeated?
                                                fragment->relax_info.fill_expression = 0;
                                                expression                  = 0;
                                                // TODO(unsure) I think we can exit already
                                                error = 1;
                                        }
                                }

                                S64 write_size = expression ? expression->integer_value * fragment->data_variable_size : 0;
                                if (write_size < 0)
                                {
                                        // TODO(low, check-gas): GNU as doesn't error on the first two passes, and on
                                        // negative values it is ignored
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->message    = String8__literal("filling directive resolves to negative value");
                                        diagnostic->location   = expression->location;
                                        diagnostic->ranges[0]  = expression->location_range;

                                        // TODO(unsure) Prevent this error from being repeated?
                                        fragment->relax_info.fill_expression = 0;
                                        write_size = 0;
                                }

                                B32 padding_invalid = section->elf.flags & ELF_Section_Header_Flags__EXECINSTR && write_size % section->elf.alignment != 0;
                                if (padding_invalid)
                                {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->message    = String8__format(arena, "filling directive total write size (%u bytes) disrupts alignment (%u bytes) in code section", write_size, section->elf.alignment);
                                        diagnostic->location   = expression->location;
                                        diagnostic->ranges[0]  = expression->location_range;

                                        // TODO(unsure) Prevent this error from being repeated?
                                        fragment->relax_info.fill_expression = 0;
                                        write_size = 0;
                                }

                                if (write_size)
                                {
                                        // Next fragment MUST exist, see `Section__finish`.
                                        growth = offset_was + fragment->data_size + write_size - fragment->next->object_file_offset;
                                }
                        } break;
                        case Relax_State__Align:
                        {
                                // TODO(medium): same consideration about boundary that can be zero.
                                U32 boundary = fragment->relax_info.alignment.boundary || 1;
                                S64 offset_was_alignment = offset_was + fragment->data_size;
                                S64 offset_alignment     = offset     + fragment->data_size;

                                U64 offset_old = align_pow_2_m(offset_was_alignment, boundary);
                                U64 offset_new = align_pow_2_m(offset_alignment,     boundary);

                                // Again, give up with above `fragment->relax_info.alignment.write_size_max`
                                U32 write_size_max = fragment->relax_info.alignment.write_size_max;
                                if (write_size_max)
                                {
                                        if (offset_old > fragment->relax_info.alignment.write_size_max) { offset_old = 0; }
                                        if (offset_new > fragment->relax_info.alignment.write_size_max) { offset_new = 0; }
                                }

                                // Could be negative, and it's fine!
                                growth = offset_new - offset_old;
                        } break;
                        case Relax_State__Jump:
                        {
                                // `riscv_relax_frag`
                                U8 size_old = fragment->relax_info.jump.instructions_total_size;
                                U8 size_new = Fragment__jump_instructions_total_size(fragment, section);
                                fragment->data_variable_size = size_new;
                                fragment->relax_info.jump.instructions_total_size = size_new;
                                growth = (S64)size_new - (S64)size_old;
                        }
                        }

                        if (growth)
                        {
                                stretch += growth;
                                stretched = 1;
                        }
                }


                index += 1;
                if (!stretched || error || index >= iterations_max)
                {
                        break;
                }
        }

        B32 stretched_at_least_once = 0;
        // Update all the addresses for this iterations.

        for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
        {
                if (!fragment)
                {
                        break;
                }

                stretched_at_least_once |= fragment->object_file_offset_last != fragment->object_file_offset;
                fragment->object_file_offset_last = fragment->object_file_offset;
                fragment = fragment->next;
        }

        return stretched_at_least_once;
}

// Akin to GNU as `cvt_frag_to_fill`, converts every fragment into a `Relax_State__Fill` of fixed, immutable size.
//
// NOTE: in GNU as, this is done when sizing a segment but it could be done in a separate place since, at least in our
// case, it's we are not changing the size of fragments.
internal void
Fragment__convert_to_fill(Fragment *fragment, Section *section, Expressions *expressions, Arena *arena)
{
        U32 data_size_before          = fragment->data_size;
        U8  data_variable_size_before = fragment->data_variable_size;

        Relax_State  relax_state =  fragment->relax_state;
        Relax_Info  *relax_info  = &fragment->relax_info;

        switch (relax_state)
        {
        case Relax_State__Fill: {} break;
        case Relax_State__Align:
        {
                U64 write_size = fragment->next->object_file_offset - fragment->object_file_offset - fragment->data_size;

                B32 code_section_is = section->elf.flags & ELF_Section_Header_Flags__EXECINSTR;
                if (code_section_is)
                {
                        U64 section_alignment = section->elf.alignment;
                        U64 boundary          = relax_info->alignment.boundary;
                        // TODO(low) at the moment we panic, but we can convert this into a fairly elaborate diagnostic.
                        // In essence, this should NOT happen due to previous steps.
                        assert_always_m(boundary < section_alignment || boundary % section_alignment == 0);
                        assert_always_m(write_size % section_alignment == 0);

                        U64 data_variable_size = array_count_m(fragment->data_variable);

                        U8  null_variable_bytes_pattern[array_count_m(fragment->data_variable)] = {0};
                        B32 null_variable_bytes_set = memory_match(fragment->data_variable, &null_variable_bytes_pattern, array_count_m(fragment->data_variable));

                        if (null_variable_bytes_set)
                        {
                                // Insert NOPs.

                                // Check whether we can get away with just no-ops or we need a compressed version.
                                B32 compressed_needed = write_size % 2 != 0;
                                assert_always_m(!compressed_needed || section_alignment != 2);

                                U32 pattern      = compressed_needed ? ENCODING_C_NOP : ENCODING_NOP;
                                U8  pattern_size = compressed_needed ? 2 : 4;

                                fragment->data_variable_size = pattern_size;
                                assert_always_m(pattern_size <= data_variable_size);
                                memory_copy(fragment->data_variable, (U8 *)&pattern, pattern_size);
                        }

                }

                U32 repeat_count = write_size / (fragment->data_variable_size || 1);
                // TODO(low): not ideal to create expressions right now though
                Expression *fill_expression = Expressions__push_constant(expressions, arena, repeat_count);
                fragment->relax_info  = (Relax_Info){ .fill_expression = fill_expression };
                fragment->relax_state = Relax_State__Fill;
        } break;
        case Relax_State__Jump:
        {
                // Expand branches into multi-instruction sequences.

                // TODO(compressed): support it
                if (relax_info->jump.compressed_is)
                {
                        unreachable_m();
                }
                else
                {
                        U8 instructions_total_size = relax_info->jump.instructions_total_size;
                        assert_always_m(fragment->data_variable_size == instructions_total_size);
                        assert_always_m(array_count_m(fragment->data_variable) >= instructions_total_size);

                        if (instructions_total_size == 8)
                        {
                                // This MUST be a branch, because we assume jumps are of the right size.
                                assert_always_m(!relax_info->jump.unconditional_is && "jumps should be assumed to be in range");
                                // Invert the condition, and branch over the jump.
                                U32 instruction_1 = MATCH_BNE | encode_immediate_b_m(8);
                                U32 instruction_2 = MATCH_JAL;

                                Fixup *fixup = Arena__push_struct_m(arena, Fixup);
                                fixup->expression          = relax_info->jump.expression;
                                fixup->fragment            = fragment;
                                fixup->offset              = fragment->data_size + sizeof(instruction_1);
                                fixup->fragment_write_size = sizeof(instruction_2);
                                fixup->relocation_type     = Relocation_RISC_V__JAL;
                                DLL_push_back_m(section->fixups.first, section->fixups.last, fixup);

                                memory_copy(fragment->data_variable,                         (U8 *)&instruction_1, sizeof(instruction_1));
                                memory_copy(fragment->data_variable + sizeof(instruction_1), (U8 *)&instruction_2, sizeof(instruction_2));

                        }
                        else if (instructions_total_size == 4)
                        {
                                U16 relocation_type = relax_info->jump.unconditional_is ? Relocation_RISC_V__JAL : Relocation_RISC_V__Branch;
                                Fixup *fixup = Arena__push_struct_m(arena, Fixup);
                                fixup->fragment            = fragment;
                                fixup->expression          = relax_info->jump.expression;
                                fixup->offset              = fragment->data_size;
                                fixup->fragment_write_size = instructions_total_size;
                                fixup->relocation_type     = relocation_type;
                                DLL_push_back_m(section->fixups.first, section->fixups.last, fixup);
                        }
                        else
                        {
                                unreachable_m();
                        }

                        Expression *repeat_expression = Expressions__push_constant(expressions, arena, 1);
                        fragment->relax_info  = (Relax_Info){ .fill_expression = repeat_expression };
                        fragment->relax_state = Relax_State__Fill;
                }
        } break;
        }

        assert_always_m(data_size_before          == fragment->data_size);
        assert_always_m(data_variable_size_before == fragment->data_variable_size);

        return;
}

