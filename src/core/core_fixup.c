internal U8 *
Fixup__write_area(Fixup *fixup)
{
        U8       *result   = 0;
        Fragment *fragment = fixup->fragment;

        B32 inside_data_variable_is = fixup->offset >= fragment->data_size;
        result = inside_data_variable_is ? fragment->data_variable : fragment->data;

        U32 offset_relative = inside_data_variable_is ? fixup->offset - fragment->data_size : fixup->offset;
        result +=  offset_relative;

        return result;
}

internal void
Fixup__apply_constant(Fixup *fixup, U32 patch_to_or_into_encoding)
{
        U32 encoding = 0;
        // TODO(medium): check whether `fragment_write_size` can't be inferred from the relocation type.
        U8  size     = min_m(sizeof(encoding), fixup->fragment_write_size);
        U8 *write_area = Fixup__write_area(fixup);
        memory_copy((U8 *)&encoding, write_area, size);
        U32 encoding_patched = encoding | patch_to_or_into_encoding;
        memory_copy(write_area, (U8 *)&encoding_patched, size);

        if (fixup->expression->evaluation == Expression_Kind__Constant)
        {
                fixup->flags |= Fixup_Flags__Done;
        }

        return;
}

internal void
Fixup__apply_jump(Fixup *fixup, U32(*encoding_callback)(S64), B32(*valid_immediate_callback)(S64), Diagnostics *diagnostics)
{
        S64 target   = fixup->expression->integer_value;
        target      += fixup->expression->symbol ? fixup->expression->symbol->value : 0;
        // The distance is measured from the start of the fixup's fragment
        // data area (which is at object_file_offset).  Unlike GAS which uses
        // (target - (frag->fr_address + frag->fr_fix)) we use data_size rather
        // than fragment_write_area offset because our frag model is simpler:
        // data_size is the fixed portion before variable-length fill data.
        S64 distance = target - (fixup->fragment->object_file_offset + fixup->fragment->data_size);

        // Works also for U16 encoding.
        U32 encoding = 0;
        U8 size = min_m(fixup->fragment_write_size, sizeof(encoding));
        U8 *write_area = Fixup__write_area(fixup);
        memory_copy((U8 *)&encoding, write_area, size);
        U32 patch = encoding_callback(distance);
        U32 encoding_patched = encoding |= patch;
        memory_copy(write_area, (U8 *)&encoding_patched, size);

        B32 valid_immediate = valid_immediate_callback && valid_immediate_callback(distance);
        if (!valid_immediate)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->kind       = Diagnostic_Kind__Error;
                diagnostic->message    = String8__format(diagnostics->arena, "invalid jump offset (%lld)", distance);
                diagnostic->location   = fixup->expression->location;
                diagnostic->ranges[0]  = fixup->expression->location_range;
        }

        // TODO(.option): support
        B32 relax = 1;
        B32 internal_is = fixup->expression->symbol && Symbol_Ref__internal_is(fixup->expression->symbol);
        if (!relax && internal_is && valid_immediate)
        {
                fixup->flags |= Fixup_Flags__Done;
        }
}

