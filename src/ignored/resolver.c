// Encodes all the instructions required during a LI pseudo-instruction. Pass `section = NULL` to count only; pass a
// valid section pointer (with `rd` set) to additionally emit the encoded instructions.
//
// The algorithm proceeds by range analysis:
//
//   - If the value fits in a 12-bit signed range, a single ADDI suffices.
//   - If it fits in a 32-bit signed range, it takes LUI alone (if the low 12 bits are zero) or LUI + ADDIW otherwise.
//     ADDIW (not ADDI) is used because the result is meant to be a 32-bit sign-extended value.
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
internal U8
LI_instruction_encode(S64 immediate, Object_File_Section* section, U8 register_destination)
{
        U8  instructions_count = 0;
        S64 immediate_low_12   = 0;
        U32 index              = 0;

        // Peeled chunks: for each level we store the shift amount AND the
        // low-12-bit tail. Shifts are at least 12, but can be larger because
        // trailing zero bits of the upper residual are absorbed into the next
        // SLLI (folding runs of zeros for free). Worst case on RV64 is 3
        // peels = 8 total instructions (LUI + ADDIW + 3 x (SLLI + ADDI)).
        struct { U8 shift; S64 tail; } peels[4];
        U32 peels_count = 0;

        for (;;)
        {
                B32 range_i_s        = S64_bits_range_in(immediate, IMMEDIATE_NOMINAL_I_S_SIZE_BIT);
                B32 range_i_s_plus_u = S64_bits_range_in(immediate, IMMEDIATE_NOMINAL_I_S_SIZE_BIT + IMMEDIATE_NOMINAL_U_SIZE_BIT);
                B32 break_should     = range_i_s || range_i_s_plus_u;

                if (range_i_s)
                {
                        instructions_count += 1;
                        if (section)
                        {
                                // Single ADDI from x0.
                                U32 encoding = instruction_i_encode_m(register_destination, 0, immediate,
                                                OPCODE_I_TYPE, FUNCT3_ADDI);
                                Object_File_Section_write_instruction(section, encoding);
                        }
                }
                else if (range_i_s_plus_u)
                {
                        immediate_low_12 = (immediate << 52) >> 52;
                        B32 lui_suffices = immediate_low_12 == 0;
                        instructions_count += lui_suffices ? 1 : 2;
                        if (section)
                        {
                                // LUI, plus ADDIW if the low 12 bits are non-zero. The LUI
                                // immediate is `immediate` with its low 12 bits cleared;
                                // ADDIW splices them back in (sign-extended to 64 bits).
                                S64 immediate_lui = immediate - immediate_low_12;
                                U32 encoding_lui  = instruction_u_encode_m(register_destination, immediate_lui, OPCODE_LUI);
                                Object_File_Section_write_instruction(section, encoding_lui);
                                if (!lui_suffices)
                                {
                                        U32 addiw_enc = instruction_i_encode_m(register_destination, register_destination, immediate_low_12,
                                                        OPCODE_I_TYPE, FUNCT3_ADDIW);
                                        Object_File_Section_write_instruction(section, addiw_enc);
                                }
                        }
                }
                else
                {
                        immediate_low_12 = (immediate << 52) >> 52;
                        // Here, we override immediate to repeat the algorithm the next iterations on a smaller number
                        // composed by the 54 highest bits. However, as we see below there might be more trailing zeros!
                        immediate        = (immediate - immediate_low_12) >> 12;

                        // Absorb trailing zero bits of the upper residual into this
                        // peel's SLLI. Each absorbed bit means the residual we recurse
                        // on is denser, potentially bottoming out in fewer iterations
                        // (e.g. a huge value like 0x8000000000000000 collapses to just
                        // ADDI + SLLI after this).
                        U8 trailing = count_trailing_zeros((U64)immediate);
                        U8 shift    = (12 + trailing);
                        immediate  >>= trailing;

                        // SLLI is always needed to shift the upper part into place;
                        // ADDI is only needed when the peeled tail is non-zero.
                        B32 addi_needed = (immediate_low_12 != 0);
                        instructions_count += 1 + (addi_needed ? 1 : 0);

                        if (section)
                        {
                                // Record (shift, tail) for later replay. No emission yet:
                                // the SLLI + (optional) ADDI can't be emitted until the
                                // upper residual has been materialized by the base case.
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
                        B32 break_should = index < 0;
                        if (break_should)
                        {
                                break;
                        }

                        U8  shift = peels[peel_index].shift;
                        S64 tail  = peels[peel_index].tail;

                        U32 encoding_slli = instruction_i_encode_m(register_destination, register_destination, shift, OPCODE_I_TYPE, FUNCT3_SLLI);
                        Object_File_Section_write_instruction(section, encoding_slli);

                        if (tail != 0)
                        {
                                U32 encoding_addi = instruction_i_encode_m(register_destination, register_destination, tail, OPCODE_I_TYPE, FUNCT3_ADDI);
                                Object_File_Section_write_instruction(section, encoding_addi);
                        }

                        peel_index -= 1;
                }
        }

        assert_always_m(instructions_count > 0);
        return instructions_count;
}

void
Resolver_error_set(Resolver *resolver, Resolver_Error_Kind kind)
{
        // TODO: set other fields as well, for example the token where it happened.
        if (!resolver->error.kind)
        {
                resolver->error.kind      = kind;
                resolver->error.statement = resolver->statement_current;

                Token token_begin      = resolver->tokens[resolver->statement_current->token_index_begin];
                Token token_end        = resolver->tokens[resolver->statement_current->token_index_end];
                U32 row_index          = token_begin.row_index;
                U32 column_index_begin = token_begin.column_index;
                U32 column_index_end   = token_end.column_index + token_end.size;

                resolver->error.row_index          = row_index;
                resolver->error.column_index_begin = column_index_begin;
                resolver->error.column_index_end   = column_index_end;
        }

#ifdef ASSEMBLY_EXPECT_PANIC
        assert_always_m(0 && "panic on expect");
#endif

        return;
}

void
Resolver_expect(Resolver *resolver, B32 condition, Resolver_Error_Kind kind)
{
        if (!condition && !resolver->error.kind)
        {
                Resolver_error_set(resolver, kind);
        }

        return;
}

internal String8
Resolver_token_string_from_index(Resolver *resolver, U32 token_index)
{
        Token token = resolver->tokens[token_index];
        String8 string =
        {
                .data  = resolver->input->data + token.index,
                .count = (U64)token.size,
        };
        return string;
}

internal void
Resolver_advance(Resolver *resolver)
{
        resolver->statements_end_reached = resolver->statement_index + 1 == resolver->statements->count;
        resolver->statement_index       += !resolver->statements_end_reached;
        resolver->statement_current      = &resolver->statements->data[resolver->statement_index];

        return;
}

internal void
Resolver_cursor_reset(Resolver *resolver)
{
        resolver->statements_end_reached = 0 >= resolver->statements->count;
        resolver->statement_index        = 0;
        resolver->statement_current      = &resolver->statements->data[0];

        return;
}

internal S64
Resolver_operation_evaluate(Resolver *resolver, Expression_Kind kind, S64 a, S64 b)
{
        S64 result = 0;

        switch (kind)
        {
        case Expression_Kind__Add:           { result = a +  b; } break;
        case Expression_Kind__Subtract:      { result = a -  b; } break;
        case Expression_Kind__Multiply:      { result = a *  b; } break;
        case Expression_Kind__Divide:        { result = a /  b; } break;
        case Expression_Kind__Modulo:        { result = a %  b; } break;

        case Expression_Kind__Bitwise_Or:    { result = a |  b; } break;
        case Expression_Kind__Bitwise_Xor:   { result = a ^  b; } break;
        case Expression_Kind__Bitwise_And:   { result = a &  b; } break;
        case Expression_Kind__Shift_Left:    { result = a << b; } break;
        case Expression_Kind__Shift_Right:   { result = a >> b; } break;

        case Expression_Kind__Equal:         { result = a == b; } break;
        case Expression_Kind__Not_Equal:     { result = a != b; } break;
        case Expression_Kind__Less_Than:     { result = a <  b; } break;
        case Expression_Kind__Less_Equal:    { result = a <= b; } break;
        case Expression_Kind__Greater_Than:  { result = a >  b; } break;
        case Expression_Kind__Greater_Equal: { result = a >= b; } break;

        case Expression_Kind__Logical_And:   { result = a && b; } break;
        case Expression_Kind__Logical_Or:    { result = a || b; } break;

        default: { Resolver_error_set(resolver, Resolver_Error_Kind__Expression_Kind_Unknown); } break;
        }

        return result;
}

// Whether two symbols have a fixed distance, so that their difference can be safely computed at assembly time. This
// requires:
//
// 1. Both symbols are local;
// 2. Both symbols are in the same section;
// 4. All statements between the two labels cannot grow.
// 3. Relax is disabled.
internal B32
Resolver_label_distance_fixed(Resolver *resolver, Symbols_Table_Entry *symbol_left, Symbols_Table_Entry *symbol_right)
{
        B32 local_left     = ELF_Symbol_bind_m(symbol_left->elf.type_and_binding)  == ELF_Symbol_Binding__Local;
        B32 local_right    = ELF_Symbol_bind_m(symbol_right->elf.type_and_binding) == ELF_Symbol_Binding__Local;
        B32 section_same   = symbol_left->elf.section_index == symbol_right->elf.section_index;
        B32 relax_disabled = resolver->statement_current->flags & Statement_Flags__Relax_Disabled;
        B32 variable_gap   = local_left && local_right && relax_disabled && section_same;

        U32 index_left  = symbol_left->index_statement;
        U32 index_right = symbol_right->index_statement;

        U32 index_low  = min_m(index_left, index_right);
        U32 index_high = max_m(index_left, index_right);

        for (;;)
        {
                B32 break_should = index_low > index_high || variable_gap;
                if (break_should)
                {
                        break;
                }

                Statement *statement = &resolver->statements->data[index_low];
                if (statement->section_index == symbol_left->elf.section_index)
                {
                        variable_gap = statement->flags & Statement_Flags__Size_Variable;
                }

                index_low += 1;
        }

        B32 fixed = !variable_gap;
        return fixed;
}

// Notes on relocation when difference between labels, e.g. label_1 - label_2 is involved.
//
// Unless both symbols are of the same section, local, and it is explicitly set .option norelax for that
// statement, you have to emit a pair of relocation:
//
// 1. R_RISCV_ADD*, for label_1
// 2. R_RISCV_SUB*, for label_2
//
// Moreover, on R_RISCV_RELAX, the condition is: .option relax is enabled and the relocation is on an
// instruction that participates in a recognized relaxation pattern.
//
// As such, a statement can have these relocation patterns:
//
// 1. A R_RISCV_RELAX, in case the instruction is suitable and .option relax is enabled.
// 2. A specific relocation, or a R_RISCV_ADD/SUB pair.

// TODO(low): Don't make this recursive.
void
Resolver_expression_evaluate(Resolver *resolver, Expression_Node *node)
{
        local_persist U8 recursion_level = 0;
        recursion_level += 1;

        assert_always_m(node && "cannot evaluate null expression");
        // TODO(low): print error immediately, and then exit.
        assert_always_m(recursion_level < expression_recursion_max && "max recursion");

        Expression_Kind kind = node->kind;

        // TODO(low): Also if it is unresolved it might not be worth it to evaluate it again?
        if (node->evaluation < Evaluation__Constant)
        {
        switch (kind)
        {
        case Expression_Kind__None:            { unreachable_m(); } break;
        case Expression_Kind__Number_Literal:
        {
                assert_always_m(node->index_left  == 0);
                assert_always_m(node->index_right == 0);
                node->evaluation = Evaluation__Constant;
        } break;
        case Expression_Kind__Char_Literal:
        {
                assert_always_m(node->index_left  == 0);
                assert_always_m(node->index_right == 0);
                node->evaluation = Evaluation__Constant;
        } break;
        case Expression_Kind__Identifier:
        {
                assert_always_m(node->index_left  == 0);
                assert_always_m(node->index_right == 0);

                Symbols_Table_Entry *symbol = node->symbols_table_entry;
                assert_always_m(symbol && "parser didn't set expression entry");

                symbol->flags |= Symbol_Flags__Used;

                B32 declared = symbol->flags & Symbol_Flags__Declared;
                B32 cyclic   = declared && symbol->flags & Symbol_Flags__Resolving;
                Resolver_expect(resolver, !cyclic, Resolver_Error_Kind__Symbol_Cyclic);

                Statement *statement = &resolver->statements->data[symbol->index_statement];
                B32 definition_is = statement->directive_kind == Directive_Kind__Equality
                                ||  statement->directive_kind == Directive_Kind__Set;

                if (definition_is && !cyclic)
                {
                        // Search its definition, and evaluate it.
                        Expression_Node *inner = &resolver->expressions->data[statement->expressions_indexes[0]];

                        symbol->flags |= Symbol_Flags__Resolving;
                        Resolver_expression_evaluate(resolver, inner);
                        symbol->flags &= ~Symbol_Flags__Resolving;

                        B32 symbol_absolute_is = definition_is && inner->evaluation >= Evaluation__Absolute;

                        if (symbol_absolute_is)
                        {
                                symbol->elf.section_index = ELF_Section_Index__Absolute;
                                node->integer_value = inner->integer_value;
                        }

                        node->evaluation = inner->evaluation;
                }
                else
                {
                        node->evaluation = Evaluation__Unresolved;
                }
        } break;
        case Expression_Kind__Label_Numeric_Reference_Backward: {} // fallthrough
        case Expression_Kind__Label_Numeric_Reference_Forward:
        {
                // This is a straighforward walk forward/backward algorithm. Numerical label reference are usually very
                // close by so this should not be here too long.

                assert_always_m(node->index_left  == 0);
                assert_always_m(node->index_right == 0);

                U16 section_current_index = resolver->section_current_index;
                U32 label_numeric_value   = node->label_numeric_value; // e.g. 1b or 2b etc.
                assert_always_m(label_numeric_value < label_numeric_max);

                S64 index      = resolver->statement_index;
                B32 forward_is = node->kind == Expression_Kind__Label_Numeric_Reference_Forward;
                S32 direction                  = forward_is ? 1 : -1;
                S64 end                        = forward_is ? (S64)resolver->statements->count : -1;

                B32 match            = 0;
                U32 offset           = 0;
                Statement *statement = 0;
                for (;;)
                {
                        index += direction;
                        B32 break_should = match || index == end;
                        if (break_should)
                        {
                                break;
                        }
                        statement = &resolver->statements->data[index];
                        match = statement->kind == Statement_Kind__Label_Numeric && statement->label_numeric_value == label_numeric_value;

                        // NOTE: GNU gas doesn't error on these crosses, but I still fail to understand how they're
                        // resolved in practice.
                        B32 crossed = match && statement->section_index != section_current_index;
                        Resolver_expect(resolver, !crossed, Resolver_Error_Kind__Label_Numeric_Section_Cross);
                        offset = statement->section_offset;

                }

                if (match)
                {
                        // NOTE: this means the symbol is referred by multiple nodes.
                        node->symbols_table_entry = statement->s_symbol;
                        statement->s_symbol->flags |= Symbol_Flags__Used;
                }

                Resolver_Error_Kind error_kind = forward_is ? Resolver_Error_Kind__Label_Numeric_Forward_Not_Found
                                                            : Resolver_Error_Kind__Label_Numeric_Backward_Not_Found;
                Resolver_expect(resolver, match, error_kind);

                node->integer_value = offset;
                node->evaluation = Evaluation__Unresolved;

        } break;
        case Expression_Kind__Current_Address:
        {
                assert_always_m(node->index_left  == 0);
                assert_always_m(node->index_right == 0);

                U32 section_current_offset = resolver->sections_offset[resolver->section_current_index];
                node->integer_value = section_current_offset;
                node->evaluation = Evaluation__Unresolved;
        } break;
        case Expression_Kind__Relocation:
        {
                Expression_Node *inner = &resolver->expressions->data[node->index_left];
                Resolver_expression_evaluate(resolver, inner);
                // NOTE: gas supports some relocation prefixes with absolute values. For example,
                // `%hi/lo(1234)` is accepted, and a relocation is not emitted. I think this is NOT desired.
                Resolver_expect(resolver, inner->evaluation == Evaluation__Unresolved, Resolver_Error_Kind__Relocation_Operator_Expression_Unresolved_Expected);
                B32 symbol_single = inner->symbol_operand == 0;
                Resolver_expect(resolver, symbol_single, Resolver_Error_Kind__Relocation_Operator_Expression_Invalid);

                assert_always_m(inner->symbols_table_entry && "expression->evaluation == Evaluation__Unresolved expression with no symbol");
                // This node absorbs the symbol and the value of the inner node.
                node->symbols_table_entry = inner->symbols_table_entry;
                node->integer_value       = inner->integer_value;
                node->evaluation = Evaluation__Unresolved;

                // NOTE: gas doesn't check that a `pcrel_lo` label points to an appropriate `pcrel_hi`, or it allows an
                // addend within it, leaving the heavy duty on the linker with basically no validation.

        } break;

        case Expression_Kind__Negate:
        {
                Expression_Node *node_left = &resolver->expressions->data[node->index_left];
                Resolver_expression_evaluate(resolver, node_left);

                B32 valid = node_left->evaluation >= Evaluation__Absolute;
                Resolver_expect(resolver, valid, Resolver_Error_Kind__Operator_Expression_Absolute_Expected);

                node->evaluation = node_left->evaluation;
                node->integer_value = ~(node_left->integer_value - 1);
        } break;
        case Expression_Kind__Bitwise_Not:
        {
                Expression_Node *node_left = &resolver->expressions->data[node->index_left];
                Resolver_expression_evaluate(resolver, node_left);

                B32 valid = node_left->evaluation >= Evaluation__Absolute;
                Resolver_expect(resolver, valid, Resolver_Error_Kind__Operator_Expression_Absolute_Expected);

                node->evaluation = node_left->evaluation;
                node->integer_value = ~node_left->integer_value;
        } break;
        case Expression_Kind__Logical_Not:
        {
                Expression_Node *node_left = &resolver->expressions->data[node->index_left];
                Resolver_expression_evaluate(resolver, node_left);

                B32 valid = node_left->evaluation >= Evaluation__Absolute;
                Resolver_expect(resolver, valid, Resolver_Error_Kind__Operator_Expression_Absolute_Expected);

                node->evaluation = node_left->evaluation;
                node->integer_value = !node_left->integer_value;
        } break;
        default:
        {

                Expression_Node *node_right = &resolver->expressions->data[node->index_right];
                Expression_Node *node_left  = &resolver->expressions->data[node->index_left];

                Resolver_expression_evaluate(resolver, node_right);
                Resolver_expression_evaluate(resolver, node_left);

                Relocation_Operator relocation_operator_right = node_right->relocation_operator;
                Relocation_Operator relocation_operator_left = node_left->relocation_operator;

                // Propagating relocation operators is important for relocation emission later without further
                // expression investigation.
                assert_always_m(node_right->relocation_operator == 0 || node_left->relocation_operator == 0 && "parser didn't catch multiple relocation operators");
                node->relocation_operator = relocation_operator_right;
                node->relocation_operator = relocation_operator_left;

                B32 unresolved_right = node_right->evaluation == Evaluation__Unresolved;
                B32 unresolved_left  = node_left->evaluation  == Evaluation__Unresolved;

                Symbols_Table_Entry *symbol_right = node_right->symbols_table_entry;
                Symbols_Table_Entry *symbol_left  = node_left->symbols_table_entry;

                Symbols_Table_Entry *symbol_operand_right = node_right->symbol_operand;
                Symbols_Table_Entry *symbol_operand_left  = node_left->symbol_operand;

                // After here, either we have two leafs that we can evaluate straight away or if we have a subexpression
                // it has been evaluated.

                // This is always okay to perform, no harm.
                S64 integer_result  = Resolver_operation_evaluate(resolver, node->kind, node_right->integer_value, node_left->integer_value);
                node->integer_value = integer_result;

                if (unresolved_left && unresolved_right)
                {
                        Resolver_expect(resolver, node->kind == Expression_Kind__Subtract, Resolver_Error_Kind__Operator_Between_Symbols_Invalid);
                        Resolver_expect(resolver, symbol_operand_right == 0 && symbol_operand_left == 0, Resolver_Error_Kind__Operator_Expression_Absolute_Expected);
                        Resolver_expect(resolver, !node->relocation_operator, Resolver_Error_Kind__Relocation_Expression_Invalid);

                        B32 distance_fixed = Resolver_label_distance_fixed(resolver, symbol_left, symbol_right);
                        if (distance_fixed)
                        {
                                node->integer_value = node_left->integer_value - node_right->integer_value;
                                // Symbols can be safely cancelled.
                                node->symbols_table_entry = 0;
                                node->symbol_operand      = 0;
                        }
                        else
                        {
                                node->symbols_table_entry = symbol_left;
                                node->symbol_operand = node_right->symbols_table_entry;
                        }
                }
                else if (unresolved_left)
                {
                        node->symbols_table_entry = symbol_left;
                        node->symbol_operand = symbol_operand_left;
                }
                else if (unresolved_right)
                {
                        node->symbols_table_entry = symbol_right;
                        node->symbol_operand = symbol_operand_right;
                }

                node->evaluation = min_m(node_left->evaluation, node_right->evaluation);
        } break;
        }
        }

        recursion_level -= 1;

        assert_always_m(node->evaluation != Evaluation__None);

        return;
}

internal Expression_Node *
Resolver_statement_expression_evaluate_index(Resolver *resolver, Statement *statement, U32 index)
{
        U32 expression_index        = statement->expressions_indexes[index];
        Expression_Node *expression = &resolver->expressions->data[expression_index];
        Resolver_expression_evaluate(resolver, expression);
        return expression;
}

internal void
Resolver_offsets_recompute(Resolver *resolver)
{
        U32 section_offsets[ELF_Section__COUNT] = {0};

        for (;;)
        {
                B32 break_should = resolver->statements_end_reached;
                if (break_should)
                {
                        break;
                }
                Statement *statement      = resolver->statement_current;
                U32 *section_offset       = &section_offsets[statement->section_index];
                statement->section_offset = *section_offset;

                *section_offset += statement->size;
                Resolver_advance(resolver);
        }
}

// Performs the "inverse" relaxation steps, starting assuming a statements minimum size and expanding them until a fixed
// point iteration is reached.
//
// NOTE: In this process, only statements which can actually expand are inspected with their expressions being evaluated. The
// others are ignored. Subsequent steps, like encoding, must take care of them.
internal B32
Resolver_relax_pass(Resolver *resolver)
{
        // TODO(high): I think here I'm not taking into account whether relaxation is enabled or not. If not, I should probably
        // consider maximum sizes, consider this example:
        //
        // ```asm
        // .option norelax
        // 1:
        // call printf
        // beqz x0, 1b
        // ```
        //
        // On GNU gas, this produces no relocation on the branch operations.

        B32 changed = 0;
        for (;;)
        {
                B32 break_should_relax = resolver->statements_end_reached || resolver->error.kind;
                if (break_should_relax)
                {
                        break;
                }

                Statement        *statement         = resolver->statement_current;
                Directive_Kind    directive_kind    = statement->directive_kind;
                Instruction_Kind  instruction_kind  = statement->instruction_kind;

                U32 size_old = statement->size;
                U32 size_new = size_old;

                switch (directive_kind)
                {
                // .align computes padding based on current offset.
                // Padding = bytes needed to reach next alignment boundary.
                case Directive_Kind__Align: {} // fallthrough, since most logic is shared with .skip.
                case Directive_Kind__Skip:
                {
                        Expression_Node *expression = Resolver_statement_expression_evaluate_index(resolver, statement, 0);

                        // NOTE: `.skip/align label_2 - label_1, global_2 - global_1` is supported. This means on every
                        // iteration we should check the value to get proper instruction size.
                        Resolver_expect(resolver, expression->evaluation >= Evaluation__Absolute, Resolver_Error_Kind__Evaluation_Absolute_Expected);
                        size_new = expression->integer_value;

                        if (statement->expressions_count > 1)
                        {
                                Resolver_statement_expression_evaluate_index(resolver, statement, 1);
                        }

                        if (directive_kind == Directive_Kind__Align)
                        {
                                U32 alignment = 1u << (U32)expression->integer_value;
                                U32 offset    = statement->section_offset;
                                U32 remainder = offset & (alignment - 1);
                                size_new = remainder == 0 ? 0 : alignment - remainder;
                        }
                } break;
                case Directive_Kind__Zero:
                {
                        U32 expression_index        = statement->expressions_indexes[0];
                        Expression_Node *expression = &resolver->expressions->data[expression_index];
                        Resolver_expression_evaluate(resolver, expression);
                        Resolver_expect(resolver, expression->evaluation >= Evaluation__Absolute, Resolver_Error_Kind__Evaluation_Absolute_Expected);
                        size_new = expression->integer_value;
                } break;

                // Ignored directives, that we keep for exhaustive match warnings.
                case Directive_Kind__None:           {} break;
                case Directive_Kind__Word_Double:    {} break;
                case Directive_Kind__Word:           {} break;
                case Directive_Kind__Word_Half:      {} break;
                case Directive_Kind__Byte:           {} break;
                case Directive_Kind__Section:        {} break;
                case Directive_Kind__Text:           {} break;
                case Directive_Kind__Data:           {} break;
                case Directive_Kind__Read_Only_Data: {} break;
                case Directive_Kind__BSS:            {} break;
                case Directive_Kind__Local:          {} break;
                case Directive_Kind__Globl:          {} break;
                case Directive_Kind__Global:         {} break;
                case Directive_Kind__Ascii:          {} break;
                case Directive_Kind__Asciz:          {} break;
                case Directive_Kind__String:         {} break;
                case Directive_Kind__Common:         {} break;
                // These two should be handled again!
                case Directive_Kind__Set:            {} break;
                case Directive_Kind__Equality:       {} break;
                case Directive_Kind__Option:         {} break;
                case Directive_Kind__COUNT:          {} break;
                }

                switch (instruction_kind)
                {
                case Instruction_Kind__LI:
                {
                        // If the immediate fits in 12 bits, sign-extended, a single addi suffices.
                        // Otherwise, we need a lui + addi, for 8 bytes total.
                        Expression_Node *expression = Resolver_statement_expression_evaluate_index(resolver, statement, 0);
                        B32 absolute = Evaluation__absolute(expression->evaluation);
                        Resolver_expect(resolver, absolute, Resolver_Error_Kind__Evaluation_Absolute_Expected);
                        S64 immediate = expression->integer_value;
                        U8 instructions_count = LI_instruction_encode(immediate, 0, 0);
                        size_new = instructions_count * INSTRUCTION_SIZE;
                } break;
                case Instruction_Kind__BEQ:  {} // fallthrough
                case Instruction_Kind__BNE:  {} // fallthrough
                case Instruction_Kind__BLT:  {} // fallthrough
                case Instruction_Kind__BGE:  {} // fallthrough
                case Instruction_Kind__BLTU: {} // fallthrough
                case Instruction_Kind__BGEU: {} // fallthrough
                // Pseudo-instructions, which expand using the same logic.
                case Instruction_Kind__BEQZ: {} // fallthrough
                case Instruction_Kind__BNEZ: {} // fallthrough
                case Instruction_Kind__BLEZ: {} // fallthrough
                case Instruction_Kind__BGEZ: {} // fallthrough
                case Instruction_Kind__BLTZ: {} // fallthrough
                case Instruction_Kind__BGTZ: {} // fallthrough
                case Instruction_Kind__BGT:  {} // fallthrough
                case Instruction_Kind__BLE:  {} // fallthrough
                case Instruction_Kind__BGTU: {} // fallthrough
                case Instruction_Kind__BLEU:
                {
                        // The behaviour of a branch instructions depends on the expression provided, and whether it's a
                        // symbol or a constant value. Remember that what will be executed by the CPU is jump at
                        // `offset - pc`.
                        //
                        // The rule is that if the assembler can prove that the target is in a 13-bit signed offset, a
                        // BRANCH relocation is emitted, otherwise a JAL relocation is emitted because the function is
                        // inverted and a JAL (`jal zero, offset`) instruction is placed.
                        //
                        // When there is a symbol, if it's a local label in the same section, we can compute its
                        // distance from the current statement, and we act accordingly. If it's a global symbol, this
                        // will be moved by the linker, as such we don't know its end size and we emit the JAL
                        // instruction. Same happens in case the expression evaluates to a constant: since we don't know
                        // the absolute value of PC at assembly time, and the linker can add more content to the
                        // section, it may well be that `constant - pc` won't fit. So, we conservatively expand it.
                        //
                        // If after our assembler attempt to expand the instruction, the offset won't fit at link time,
                        // the linker will error.
                        //
                        // Lastly, a BRANCH relocation is emitted only if relaxation is enabled.
                        Expression_Node *expression = Resolver_statement_expression_evaluate_index(resolver, statement, 0);
                        B32 unresolved = Evaluation__unresolved(expression->evaluation);
                        if (unresolved)
                        {
                                Symbols_Table_Entry *entry = expression->symbols_table_entry;
                                B32 local = ELF_Symbol_bind_m(entry->elf.type_and_binding) == ELF_Symbol_Binding__Local;
                                Statement *statement_declaration = &resolver->statements->data[entry->index_statement];
                                B32 section_same = statement->section_index == statement_declaration->section_index;
                                S64 distance = (S64)statement->section_offset - (S64)statement_declaration->section_offset;
                                B32 range_in = S64_bits_range_in(distance, IMMEDIATE_NOMINAL_B_SIZE_BIT);
                                B32 expand = !local || !section_same || !range_in;
                                if (expand)
                                {
                                        size_new = 8;
                                }
                        }
                        else
                        {
                                size_new = 8;
                        }
                } break;
                // NOTE: While both TAIL and CALL could be reduced to one instruction at assembly time (e.g. `call 0`),
                // this is not done in practice, and it is always expanded to a `auipc + jalr` pair. The reason is that
                // both instructions should emit the (now preferred) relocation `CALL_PLT`, and when the linker reads
                // it, it expects two instructions.
                case Instruction_Kind__TAIL: {} break;
                case Instruction_Kind__CALL: {} break;
                // Expands to `jal, x0, immediate`. In code, an expression->evaluation == Evaluation__Unresolved
                // expression can be in place, and e JAL relocation is emitted.
                case Instruction_Kind__J:    {} break;
                // TODO(low): remove this default.
                default: {} break;
                }

                if (size_new > size_old)
                {
                        statement->size = size_new;
                        changed = 1;
                }

                Resolver_advance(resolver);
        }

        return changed;
}

U32
Resolver_relax(Resolver *resolver)
{
        U32 pass_count = 0;
        for (;;)
        {
                pass_count += 1;
                Resolver_offsets_recompute(resolver);
                Resolver_cursor_reset(resolver);

                B32 changed = Resolver_relax_pass(resolver);
                Resolver_cursor_reset(resolver);

                if (!changed)
                {
                        break;
                }
        }

        Resolver_offsets_recompute(resolver);
        return pass_count;
}

// Encode instructions and directives to object files, emitting relocations if needed.
//
// Assumes Resolver_relax has been called.

void
Resolver_encode(Resolver *resolver)
{
        Resolver_cursor_reset(resolver);

        U8 data_directive_size = 0;

        for (;;)
        {
                // TODO(high): ensure that immediate values calculated can fit the instruction encoding.
                B32 break_should_encoding = resolver->statements_end_reached || resolver->error.kind;
                if (break_should_encoding)
                {
                        break;
                }

                Statement       *statement         = resolver->statement_current;
                Directive_Kind   directive_kind    = statement->directive_kind;
                Instruction_Kind instruction_kind  = statement->instruction_kind;

                Object_File_Section *section            = &resolver->sections[statement->section_index];
                Object_File_Section *section_relocation = &resolver->sections[statement->section_index];

                switch (directive_kind)
                {
                case Directive_Kind__Word_Double: { data_directive_size += 4; } // fallthrough
                case Directive_Kind__Word:        { data_directive_size += 2; } // fallthrough
                case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
                case Directive_Kind__Byte:
                {
                        local_persist const struct { Relocation_RISC_V add; Relocation_RISC_V sub; } relocation_pair_table[] =
                        {
                                { Relocation_RISC_V__Add_8,  Relocation_RISC_V__Sub_8  },
                                { Relocation_RISC_V__Add_16, Relocation_RISC_V__Sub_16 },
                                { Relocation_RISC_V__Add_32, Relocation_RISC_V__Sub_32 },
                                { Relocation_RISC_V__Add_64, Relocation_RISC_V__Sub_64 },
                        };
                        local_persist const Relocation_RISC_V relocation_table[] =
                        {
                                Relocation_RISC_V__None,
                                Relocation_RISC_V__None,
                                Relocation_RISC_V__32_Bit,
                                Relocation_RISC_V__64_Bit,
                        };
                        U8 relocation_table_index = (data_directive_size / 2) - 1;

                        U32 index = 0;
                        for (;;)
                        {
                                U32 expression_index        = statement->expressions_indexes[index];
                                Expression_Node *expression = &resolver->expressions->data[expression_index];
                                if (expression->evaluation == Evaluation__Unresolved)
                                {
                                        if (expression->symbol_operand)
                                        {
                                                Relocation_RISC_V relocation_kind_add = relocation_pair_table[relocation_table_index].add;
                                                Relocation_RISC_V relocation_kind_sub = relocation_pair_table[relocation_table_index].sub;

                                                ELF64_Relocation_Addend relocation_add =
                                                {
                                                        .offset = statement->section_offset + index,
                                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry, relocation_kind_add),
                                                        .addend = expression->integer_value,
                                                };
                                                ELF64_Relocation_Addend relocation_sub =
                                                {
                                                        .offset = statement->section_offset + index,
                                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry, relocation_kind_sub),
                                                        .addend = 0,
                                                };

                                                Object_File_Section_relocation_write(section_relocation, &relocation_add);
                                                Object_File_Section_relocation_write(section_relocation, &relocation_sub);
                                        }
                                        else
                                        {
                                                Relocation_RISC_V relocation_kind = relocation_table[data_directive_size];
                                                Resolver_expect(resolver, relocation_kind, Resolver_Error_Kind__Evaluation_Absolute_Expected);

                                                ELF64_Relocation_Addend relocation =
                                                {
                                                        .offset = statement->section_offset + index,
                                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry, relocation_kind),
                                                        .addend = expression->integer_value,
                                                };
                                                Object_File_Section_relocation_write(section_relocation, &relocation);
                                        }
                                }
                                else
                                {
                                        // TODO: conversions etc might bite me off here.
                                        S64 value = expression->integer_value;
                                        U64 limit_low  =  1 << (8 * data_directive_size - 1);
                                        U64 limit_high = (1 << (8 * data_directive_size)) - 1;
                                        B32 range_in = -limit_low <= value && value <= limit_high;
                                        // TODO: this could be a warning, and truncation could be performed.
                                        Resolver_expect(resolver, range_in, Resolver_Error_Kind__Expression_Value_Bounds_Outside);
                                        Object_File_Section_write_bytes(section, (U8 *)&value, data_directive_size);
                                }

                                index += 1;
                                B32 break_should = index >= statement->expressions_count || resolver->error.kind;
                                if (break_should)
                                {
                                        break;
                                }
                        }

                        data_directive_size = 0;
                } break;
                case Directive_Kind__Align:
                {
                        // TODO: depending on whether the section is executable or not, we should pad here with zeros or
                        // `nop` operations.
                        //
                        // Note that the assembler usually sets some flags to some section and the linker respects it,
                        // even though some overrides could be performed.
                        //
                        // Moreover, align emits always a relocation `R_RISCV_ALIGN`, with the pessimistic number of
                        // bytes that can guarantee the alignment, given there might be previous expression->evaluation == Evaluation__Unresolved symbols.
                } // fallthrough
                case Directive_Kind__Skip:
                {
                        U8 value = 0;
                        if (statement->expressions_count > 1)
                        {
                                U32 expression_index = statement->expressions_indexes[1];
                                Expression_Node *expression = &resolver->expressions->data[expression_index];
                                if (expression->evaluation == Evaluation__Unresolved)
                                {
                                        // Emit relocations for each byte
                                        Resolver_expect(resolver, expression->symbol_operand != 0, Resolver_Error_Kind__Relocation_Byte);
                                        assert_always_m(expression->kind == Expression_Kind__Subtract);
                                        U32 index = 0;
                                        for (;;)
                                        {
                                                B32 break_should = index >= statement->size;
                                                if (break_should)
                                                {
                                                        break;
                                                }
                                                // NOTE: only one relocation will have a non-zero addend if
                                                // expression->integer_value is non-zero.
                                                ELF64_Relocation_Addend relocation_add =
                                                {
                                                        .offset = statement->section_offset + index,
                                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry, Relocation_RISC_V__Add_8),
                                                        .addend = expression->integer_value,
                                                };
                                                ELF64_Relocation_Addend relocation_sub =
                                                {
                                                        .offset = statement->section_offset + index,
                                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry, Relocation_RISC_V__Sub_8),
                                                        .addend = 0,
                                                };
                                                Object_File_Section_relocation_write(section_relocation, &relocation_add);
                                                Object_File_Section_relocation_write(section_relocation, &relocation_sub);

                                                index += 1;
                                        }
                                }
                                else
                                {
                                        value = expression->integer_value;
                                }
                        }
                        Object_File_Section_write_byte(section, value, statement->size);
                }
                case Directive_Kind__Zero:
                {
                        Object_File_Section_write_byte(section, 0, statement->size);
                }

                // Ignored directives, that we keep for exhaustive match warnings.
                case Directive_Kind__Section:        {} break;
                case Directive_Kind__Text:           {} break;
                case Directive_Kind__Data:           {} break;
                case Directive_Kind__Read_Only_Data: {} break;
                case Directive_Kind__BSS:            {} break;
                case Directive_Kind__Local:          {} break;
                case Directive_Kind__Globl:          {} break;
                case Directive_Kind__Global:         {} break;
                case Directive_Kind__Ascii:          {} break;
                case Directive_Kind__Asciz:          {} break;
                case Directive_Kind__String:         {} break;
                // Both .set and .equ are zero-sized instructions!
                case Directive_Kind__Set:            {} break;
                case Directive_Kind__Equality:       {} break;
                case Directive_Kind__Common:         {} break;
                case Directive_Kind__Option:         {} break;
                case Directive_Kind__None:           {} break;
                case Directive_Kind__COUNT:          { unreachable_m(); } break;
                }

                // Instruction boilerplate
                U8  rd         = statement->register_destination;
                U8  rs1        = statement->register_source_1;
                U8  rs2        = statement->register_source_2;
                S64 immediate  = 0;

                Instruction_Encoding instruction_encoding = Instruction_Encoding_table[instruction_kind];
                U8 opcode             = instruction_encoding.opcode;
                U8 funct3             = instruction_encoding.funct3;
                U8 funct7             = instruction_encoding.funct7;
                U8 instruction_flags  = instruction_encoding.flags;

                if (instruction_flags & Instruction_Flags__Swap_1)
                {
                        swap_m(U8, rs2, rs1);
                }
                else if (instruction_flags & Instruction_Flags__Swap_2)
                {
                        rs2 = rs1; rs1 = 0;
                }

                // We do some expression boilerplate handling since every instruction has at most one expression and
                // doesn't allow difference between expression->evaluation == Evaluation__Unresolved symbols.
                Expression_Node *expression = 0;
                B32 absolute   = 0;
                B32 unresolved = 0;
                if (statement->expressions_count)
                {
                        U32 expression_index = statement->expressions_indexes[0];
                        expression = &resolver->expressions->data[expression_index];
                        if (expression->evaluation != Evaluation__None)
                        {
                                Resolver_expression_evaluate(resolver, expression);
                        }
                        Resolver_expect(resolver, expression->symbol_operand == 0, Resolver_Error_Kind__Instruction_Expression_Unresolved_Symbols);
                        absolute   = Evaluation__absolute(expression->evaluation);
                        unresolved = Evaluation__unresolved(expression->evaluation);
                        if (absolute)
                        {
                                immediate = expression->integer_value;
                        }

                }

                B32 relax_enabled = !(statement->flags & Statement_Flags__Relax_Disabled);
                B32 relaxable = relax_enabled
                             && (instruction_flags & Instruction_Flags__Relax_Hint)
                             && (expression->evaluation == Evaluation__Unresolved);

                if (relaxable)
                {
                        ELF64_Relocation_Addend relocation_relax =
                        {
                                .offset = statement->section_offset,
                                .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, Relocation_RISC_V__Relax),
                                .addend = 0,
                        };
                        Object_File_Section_relocation_write(section_relocation, &relocation_relax);
                }

                // Giant switch ahead. One could point that dividing cases by instruction formats is more logical. Well,
                // I try to do this, but sometimes there are that minor exceptions that need separate branches. While
                // formats are a inherently CPU-friendly concept, the data of some instructions contains in the same
                // family can changed a lot.
                //
                // For example:
                //
                // 1. Encoding S-types shared 99% of the logic of I-types, so they should be grouped together.
                // 2. Shift and shift wide instructions have different funct6/7 size on 64 bits, and have stricter
                //    requirements on their expressions.

                // TODO(urgent): check that when expansion happens, relocations are placed at the right offset, that may
                // be shifted.

                switch (instruction_kind)
                {
                case Instruction_Kind__LUI:       {} // fallthrough
                case Instruction_Kind__AUIPC:
                {
                        if (unresolved)
                        {
                                Resolver_expect(resolver, expression->relocation_operator, Resolver_Error_Kind__Relocation_Operator_Expected);
                                Relocation_RISC_V relocation_kind = Relocation_RISC_V_from_Relocation_Operator(statement->relocation_operator, statement->instruction_format);

                                ELF64_Relocation_Addend relocation =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, relocation_kind),
                                        .addend = expression->integer_value,
                                };
                                Object_File_Section_relocation_write(section_relocation, &relocation);
                        }

                        U32 encoding = instruction_u_encode_m(rd, immediate, opcode);
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                case Instruction_Kind__J:   {} // fallthrough, it's `jal 0, offset`.
                case Instruction_Kind__JAL:
                {
                        if (statement->flags & Statement_Flags__JAL_Register_Destination_Unset)
                        {
                                rd = 1; // ra
                        }

                        if (unresolved)
                        {
                                ELF64_Relocation_Addend relocation =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, Relocation_RISC_V__JAL),
                                        .addend = expression->integer_value,
                                };
                                Object_File_Section_relocation_write(section_relocation, &relocation);
                        }
                        else
                        {
                                B32 fits = S64_bits_range_in(expression->integer_value, IMMEDIATE_NOMINAL_J_SIZE_BIT);
                                Resolver_expect(resolver, fits, Resolver_Error_Kind__Immediate_Large);
                        }

                        U32 encoding = instruction_j_encode_m(rd, immediate, OPCODE_JAL);
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                case Instruction_Kind__BEQ:  {} // fallthrough
                case Instruction_Kind__BNE:  {} // fallthrough
                case Instruction_Kind__BLT:  {} // fallthrough
                case Instruction_Kind__BGE:  {} // fallthrough
                case Instruction_Kind__BLTU: {} // fallthrough
                case Instruction_Kind__BGEU:
                // Pseudo-instructions
                case Instruction_Kind__BEQZ: {} // fallthrough
                case Instruction_Kind__BNEZ: {} // fallthrough
                case Instruction_Kind__BLEZ: {} // fallthrough
                case Instruction_Kind__BGEZ: {} // fallthrough
                case Instruction_Kind__BLTZ: {} // fallthrough
                case Instruction_Kind__BGTZ: {} // fallthrough
                case Instruction_Kind__BGT:  {} // fallthrough
                case Instruction_Kind__BLE:  {} // fallthrough
                case Instruction_Kind__BGTU: {} // fallthrough
                case Instruction_Kind__BLEU:
                {
                        U8 size      = statement->size;
                        assert_always_m(size == 4 || size == 8);
                        B32 expanded = size == 8;
                        S64 immediate_first = immediate;

                        if (size == 4 && relax_enabled)
                        {
                                ELF64_Relocation_Addend relocation_branch =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, Relocation_RISC_V__Branch),
                                        .addend = expression->integer_value,
                                };
                                Object_File_Section_relocation_write(section_relocation, &relocation_branch);
                        }

                        if (expanded)
                        {
                                // Invert the operation by flipping the last bit.
                                funct3 ^= 1;
                                immediate_first = size;
                        }

                        U32 encoding = instruction_b_encode_m(rs2, rs1, immediate_first, OPCODE_BRANCH, funct3);
                        Object_File_Section_write_instruction(section, encoding);

                        if (expanded)
                        {
                                // NOTE: there might NOT be a symbol here, e.g. `beqz x1, 0`.
                                U32 encoding_jal = instruction_j_encode_m(0, immediate, OPCODE_JAL);
                                Symbols_Table_Entry *entry = expression->symbols_table_entry;
                                U32 symbol_index = entry ? entry->index : 0;
                                ELF64_Relocation_Addend relocation =
                                {
                                        // Add instruction size because it is the instruction next to it.
                                        .offset = statement->section_offset + 4,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, Relocation_RISC_V__JAL),
                                        .addend = expression->integer_value,
                                };

                                Object_File_Section_write_instruction(section, encoding_jal);
                                Object_File_Section_relocation_write(section_relocation, &relocation);
                        }
                } break;

                // Apart from small encoding different with stores, the logic of I and S format is the same, and thus
                // can be grouped together.
                case Instruction_Kind__JALR:      {} // fallthrough
                case Instruction_Kind__JR:        {} // fallthrough
                case Instruction_Kind__LB:        {} // fallthrough
                case Instruction_Kind__LH:        {} // fallthrough
                case Instruction_Kind__LW:        {} // fallthrough
                case Instruction_Kind__LBU:       {} // fallthrough
                case Instruction_Kind__LHU:       {} // fallthrough
                case Instruction_Kind__SB:        {} // fallthrough
                case Instruction_Kind__SH:        {} // fallthrough
                case Instruction_Kind__SW:        {} // fallthrough
                case Instruction_Kind__ADDI:      {} // fallthrough
                case Instruction_Kind__SLTI:      {} // fallthrough
                case Instruction_Kind__SLTIU:     {} // fallthrough
                case Instruction_Kind__XORI:      {} // fallthrough
                case Instruction_Kind__ORI:       {} // fallthrough
                case Instruction_Kind__ANDI:
                // RV64-specific
                case Instruction_Kind__LD:        {} // fallthrough
                case Instruction_Kind__LWU:       {} // fallthrough
                case Instruction_Kind__SD:        {} // fallthrough
                case Instruction_Kind__ADDIW:     {} // fallthrough
                // Pseudo-instructions
                case Instruction_Kind__NOT:       {} // fallthrough
                case Instruction_Kind__MV:        {} // fallthrough
                case Instruction_Kind__SEXT_W:    {} // fallthrough
                case Instruction_Kind__SEQZ:
                {
                        if (instruction_kind == Instruction_Kind__NOT)
                        {
                                immediate = -1;
                        }
                        if (instruction_kind == Instruction_Kind__SEQZ)
                        {
                                immediate = 1;
                        }

                        if (unresolved)
                        {
                                Relocation_RISC_V relocation_kind =
                                        Relocation_RISC_V_from_Relocation_Operator(statement->relocation_operator, statement->instruction_format);
                                Resolver_expect(resolver, relocation_kind, Resolver_Error_Kind__Relocation_Operator_Expected);
                                ELF64_Relocation_Addend relocation =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, relocation_kind),
                                        .addend = expression->integer_value,
                                };
                                Object_File_Section_relocation_write(section_relocation, &relocation);

                        }
                        else
                        {
                                B32 fits = S64_bits_range_in(expression->integer_value, IMMEDIATE_NOMINAL_I_S_SIZE_BIT);
                                Resolver_expect(resolver, fits, Resolver_Error_Kind__Immediate_Large);
                        }

                        U32 encoding = instruction_i_encode_m(rd, rs1, immediate, opcode, funct3);
                        if (opcode == OPCODE_STORE)
                        {
                                encoding = instruction_s_encode_m(rs2, rs1, immediate, opcode, funct3);
                        }
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                case Instruction_Kind__SLLI:   {} // fallthrough
                case Instruction_Kind__SRLI:   {} // fallthrough
                case Instruction_Kind__SRAI:   {} // fallthrough
                {
                        Resolver_expect(resolver, absolute, Resolver_Error_Kind__Evaluation_Absolute_Expected);
                        B32 range_in = 0 <= immediate && immediate < 64; // Architecture bit size.
                        Resolver_expect(resolver, range_in, Resolver_Error_Kind__Shift_Amount_Invalid);

                        U32 encoding = instruction_i_shift_encode_m(rd, rs1, immediate, opcode, funct3, funct7);
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                case Instruction_Kind__SLLIW: {} // fallthrough
                case Instruction_Kind__SRLIW: {} // fallthrough
                case Instruction_Kind__SRAIW:
                {
                        Resolver_expect(resolver, absolute, Resolver_Error_Kind__Evaluation_Absolute_Expected);
                        U32 encoding = instruction_i_shift_wide_encode_m(rd, rs1, immediate, opcode, funct3, funct7);
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                case Instruction_Kind__ADD:  {} // fallthrough
                case Instruction_Kind__SUB:  {} // fallthrough
                case Instruction_Kind__SLL:  {} // fallthrough
                case Instruction_Kind__SLT:  {} // fallthrough
                case Instruction_Kind__SLTU: {} // fallthrough
                case Instruction_Kind__XOR:  {} // fallthrough
                case Instruction_Kind__SRL:  {} // fallthrough
                case Instruction_Kind__SRA:  {} // fallthrough
                case Instruction_Kind__OR:   {} // fallthrough
                case Instruction_Kind__AND:  {} // fallthrough
                case Instruction_Kind__ADDW: {} // fallthrough
                case Instruction_Kind__SUBW: {} // fallthrough
                case Instruction_Kind__SLLW: {} // fallthrough
                case Instruction_Kind__SRLW: {} // fallthrough
                case Instruction_Kind__SRAW: {} // fallthrough
                // Pseudo-instructions (rd, rs → R-type with x0)
                case Instruction_Kind__NEG:  {} // fallthrough
                case Instruction_Kind__NEGW: {} // fallthrough
                case Instruction_Kind__SNEZ: {} // fallthrough
                case Instruction_Kind__SLTZ: {} // fallthrough
                case Instruction_Kind__SGTZ:
                {
                        if (unresolved)
                        {
                                assert_always_m(instruction_kind == Instruction_Kind__ADD);
                                ELF64_Relocation_Addend relocation =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, Relocation_RISC_V__Thread_Pointer_Relative_Add),
                                        .addend = expression->integer_value,
                                };
                                Object_File_Section_relocation_write(section_relocation, &relocation);

                        }

                        U32 encoding = instruction_r_encode_m(rd, rs1, rs2, opcode, funct3, funct7);
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                // Pseudo-case instructions
                // TODO: check
                case Instruction_Kind__CALL:
                case Instruction_Kind__TAIL:
                {
                        if (statement->flags & Statement_Flags__CALL_Register_Destination_Unset)
                        {
                                rd = 1; // ra
                        }


                        // FIX: blocked by symbol index being wrong, and care between the two instructions.

                        if (absolute)
                        {
                                ELF64_Relocation_Addend relocation =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(0, Relocation_RISC_V__Call_PLT),
                                        .addend = expression->integer_value,
                                };
                        }

                        U32 auipc_encoding = instruction_u_encode_m(rd, 0, OPCODE_AUIPC);
                        U32 jalr_encoding  = instruction_i_encode_m(rd, 0, 0, OPCODE_JALR, FUNCT3_JALR);

                        Object_File_Section_write_instruction(section, auipc_encoding);
                        Object_File_Section_write_instruction(section, jalr_encoding);
                } break;
                // TODO: check
                case Instruction_Kind__LI:
                {
                        // TODO(low): See other comments, this is redundant, unfortunately. This is very inefficient,
                        // although not many iterations.
                        LI_instruction_encode(immediate, section, rd);
                } break;
                // TODO: check
                case Instruction_Kind__LA:
                {
                        S64 value = 0;
                        S64 upper = 0;
                        S64 lower = 0;

                        if (absolute)
                        {
                                value = expression->integer_value;
                                upper = (value + 0x800) >> 12;
                                lower = value & 0xFFF;
                        }

                        if (unresolved)
                        {
                                Relocation_RISC_V relocation_hi = Relocation_RISC_V__High_20;
                                ELF64_Relocation_Addend relocation =
                                {
                                        .offset = statement->section_offset,
                                        .info   = ELF64_Relocation_info_m(expression->symbols_table_entry->index, relocation_hi),
                                        .addend = expression->integer_value,
                                };
                                Object_File_Section_relocation_write(section_relocation, &relocation);

                                upper = 0;
                        }

                        U32 lui_encoding = instruction_u_encode_m(rd, upper, OPCODE_LUI);
                        U32 addi_encoding = instruction_i_encode_m(rd, rd, lower, OPCODE_I_TYPE, FUNCT3_ADDI);
                        Object_File_Section_write_instruction(section, lui_encoding);
                        Object_File_Section_write_instruction(section, addi_encoding);
                } break;

                case Instruction_Kind__FENCE:
                {
                        U32 immediate_fence = (rs1 << 4) | rs2;  // pred(4) | succ(4)
                        U32 encoding = instruction_i_encode_m(0, 0, immediate_fence, OPCODE_FENCE, 0);
                        Object_File_Section_write_instruction(section, encoding);
                } break;

                case Instruction_Kind__NOP:       { Object_File_Section_write_instruction(section, ENCODING_NOP);    } break;
                case Instruction_Kind__RET:       { Object_File_Section_write_instruction(section, ENCODING_RET);    } break;
                case Instruction_Kind__ECALL:     { Object_File_Section_write_instruction(section, ENCODING_ECALL);  } break;
                case Instruction_Kind__EBREAK:    { Object_File_Section_write_instruction(section, ENCODING_EBREAK); } break;
                case Instruction_Kind__PAUSE:     { Object_File_Section_write_instruction(section, ENCODING_PAUSE);  } break;
                case Instruction_Kind__FENCE_TSO: { Object_File_Section_write_instruction(section, ENCODING_TSO);    } break;

                case Instruction_Kind__None:      {} break;
                case Instruction_Kind__COUNT:     { unreachable_m(); } break;
                }

                Resolver_advance(resolver);
        }

        return;
}

