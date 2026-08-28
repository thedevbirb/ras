internal B32
token_is_register(Token_Cursor *cursor)
{
        String8 text = Token_Cursor__text(cursor);
        B32 result = Register_List__lookup(RISCV_register_list, text, 0) != 0;
        return result;
}

internal U32
encode_compressed_offset(U8 field, S64 value)
{
        U32 result = U32_max;
        switch (field)
        {
        case OPF_O_C__LWSP: { if (validate_immediate_ci_lwsp(value))  { result = encode_immediate_ci_lwsp(value);  }} break;
        case OPF_O_C__LDSP: { if (validate_immediate_ci_ldsp(value))  { result = encode_immediate_ci_ldsp(value);  }} break;
        case OPF_O_C__LW:   { if (validate_immediate_cl_lw(value))    { result = encode_immediate_cl_lw(value);    }} break;
        case OPF_O_C__LD:   { if (validate_immediate_cl_ld(value))    { result = encode_immediate_cl_ld(value);    }} break;
        case OPF_O_C__SWSP: { if (validate_immediate_css_swsp(value)) { result = encode_immediate_css_swsp(value); }} break;
        case OPF_O_C__SDSP: { if (validate_immediate_css_sdsp(value)) { result = encode_immediate_css_sdsp(value); }} break;
        default: { unreachable_m(); }
        }
        return result;
}