internal void
Fixup__apply(Fixup *fixup, Section *section, Arena *arena, Diagnostics *diagnostics)
{
        // Whether a RELAX relocation can be emitted
        B32 relaxable = 0;

        // Try to patch, will warn later if the operation wasn't possible.

        // Try to simply the fixup expression in case we have just a chain of equations to undefined / common symbols.
        // Example:
        // ```asm
        // .set a, global + 1
        // .set b, a - 2
        // addi a0, zero, %lo(b)
        // ```
        // The fixup expression should simplify to `global - 2`.
        for (;;)
        {
                Symbol_Ref *symbol           = fixup->expression->symbol;
                Expression *expression_inner = symbol           ? symbol->expression       : 0;
                Symbol_Ref *symbol_inner     = expression_inner ? expression_inner->symbol : 0;

                B32 undefined_or_common_inner = symbol_inner
                        ? symbol_inner->section == &Section__undefined || symbol_inner->section == &Section__common
                        : 0;

                if (undefined_or_common_inner)
                {
                        fixup->expression->symbol = symbol_inner;
                        fixup->expression->integer_value += expression_inner->integer_value;
                }
                else
                {
                        break;
                }
        }

        switch (fixup->relocation_type)
        {
        default: { unreachable_m(); }

        case Relocation_RISC_V__High_20:       { Fixup__apply_constant(fixup, encode_immediate_u_m(fixup->expression->integer_value)); relaxable = 1; } break;
        case Relocation_RISC_V__Low_12_I_Type: { Fixup__apply_constant(fixup, encode_immediate_i_m(fixup->expression->integer_value)); relaxable = 1; } break;
        case Relocation_RISC_V__Low_12_S_Type: { Fixup__apply_constant(fixup, encode_immediate_s_m(fixup->expression->integer_value)); relaxable = 1; } break;

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

        case Relocation_RISC_V__Align:  {} break;

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

        case Relocation_RISC_V__Call:     { relaxable = 1; } break;
        case Relocation_RISC_V__Call_PLT: { relaxable = 1; } break;

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
        case Fixup__8_Bit:              {} // fallthrough
        case Fixup__16_Bit:             {} // fallthrough
        case Relocation_RISC_V__64_Bit:
        {
                if (fixup->expression->evaluation == Expression_Kind__Subtract)
                {
                        // The idea is that: since this can only be valid if it's a subtract,
                        // unpack it into an "add" and "sub" relocation by looking at the left and right subexpressions.

                        Fixup *fixup_sub = Arena__push_struct_m(arena, Fixup);
                              *fixup_sub = *fixup;

                        fixup_sub->expression = fixup->expression->right;
                        fixup->expression     = fixup->expression->left;
                        DLL_insert_m(section->fixups.first, section->fixups.last, fixup, fixup_sub);
                }
                else if (fixup->expression->evaluation == Expression_Kind__Constant)
                {
                        U8 size = min_m(fixup->fragment_write_size, sizeof(fixup->expression->integer_value));
                        U8 *write_area = Fixup__write_area(fixup);
                        memory_copy(write_area, (U8 *)&fixup->expression->integer_value, size);
                        fixup->flags |= Fixup_Flags__Done;
                }
                else if (fixup->relocation_type == Fixup__8_Bit || fixup->relocation_type == Fixup__16_Bit)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->message    = String8__literal("cannot represent an 8-bit or 16-bit relocation on RISC-V/ELF object file");
                        diagnostic->location   = fixup->expression->location_range.v[0];
                        diagnostic->ranges[0]  = fixup->expression->location_range;
                }
        } break;

        case Relocation_RISC_V__JAL:               { Fixup__apply_jump(fixup, encode_immediate_j,  validate_immediate_j,  diagnostics); } break;
        case Relocation_RISC_V__Branch:            { Fixup__apply_jump(fixup, encode_immediate_b,  validate_immediate_b,  diagnostics); } break;
        case Relocation_RISC_V__Jump_Compressed:   { Fixup__apply_jump(fixup, encode_immediate_cj, validate_immediate_cj, diagnostics); } break;
        case Relocation_RISC_V__Branch_Compressed: { Fixup__apply_jump(fixup, encode_immediate_cb, validate_immediate_cb, diagnostics); } break;

        case Relocation_RISC_V__PC_Relative_High_20:
        {
                B32 symbol_internal_is = fixup->expression->symbol && Symbol_Ref__internal_is(fixup->expression->symbol);
                B32 evaluatable = symbol_internal_is && fixup->expression->symbol->section == section;
                if (evaluatable)
                {
                        S64 position = fixup->expression->symbol->value;
                        S64 offset   = fixup->expression->integer_value;
                        S64 target   = (position + offset) - fixup->fragment->object_file_offset;

                        PC_Relative_High *pc_relative_high = Arena__push_struct_m(arena, PC_Relative_High);
                        pc_relative_high->section            = section;
                        pc_relative_high->object_file_offset = fixup->fragment->object_file_offset;
                        pc_relative_high->expression         = fixup->expression;

                        SLL_stack_push_m(section->fixups.pc_relative_high, pc_relative_high);

                        // NOTE: we want to encode the upper bits of the `target`, knowing that the lower bits
                        // will be added using a _sign extended_ operation, that is, instead of adding a number in the
                        // range `[0, RISCV_IMMEDIATE_REACH)`, we'll be adding a number in the range
                        // `[-RISCV_IMMEDIATE_REACH/2, RISCV_IMMEDIATE_REACH/2)`. To compensate this, we will add it to
                        // the value we're encoding:
                        S64 target_compensated = target + (RISCV_IMMEDIATE_REACH / 2);
                        B32 fits = S64_bits_range_in(target_compensated, 32);
                        if (!fits)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message    = String8__format(diagnostics->arena, "invalid pc-relative high offset: %lld", target);
                                diagnostic->location   = fixup->expression->location;
                                diagnostic->ranges[0]  = fixup->expression->location_range;
                        }


                        U32 encoding       = 0;
                        U32 encoding_patch = encode_immediate_u_m((U32)target_compensated);
                        U8  size = min_m(fixup->fragment_write_size, sizeof(encoding));
                        U8 *write_area = Fixup__write_area(fixup);
                        memory_copy((U8 *)&encoding, write_area, size);
                        U32 encoding_patched = encoding | encoding_patch;
                        memory_copy(write_area, (U8 *)&encoding_patched, size);

                        B32 relax = 1;
                        if (!relax && fits)
                        {
                                fixup->flags |= Fixup_Flags__Done;
                        }
                }

                relaxable = 1;
        } break;

        case Relocation_RISC_V__PC_Relative_Low_12_S_Type: {} // fallthrough
        case Relocation_RISC_V__PC_Relative_Low_12_I_Type:
        {
                U64 object_file_offset = fixup->expression->symbol->value + fixup->expression->integer_value;
                PC_Relative_High *entry = PC_Relative_High__find(section->fixups.pc_relative_high, section, object_file_offset);

                B32 evaluatable = 0;
                if (entry)
                {
                    B32 symbol_internal_is = entry->expression->symbol && Symbol_Ref__internal_is(entry->expression->symbol);
                    evaluatable = symbol_internal_is && entry->expression->symbol->section == section;
                }

                if (evaluatable)
                {
                        S64 position = entry->expression->symbol->value;
                        S64 offset   = entry->expression->integer_value;
                        S64 target   = (position + offset) - entry->object_file_offset;

                        // Finding the entry already assumes the ranges are valid and checked by the corresponding
                        // %pcrel_hi.

                        U32 encoding       = 0;
                        U32 encoding_patch = fixup->relocation_type == Relocation_RISC_V__PC_Relative_Low_12_S_Type
                                ? encode_immediate_s_m((U32)target)
                                : encode_immediate_i_m((U32)target);
                        U8  size = min_m(fixup->fragment_write_size, sizeof(encoding));
                        U8 *write_area = Fixup__write_area(fixup);
                        memory_copy((U8 *)&encoding, write_area, size);
                        U32 encoding_patched = encoding | encoding_patch;
                        memory_copy(write_area, (U8 *)&encoding_patched, size);

                        B32 relax = 1;
                        if (!relax)
                        {
                                // TODO(low): we could even pop `entry`?
                                fixup->flags |= Fixup_Flags__Done;
                        }
                }

                relaxable = 1;
        } break;
        }

        if (!(fixup->flags & Fixup_Flags__Done) && fixup->expression->evaluation == Expression_Kind__Subtract)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__format
                        (
                                diagnostics->arena,
                                "Cannot resolve %.*s - %.*s", String8__varg(*(fixup->expression->left->symbol->name)), String8__varg(*(fixup->expression->right->symbol->name))
                        );
                diagnostic->location   = fixup->expression->location;
                diagnostic->ranges[0]  = fixup->expression->location_range;
        }

        if (relaxable && fixup->expression->symbol)
        {
                Fixup *fixup_relax = Arena__push_struct_m(arena, Fixup);
                *fixup_relax = *fixup;

                fixup_relax->fragment_write_size = 0;
                fixup_relax->relocation_type = Relocation_RISC_V__Relax;
                DLL_insert_m(section->fixups.first, section->fixups.last, fixup, fixup_relax);
        }

        if (!(fixup->flags & Fixup_Flags__Done))
        {
                if (fixup->expression->symbol) { fixup->expression->symbol->flags |= Symbol_Flags__Relocation; }
                section->fixups.unresolved += 1;
                section->symbol->flags |= Symbol_Flags__Relocation;
        }

        return;
}

internal void
Section__resolve_fixups(Section *section, Arena *arena, Diagnostics *diagnostics)
{
        for each_node_m(section->fixups.first, fixup)
        {
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
                        Fixup__apply(fixup, section, arena, diagnostics);
                }
        }
}

