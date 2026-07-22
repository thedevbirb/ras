internal void
write_object_file
(
        Arena           *arena,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        Sections_Table  *sections_table,
        Fixups          *fixups
)
{
        // NOTE: GNU as creates it after finishing to size all sections, and then after creating it, it if finished and
        // relaxed. While it may result in a couple more iterations, I think we can just do it now.
        Section__create_riscv_attributes(sections_table);
        Sections_Table__finish(sections_table);
        Sections_Table__relax(sections_table, arena, diagnostics);

        // Convert frag to fill
        {
        Sections_Trie_Chunk *chunk = sections_table->chunks->first;
        for (;;)
        {
                if (!chunk)
                {
                        break;
                }

                U32 index = 0;
                for (;;)
                {
                        if (index >= chunk->count)
                        {
                                break;
                        }
                        Section  *section  = &chunk->nodes[index].section;
                        Fragment *fragment = section->fragments.first;
                        for (;;)
                        {
                                if (!fragment)
                                {
                                        break;
                                }

                                Fragment__convert_to_fill(fragment, section, expressions, arena, fixups);
                                fragment = fragment->next;
                        }

                        index += 1;
                }

                chunk = chunk->next;
        }
        }

        // Size sections
        {
        Sections_Trie_Chunk *chunk = sections_table->chunks->first;
        for (;;)
        {
                if (!chunk)
                {
                        break;
                }

                U32 index = 0;
                for (;;)
                {
                        if (index >= chunk->count)
                        {
                                break;
                        }
                        Section  *section       = &chunk->nodes[index].section;
                        Fragment *fragment_last = section->fragments.last;

                        assert_always_m(fragment_last->relax_state == Relax_State__Fill);
                        assert_always_m(fragment_last->data_variable_size == 0);

                        section->elf.size = fragment_last->object_file_offset + fragment_last->data_size;

                        if (section->elf.entry_size && (section->elf.size % section->elf.entry_size))
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->kind       = Diagnostic_Kind__Warning;
                                diagnostic->message    = String8__format(arena, "section '%*s' size (%u bytes) is not a multiple of its entry size (%u bytes)",
                                                                         section->name.count, section->name.data, section->elf.size, section->elf.entry_size);
                                diagnostic->location   = section->location;
                        }

                        index += 1;
                }


                chunk = chunk->next;
        }
        }

        // TODO(low): another hint for the expressions section :), this can result in some footguns.
        Symbols_Table__finalize(symbols_table, diagnostics);
        Expressions__finalize(expressions, diagnostics);
}