internal Instruction_Parsed
RISCV_Instruction__parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Diagnostics        *diagnostics,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,
        U32                 instruction_hash
)
{
        Instruction_Parsed parsed = {0};

        B32 skip_compressed = !options->compressed;
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash, skip_compressed);
        String8 opcode_name = (String8){ .data = opcode->name, .count = opcode->count };

        Token opcode_token = cursor->current;
        token_next(cursor, diagnostics);
        Token_Cursor cursor_start = *cursor;

        Expression *expression = 0;

        B32 match = 0;

        // Iterate over opcode entries with the same name.
        for (;;)
        {
                parsed.data = RISCV_Instruction__create(opcode, opcode_token.location);
                U64 arguments = opcode->arguments;
                U32 arguments_index = 0;

                B32 xlen_mismatch   = opcode->xlen_requirement != 0 && opcode->xlen_requirement != options->xlen;
                B32 class_mismatch  = !RISCV_extensions_supports_class(&options->extensions, opcode->class);
                B32 try_next        = xlen_mismatch || class_mismatch;

                // Iterate over opcode arguments.
                for (;;)
                {
                        U8 slot = (U8)(arguments >> (8 * arguments_index));
                        if (!slot || try_next || xlen_mismatch)
                        {
                                // If we haven't consumed all the arguments, something went wrong.
                                match = !slot
                                     && !try_next
                                     && opcode->hash
                                     && (!opcode->match_function || opcode->match_function(opcode, parsed.data.encoding));
                                break;
                        }

                        // Snapshot the slot index for proper comma checking at the bottom: some branches, like
                        // load/store offsets, may advance `arguments_index` internally.
                        U32 current_slot_index = arguments_index;

                        switch (OP_KIND(slot))
                        {
                        case OPK__Syntax:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_SX__Comma:
                                {
                                        // NOTE: This whole thing could extracted into a `expect_comma_and_advance`.
                                        try_next = cursor->current.kind != Token_Kind__Comma;
                                        token_next(cursor, diagnostics);
                                } break;
                                case OPF_SX__PL:
                                {
                                        try_next = cursor->current.kind != Token_Kind__Parenthesis_Left;
                                        token_next(cursor, diagnostics);
                                } break;
                                case OPF_SX__PR:
                                {
                                        try_next = cursor->current.kind != Token_Kind__Parenthesis_Right;
                                        token_next(cursor, diagnostics);
                                } break;
                                }
                        } break;
                        case OPK__GPR:
                        {
                                String8 text = Token_Cursor__text(cursor);
                                const Register *reg = Register_List__lookup(RISCV_register_list, text, options->embedded);

                                U8 register_number = reg ? reg->number : 0;
                                switch (OP_FIELD(slot))
                                {
                                       case OPF_R__D:   { INSERT_OPERAND(RD,  parsed.data, register_number); } break;
                                       case OPF_R__S3:  { INSERT_OPERAND(RS3, parsed.data, register_number); } break;
                                       case OPF_R__S2:  { INSERT_OPERAND(RS2, parsed.data, register_number); } break;
                                       case OPF_R__S1:  { INSERT_OPERAND(RS1, parsed.data, register_number); } break;
                                       default: { unreachable_m(); }
                                }

                                try_next |= !reg;
                                token_next(cursor, diagnostics);
                        } break;
                        case OPK__GPR_C:
                        {
                                String8 text = Token_Cursor__text(cursor);
                                const Register *reg = Register_List__lookup(RISCV_register_list, text, options->embedded);

                                U8 register_number = reg ? reg->number : 0;
                                switch (OP_FIELD(slot))
                                {
                                        // Compressed registers (x8-x15, encoded as reg-8).
                                       case OPF_R_C__D_C:
                                       {
                                               try_next |= !riscv_compressed_register_is(register_number);
                                               if (riscv_compressed_register_is(register_number))
                                               {
                                                       INSERT_OPERAND(CRS2S, parsed.data, riscv_compressed_register_encode(register_number));
                                               }
                                       } break;
                                       case OPF_R_C__S1_C:
                                       {
                                               try_next |= !riscv_compressed_register_is(register_number);
                                               if (riscv_compressed_register_is(register_number))
                                               {
                                                       INSERT_OPERAND(CRS1S, parsed.data, riscv_compressed_register_encode(register_number));
                                               }
                                       } break;
                                       case OPF_R_C__S2_C:
                                       {
                                                try_next |= !riscv_compressed_register_is(register_number);
                                                if (riscv_compressed_register_is(register_number))
                                                {
                                                        INSERT_OPERAND(CRS2S, parsed.data, riscv_compressed_register_encode(register_number));
                                                }
                                       } break;
                                       // Full 5-bit compressed register (c.add/c.mv/c.swsp/c.sdsp rs2).
                                       case OPF_R_C__S2_C5: { INSERT_OPERAND(CRS2, parsed.data, register_number); } break;
                                       // Constrained compressed registers: consume the operand but do not insert.
                                       case OPF_R_C__CU:
                                       {
                                                U8 rd = (U8)((parsed.data.encoding >> OP_SH_RD) & OP_MASK_RD);
                                                try_next |= reg != 0 && register_number != rd;
                                       } break;
                                       case OPF_R_C__CC: { try_next |= reg != 0 && register_number != X_SP; } break;
                                       case OPF_R_C__CZ: { try_next |= reg != 0 && register_number != X_ZERO; } break;
                                       case OPF_R_C__CW:
                                       {
                                                U8 rd = (U8)((parsed.data.encoding >> OP_SH_CRS1S) & OP_MASK_CRS1S);
                                                try_next |= !riscv_compressed_register_is(register_number)
                                                         || riscv_compressed_register_encode(register_number) != rd;
                                       } break;
                                       default: { unreachable_m(); }
                                }

                                try_next |= !reg;
                                token_next(cursor, diagnostics);
                        } break;
                        case OPK__FPR:
                        {
                                // NOTE: the embedded (RVE) register restriction applies to the GPR
                                // file only; the FPR file is always 32 registers wide.
                                String8 text = Token_Cursor__text(cursor);
                                const Register *reg = Register_List__lookup(RISCV_fp_register_list, text, 0);

                                U8 register_number = reg ? reg->number : 0;
                                switch (OP_FIELD(slot))
                                {
                                       case OPF_FPR__D:    { INSERT_OPERAND(RD,  parsed.data, register_number); } break;
                                       case OPF_FPR__S3:   { INSERT_OPERAND(RS3, parsed.data, register_number); } break;
                                       case OPF_FPR__S2:   { INSERT_OPERAND(RS2, parsed.data, register_number); } break;
                                       case OPF_FPR__S1:   { INSERT_OPERAND(RS1, parsed.data, register_number); } break;

                                        // Copy the same register into the two positions.
                                       case OPF_FPR__S12:  { INSERT_OPERAND(RS1, parsed.data, register_number);
                                                             INSERT_OPERAND(RS2, parsed.data, register_number); } break;
                                       default: { unreachable_m(); }
                                }

                                try_next |= !reg;
                                token_next(cursor, diagnostics);
                        } break;
                        case OPK__FPR_C:
                        {
                                // NOTE: the embedded (RVE) register restriction applies to the GPR
                                // file only; the FPR file is always 32 registers wide.
                                String8 text = Token_Cursor__text(cursor);
                                const Register *reg = Register_List__lookup(RISCV_fp_register_list, text, 0);

                                U8 register_number = reg ? reg->number : 0;
                                switch (OP_FIELD(slot))
                                {
                                        // Compressed floating-point registers.
                                       case OPF_FPR_C__D_C:
                                       {
                                               try_next |= !riscv_compressed_register_is(register_number);
                                               if (riscv_compressed_register_is(register_number))
                                               {
                                                       INSERT_OPERAND(CRS2S, parsed.data, riscv_compressed_register_encode(register_number));
                                               }
                                       } break;
                                       case OPF_FPR_C__D_C5:  { INSERT_OPERAND(RD,   parsed.data, register_number); } break;
                                       case OPF_FPR_C__S2_C5: { INSERT_OPERAND(CRS2, parsed.data, register_number); } break;
                                       default: { unreachable_m(); }
                                }

                                try_next |= !reg;
                                token_next(cursor, diagnostics);
                        } break;
                        case OPK__Constant:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_C__Address:
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        expression_evaluate(expression);

                                        B32 symbol_is   = expression->evaluation == Expression_Kind__Symbol;
                                        B32 constant_is = expression->evaluation == Expression_Kind__Constant;
                                        if (!(symbol_is || constant_is))
                                        {
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Expression_Symbol_Or_Constant);
                                                diagnostic->location   = expression->location_range.v[0];
                                                diagnostic->ranges[0]  = expression->location_range;
                                        }

                                        if (symbol_is)
                                        {
                                                parsed.relocation = Relocation_RISC_V__32_Bit;
                                        }


                                        B32 constant_fits = sign_extended_32_bit_is_m(expression->integer_value);
                                        if (constant_is && !constant_fits)
                                        {
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Offset_Too_Large);
                                                diagnostic->location   = expression->location_range.v[0];
                                                diagnostic->ranges[0]  = expression->location_range;
                                        }
                                } break;
                                case OPF_C__Large:
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        expression_evaluate(expression);
                                        expression->integer_value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);

                                        if (expression->evaluation != Expression_Kind__Constant)
                                        {
                                               Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Constant_Expression_Expected);
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                        }
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Offset:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_O__Jal:
                                {
                                        // NOTE: we use GNU as approach to add mark a branch relocation immediately.
                                        // This relocation is temporary, and could be changed, since it depends on the
                                        // value of the expression and the symbols required.
                                        //
                                        // At assembly time, we may not know how many instructions this will expand to. It is
                                        // deferred later when we know all instructions. It is a different situation compared to
                                        // a `li` or `call` instruction which, during instruction parsing, are already expanded
                                        // into a known number of instructions (`INSN_MACRO`)
                                        parsed.relocation = Relocation_RISC_V__JAL;
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                } break;
                                case OPF_O__Branch:
                                {
                                        // See notes for `OPF_O__Jal`.
                                        parsed.relocation = Relocation_RISC_V__Branch;
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                } break;
                                case OPF_O__Store:
                                {
                                        U8 next = (U8)(arguments >> (8 * (arguments_index + 1)));
                                        if (next == OP_PL && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                        {
                                               // Omitted immediate, e.g. sw t1, (t0)
                                               arguments_index += 1;
                                               token_next(cursor, diagnostics);
                                        }
                                        else
                                        {
                                                try_parse_relocation_prefix(cursor, diagnostics, &parsed.relocation, Relocation_Operator_List__stype);
                                                expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                                SLL_queue_push_m(expressions->first, expressions->last, expression);

                                                if (!parsed.relocation)
                                                {
                                                        expression_evaluate(expression);
                                                        if (expression->evaluation == Expression_Kind__Constant)
                                                        {
                                                                expression->integer_value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);
                                                        }
                                                        B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                                        if (expression->evaluation == Expression_Kind__Constant && fits)
                                                        {
                                                                // TODO(medium): GNU as does this at a later step, and by default emits a
                                                                // relocation. Consider doing the same.
                                                                U32 encoding_immediate = encode_immediate_s_m(expression->integer_value);
                                                                parsed.data.encoding |= encoding_immediate;
                                                        }
                                                        else
                                                        {
                                                                try_next = 1;
                                                        }
                                                }
                                        }
                                } break;
                                case OPF_O__Load:
                                {
                                        U8 next = (U8)(arguments >> (8 * (arguments_index + 1)));
                                        if (next == OP_PL && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                        {
                                               // Omitted immediate, e.g. lw t1, (t0)
                                               arguments_index += 1;
                                               token_next(cursor, diagnostics);
                                        }
                                        else
                                        {
                                                try_parse_relocation_prefix(cursor, diagnostics, &parsed.relocation, Relocation_Operator_List__itype);
                                                expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                                SLL_queue_push_m(expressions->first, expressions->last, expression);

                                                if (!parsed.relocation)
                                                {
                                                        expression_evaluate(expression);
                                                        if (expression->evaluation == Expression_Kind__Constant)
                                                        {
                                                                expression->integer_value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);
                                                        }
                                                        B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                                        if (expression->evaluation == Expression_Kind__Constant && fits)
                                                        {
                                                                // TODO(medium): GNU as does this at a later step, and by default emits a
                                                                // relocation. Consider doing the same.
                                                                U32 encoding_immediate = encode_immediate_i_m(expression->integer_value);
                                                                parsed.data.encoding |= encoding_immediate;
                                                        }
                                                        else
                                                        {
                                                                try_next = 1;
                                                        }
                                                }
                                        }
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Offset_C:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_O_C__LWSP: {} // fallthrough
                                case OPF_O_C__LDSP: {} // fallthrough
                                case OPF_O_C__LW:   {} // fallthrough
                                case OPF_O_C__LD:   {} // fallthrough
                                case OPF_O_C__SWSP: {} // fallthrough
                                case OPF_O_C__SDSP:
                                {
                                        // Compressed load/store offsets must be constants: a symbolic
                                        // (or out-of-range) offset falls through to the uncompressed
                                        // instruction, matching GNU as. No relocation is produced.
                                        U8 offset_field = OP_FIELD(slot);

                                        U8 next = (U8)(arguments >> (8 * (arguments_index + 1)));
                                        if (next == OP_PL && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                        {
                                                // Omitted immediate, e.g. c.lw t1, (t0): the offset is zero,
                                                // which every compressed load/store form can represent.
                                                arguments_index += 1;
                                                token_next(cursor, diagnostics);
                                                parsed.data.encoding |= encode_compressed_offset(offset_field, 0);
                                        }
                                        else
                                        {
                                                // Compressed load/store offsets must be constants; a relocation prefix
                                                // (`%hi`, `%lo`, ...) or a register name makes the form fall through to
                                                // the uncompressed instruction.
                                                B32 parsable = cursor->current.kind == Token_Kind__Percentage
                                                           || token_is_register(cursor);
                                                B32 valid = 0;
                                                if (!parsable)
                                                {
                                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                                        SLL_queue_push_m(expressions->first, expressions->last, expression);
                                                        expression_evaluate(expression);
                                                        if (expression->evaluation == Expression_Kind__Constant)
                                                        {
                                                                S64 value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);
                                                                U32 encoded = encode_compressed_offset(offset_field, value);
                                                                if (encoded != U32_max)
                                                                {
                                                                        parsed.data.encoding |= encoded;
                                                                        valid = 1;
                                                                }
                                                        }
                                                }
                                                try_next |= !valid;
                                        }
                                } break;
                                case OPF_O_C__Jal_C:
                                {
                                        // c.j / c.jal: a relaxable compressed jump.
                                        parsed.relocation = Relocation_RISC_V__Jump_Compressed;
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);
                                } break;
                                case OPF_O_C__Branch_C:
                                {
                                        // c.beqz / c.bnez: a relaxable compressed branch.
                                        parsed.relocation = Relocation_RISC_V__Branch_Compressed;
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Immediate:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_I__I:
                                {
                                        try_parse_relocation_prefix(cursor, diagnostics, &parsed.relocation, Relocation_Operator_List__itype);
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        if (!parsed.relocation)
                                        {
                                               expression_evaluate(expression);
                                               if (expression->evaluation == Expression_Kind__Constant)
                                               {
                                                       expression->integer_value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);
                                               }
                                               B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                               if (expression->evaluation == Expression_Kind__Constant && fits)
                                               {
                                                       // TODO(medium): GNU as does this at a later step, and by default emits a
                                                       // relocation. Consider doing the same.
                                                       U32 encoding_immediate = encode_immediate_i_m(expression->integer_value);
                                                       parsed.data.encoding |= encoding_immediate;
                                               }
                                               else
                                               {
                                                       try_next = 1;
                                               }
                                        }
                                } break;
                                case OPF_I__U:
                                {

                                        U32 location_current = cursor->current.location;
                                        try_parse_relocation_prefix(cursor, diagnostics, &parsed.relocation, Relocation_Operator_List__utype);
                                        U32 location_end_of_relocation = cursor->current.location;
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        if (parsed.relocation)
                                        {
                                                B32 lui_must_be   = parsed.relocation == Relocation_RISC_V__High_20
                                                                 || parsed.relocation == Relocation_RISC_V__Thread_Pointer_Relative_High_20;
                                                U32 hash = parsed.data.opcode->hash;
                                                B32 valid = (lui_must_be && hash == HASH_lui) || hash == HASH_auipc;
                                                if (!valid)
                                                {
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Relocation_Operator_Invalid_Instruction);
                                                        diagnostic->location   = location_current;
                                                        diagnostic->ranges[0]  = Range1_U32_m(opcode_token.location, opcode_token.size);
                                                        diagnostic->ranges[1]  = (Range1_U32){{ location_current, location_end_of_relocation }};
                                                }
                                        }
                                        else
                                        {
                                                expression_evaluate(expression);
                                                if (expression->evaluation == Expression_Kind__Constant)
                                                {
                                                        S64 result = expression->integer_value;
                                                        B32 fits = 0 <= result && result < (S64)(1 << 20);
                                                        if (!fits)
                                                        {
                                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Constant_Value_Range);
                                                                diagnostic->location   = expression->location_range.v[0];
                                                                diagnostic->ranges[0]  = expression->location_range;
                                                        }


                                                        // TODO(medium): GNU as does this at a later step, and by default emits a
                                                        // relocation. Consider doing the same.
                                                        U32 encoding_immediate = encode_immediate_u_m(expression->integer_value);
                                                        parsed.data.encoding |= encoding_immediate;
                                                }
                                                else
                                                {


                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Relocation_Non_Constant);
                                                        diagnostic->location   = expression->location_range.v[0];
                                                        diagnostic->ranges[0]  = expression->location_range;
                                                }
                                        }
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Immediate_C:
                        {
                                // Compressed immediates are constants-only.  A relocation prefix (`%hi`, `%lo`, ...) or
                                // a register name cannot be a constant, so the expression parser is guarded from them
                                // (parsing would emit a spurious null-denotation node / symbol reference); the form
                                // then falls through to the uncompressed instruction.
                                B32 parsable = cursor->current.kind == Token_Kind__Percentage || token_is_register(cursor);
                                B32 valid = 0;
                                S64 value = 0;
                                if (!parsable)
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);
                                        expression_evaluate(expression);
                                        if (expression->evaluation == Expression_Kind__Constant)
                                        {
                                                value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);
                                                valid = 1;
                                        }
                                }

                                // Per-kind validation and encoding.
                                switch (OP_FIELD(slot))
                                {
                                case OPF_I_C__I_C:
                                {
                                        valid = valid && validate_immediate_ci(value);
                                        if (valid) { parsed.data.encoding |= encode_immediate_ci(value); }
                                } break;
                                case OPF_I_C__I_C_NZ:
                                {
                                        // c.addi: 6-bit signed immediate, zero disallowed.
                                        valid = valid && validate_immediate_ci(value) && value != 0;
                                        if (valid) { parsed.data.encoding |= encode_immediate_ci(value); }
                                } break;
                                case OPF_I_C__Shift:
                                {
                                        valid = valid && 0 <= value && value < options->xlen && value != 0;
                                        if (valid) { parsed.data.encoding |= encode_immediate_ci_m(value); }
                                } break;
                                case OPF_I_C__LUI:
                                {
                                        // `lui rd, imm`: imm is the 20-bit upper value; only a few
                                        // values are representable in c.lui's 6-bit field.
                                        B32 in_low  = 0 < value && value < (S64)(1 << 5);
                                        B32 in_high = (S64)((1 << 20) - (1 << 5)) <= value && value < (S64)(1 << 20);
                                        valid = valid && (in_low || in_high);
                                        if (valid) { parsed.data.encoding |= encode_immediate_ci_m(value); }
                                } break;
                                case OPF_I_C__LI_LUI:
                                {
                                        // `li rd, imm`: c.lui applies when imm is a 12-bit-aligned
                                        // 32-bit value whose upper 20 bits fit c.lui.
                                        S64 upper = (value & 0xfff) == 0 && (S32)value == (S32)(value & 0xffffffff)
                                                  ? (U32)value >> 12 : 0;
                                        B32 in_low  = 0 < upper && upper < (S64)(1 << 5);
                                        B32 in_high = (S64)((1 << 20) - (1 << 5)) <= upper && upper < (S64)(1 << 20);
                                        valid = valid && (in_low || in_high);
                                        if (valid) { parsed.data.encoding |= encode_immediate_ci_m(upper); }
                                } break;
                                case OPF_I_C__ADDI16SP:
                                {
                                        valid = valid && validate_immediate_ci_addi16sp(value);
                                        if (valid) { parsed.data.encoding |= encode_immediate_ci_addi16sp(value); }
                                } break;
                                default: { unreachable_m(); }
                                }

                                try_next |= !valid;
                        } break;
                        case OPK__Immediate_CL:
                        {
                                // Same shared constant-expression parse as OPK__Immediate_C.
                                B32 parsable = cursor->current.kind == Token_Kind__Percentage
                                           || token_is_register(cursor);
                                B32 valid = 0;
                                S64 value = 0;
                                if (!parsable)
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);
                                        expression_evaluate(expression);
                                        if (expression->evaluation == Expression_Kind__Constant)
                                        {
                                                value = RISCV_normalize_constant_expression(expression->integer_value, options->xlen);
                                                valid = 1;
                                        }
                                }

                                // Per-kind validation and encoding.
                                switch (OP_FIELD(slot))
                                {
                                case OPF_I_CL__CIW_ADDI4SPN:
                                {
                                        valid = valid && validate_immediate_ciw_addi4spn(value) && value != 0;
                                        if (valid) { parsed.data.encoding |= encode_immediate_ciw_addi4spn(value); }
                                } break;
                                case OPF_I_CL__ZERO:
                                {
                                        valid = valid && value == 0;
                                } break;
                                default: { unreachable_m(); }
                                }

                                try_next |= !valid;
                        } break;
                        case OPK__Shift:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_S__Shift:
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        expression_evaluate(expression);
                                        S64 value = expression->integer_value;
                                        B32 fits = 0 <= value && value < options->xlen;
                                        if (expression->evaluation != Expression_Kind__Constant || !fits)
                                        {
                                               Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Shift_Doesnt_Fit);
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                        }


                                        INSERT_OPERAND(SHAMT, parsed.data, value);
                                } break;
                                case OPF_S__Shift_5:
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        expression_evaluate(expression);
                                        S64 value = expression->integer_value;
                                        B32 fits = 0 <= value && value < (1 << 5);
                                        if (expression->evaluation != Expression_Kind__Constant || !fits)
                                        {
                                               Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Shift_Doesnt_Fit);
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                        }


                                        INSERT_OPERAND(SHAMT, parsed.data, value);
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Unique:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_U__Call:
                                {
                                        expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                                        SLL_queue_push_m(expressions->first, expressions->last, expression);

                                        parsed.relocation = Relocation_RISC_V__Call_PLT;

                                        Token   peek      = token_peek(cursor, diagnostics);
                                        String8 peek_text = Source__text_at(cursor->source, peek.location, peek.size);
                                        B32 at_plt_suffix = cursor->current.kind == Token_Kind__At
                                                         && String8__match_exact(peek_text, String8__literal("plt"));
                                        if (at_plt_suffix)
                                        {
                                                // Skip @
                                                token_next(cursor, diagnostics);
                                                // Skip PLT
                                                token_next(cursor, diagnostics);
                                        }
                                } break;
                                case OPF_U__Predecessor: {} // fallthrough
                                case OPF_U__Successor:
                                {
                                        String8 text    = Token_Cursor__text(cursor);

                                        U8 index    = 0;
                                        U8 ps_entry = 0;
                                        for (;;)
                                        {
                                                B32 break_should = ps_entry || index >= array_count_m(RISCV_predecessor_successor_table);
                                                if (break_should)
                                                {
                                                        break;
                                                }

                                                String8 entry = RISCV_predecessor_successor_table[index];
                                                ps_entry = String8__match_exact(text, entry) ? index : 0;
                                                index += 1;
                                        }
                                        try_next |= !ps_entry;

                                        if (OP_FIELD(slot) == OPF_U__Predecessor)
                                        {
                                                INSERT_OPERAND(PRED, parsed.data, ps_entry);
                                        }
                                        else
                                        {
                                                INSERT_OPERAND(SUCC, parsed.data, ps_entry);
                                        }

                                        token_next(cursor, diagnostics);
                                } break;
                                case OPF_U__Rounding_Mode:
                                {
                                        // The preceding comma is consumed by the implicit comma logic.
                                        String8 text = Token_Cursor__text(cursor);

                                        U8 index  = 0;
                                        U8 rm     = 0;
                                        B32 found = 0;
                                        for (;;)
                                        {
                                                B32 break_should = found || index >= array_count_m(RISCV_rounding_mode_table);
                                                if (break_should)
                                                {
                                                        break;
                                                }

                                                String8 entry = RISCV_rounding_mode_table[index];
                                                B32 matched   = String8__match_exact(text, entry);

                                                found = matched ? 1 : 0;
                                                rm    = matched ? index : 0;

                                                index += 1;
                                        }
                                        try_next |= !found;

                                        INSERT_OPERAND(RM, parsed.data, rm);
                                        token_next(cursor, diagnostics);
                                } break;
                                }
                        } break;
                        default: { unreachable_m(); }
                        }

                        // Check implicit comma: two consecutive argument slots MUST be separated by a comma.
                        // Syntax slots (parenthesis, commas itself) escape hatch break the chain, so no comma is
                        // expected around those.
                        //
                        // This is done to keep the encoding small, where relying on an invariant that holds on 99% of
                        // instructions.
                        U8  next_slot           = (U8)(arguments >> (8 * (current_slot_index + 1)));
                        B32 current_is_argument = OP_KIND(slot)      != OPK__Syntax;
                        B32 next_is_argument    = next_slot != 0 && OP_KIND(next_slot) != OPK__Syntax;
                        if (current_is_argument && next_is_argument)
                        {
                                // Branches should move the cursor individually, so we already expect to be on the comma
                                if (cursor->current.kind != Token_Kind__Comma)
                                {
                                        try_next = 1;
                                        break;
                                }
                                token_next(cursor, diagnostics);
                        }

                        arguments_index += 1;
                }

                String8 opcode_string = String8__new(opcode->name, opcode->count);
                B32 same_name = String8__match_exact(opcode_name, opcode_string);
                // NOTE: a class/xlen-mismatched entry is skipped by falling through to the
                // next opcode with the same name (e.g. a compressed alias when `c` is not
                // enabled must fall back to the 32-bit form).
                if (match || opcode->hash == 0 || !same_name)
                {
                        break;
                }

                *cursor = cursor_start;
                opcode += 1;
        }

        if (!match)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Opcode_Format_Unrecognized);
                diagnostic->location   = opcode_token.location;
                diagnostic->ranges[0]  = Range1_U32_m(opcode_token.location, opcode_token.size);
        }

        parsed.expression = expression;

        return parsed;
}