// ============================================================================
// RISC-V Assembler Relocation Decision Logic
// ============================================================================
//
// ELF Structures Reference (from <elf.h>):
// -----------------------------------------
//
//   Elf64_Sym / Elf32_Sym — Symbol table entry
//     .st_info   -> ELF64_ST_BIND() extracts binding: STB_LOCAL, STB_GLOBAL, STB_WEAK
//                -> ELF64_ST_TYPE() extracts type: STT_NOTYPE, STT_OBJECT, STT_FUNC, ...
//     .st_shndx  -> Section index where the symbol is defined:
//                     SHN_UNDEF   (0)      — symbol is not defined in this translation unit
//                     SHN_COMMON  (0xFFF2) — tentative (common) definition
//                     SHN_ABS     (0xFFF1) — absolute, not relocated
//                     1..N                 — index into the section header table
//     .st_value  -> Offset within the section (for defined symbols)
//     .st_size   -> Size of the symbol (optional, informational)
//
//   Elf64_Rela / Elf32_Rela — Relocation entry (RISC-V uses SHT_RELA exclusively)
//     .r_offset  -> Offset within the section where the relocation applies
//     .r_info    -> ELF64_R_SYM()  — index into the symbol table
//                -> ELF64_R_TYPE() — relocation type (R_RISCV_*)
//     .r_addend  -> Signed addend used in the relocation computation
//
//   Elf64_Shdr / Elf32_Shdr — Section header (for determining "same section")
//     .sh_type   -> SHT_RELA for relocation sections
//     .sh_link   -> Index of the associated symbol table
//     .sh_info   -> Index of the section to which relocations apply
//
// Relocation sections are named .rela.<section>, e.g. .rela.text, .rela.data.
// Each Elf64_Rela entry targets the section identified by the parent
// section header's sh_info field.
//
// ============================================================================
// Decision Procedure
// ============================================================================
//
// For each statement that references a symbol in an expression, the assembler
// walks the following cases in order. The FIRST matching case applies.
//
// ----------------------------------------------------------------------------
// CASE 1: Symbol is UNDEFINED (st_shndx == SHN_UNDEF)
// ----------------------------------------------------------------------------
//   -> Always emit a relocation. The linker must resolve this symbol.
//
//   Relocation type depends on the reference context:
//
//     Assembler modifier / context            Relocation type
//     ─────────────────────────────────────    ──────────────────────────
//     %hi(symbol)                             R_RISCV_HI20
//     %lo(symbol)        in I-type instr      R_RISCV_LO12_I
//     %lo(symbol)        in S-type instr      R_RISCV_LO12_S
//     %pcrel_hi(symbol)                       R_RISCV_PCREL_HI20
//     %pcrel_lo(label)   in I-type instr      R_RISCV_PCREL_LO12_I
//     %pcrel_lo(label)   in S-type instr      R_RISCV_PCREL_LO12_S
//     %got_pcrel_hi(symbol)                   R_RISCV_GOT_HI20
//     %tprel_hi(symbol)                       R_RISCV_TPREL_HI20
//     %tprel_lo(symbol)  in I-type instr      R_RISCV_TPREL_LO12_I
//     %tprel_lo(symbol)  in S-type instr      R_RISCV_TPREL_LO12_S
//     %tprel_add(symbol)                      R_RISCV_TPREL_ADD
//     %tls_ie_pcrel_hi(symbol)                R_RISCV_TLS_GOT_HI20
//     %tls_gd_pcrel_hi(symbol)                R_RISCV_TLS_GD_HI20
//     B-type branch target                    R_RISCV_BRANCH
//     J-type jump target (jal)                R_RISCV_JAL
//     call/tail pseudo-instruction            R_RISCV_CALL or R_RISCV_CALL_PLT
//     .word symbol                            R_RISCV_32
//     .dword symbol                           R_RISCV_64
//
//   Note on .byte/.half: An absolute symbol reference in a .byte or .half
//   directive has no standard RISC-V relocation type for 8-bit or 16-bit
//   absolute addresses. Most assemblers reject this as an error. Do not
//   confuse this with the R_RISCV_ADD8/SUB8 or ADD16/SUB16 pairs, which
//   apply only to symbol DIFFERENCES (see Case 7).
//
// ----------------------------------------------------------------------------
// CASE 2: Symbol is COMMON (st_shndx == SHN_COMMON)
// ----------------------------------------------------------------------------
//   -> Always emit a relocation. Common symbols are tentative definitions;
//      the linker merges them and assigns their final address.
//   Relocation type selection: same table as Case 1.
//
// ----------------------------------------------------------------------------
// CASE 3: Symbol is GLOBAL or WEAK (ELF64_ST_BIND(st_info) == STB_GLOBAL
//         or STB_WEAK), defined in this translation unit
// ----------------------------------------------------------------------------
//   -> Always emit a relocation.
//      - STB_GLOBAL: needed for shared library interposition (the dynamic
//        linker may redirect the symbol to a different definition).
//      - STB_WEAK: may be overridden by a strong definition from another
//        translation unit.
//      A non-PIC static assembler targeting a known-final executable could
//      in principle resolve some of these, but the safe and standard behavior
//      is to emit the relocation.
//   Relocation type selection: same table as Case 1.
//
// ----------------------------------------------------------------------------
// CASE 4: Symbol is LOCAL (STB_LOCAL), defined in a DIFFERENT section
// ----------------------------------------------------------------------------
//   -> Always emit a relocation. The assembler does not know the final
//      distance or offset between sections; only the linker does.
//   Relocation type selection: same table as Case 1.
//
// ----------------------------------------------------------------------------
// CASE 5: Symbol is LOCAL (STB_LOCAL), defined in the SAME section
// ----------------------------------------------------------------------------
//   The assembler knows the offset between the reference and the target
//   within the section. Whether it can resolve the reference depends on
//   the reference type and whether linker relaxation is active.
//
//   5a. ABSOLUTE reference (%hi, %lo, .word, .dword, etc.):
//       -> Emit a relocation. Even though the intra-section offset is known,
//          the absolute virtual address depends on where the linker places
//          the section.
//       Relocation types:
//         %hi(symbol)                          R_RISCV_HI20
//         %lo(symbol)        in I-type         R_RISCV_LO12_I
//         %lo(symbol)        in S-type         R_RISCV_LO12_S
//         .word symbol                         R_RISCV_32
//         .dword symbol                        R_RISCV_64
//
//   5b. PC-RELATIVE reference, linker relaxation ENABLED:
//       -> Emit the PC-relative relocation AND a companion R_RISCV_RELAX.
//          Relaxation passes may shrink or expand instructions between the
//          reference and the target (e.g., relaxing auipc+jalr into jal),
//          invalidating the assembler-computed offset.
//       Relocation types (each paired with R_RISCV_RELAX):
//         %pcrel_hi(symbol)                    R_RISCV_PCREL_HI20  + R_RISCV_RELAX
//         %pcrel_lo(label)   in I-type         R_RISCV_PCREL_LO12_I + R_RISCV_RELAX
//         %pcrel_lo(label)   in S-type         R_RISCV_PCREL_LO12_S + R_RISCV_RELAX
//         B-type branch target                 R_RISCV_BRANCH       + R_RISCV_RELAX
//         J-type jump target (jal)             R_RISCV_JAL          + R_RISCV_RELAX
//         call/tail pseudo-instruction         R_RISCV_CALL(_PLT)   + R_RISCV_RELAX
//
//   5c. PC-RELATIVE reference, linker relaxation DISABLED:
//       -> Do NOT emit a relocation. The assembler can compute the final
//          PC-relative offset directly: both reference and target are in the
//          same section, and no relaxation pass will alter the distance.
//          Encode the offset into the instruction's immediate field.
//
// ----------------------------------------------------------------------------
// CASE 6: Expression is a CONSTANT (no symbol reference, purely numeric)
// ----------------------------------------------------------------------------
//   -> Do NOT emit a relocation. Encode the value directly into the
//      instruction or data directive.
//
// ----------------------------------------------------------------------------
// CASE 7: Expression is a SYMBOL DIFFERENCE (sym_a - sym_b)
// ----------------------------------------------------------------------------
//   Symbol differences produce a pair of relocations (R_RISCV_ADDn +
//   R_RISCV_SUBn) so the linker can evaluate the subtraction after
//   final layout. The width of the pair matches the data context:
//
//     Context       ADD relocation    SUB relocation
//     ───────       ──────────────    ──────────────
//     .byte         R_RISCV_ADD8      R_RISCV_SUB8
//     .half         R_RISCV_ADD16     R_RISCV_SUB16
//     .word         R_RISCV_ADD32     R_RISCV_SUB32
//     .dword        R_RISCV_ADD64     R_RISCV_SUB64
//     6-bit (rare)  R_RISCV_SET6      R_RISCV_SUB6
//
//   Which sub-case applies:
//
//   7a. Both symbols are LOCAL, SAME section, relaxation DISABLED:
//       -> Do NOT emit a relocation. The difference is a fixed constant
//          that the assembler can compute at assembly time.
//
//   7b. Both symbols are LOCAL, SAME section, relaxation ENABLED:
//       -> Emit the ADDn + SUBn relocation pair. Relaxation may change the
//          distance between the two symbols.
//
//   7c. Symbols are in DIFFERENT sections, or either symbol is STB_GLOBAL,
//       STB_WEAK, SHN_COMMON, or SHN_UNDEF:
//       -> Emit the ADDn + SUBn relocation pair.
//
//       On UNDEFINED symbols in difference expressions:
//       The assembler CAN emit the pair when one or both symbols are
//       undefined (st_shndx == SHN_UNDEF). It emits R_RISCV_ADDn against
//       sym_a and R_RISCV_SUBn against sym_b, deferring resolution entirely
//       to the linker. However, the linker will typically require that both
//       symbols, once resolved, reside in the same output section — otherwise
//       the difference is not a link-time constant and will produce a link
//       error. In practice, GNU as permits this; the correctness check is
//       deferred to link time. This is uncommon in handwritten assembly but
//       can appear in compiler-generated metadata (e.g., exception tables,
//       DWARF debug info).
//
// ============================================================================
// Notes
// ============================================================================
//
// - RISC-V uses SHT_RELA sections exclusively (not SHT_REL). Every relocation
//   entry carries an explicit r_addend, even when it is zero.
//
// - The R_RISCV_RELAX relocation is a marker, not a standalone fix-up. It
//   occupies its own Elf64_Rela entry at the same r_offset as the primary
//   relocation, with r_addend = 0 and r_sym typically 0. It signals to the
//   linker that the associated instruction sequence is eligible for
//   relaxation.
//
// - %pcrel_lo(label) does NOT reference the target symbol directly. It
//   references a label on the instruction that carries the corresponding
//   %pcrel_hi relocation. The linker uses the %pcrel_hi's target to compute
//   the low 12 bits. This means the r_sym in the %pcrel_lo relocation entry
//   points to the label, not to the ultimate target.
//
// - The call/tail pseudo-instructions expand to an auipc+jalr pair. The
//   R_RISCV_CALL / R_RISCV_CALL_PLT relocation covers the full 8-byte
//   sequence. With relaxation enabled, the linker may replace this with a
//   single jal instruction (4 bytes) plus a NOP or compressed NOP, or
//   just a single jal if alignment permits.
//
// ============================================================================
// internal void
// relocation_emit(Statement *statement, Expression_Node *expression, Relocation_RISC_V kind)
// {
//      ELF_Section section_index = ELF_Section_relocations[statement->section_index];
//      Object_File_Section *section = &resolver->sections[section_index];
//
//      ELF64_Relocation_Addend relocation =
//      {
//              .offset = statement->section_offset,
//              .info   = ELF64_Relocation_info_m(symbol_index, relocation_kind),
//              .addend = addend,
//      };
//
//      Object_File_Section_write(section, (U8 *)&relocation, sizeof(ELF64_Relocation_Addend));
//      return;
// }