internal void
Fixup__apply(Fixup *fixup, Fixups *fixups, Diagnostics *diagnostics)
{
        // Whether a RELAX relocation can be emitted
        B32 relaxable = 0;
        U8  *write_area = fixup->fragment_write_area;
        U8   write_size = fixup->fragment_write_size;
        Expression *expression = fixup->expression;
        S64 expression_result = expression->integer_value;

        // Try to patch, will warn later if the operation wasn't possible.

        switch (fixup->relocation_type)
        {
        default: { unreachable_m(); }

        // TODO(medium): these four cases are too similar, extract them in an helper.
        case Relocation_RISC_V__High_20:
        {
                // TODO(medium): check whether `fragment_write_size` can't be inferred from the relocation type.
                assert_always_m(fixup->fragment_write_size == sizeof(U32));
                U32 encoding         = U32_little_endian_get(write_area);
                U32 encoding_patched = encoding | encode_immediate_u_m(expression_result);
                U32_little_endian_put(write_area, encoding_patched);

                if (expression->evaluation == Expression_Kind__Constant)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }
                relaxable = 1;
        } break;
        case Relocation_RISC_V__Low_12_I_Type:
        {
                // TODO(medium): check whether `fragment_write_size` can't be inferred from the relocation type.
                assert_always_m(fixup->fragment_write_size == sizeof(U32));
                U32 encoding         = U32_little_endian_get(write_area);
                U32 encoding_patched = encoding | encode_immediate_i_m(expression_result);
                U32_little_endian_put(write_area, encoding_patched);

                if (expression->evaluation == Expression_Kind__Constant)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }
                relaxable = 1;
        } break;
        case Relocation_RISC_V__Low_12_S_Type:
        {
                // TODO(medium): check whether `fragment_write_size` can't be inferred from the relocation type.
                assert_always_m(fixup->fragment_write_size == sizeof(U32));
                // `memory_copy` instead of this?
                U32 encoding         = U32_little_endian_get(write_area);
                U32 encoding_patched = encoding | encode_immediate_s_m(expression_result);
                U32_little_endian_put(write_area, encoding_patched);

                if (expression->evaluation == Expression_Kind__Constant)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }
                relaxable = 1;
        } break;

        case Relocation_RISC_V__GOT_High_20:
        {
                // TODO(GOT, check-gas):
                // R_RISCV_GOT_HI20 and the following R_RISCV_LO12_I are relaxable
                // only if it is created as a result of la or lga assembler macros.
                if (0)
                {
                        relaxable = 1;
                }
                todo_m();
        } break;

        case Relocation_RISC_V__Add_8:  {} break;
        case Relocation_RISC_V__Add_16: {} break;
        case Relocation_RISC_V__Add_32: {} break;
        case Relocation_RISC_V__Add_64: {} break;
        case Relocation_RISC_V__Sub_8:  {} break;
        case Relocation_RISC_V__Sub_16: {} break;
        case Relocation_RISC_V__Sub_32: {} break;
        case Relocation_RISC_V__Sub_64: {} break;

        case Relocation_RISC_V__Relax:  {} break;

        case Relocation_RISC_V__Set_Unsigned_LEB128: { todo_m(); } break;
        case Relocation_RISC_V__Sub_Unsigned_LEB128: { todo_m(); } break;

        // TODO(tprel): support
        case Relocation_RISC_V__Thread_Pointer_Relative_High_20:       { relaxable = 1; } break;
        case Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type: { relaxable = 1; } break;
        case Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type: { relaxable = 1; } break;
        case Relocation_RISC_V__Thread_Pointer_Relative_Add:           { relaxable = 1; } break;

        // TODO(TLS): support
        case Relocation_RISC_V__TLS_GOT_High_20:                        { todo_m(); } break;
        case Relocation_RISC_V__TLS_Global_Dynamic_High_20:             { todo_m(); } break;
        case Relocation_RISC_V__TLS_Dynamic_Thread_Private_Relative_32: { todo_m(); } break;
        case Relocation_RISC_V__TLS_Dynamic_Thread_Private_Relative_64: { todo_m(); } break;

        case Relocation_RISC_V__32_Bit:
        {
                // TODO(.eh_frame, low, check-gas): use pc-relative relocation for FDE initial location.
                if (0) { break; }
        } // fallthrough
        case Relocation_RISC_V__64_Bit:
        {
                if (expression->symbol && expression->symbol_operand)
                {
                        // The idea is that: since this can only be valid if it's a subtract,
                        // unpack it into an "add" and "sub" relocation by looking at the left and right subexpressions.

                        Fixup *fixup_sub = Fixups__push_at(fixups, fixup);
                        // TODO(low): see checks before calling this functions. This holds but it should be made
                        // clearer.
                        fixup->expression     = expression->left;
                        fixup_sub->expression = expression->right;
                }

                if (!expression->symbol)
                {
                        // TODO(low): remove this assertion, make these possible states more clear.
                        assert_always_m(expression->evaluation == Expression_Kind__Constant);
                        memory_copy(write_area, (U8 *)&expression->integer_value, write_size);
                        fixup->flags |= Fixup_Flags__Done;
                }
        } break;
        // TODO(high): `Relocation_RISC_V__16_Bit` and `Relocation_RISC_V__8_bit` don't exist in the ELF specification,
        // yet we want to make fixups for them that can only be resolved, otherwise error.

        case Relocation_RISC_V__JAL:
        {
                assert_always_m(fixup->fragment_write_size == sizeof(U32));

                S64 target = expression->integer_value;
                target    += expression->symbol ? expression->symbol->value : 0;
                S64 distance = target - (fixup->fragment->object_file_offset + fixup->fragment->data_size);

                U32 encoding = U32_little_endian_get(write_area);
                encoding    |= encode_immediate_j_m(distance);
                U32_little_endian_put(write_area, encoding);

                B32 valid_immediate = validate_immediate_j_m(distance);
                if (!valid_immediate)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Error;
                        diagnostic->message    = String8__format(diagnostics->arena, "invalid J-type offset (%lld)", distance);
                        diagnostic->location   = expression->location;
                        diagnostic->ranges[0]  = expression->location_range;
                }

                // TODO(.option): support
                B32 relax = 1;
                // TODO(check-gas)
                B32 local_label = expression->symbol
                               && !expression->symbol->expression
                               && expression->symbol->binding == ELF_Symbol_Binding__Local;

                if (!relax && local_label && valid_immediate)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }
        } break;

        case Relocation_RISC_V__Branch:
        {
                assert_always_m(fixup->fragment_write_size == sizeof(U32));

                S64 target = expression->integer_value;
                target    += expression->symbol ? expression->symbol->value : 0;
                S64 distance = target - (fixup->fragment->object_file_offset + fixup->fragment->data_size);

                U32 encoding = U32_little_endian_get(write_area);
                encoding    |= encode_immediate_b_m(distance);
                U32_little_endian_put(write_area, encoding);

                B32 valid_immediate = validate_immediate_b_m(distance);
                if (!valid_immediate)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Error;
                        diagnostic->message    = String8__format(diagnostics->arena, "B-type offset out of range (%lld)", distance);
                        diagnostic->location   = expression->location;
                        diagnostic->ranges[0]  = expression->location_range;
                }

                // TODO(.option): support
                B32 relax = 1;
                // TODO(check-gas)
                B32 local_label = expression->symbol
                               && !expression->symbol->expression
                               && expression->symbol->binding == ELF_Symbol_Binding__Local;

                if (!relax && local_label && valid_immediate)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }

                relaxable = 1;
        } break;

        case Relocation_RISC_V__Branch_Compressed:
        {
                assert_always_m(fixup->fragment_write_size == sizeof(U16));

                S64 target = expression->integer_value;
                target    += expression->symbol ? expression->symbol->value : 0;
                S64 distance = target - (fixup->fragment->object_file_offset + fixup->fragment->data_size);

                U16 encoding = 0;
                memory_copy((U8 *)&encoding, write_area, sizeof(encoding));
                encoding |= encode_immediate_cb_m(distance);
                memory_copy(write_area, (U8 *)&encoding, sizeof(encoding));

                B32 valid_immediate = validate_immediate_cb_m(distance);
                if (!valid_immediate)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Error;
                        diagnostic->message    = String8__format(diagnostics->arena, "compressed B-type offset out of range (%lld)", distance);
                        diagnostic->location   = expression->location;
                        diagnostic->ranges[0]  = expression->location_range;
                }

                // TODO(.option): support
                B32 relax = 1;
                // TODO(check-gas)
                B32 local_label = expression->symbol
                               && !expression->symbol->expression
                               && expression->symbol->binding == ELF_Symbol_Binding__Local;

                if (!relax && local_label && valid_immediate)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }

                relaxable = 1;
        } break;

        case Relocation_RISC_V__Jump_Compressed:
        {
                assert_always_m(fixup->fragment_write_size == sizeof(U16));

                S64 target = expression->integer_value;
                target    += expression->symbol ? expression->symbol->value : 0;
                S64 distance = target - (fixup->fragment->object_file_offset + fixup->fragment->data_size);

                U16 encoding = 0;
                memory_copy((U8 *)&encoding, write_area, sizeof(encoding));
                encoding |= encode_immediate_cj_m(distance);
                memory_copy(write_area, (U8 *)&encoding, sizeof(encoding));

                B32 valid_immediate = validate_immediate_cj_m(distance);
                if (!valid_immediate)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Error;
                        diagnostic->message    = String8__format(diagnostics->arena, "compressed J-type offset out of range (%lld)", distance);
                        diagnostic->location   = expression->location;
                        diagnostic->ranges[0]  = expression->location_range;
                }

                // TODO(.option): support
                B32 relax = 1;
                // TODO(check-gas)
                B32 local_label = expression->symbol
                               && !expression->symbol->expression
                               && expression->symbol->binding == ELF_Symbol_Binding__Local;

                if (!relax && local_label && valid_immediate)
                {
                        fixup->flags |= Fixup_Flags__Done;
                }

                relaxable = 1;
        } break;
        }

        if (!(fixup->flags & Fixup_Flags__Done) && expression->symbol_operand)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__format
                        (
                                diagnostics->arena,
                                "Cannot resolve %*s - %s", String8__varg(*(expression->symbol->name)), String8__varg(*(expression->symbol_operand->name))
                        );
                diagnostic->location   = expression->location;
                diagnostic->ranges[0]  = expression->location_range;
        }

        if (relaxable && expression->symbol)
        {
                Fixup *fixup_relax = Fixups__push_at(fixups, fixup);
                fixup_relax->relocation_type = Relocation_RISC_V__Relax;
        }
}

internal void
Fixups__resolve(Fixups *fixups, Diagnostics *diagnostics)
{
        Fixup *fixup = fixups->first;

        for (;;)
        {
                if (!fixup)
                {
                        break;
                }

                // TODO(medium) Should we filter away those fixups whose expressions are unsolvable?

                Expression *expression     = fixup->expression;
                Expression_Kind evaluation = expression->evaluation;

                B32 subtractable_is = evaluation == Expression_Kind__Subtract
                                   && expression->left->evaluation == Expression_Kind__Symbol
                                   && expression->left->evaluation == Expression_Kind__Symbol;


                // We should have warned earlier about them, during `Symbol_Ref__resolve`.
                //
                // TODO(low): maybe these checks should be moved just inside the function.
                B32 processable_is = evaluation == Expression_Kind__Constant
                                  || evaluation == Expression_Kind__Symbol
                                  || subtractable_is;

                if (processable_is)
                {
                        Fixup__apply(fixup, fixups, diagnostics);
                }

                fixup = fixup->next;
        }
}





