internal void
RISCV_Instruction__append
(
        Arena       *arena,
        Section     *section,
        Options     *options,

        Instruction_Parsed *instruction
)
{
        Fixup *fixup                 = 0;
        // NOTE: although jumps are assumed to be in range, if the compressed extension is enabled
        // then this might get reduced to a compressed 2-byte instruction.
        B32    jump_unconditional_is = instruction->relocation == Relocation_RISC_V__JAL
                                    || instruction->relocation == Relocation_RISC_V__Jump_Compressed;
        B32    jump_is               = instruction->relocation == Relocation_RISC_V__Branch
                                    || instruction->relocation == Relocation_RISC_V__Branch_Compressed
                                    || jump_unconditional_is;
        // NOTE: fixups, which are deferred patches, can be created only for fixed size instructions
        // (non-jump_is) because they need a precise location to be applied. Jump instructions,
        // like branches, break this invariant. However, some kind of fixup AND relocation will be needed, so for those
        // instruction we emit a tentative fixup attached to the relaxation information.
        U32    encoding              = instruction->data.encoding;
        U8     encoding_size         = RISCV_instruction_size(encoding);
        U32    location              = instruction->data.location;

        if (instruction->relocation)
        {
                fixup                  = Arena__push_struct_m(arena, Fixup);
                fixup->expression      = instruction->expression;
                fixup->relocation_type = instruction->relocation;
                if (options->relax)
                {
                        fixup->flags |= Fixup_Flags__Relax;
                }
                if (instruction->macro_generated_is)
                {
                        fixup->flags |= Fixup_Flags__Macro;
                }
                DLL_push_back_m(section->fixups.first, section->fixups.last, fixup);
        }

        if (jump_is)
        {
                Relax_Info relax_info =
                {
                        .jump =
                        {
                                .expression              = instruction->expression,
                                .fixup                   = fixup,
                                .compressed_is           = encoding_size == 2,
                                .unconditional_is        = jump_unconditional_is,
                                .instructions_total_size = encoding_size
                        }
                };

                Fragment *sealed = Fragments__variable
                (
                        &section->fragments,
                        location,
                        relax_info,
                        Relax_State__Jump,
                        (U8 *)&encoding,
                        encoding_size
                );

                sealed->relax_info.jump.fixup->fragment            = sealed;
                sealed->relax_info.jump.fixup->offset              = sealed->data_size;
                sealed->relax_info.jump.fixup->fragment_write_size = encoding_size;
        }
        else
        {
                Section__add_instruction_fixed
                (
                        section,
                        fixup,
                        encoding,
                        encoding_size,
                        instruction->data.location
                );
        }

        return;
}

internal void
RISCV_macro_build
(
        Arena      *arena,
        Section    *section,
        Options    *options,
        Macro_Info *macro
)
{
        U32 instruction_hash = FNV_hash_U32(macro->instruction_name);
        // Here, we forcefully skip compressed variants, for two reasons:
        // 1. Some macros will result in relocations, like a `call` expansions, and downstream consumers of relocatable
        //    object files, like linkers, expect a 8-byte format patch.
        // 2. Compatibility with GNU as source code.
        B32 skip_compressed = 1;
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash, skip_compressed);
        assert_always_m(opcode && opcode->hash);

        RISCV_Instruction instruction = RISCV_Instruction__create(opcode, macro->location);

        U16 relocation = 0;
        U32 arguments_index = 0;

        for (;;)
        {
                U8 slot = (U8)(macro->arguments >> (8 * arguments_index));
                B32 break_should = !slot || arguments_index >= macro->values_count;
                if (break_should)
                {
                        break;
                }

                S32 value = macro->values[arguments_index];
                switch (OP_KIND(slot))
                {
                        default: { unreachable_m(); } break;
                        case OPK__Relocation: { relocation = (U16)value; } break;
                        case OPK__GPR:
                        {
                                switch (OP_FIELD(slot))
                                {
                                        default: { unreachable_m(); } break;
                                        case OPF_R__D:  { INSERT_OPERAND(RD,  instruction, value); } break;
                                        case OPF_R__S3: { INSERT_OPERAND(RS3, instruction, value); } break;
                                        case OPF_R__S2: { INSERT_OPERAND(RS2, instruction, value); } break;
                                        case OPF_R__S1: { INSERT_OPERAND(RS1, instruction, value); } break;
                                }
                        } break;
                }

                arguments_index += 1;
        }

        assert_always_m(relocation ? macro->expression != 0 : 1);

        Instruction_Parsed parsed =
        {
                .expression = macro->expression,
                .data = instruction,
                .relocation = relocation,
                .macro_generated_is = !!macro
        };
        RISCV_Instruction__append(arena, section, options, &parsed);
        return;
}

// Expand a call pseudo instruction into an `auipc + jalr` pair with the provided register for `jalr`.
internal void
RISCV_call_expand
(
        Arena     *arena,
        Section   *section,
        Options   *options,

        U8          rd,
        U8          rs1,
        Expression *expression,
        U16         relocation,
        U32         location
)
{
        U64 arguments_auipc = OP_m(OP_GPR(OPF_R__D), OP_Relocation);
        S32 values_auipc[2] = {rs1, relocation};
        U64 arguments_jalr  = OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1));
        S32 values_jalr[2]  = {rd, rs1};

        Macro_Info macro_auipc =
        {
                .instruction_name = String8__literal("auipc"),
                .location         = location,
                .expression       = expression,
                .arguments        = arguments_auipc,
                .values           = values_auipc,
                .values_count     = array_count_m(values_auipc)
        };

        Macro_Info macro_jalr =
        {
                .instruction_name = String8__literal("jalr"),
                .location         = location,
                .expression       = expression,
                .arguments        = arguments_jalr,
                .values           = values_jalr,
                .values_count     = array_count_m(values_jalr)
        };

        // Ensure both instructions land in the same fragment.
        Fragments__ensure(&section->fragments, 8);
        RISCV_macro_build
        (
                arena,
                section,
                options,
                &macro_auipc
        );
        RISCV_macro_build
        (
                arena,
                section,
                options,
                &macro_jalr
        );
        // NOTE: I trust GNU as that is better to seal the fragment now.
        Fragment__wane(section->fragments.last);
        Fragments__push_empty_fragment(&section->fragments, location);
}

// Encodes all the instructions required during a LI pseudo-instruction. Pass `section = NULL` to count only; pass a
// valid section pointer (with `rd` set) to additionally emit the encoded instructions.
//
// The algorithm proceeds by range analysis:
//
//   - If the value fits in a 12-bit signed range, a single ADDI suffices.
//   - If it fits in a 32-bit signed range, it takes LUI alone (if the low 12 bits are zero) or LUI + ADDIW otherwise.
//     ADDIW (not ADDI) is used on RV64 because the result is meant to be a 32-bit sign-extended value; on RV32 plain
//     ADDI is emitted instead (GNU as: ADD32_INSN).
//
// Otherwise, we peel the low 12 bits off as a sign-extended tail (to be spliced back with an ADDI later),
// arithmetic-shift the remainder right by 12, and recurse on the upper portion. Each recursive level contributes one
// SLLI (to shift the upper part back into place) plus one ADDI (to splice in the peeled 12 bits, if non-zero).
//
// Note: after the initial LUI + ADDIW builds the topmost 32-bit chunk, every subsequent low-bit insertion uses plain
// ADDI, not ADDIW. ADDIW would discard the upper 32 bits we just shifted in.
//
// Example: li x1, 0x12345111333555
//
// Peeling (top-down analysis):
//
//   value = 0x12345111333555
//     peel low 12 bits = 0x555, shift right by 12
//   value = 0x12345111333
//     peel low 12 bits = 0x333, shift right by 12
//   value = 0x12345111
//     fits in 32-bit signed -> LUI 0x12345, ADDIW 0x111
//
// Emission (bottom-up assembly, 6 instructions):
//
//   lui   ra, 0x12345    ; ra = 0x0000000012345000
//   addiw ra, ra, 0x111  ; ra = 0x0000000012345111   <- base case
//   slli  ra, ra, 12     ; ra = 0x0000012345111000
//   addi  ra, ra, 0x333  ; ra = 0x0000012345111333   <- splice 0x333
//   slli  ra, ra, 12     ; ra = 0x0012345111333000
//   addi  ra, ra, 0x555  ; ra = 0x0012345111333555   <- splice 0x555
//
// The symmetry is the key insight: each level of peeling on the way down (shift right by 12, record a tail) becomes one
// SLLI + ADDI pair on the way back up (shift left by 12, replay the tail). The base case at the bottom of the recursion
// is the LUI (+ optional ADDIW) that seeds the topmost 32-bit chunk.
//
// Other minor optimizations are in place. In particular, the algorithm will also take into account additional trailing
// zeros after shifting right by 12, so that numbers with many trailing zero don't need more instructions than needed.
//
// NOTE: in case of compressed instructions the algorithm doesn't change too much, the heuristic is that, if possible, a
// compressed instruction is used instead of a full-sized one.
internal U8
RISCV_li_expand
(
        Section *section,
        U8 xlen,
        B32 compressed,

        S64 immediate,
        U8  register_destination,
        U32 location
)
{
        U8  instructions_count = 0;
        S64 immediate_low_12   = 0;
        U32 index              = 0;

        // The base case seeds the topmost chunk with a 32-bit sign-extending add. On RV64 that is `addiw`; on RV32 the
        // same word width is reached with plain `addi`.
        U32 opcode_add32 = (xlen == XLEN_64) ? OPCODE_I_TYPE_W : OPCODE_I_TYPE;

        // Peeled chunks: for each level we store the shift amount AND the low-12-bit tail. Shifts are at least 12, but
        // can be larger because trailing zero bits of the upper residual are absorbed into the next SLLI
        // (folding runs of zeros for free).
        //
        // Worst case on RV64 is 3 peels = 8 total instructions (LUI + ADDIW + 3 x (SLLI + ADDI)).
        struct { U8 shift; S64 tail; } peels[4];
        U32 peels_count = 0;

        for (;;)
        {
                B32 range_12     = S64_bits_range_in(immediate, 12);
                B32 range_32     = S64_bits_range_in(immediate, 32);
                B32 break_should = range_12 || range_32;

                if (range_12)
                {
                        instructions_count += 1;
                        if (section)
                        {
                                // Single ADDIW (or ADDI on RV32) from x0
                                U32 addiw_encoding = 0;
                                if (compressed && xlen == XLEN_32 && register_destination != X_ZERO && validate_immediate_ci_m(immediate))
                                {
                                        // c.li rd, imm  (only the RV32 `addi rd, x0, imm` form can compress)
                                        addiw_encoding = MATCH_C_LI | (U32)register_destination << OP_SH_RD | encode_immediate_ci_m(immediate);
                                }
                                else
                                {
                                        addiw_encoding = instruction_i_encode_m(register_destination, 0, immediate, opcode_add32, FUNCT3_ADDIW);
                                }
                                U8  addiw_encoding_size = RISCV_instruction_size(addiw_encoding);
                                Section__add_instruction_fixed(section, 0, addiw_encoding, addiw_encoding_size, location);
                        }
                }
                else if (range_32)
                {
                        immediate_low_12 = (immediate << 52) >> 52;
                        B32 lui_suffices = immediate_low_12 == 0;
                        instructions_count += lui_suffices ? 1 : 2;
                        if (section)
                        {
                                // LUI, plus ADDIW/ADDI if the low 12 bits are non-zero. The LUI
                                // immediate is `immediate` with its low 12 bits cleared;
                                // ADDIW/ADDI splices them back in (sign-extended to register width).
                                // instruction_u_encode_m expects the 20-bit U-field (the value already shifted right by 12).
                                S64 lui_immediate     = (S64)((U32)(immediate - immediate_low_12) >> 12);
                                U32 lui_encoding = 0;
                                if (compressed && register_destination != X_ZERO && register_destination != X_SP && riscv_compressed_lui_immediate_is(lui_immediate))
                                {
                                        // c.lui rd, uimm
                                        lui_encoding = MATCH_C_LUI | (U32)register_destination << OP_SH_RD | encode_immediate_ci_m(lui_immediate);
                                }
                                else
                                {
                                        lui_encoding = instruction_u_encode_m(register_destination, lui_immediate, OPCODE_LUI);
                                }
                                U8  lui_encoding_size = RISCV_instruction_size(lui_encoding);
                                Section__add_instruction_fixed(section, 0, lui_encoding, lui_encoding_size, location);
                                if (!lui_suffices)
                                {
                                        U32 addiw_encoding = 0;
                                        if (compressed && register_destination != X_ZERO && validate_immediate_ci_m(immediate_low_12))
                                        {
                                                // c.addiw (RV64) / c.addi (RV32) rd, rd, imm
                                                addiw_encoding = (xlen == XLEN_64 ? MATCH_C_ADDIW : MATCH_C_ADDI)
                                                               | (U32)register_destination << OP_SH_RD | encode_immediate_ci_m(immediate_low_12);
                                        }
                                        else if (compressed && xlen == XLEN_32 && register_destination == X_SP && validate_immediate_ci_addi16sp_m(immediate_low_12))
                                        {
                                                // c.addi16sp sp, sp, imm (RV32 `addi` form only)
                                                addiw_encoding = MATCH_C_ADDI16SP | encode_immediate_ci_addi16sp_m(immediate_low_12);
                                        }
                                        else
                                        {
                                                addiw_encoding = instruction_i_encode_m(register_destination, register_destination, immediate_low_12,
                                                        opcode_add32, FUNCT3_ADDIW);
                                        }
                                        U8  addiw_encoding_size = RISCV_instruction_size(addiw_encoding);
                                        Section__add_instruction_fixed(section, 0, addiw_encoding, addiw_encoding_size, location);
                                }
                        }
                }
                else
                {
                        immediate_low_12 = (immediate << 52) >> 52;
                        // Here, we override immediate to repeat the algorithm the next iterations on a smaller number
                        // composed by the 54 highest bits. However, as we see below there might be more trailing zeros!
                        immediate        = (immediate - immediate_low_12) >> 12;

                        // Absorb trailing zero bits of the upper residual into this peel's SLLI. Each absorbed bit
                        // means the residual we recurse on is denser, potentially bottoming out in fewer iterations
                        // (e.g. a huge value like 0x8000000000000000 collapses to just ADDI + SLLI after this).
                        U8 trailing = count_trailing_zeros((U64)immediate);
                        U8 shift    = (12 + trailing);
                        immediate  >>= trailing;

                        // SLLI is always needed to shift the upper part into place; ADDI is only needed when the peeled
                        // tail is non-zero.
                        B32 addi_needed = (immediate_low_12 != 0);
                        instructions_count += 1 + (addi_needed ? 1 : 0);

                        if (section)
                        {
                                // Record (shift, tail) for later replay. No emission yet: the SLLI + (optional) ADDI
                                // can't be emitted until the upper residual has been materialized by the base case.
                                assert_always_m(peels_count < 4 && "LI expansion exceeded worst case");
                                peels[peels_count].shift = shift;
                                peels[peels_count].tail  = immediate_low_12;
                                peels_count += 1;
                        }
                }

                if (break_should)
                {
                        break;
                }
                index += 1;
                assert_always_m(index < 8 && "infinite loop");
        }

        // Replay phase: emit SLLI + optional ADDI for each peeled level in
        // reverse order. `register_destination` already holds the base-case residual; each
        // iteration shifts it left by the recorded amount (12 + absorbed
        // trailing zeros) and splices the next tail back in (when non-zero).
        // Plain ADDI (not ADDIW) is used because we're building a 64-bit
        // value; ADDIW would discard the upper bits just shifted into place
        // by SLLI.
        if (section)
        {
                S32 peel_index = peels_count - 1;
                for (;;)
                {
                        B32 break_should = peel_index < 0;
                        if (break_should)
                        {
                                break;
                        }

                        U8  shift = peels[peel_index].shift;
                        S64 tail  = peels[peel_index].tail;

                        U32 slli_encoding = 0;
                        if (compressed && register_destination != X_ZERO && shift != 0 && shift < xlen)
                        {
                                // c.slli rd, rd, shamt
                                slli_encoding = MATCH_C_SLLI | (U32)register_destination << OP_SH_RD | encode_immediate_ci_m(shift);
                        }
                        else
                        {
                                slli_encoding = instruction_i_encode_m(register_destination, register_destination, shift, OPCODE_I_TYPE, FUNCT3_SLLI);
                        }
                        U8 slli_encoding_size = RISCV_instruction_size(slli_encoding);
                        Section__add_instruction_fixed(section, 0, slli_encoding, slli_encoding_size, location);

                        if (tail != 0)
                        {
                                U32 addi_encoding = 0;
                                // c.addi rd, rd, imm
                                if (compressed && register_destination != X_ZERO && validate_immediate_ci_m(tail))
                                {
                                        addi_encoding = MATCH_C_ADDI | (U32)register_destination << OP_SH_RD | encode_immediate_ci_m(tail);
                                }
                                // c.addi16sp sp, sp, imm
                                else if (compressed && register_destination == X_SP && validate_immediate_ci_addi16sp_m(tail))
                                {
                                        addi_encoding = MATCH_C_ADDI16SP | encode_immediate_ci_addi16sp_m(tail);
                                }
                                else
                                {
                                        addi_encoding = instruction_i_encode_m(register_destination, register_destination, tail, OPCODE_I_TYPE, FUNCT3_ADDI);
                                }
                                U8  addi_encoding_size = RISCV_instruction_size(addi_encoding);
                                Section__add_instruction_fixed(section, 0, addi_encoding, addi_encoding_size, location);
                        }

                        peel_index -= 1;
                }
        }

        assert_always_m(instructions_count > 0);
        return instructions_count;
}

internal void
RISCV_la_pcrel_expand
(
        Arena              *arena,
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        U8                  rd,
        Expression         *expression,
        U32                 location
)
{
        // We just expand to a `auipc + addi` combination. How it works:
        //
        // Suppose we have a symbol with 32-bit address `a`. We have to split its value
        // into two instructions. The %pcrel_hi relocation operator computes `(a - pc) >> 12` (returns
        // the upper 20 bits) while `auipc rd, immediate` computes `pc + (immediate << 12)` and saves it
        // into `rd`, so that yields (once computed by linker) the value
        //      `pc + ((a - pc) >> 12) << 12` == `pc + hi20(a - pc) << 12
        // into `rd`. Lastly, the program counter is increased.
        //
        // Now, we have to add the remaining lower 12-bits of `(a - pc)` i.e. `lo12(a - pc)`, so that we
        // erase `pc` from `rd` and get the final address `a`.
        // We could use an `addi` paired with `%pcrel_lo`. However, now `pc` has been increased by 4
        // bytes or whatever the instruction size is, so it would be off.
        //
        // To mitigate this in a standardized way, the RISC-V ELF psABI mandates the following steps:
        //
        // 1. A symbol (label) inside `%pcrel_lo` must point to the matching `%pcrel_hi` relocation;
        // 2. The linker will discover the value of `a` by looking at the matching relocation, and
        //    complete the computation by adding the sign-extended, lower 12-bits of `(a - pc)`.
        //
        // In essence, the `%pcrel_lo` relocation is just an artificial way to point to the matching
        // `%pcrel_hi` because the value `(label - pc_of_addi) >> 20` is never used (it would equal -4
        // in most cases, by the way).
        //
        // This is why we have to create a local label like ".L0 " is created above the `auipc`
        // instruction.

        U64 arguments_auipc = OP_m(OP_GPR(OPF_R__D), OP_Relocation);
        S32 values_auipc[]  = {rd, Relocation_RISC_V__PC_Relative_High_20};
        U64 arguments_addi  = OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Relocation);
        S32 values_addi[]   = {rd, rd, Relocation_RISC_V__PC_Relative_Low_12_I_Type};

        Symbol_Ref *internal_label          = Symbols_Table__create_internal(symbols_table, section, arena);
                    internal_label->flags  |= Symbol_Flags__Relocation;
        Expression *expression_addi         = Arena__push_struct_m(arena, Expression);
                    expression_addi->symbol = internal_label;
                    expression_addi->kind   = Expression_Kind__Symbol;
        SLL_queue_push_m(expressions->first, expressions->last, expression);

        Macro_Info macro_auipc =
        {
                .instruction_name = String8__literal("auipc"),
                .location         = location,
                .expression       = expression,
                .arguments        = arguments_auipc,
                .values           = values_auipc,
                .values_count     = array_count_m(values_auipc)
        };

        Macro_Info macro_addi =
        {
                .instruction_name = String8__literal("addi"),
                .location         = location,
                .expression       = expression_addi,
                .arguments        = arguments_addi,
                .values           = values_addi,
                .values_count     = array_count_m(values_addi)
        };

        // Ensure the instructions are in the same fragment
        Fragments__ensure(&section->fragments, 8);
        RISCV_macro_build
        (
                arena,
                section,
                options,
                &macro_auipc
        );
        // NOTE: GNU as creates also a second expression with an fake label for addi, why?
        RISCV_macro_build
        (
                arena,
                section,
                options,
                &macro_addi
        );
        // TODO(medium, check-gas): wane and new here?
        return;
}

internal void
RISCV_la_got_expand
(
        Arena              *arena,
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        U8                  rd,
        Expression         *expression,
        U32                 location
)
{
        // Load the address of a global symbol through the GOT. We expand to an `auipc + ld/lw`
        // combination:
        //
        //   auipc rd, %got_pcrel_hi(symbol)     // address of the symbol's GOT entry
        //   ld/lw rd, %pcrel_lo(.L0)(rd)        // load the entry, yielding the symbol address
        //
        // `%pcrel_lo` references an internal label placed just above the `auipc`, pointing to the
        // matching `%got_pcrel_hi`, following the same psABI rule as for `%pcrel_hi`. The GOT entry
        // itself is created by the linker; the assembler only emits the relocation pair.

        U64 arguments_auipc = OP_m(OP_GPR(OPF_R__D), OP_Relocation);
        S32 values_auipc[]  = {rd, Relocation_RISC_V__GOT_High_20};
        U64 arguments_load  = OP_m(OP_GPR(OPF_R__D), OP_Relocation, OP_GPR(OPF_R__S1));
        S32 values_load[]   = {rd, Relocation_RISC_V__PC_Relative_Low_12_I_Type, rd};

        Symbol_Ref *internal_label          = Symbols_Table__create_internal(symbols_table, section, arena);
                    internal_label->flags  |= Symbol_Flags__Relocation;
        Expression *expression_load         = Arena__push_struct_m(arena, Expression);
                    expression_load->symbol = internal_label;
                    expression_load->kind   = Expression_Kind__Symbol;
        SLL_queue_push_m(expressions->first, expressions->last, expression);

        Macro_Info macro_auipc =
        {
                .instruction_name = String8__literal("auipc"),
                .location         = location,
                .expression       = expression,
                .arguments        = arguments_auipc,
                .values           = values_auipc,
                .values_count     = array_count_m(values_auipc),
                // GNU as only relaxes R_RISCV_GOT_HI20 when it was created by the `la`/`lga` macros: the
                // relaxation rewrites this canonical `auipc %got_pcrel_hi` + `ld/lw %pcrel_lo` pair into a
                // `auipc %pcrel_hi` + `addi %pcrel_lo` pair, which is only correct for a canonical source (a
                // macro). We match that behaviour, so this is a macro-generated (hence relaxable) GOT.
                // .macro_relaxable  = 1
        };

        Macro_Info macro_load =
        {
                .instruction_name = options->xlen == XLEN_64 ? String8__literal("ld") : String8__literal("lw"),
                .location         = location,
                .expression       = expression_load,
                .arguments        = arguments_load,
                .values           = values_load,
                .values_count     = array_count_m(values_load)
        };

        // Ensure the instructions are in the same fragment
        Fragments__ensure(&section->fragments, 8);
        RISCV_macro_build
        (
                arena,
                section,
                options,
                &macro_auipc
        );
        RISCV_macro_build
        (
                arena,
                section,
                options,
                &macro_load
        );
        return;
}

internal void
RISCV_instruction_pseudo_append
(
        Arena              *arena,
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        Instruction_Parsed *instruction
)
{
        U8 rd  = (instruction->data.encoding >> OP_SH_RD)  & OP_MASK_RD;
        U8 rs1 = (instruction->data.encoding >> OP_SH_RS1) & OP_MASK_RS1;
        U8 rs2 = (instruction->data.encoding >> OP_SH_RS2) & OP_MASK_RS2;
        unused_m(rs2);

        U32 pseudo_type = instruction->data.opcode->mask;

        switch (pseudo_type)
        {
        default: { unreachable_m(); } break;
        case MACRO_CALL:
        {
                RISCV_call_expand
                (
                        arena,
                        section,
                        options,
                        rd,
                        rs1,
                        instruction->expression,
                        instruction->relocation,
                        instruction->data.location
                );
        } break;
        case MACRO_LA:  {} // fallthrough
        case MACRO_LLA: {} // fallthrough
        case MACRO_LGA:
        {
                // A constant can be materialized directly, no relocation involved.
                if (instruction->expression->evaluation == Expression_Kind__Constant)
                {
                        RISCV_li_expand
                        (
                                section,
                                options->xlen,
                                options->compressed,
                                instruction->expression->integer_value,
                                rd,
                                instruction->data.location
                        );
                        break;
                }

                // `la` loads a (possibly global) address: under PIC it goes through the GOT.
                // `lga` always loads through the GOT. `lla` always loads the local address.
                B32 got_is = (pseudo_type == MACRO_LA && options->position_indipendent_code)
                          || pseudo_type == MACRO_LGA;
                if (got_is)
                {
                        RISCV_la_got_expand
                        (
                                arena,
                                section,
                                expressions,
                                symbols_table,
                                options,
                                rd,
                                instruction->expression,
                                instruction->data.location
                        );
                }
                else
                {
                        RISCV_la_pcrel_expand
                        (
                                arena,
                                section,
                                expressions,
                                symbols_table,
                                options,
                                rd,
                                instruction->expression,
                                instruction->data.location
                        );
                }
        } break;
        case MACRO_LI:
        {
                RISCV_li_expand(section, options->xlen, options->compressed, instruction->expression->integer_value, rd, instruction->data.location);
        } break;
        }

        return;
}

