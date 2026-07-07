// NOTE: most of this is riscv-backend exclusive, so it should move somewhere accordingly.

// Macros for encoding relaxation state for RVC branches and far jumps.
#define RELAX_BRANCH_ENCODE(uncond, rvc, length)        \
  ((U32)                                                \
   (0xc0000000                                          \
    | ((uncond) ? 1 : 0)                                \
    | ((rvc) ? 2 : 0)                                   \
    | ((length) << 2)))
#define RELAX_BRANCH_P(i)      (((i) & 0xf0000000) == 0xc0000000)
#define RELAX_BRANCH_LENGTH(i) (((i) >> 2) & 0xF)
#define RELAX_BRANCH_RVC(i)    (((i) & 2) != 0)
#define RELAX_BRANCH_UNCOND(i) (((i) & 1) != 0)

typedef enum Expression_Flags
{
	Expression_Flags__None      = 0,
        Expression_Flags__Defer_Dot = 1,
	Expression_Flags__COUNT,
}
Expression_Flags;


internal Expression_Node *
expression_parse_with_flags
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Section            *section,
        Diagnostic_List    *diagnostics,
        Expression_Flags    flags
)
{
        // A stack of expression frames. Each sub-expression creates a frame associated to it.
        // When the sub-expression is parsed, the frame should be popped.
        //
        // A frame should have a node attached. An exception to this is when a new frame is created due to a left
        // parenthesis, because it defers creating such node.
        typedef struct Frame Frame;
        struct Frame
        {
                Frame            *next;
                Expression_Node  *node;
                Binding_Power     binding_power_minimum;
                B32               is_right_side_of_next;
                B32               null_denotation_parsed;
                U32               location_begin;

        };

        typedef struct Parenthesis_Frame Parenthesis_Frame;
        struct Parenthesis_Frame
        {
                Parenthesis_Frame *next;
                U32 location;
        };


        Arena_Temporary scratch = Arena__scratch_begin_m(&arena, 1);

        Diagnostic *error = 0;

        Frame *frame = Arena__push_struct_m(scratch.arena, Frame);
        frame->node  = Expressions_push_empty(expressions, arena);

        frame->node->location = cursor->current.location;
        frame->location_begin = cursor->current.location;

        // ZII node as initial result;
        Expression_Node *result = frame->node;

        Parenthesis_Frame *parenthesis_frame = 0;

        for (;;)
        {
                // Iterate until we pop the last frame.

                // Start by reading a null-denotation. We mark we've done this process be setting
                // `frame->null_denotation_parsed = 1`.
                //
                // Every branch advances the current token since it has its own custom logic.
                switch (cursor->current.kind)
                {
                case Token_Kind__Number:
                {
                        Token   peek      = token_peek(cursor->source, cursor->source_index, diagnostics, arena);
                        String8 peek_text = Source__text_at(cursor->source, peek.location, peek.size);

                        B32 identifier_is    = peek.kind == Token_Kind__Identifier;
                        B32 single_letter_is = identifier_is && (peek_text.count == 1);
                        B32 forward          = single_letter_is && peek_text.data[0] == 'f';
                        B32 backward         = single_letter_is && peek_text.data[0] == 'b';
                        Symbol_Ref *label    = 0;

                        // These two paths can probably collapse.
                        if (forward || backward)
                        {
                                U32            number        = U32_cast_safe(cursor->current.numerical_value);
                                Label_Numeric *label_numeric = Symbols_Table__label_numeric_get_or_default(symbols_table, number);
                                               label_numeric->instances += (U32)forward;
                                String8        label_name    = label_numeric_string(scratch.arena, *label_numeric);
                                               label         = Symbols_Table__get_or_default(symbols_table, label_name);

                                if (backward && !label->elf.section_index)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("backward label reference not found");
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, peek.location + peek.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                assert_always_m(!forward || label->elf.section_index == 0);

                                frame->node->kind       = Expression_Kind__Symbol;
                                frame->node->evaluation = Expression_Kind__Symbol;
                                frame->node->symbol     = label;
                                // Skip the letter
                                token_next(cursor, diagnostics, arena);
                        }
                        else
                        {
                                frame->node->kind          = Expression_Kind__Constant;
                                frame->node->evaluation    = Expression_Kind__Constant;
                                frame->node->integer_value = cursor->current.numerical_value;
                        }

                        frame->node->location         = cursor->current.location;
                        frame->null_denotation_parsed = 1;

                        token_next(cursor, diagnostics, arena);
                } break;

                case Token_Kind__Identifier:
                {
                        String8 name = Token_Cursor__text(cursor);
                        B32 dot = name.count == 1 && name.data[0] == '.';

                        Symbol_Ref *symbol = 0;
                        if (dot)
                        {
                                Symbols_Trie *dot_trie = Symbols_Table__dot(symbols_table);
                                Symbol_Ref__update_section(&dot_trie->symbol, section);
                                symbol = flags & Expression_Flags__Defer_Dot
                                       ? &dot_trie->symbol
                                       : Symbols_Table__clone(symbols_table, &dot_trie->symbol, dot_trie->name);
                        }
                        else
                        {
                                symbol = Symbols_Table__get_or_default(symbols_table, name);
                        }

                        symbol->flags |= Symbol_Flags__Used;

                        frame->node->kind             = Expression_Kind__Symbol;
                        frame->node->evaluation       = Expression_Kind__Symbol;
                        frame->node->symbol           = symbol;
                        frame->node->location         = cursor->current.location;
                        frame->null_denotation_parsed = 1;

                        token_next(cursor, diagnostics, arena);
                } break;

                case Token_Kind__Percentage:
                case Token_Kind__Minus:
                case Token_Kind__Tilde:
                case Token_Kind__Bang:
                {
                        // unary_operator <expression>
                        frame->node->kind = Expression_Kind_from_unary_Token_Kind(cursor->current.kind);
                        frame->null_denotation_parsed = 1;

                        Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
                        frame_new->node = Expressions_push_empty(expressions, arena);
                        frame_new->node->location = cursor->current.location;
                        frame_new->binding_power_minimum = Binding_Power__Unary;
                        frame_new->is_right_side_of_next = 1;

                        token_next(cursor, diagnostics, arena);
                        frame_new->location_begin = cursor->current.location;

                        SLL_stack_push_m(frame, frame_new);
                        continue;
                } break;

                case Token_Kind__Parenthesis_Left:
                {
                        // ( <expression> )
                        Parenthesis_Frame *parenthesis_frame_new = Arena__push_struct_m(scratch.arena, Parenthesis_Frame);
                        parenthesis_frame_new->location = cursor->current.location;
                        SLL_stack_push_m(parenthesis_frame, parenthesis_frame_new);

                        token_next(cursor, diagnostics, arena);
                        continue;
                } break;

                default:
                {
                        if (!frame->null_denotation_parsed)
                        {
                                // Don't pollute codepaths to exit: mark an empty node, which is safe, and mark the error with
                                // its diagnostic.
                                frame->node = Expressions_push_empty(expressions, arena);
                                frame->null_denotation_parsed = 1;
                                frame->node->location = cursor->current.location;

                                error = Arena__push_struct_m(arena, Diagnostic);
                                error->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Expression_Null_Denotation_Expected];
                                error->location = cursor->current.location;
                                error->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, error);

                                token_next(cursor, diagnostics, arena);
                        }
                } break;
                }

                // Don't check binding power of a ')', handle it and then check what's next.
                if (cursor->current.kind == Token_Kind__Parenthesis_Right)
                {
                        if (parenthesis_frame)
                        {
                                SLL_stack_pop_m(parenthesis_frame);
                        }
                        else
                        {
                                error = Arena__push_struct_m(arena, Diagnostic);
                                error->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Expression_Parenthesis_Right_Unmatching];
                                error->location = cursor->current.location;
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, error);
                        }
                        token_next(cursor, diagnostics, arena);
                }


                // The `binding_power_minimum` is used to describe precedence. Operator tokens have an associated power
                // that is transferred to the next null denotation to preserve context.
                //
                // Consider the example `4 + 3 * 5`. When 3 is parsed, we want to remember that it is currently the
                // right side of an additive operation. When we peek the next token, that will be a star, that will have
                // an associated multiplicative power, which is higher. This means 3 should be "absorbed" i.e.,
                // considered the left node of start. As such, the tree should look as follows.
                //
                //               +
                //             4   *
                //                3 5
                //
                // If we consider `4 + 3 - 5` instead, now the minus sign has the same additive power, which means that
                // `4 + 3` concludes an expression, and the current expression frame can be popped.
                //
                // Unary operators have the highest binding power so that they mark the end of an expression
                // immediately. Consider `-4 + 3`, when the plus sign is read the minimum binding power would be unary,
                // so we know that the expression is completed.
                //
                // From this, we can understand that parenthesis are simply tokens with zero binding power, used to
                // conclude expressions. If we had `(4 + 3) * 5`, reading the right parenthesis after 3, where the
                // former has zero binding power, would conclude reading the expression `4 + 3`.

                Binding_Power next_power = Binding_Power_from_Token_Kind(cursor->current.kind);
                B32 pop = next_power <= frame->binding_power_minimum || cursor->source_index >= cursor->source->count;
                if (pop)
                {
                        if (frame->is_right_side_of_next)
                        {
                                assert_always_m(frame->next);
                                frame->next->node->index_right = frame->node->index;
                        }

                        if (!frame->next || error)
                        {
                                // Save the result before popping the last frame.
                                result = frame->node;
                        }
                        U32 location_end = cursor->previous.location + cursor->previous.size;
                        frame->node->location_range = (Range1_U32){{ frame->location_begin, location_end }};
                        SLL_stack_pop_n_m(frame, next);
                }
                else
                {
                        // <expression> binary_operator <expression>
                        Expression_Node *left = frame->node;

                        // Set central node.
                        frame->node = Expressions_push_empty(expressions, arena);
                        frame->node->kind = Expression_Kind__binary_from_Token_Kind(cursor->current.kind);
                        frame->node->index_left = left->index;
                        frame->node->location   = cursor->current.location;

                        token_next(cursor, diagnostics, arena);

                        // Prepare new frame
                        Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
                        frame_new->node = Expressions_push_empty(expressions, arena);
                        frame_new->node->location = cursor->current.location;
                        frame_new->binding_power_minimum = next_power;
                        frame_new->is_right_side_of_next = 1;
                        frame_new->location_begin = cursor->current.location;
                        SLL_stack_push_m(frame, frame_new);
                }

                B32 break_should = frame == 0;
                if (break_should)
                {
                        break;
                }

        }

        if (parenthesis_frame)
        {
                error = Arena__push_struct_m(arena, Diagnostic);
                error->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Expression_Parenthesis_Left_Unclosed];
                error->location = parenthesis_frame->location;
                SLL_queue_push_m(diagnostics->first, diagnostics->last, error);
        }

        Arena_Temporary__end(scratch);
        return result;
}

// NOTE: parsing an expression right now mixes machine-dependent and independent code. It would be nice to provide a
// common ground for it if it makes sense.
//
// TODO: add a "defer" mode here or similar to not create a clone of the dot if found but rather keep a reference to the
// global dot symbol, analogous to GNU as "expr_defer_incl_dot`. Needed for `.eqv` directive.
internal Expression_Node *
expression_parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Section            *section,
        Diagnostic_List    *diagnostics
)
{
        Expression_Node *result = expression_parse_with_flags
        (
                arena,
                cursor,
                expressions,
                symbols_table,
                section,
                diagnostics,
                Expression_Flags__None
        );
        return result;
}


// NOTE: both LLVM and GNU as have a precise way of handle relocation operators. They must appear at the beginning of
// the expression, and everything else is absorbed by it. Examples:
//
// - `addi x1, x0, %lo(foo) + 1` is equivalent to `addi x1, x0, %lo(foo + 1)`.
// - `addi x1, x0, 1 + %lo(foo)` is invalid.
internal Expression_Node *
expression_parse_with_relocation
(
        Arena               *arena,
        Token_Cursor        *cursor,
        Expressions         *expressions,
        Symbols_Table       *symbols_table,
        Section             *section,
        Diagnostic_List     *diagnostics,
        // Machine-dependent
        U16                 *relocation_out,
        // Zero-terminated.
        const Relocation_Operator *relocation_match
)
{

        if (cursor->current.kind == Token_Kind__Percentage)
        {
                assert_always_m(relocation_out && "relocation_out should be set");

                // Parse relocation
                token_next(cursor, diagnostics, arena);
                String8 text = Token_Cursor__text(cursor);

                B32 found = 0;
                for (;;)
                {
                        B32 break_should = found || !relocation_match->relocation;
                        if (break_should)
                        {
                                break;
                        }
                        found = String8__match_exact(relocation_match->text, text);
                        relocation_match += 1;
                }

                if (!found)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                        diagnostic->message    = String8__literal("invalid relocation operator for instruction");
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }

                *relocation_out = relocation_match->relocation;
                token_next(cursor, diagnostics, arena);
        }
        Expression_Node *result = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
        return result;
}

// Maybe find statement boundary?
// Giving good diagnostics now it's harder because I can't reference past tokens.
//

internal void
RISCV_Instruction__parse
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Section                 *section,
        U32                      instruction_hash,

        U16                     *relocation_out,
        RISCV_Instruction       *instruction_out,
        U32                     *expression_index_out
)
{
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
        const char *opcode_name = opcode->name;

        U32 location_begin = cursor->current.location;
        token_next(cursor, diagnostics, arena);

        Expression_Node *expression = 0;
        B32 match = 0;

        // Iterate over opcode entries with the same name.
        for (;;)
        {
                *instruction_out = RISCV_Instruction__create(opcode, location_begin);
                OP_Argument *arguments = opcode->arguments;

                // Iterate over opcode arguments.
                for (;;)
                {
                        OP_Argument argument = *arguments;
                        if (!argument)
                        {
                                match = opcode->hash && (!opcode->match_function || opcode->match_function(opcode, instruction_out->encoding));
                                break;
                        }

                        switch (argument)
                        {
                        case OP_Argument__Comma:
                        {
                                // NOTE: This whole thing could extracted into a `expect_comma_and_advance`.
                                Token token_before_comma = cursor->previous;
                                if (cursor->current.kind == Token_Kind__Comma)
                                {
                                        token_next(cursor, diagnostics, arena);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = token_before_comma.location + token_before_comma.size;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Parenthesis_Left:
                        {
                                if (cursor->current.kind != Token_Kind__Parenthesis_Left)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("'(' expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Parenthesis_Right:
                        {
                                if (cursor->current.kind != Token_Kind__Parenthesis_Right)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("')' expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__RD:  {} // fallthrough
                        case OP_Argument__RS3: {} // fallthrough
                        case OP_Argument__RS2: {} // fallthrough
                        case OP_Argument__RS1:
                        {
                                U8 reg = register_lookup(Token_Cursor__text(cursor));
                                if (reg == register_invalid)
                                {
                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                       diagnostic->location   = cursor->current.location;
                                       diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Register_Invalid];
                                       diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                switch (argument)
                                {
                                       case OP_Argument__RD:  { INSERT_OPERAND(RD,  *instruction_out, reg); } break;
                                       case OP_Argument__RS3: { INSERT_OPERAND(RS3, *instruction_out, reg); } break;
                                       case OP_Argument__RS2: { INSERT_OPERAND(RS2, *instruction_out, reg); } break;
                                       case OP_Argument__RS1: { INSERT_OPERAND(RS1, *instruction_out, reg); } break;
                                }
                                token_next(cursor, diagnostics, arena);
                        } break;
                        case OP_Argument__Offset_PC_Relative_20:
                        {
                                // NOTE: we use GNU as approach to add mark a branch relocation immediately.
                                // This relocation is temporary, and could be changed, since it depends on the
                                // value of the expression and the symbols required.
                                //
                                // At assembly time, we may not know how many instructions this will expand to. It is
                                // deferred later when we know all instructions. It is a different situation compared to
                                // a `li` or `call` instruction which, during instruction parsing, are already expanded
                                // into a known number of instructions (`INSN_MACRO`)
                                *relocation_out = Relocation_RISC_V__JAL;
                                expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);

                                // For branches we can't support a fixup. While GNU as silently ignores additional
                                // symbols, here we either warn or error.
                                expression_evaluate(expressions, expression->index);
                                if (expression->symbol_operand)
                                {
                                        // TODO: this diagnostic could be better, I should probably support re-lexing
                                        // from a specific location to get the exact token.
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("PC relative offset expression contains operand symbol which will be skipped");
                                        diagnostic->location   = expression->location_range.v[0];
                                        diagnostic->ranges[0]  = expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Offset_PC_Relative_12:
                        {
                                // See notes for `OP_Argument__Offset_PC_Relative_20`.
                                *relocation_out = Relocation_RISC_V__Branch;
                                expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);

                                // For branches we can't support a fixup. While GNU as silently ignores additional
                                // symbols, here we either warn or error.
                                expression_evaluate(expressions, expression->index);
                                if (expression->symbol_operand)
                                {
                                        // TODO: this diagnostic could be better, I should probably support re-lexing
                                        // from a specific location to get the exact token.
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("PC relative offset expression contains operand symbol which will be skipped");
                                        diagnostic->location   = expression->location_range.v[0];
                                        diagnostic->ranges[0]  = expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Offset_Load:
                        {
                                OP_Argument *next = arguments + 1;
                                assert_always_m(next && "invalid operand list");

                                Token peek = token_peek(cursor->source, cursor->source_index, diagnostics, arena);
                                if (*next == OP_Argument__Parenthesis_Left && peek.kind == Token_Kind__Parenthesis_Left)
                                {
                                       // Omitted immediate, e.g. lw t1, (t0)
                                       arguments += 2;
                                }
                        } // fallthrough;
                        case OP_Argument__Immediate_I:
                        {
                                expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, section, diagnostics, relocation_out, Relocation_Operator__itype);
                                if (!*relocation_out)
                                {
                                       if (expression->kind == Expression_Kind__Constant)
                                       {
                                               // TODO: normalize constant expression? See GNU as.
                                               B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                               if (!fits)
                                               {
                                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                       diagnostic->location   = expression->location_range.v[0];
                                                       diagnostic->message    = String8__literal("constant expression value must fits in 12 bits");
                                                       diagnostic->ranges[0]  = expression->location_range;
                                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                               }

                                               // TODO: GNU as does this at a later step, and by default emits a
                                               // relocation. Consider doing the same.
                                               U32 encoding_immediate = encode_immediate_i_m(expression->integer_value);
                                               instruction_out->encoding |= encoding_immediate;
                                       }
                                       else
                                       {
                                               Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->message    = String8__literal("Non-constant expression must have an appropriate relocation operator");
                                               diagnostic->ranges[0]  = expression->location_range;
                                               SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                       }
                                }
                        } break;
                        case OP_Argument__Immediate_U:
                        {
                                expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, section, diagnostics, relocation_out, Relocation_Operator__utype);
                                if (!*relocation_out)
                                {
                                       if (expression->kind == Expression_Kind__Constant)
                                       {
                                               S64 result = expression->integer_value;
                                               B32 fits = 0 <= result && result < (S64)(1 << 20);
                                               if (!fits)
                                               {
                                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                       diagnostic->location   = expression->location_range.v[0];
                                                       diagnostic->message    = String8__literal("constant expression value must in the range 0..1048576");
                                                       diagnostic->ranges[0]  = expression->location_range;
                                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                               }

                                               // TODO: GNU as does this at a later step, and by default emits a
                                               // relocation. Consider doing the same.
                                               U32 encoding_immediate = encode_immediate_u_m(expression->integer_value);
                                               instruction_out->encoding |= encoding_immediate;
                                       }
                                       else
                                       {

                                               Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                               diagnostic->message    = String8__literal("Non-constant expression must have an appropriate relocation operator");
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                               SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                       }
                                }
                        } break;
                        case OP_Argument__Immediate_Large:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                if (expression->kind != Expression_Kind__Constant)
                                {
                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                       diagnostic->message    = String8__literal("Constant expression expected");
                                       diagnostic->location   = expression->location_range.v[0];
                                       diagnostic->ranges[0]  = expression->location_range;
                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Call_Expression:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                *relocation_out = Relocation_RISC_V__Call_PLT;
                        } break;
                        default: { unreachable_m(); }
                        }

                        arguments += 1;
                }

                if (match || opcode->hash == 0 || opcode->name != opcode_name)
                {
                        break;
                }

                opcode += 1;
        }

        if (!match)
        {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->location   = location_begin;
                diagnostic->message    = String8__literal("unrecognized opcode");
                diagnostic->ranges[0]  = (Range1_U32){{ location_begin, cursor->current.location + cursor->current.size }};
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
        }

        *expression_index_out = expression ? expression->index : 0;

        return;
}

internal void
add_instruction_relaxed
(
        Section *section,
        U32      encoding,
        U32      location,
        U8       worst_case_size,
        U8       best_case_size,
        U32      expression_index,
        U32      subtype
)
{
        U8 *data = Fragment_List__variable
        (
                &section->fragment_list,
                section->arena,
                location,
                worst_case_size,
                best_case_size,
                expression_index,
                subtype,
                Relax_State__Machine
        );

        memory_copy(data, (U8 *)&encoding, RISCV_instruction_size(encoding));
        return;
}

// Add a fixed size instruction into a fragment. If there is a fixup associated to this function (fixup != 0),
// track the information of where this instruction has been placed.
internal void
add_instruction_fixed
(
        Section *section,
        Fixup   *fixup,

        U32      instruction_encoding,
        U32      instruction_location
)
{
        U8 instruction_size = RISCV_instruction_size(instruction_encoding);
        U8 *data = Fragment_List__fixed
        (
                 &section->fragment_list,
                 section->arena,
                 instruction_location,
                 instruction_size
        );

        // Track its precise location within the fragment. Important to do it _after_ we've written it
        // since it might have landed into another fragment because of low capacity of the previous.
        if (fixup)
        {
                Fragment *last = section->fragment_list.last;
                U32 encoding_offset = last->size_fixed - instruction_size;

                fixup->fragment        = last;
                fixup->encoding_offset = encoding_offset;
        }

        memory_copy(data, (U8 *)&instruction_encoding, instruction_size);
        return;
}

internal void
RISCV_Instruction__append
(
        Section           *section,
        Fixups            *fixups,

        RISCV_Instruction *instruction,
        U32                expression_index,
        U16                relocation
)
{
        Fixup *fixup     = 0;
        B32    jump_is   = relocation == Relocation_RISC_V__JAL;
        B32    relaxable = relocation == Relocation_RISC_V__Branch || jump_is;
        B32    fixable   = relocation && !relaxable;

        if (fixable)
        {
                // NOTE: fixups, which are deferred patches, can be created only for fixed size instructions
                // (non-relaxable) because they need a precise location to be applied. Relaxable instructions,
                // like branches, break this invariant.
                fixup = Fixups__push(fixups);
                fixup->expression_index = expression_index;
                fixup->relocation_type  = relocation;
        }

        if (relaxable)
        {
                U8 best_case_size  = RISCV_instruction_size(instruction->encoding);
                U8 worst_case_size = 8;

                U32 subtype = RELAX_BRANCH_ENCODE (jump_is, best_case_size == 2, worst_case_size);
                add_instruction_relaxed
                (
                        section,
                        instruction->encoding,
                        instruction->location,
                        worst_case_size,
                        best_case_size,
                        expression_index,
                        subtype
                );
        }
        else
        {
                add_instruction_fixed
                (
                        section,
                        fixup,
                        instruction->encoding,
                        instruction->location
                );
        }

        return;
}

internal void
RISCV_macro_build
(

        Section     *section,
        Fixups      *fixups,

        String8      instruction_name,
        U32          location,
        U32          expression_index,
        OP_Argument *arguments,
        S32         *values
)
{
        U32 instruction_hash = FNV_hash_U32(instruction_name);
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
        assert_always_m(opcode && opcode->hash);

        RISCV_Instruction instruction = RISCV_Instruction__create(opcode, location);

        U16 relocation = 0;

        for (;;)
        {
                OP_Argument argument = *arguments;
                if (argument == 0)
                {
                        break;
                }

                S32 value = *values;

                switch (argument)
                {
                        default: { unreachable_m(); } break;

                        case OP_Argument__RD:  { INSERT_OPERAND(RD,  instruction, value); } break;
                        case OP_Argument__RS3: { INSERT_OPERAND(RS3, instruction, value); } break;
                        case OP_Argument__RS2: { INSERT_OPERAND(RS2, instruction, value); } break;
                        case OP_Argument__RS1: { INSERT_OPERAND(RS1, instruction, value); } break;

                        case OP_Argument__Immediate_I: {} // fallthrough
                        case OP_Argument__Immediate_U: { relocation = value; } break;
                }

                arguments += 1;
                values    += 1;
        }

        assert_always_m(relocation ? expression_index : 1);

        RISCV_Instruction__append
        (
                section,
                fixups,
                &instruction,
                expression_index,
                relocation
        );
}

// Expand a call pseudo instruction into an `auipc + jalr` pair with the provided register for `jalr`.
internal void
RISCV_call_expand
(
        Section         *section,
        Fixups          *fixups,

        U8  rd,
        U8  rs1,
        U32 expression_index,
        U16 relocation,
        U32 location
)
{
        // Ensure the instructions are in the same fragment
        OP_Argument *arguments_auipc   = OP_arguments_m(OP_Argument__RD, OP_Argument__Immediate_U);
        S32 values_auipc[2]            = {rs1, relocation};
        OP_Argument *arguments_jalr    = OP_arguments_m(OP_Argument__RD, OP_Argument__RS1);
        S32 values_jalr[2]             = {rd, rs1};

        // TODO: add equivalent of frag_grow. Not yet super clear how to accomplish that.
        // Fragment_List__ensure(&section->fragment_list, section->arena, 8);
        RISCV_macro_build
        (
                section,
                fixups,

                String8__literal("auipc"),
                location,
                expression_index,
                arguments_auipc,
                values_auipc
        );
        RISCV_macro_build
        (
                section,
                fixups,

                String8__literal("jalr"),
                location,
                0,
                arguments_jalr,
                values_jalr
        );
        // then?
}

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
RISCV_li_expand
(
        Section         *section,

        S64 immediate,
        U8  register_destination,
        U32 location
)
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
                B32 range_12     = S64_bits_range_in(immediate, 12);
                B32 range_32     = S64_bits_range_in(immediate, 32);
                B32 break_should = range_12 || range_32;

                if (range_12)
                {
                        instructions_count += 1;
                        if (section)
                        {
                                // Single ADDI from x0.
                                U32 encoding = instruction_i_encode_m(register_destination, 0, immediate,
                                                OPCODE_I_TYPE, FUNCT3_ADDI);
                                add_instruction_fixed(section, 0, encoding, location);
                        }
                }
                else if (range_32)
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
                                add_instruction_fixed(section, 0, encoding_lui, location);
                                if (!lui_suffices)
                                {
                                        U32 addiw_enc = instruction_i_encode_m(register_destination, register_destination, immediate_low_12,
                                                        OPCODE_I_TYPE, FUNCT3_ADDIW);
                                        add_instruction_fixed(section, 0, addiw_enc, location);
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
                        add_instruction_fixed(section, 0, encoding_slli, location);

                        if (tail != 0)
                        {
                                U32 encoding_addi = instruction_i_encode_m(register_destination, register_destination, tail, OPCODE_I_TYPE, FUNCT3_ADDI);
                                add_instruction_fixed(section, 0, encoding_addi, location);
                        }

                        peel_index -= 1;
                }
        }

        assert_always_m(instructions_count > 0);
        return instructions_count;
}


internal void
RISCV_instruction_pseudo_append
(
        Section                 *section,
        Fixups                  *fixups,

        RISCV_Instruction       *instruction,
        Expression_Node         *expression,
        U16                      relocation
)
{
        U8 rd  = (instruction->encoding >> OP_SH_RD)  & OP_MASK_RD;
        U8 rs1 = (instruction->encoding >> OP_SH_RS1) & OP_MASK_RS1;
        U8 rs2 = (instruction->encoding >> OP_SH_RS2) & OP_MASK_RS2;
        unused_m(rs2);

        U32 pseudo_type = instruction->opcode->mask;

        switch (pseudo_type)
        {
        default: { unreachable_m(); } break;
        case M_CALL:
        {
                RISCV_call_expand
                (
                        section,
                        fixups,
                        rd,
                        rs1,
                        expression->index,
                        relocation,
                        instruction->location
                );
        } break;
        case M_LI:
        {
                RISCV_li_expand
                (
                        section,
                        expression->integer_value,
                        rd,
                        instruction->location
                );
        } break;
        }
}

// Handles .local, .weak, .global directive. Those simply try to set the binding of a symbol, and nothing else. It is
// created if missing.
//
// TODO: should I just set the binding or in case of a promotion should I "delete" the other symbol and create a new
// one? This
internal void
binding_set
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Symbols_Table           *symbols_table,
        ELF_Symbol_Binding       binding
)
{
        token_next(cursor, diagnostics, arena);
        if (cursor->current.kind != Token_Kind__Identifier)
        {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
        }

        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);
        if (!(symbol->flags & Symbol_Flags__Declared))
        {
                // Still give a preliminary location for it so that we can show diagnostics.
                symbol->location = cursor->current.location;
        }

        U8 binding_old = ELF_Symbol_bind_m(symbol->elf.type_and_binding);
        B32 demoted = binding < binding_old;
        if (demoted)
        {
                {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->kind       = Diagnostic_Kind__Warning;
                diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Symbol_Demoted];
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
                {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->kind       = Diagnostic_Kind__Note;
                diagnostic->message    = Diagnostic__previous_declaration_String8;
                diagnostic->location   = symbol->location;
                diagnostic->ranges[0] = (Range1_U32){{ symbol->location, symbol->location + name.count }};
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
        }
        else
        {
                U8 type_and_binding = ELF_Symbol_info_m(binding, ELF_Symbol_type_m(symbol->elf.type_and_binding));
                symbol->elf.type_and_binding = type_and_binding;
        }

        token_next(cursor, diagnostics, arena);
}

typedef enum Set_Mode
{
        // Used in `.set/.equ`.
	Set_Mode__Override = 0,
        // Used in `.eqv`.
	Set_Mode__Strict_Forward,
        // Used in `.equiv`.
	Set_Mode__Strict,
}
Set_Mode;

internal void
directive_set_like
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Section                 *section,
        Sections_Table          *section_table,
        Set_Mode                 mode
)
{
        // TODO: check no conflicts with section names and register names. GNU as doesn't seem to error on using a
        // register name like `sp` though, which I think can be quite confusing/error prone.

        token_next(cursor, diagnostics, arena);
        if (cursor->current.kind != Token_Kind__Identifier)
        {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
        }

        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

        B32 already_defined_or_equated = symbol->elf.section_index || symbol->expression_index;
        if (already_defined_or_equated)
        {
                B32 frozen = mode != Set_Mode__Override || !(symbol->flags & Symbol_Flags__Volatile);
                if (frozen)
                {
                        {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->message    = String8__literal("symbol cannot be redefined");
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = Token__range(cursor->current);
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }
                        {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind       = Diagnostic_Kind__Note;
                        diagnostic->message    = Diagnostic__previous_declaration_String8;
                        diagnostic->location   = symbol->location;
                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + name.count }};
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }
                }

                symbol = Symbols_Table__clone(symbols_table, symbol, name);
        }

        Section *section_maybe = Sections_Table__get(section_table, name);
        if (section_maybe)
        {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->message    = String8__literal("cannot create a symbol with the same name of a section");
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Token__range(cursor->current);
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
        }

        symbol->location = cursor->current.location;
        Expression_Flags expression_flags = 0;

        if (mode == Set_Mode__Override)
        {
                symbol->flags |= Symbol_Flags__Volatile;
        }
        else if (mode == Set_Mode__Strict_Forward)
        {
                symbol->flags    |= Symbol_Flags__Forward_Reference;
                expression_flags |= Expression_Flags__Defer_Dot;
        }

        token_next(cursor, diagnostics, arena);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                token_next(cursor, diagnostics, arena);
        }
        else
        {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
        }

        Expression_Node *expression = expression_parse_with_flags
        (
                arena,
                cursor,
                expressions,
                symbols_table,
                section,
                diagnostics,
                expression_flags
        );
        symbol->expression_index = expression->index;

        if (mode != Set_Mode__Strict_Forward)
        {

                S64 result = expression_evaluate(expressions, expression->index);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        symbol->elf.section_index = ELF_Section_Index__Absolute;
                        symbol->elf.value         = result;
                }
        }

        return;
}

internal void
statement_read
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Section                 *section,
        Sections_Table          *sections_table,
        Fixups                  *fixups
)
{

        U32 source_index_start = cursor->source_index;
        token_next(cursor, diagnostics, arena);

        B32 progress = 1;
        B32 error =  0;

        for (;;)
        {
                Directive_Kind directive_kind         = 0;
                U32            instruction_hash       = 0;
                B32            null_terminated_string = 0;
                B32            empty_line             = 0;

                // TODO: when to exit?
                progress = source_index_start < cursor->source_index;
                B32 break_should_outer = cursor->current.kind == Token_Kind__None
                                      || cursor->current.kind == Token_Kind__Error
                                      || error;
                assert_always_m((progress || break_should_outer) && "infinite loop detected");

                if (break_should_outer)
                {
                        break;
                }

                switch (cursor->current.kind)
                {
                // no-op, continue;
                case Token_Kind__Newline:
                {
                        token_next(cursor, diagnostics, arena);
                        empty_line = 1;
                } break;
                case Token_Kind__Semicolon:
                {
                        token_next(cursor, diagnostics, arena);
                        empty_line = 1;
                } break;
                case Token_Kind__Number:
                {
                        Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);

                        // Should be a numeric label definition, e.g. 1:
                        U32 number = U32_cast_safe(cursor->current.numerical_value);
                        token_next(cursor, diagnostics, arena);
                        B32 label_definition = cursor->current.kind == Token_Kind__Colon;
                        if (!label_definition)
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->message    = String8__literal("expected ':' for numeric label declaration");
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        Label_Numeric *label_numeric = Symbols_Table__label_numeric_get_or_default(symbols_table, number);
                        String8        label_name    = label_numeric_string(scratch.arena, *label_numeric);
                        Symbol_Ref    *label         = Symbols_Table__get(symbols_table, label_name);

                        B32 missing_or_already_declared = !label || label->elf.section_index != 0;
                        if (missing_or_already_declared)
                        {
                                label_numeric->instances += 1;
                                String8 label_name_new = label_numeric_string(scratch.arena, *label_numeric);
                                label = Symbols_Table__create(symbols_table, label_name_new);
                        }
                        Symbol_Ref__update_section(label, section);

                        Arena__scratch_end_m(scratch);

                        if (label_definition)
                        {
                                token_next(cursor, diagnostics, arena);
                        }

                } break;
                // Instructions, directives and label start with an identifier. We have to discriminate further.
                case Token_Kind__Identifier:
                {

                        String8 identifier = Token_Cursor__text(cursor);

                        B32 dot_start = identifier.data[0] == '.';
                        if (dot_start)
                        {
                                directive_kind = Directive_Kind__from_String8(identifier);
                        }

                        Token next = token_peek(cursor->source, cursor->source_index, diagnostics, arena);
                        B32 label_found = next.kind == Token_Kind__Colon;
                        if (label_found)
                        {
                                // Label declaration!
                                Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, identifier);
                                if (symbol->fragment)
                                {
                                        // NOTE: GNU as accepts the case where the fragment is the same AND same offset.
                                        {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Label_Duplicate];
                                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                        {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind       = Diagnostic_Kind__Note;
                                        diagnostic->location   = symbol->location;
                                        diagnostic->message    = Diagnostic__previous_declaration_String8;
                                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + identifier.count }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                }
                                symbol->flags            |= Symbol_Flags__Declared;
                                symbol->location          = cursor->current.location;
                                symbol->fragment          = section->fragment_list.last;
                                symbol->elf.value         = section->fragment_list.last->size_fixed;
                                symbol->elf.section_index = section->index;

                                token_next(cursor, diagnostics, arena);
                                token_next(cursor, diagnostics, arena);
                        }

                        B32 instruction_expected = !label_found && !directive_kind;
                        if (instruction_expected)
                        {
                                instruction_hash = FNV_hash_U32(identifier);
                        }
                } break;
                default:
                {
                        // Sort of catch-all
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Line_Invalid];
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        error = 1;
                } break;
                }

                if (instruction_hash)
                {
                        U16               relocation  =  0;
                        RISCV_Instruction instruction = {0};

                        U32 expression_index_parsed = 0;
                        RISCV_Instruction__parse
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                instruction_hash,
                                &relocation,
                                &instruction,
                                &expression_index_parsed
                        );

                        if (instruction.opcode->info & INSN_MACRO)
                        {
                                Expression_Node *expression = xar_get_m(expressions, expression_index_parsed);
                                RISCV_instruction_pseudo_append
                                (
                                        section,
                                        fixups,
                                        &instruction,
                                        expression,
                                        relocation
                                );
                        }
                        else
                        {
                                RISCV_Instruction__append
                                (
                                        section,
                                        fixups,
                                        &instruction,
                                        expression_index_parsed,
                                        relocation
                                );
                        }
                }

                U8   data_directive_size = 0;
                S64  fill_size           = 0;
                B32  fill_size_set       = 0;
                S64  fill_pattern        = 0;
                B32  fill_pattern_set    = 0;

                switch (directive_kind)
                {
                case Directive_Kind__None: {} break;

                case Directive_Kind__Word_Double: { data_directive_size += 4; } // fallthrough
                case Directive_Kind__Word:        { data_directive_size += 2; } // fallthrough
                case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
                case Directive_Kind__Byte:
                {
                        data_directive_size += 1;

                        // Format: .byte|half|word|dword <expr_1> , ..., <expr_n>.
                        //
                        // Advance to reach the first expression token.
                        token_next(cursor, diagnostics, arena);
                        for (;;)
                        {
                                Expression_Node *expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                // We explicitly convert it to an unsigned value since this is how it's treated as.
                                //
                                // TODO: not very clear behaviour when in case of signed overflow.
                                S64 result = expression_evaluate(expressions, expression->index);
                                U64 result_unsigned = (U64)result;

                                U8 *data = Fragment_List__fixed(&section->fragment_list, section->arena, cursor->current.location, data_directive_size);
                                if (expression->evaluation != Expression_Kind__Constant)
                                {
                                        U32 encoding_offset = section->fragment_list.last->size_fixed - data_directive_size;

                                        Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
                                        fixup->expression_index = expression->index;
                                        fixup->fragment         = section->fragment_list.last;
                                        fixup->encoding_offset  = encoding_offset;
                                        fixup->size             = data_directive_size;

                                        SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
                                }
                                else
                                {
                                        U8 bit_size = data_directive_size * 8;
                                        B32 fits = bit_size == 64 ? 1 : (result_unsigned < (1 << (data_directive_size * 8)));
                                        memory_copy(data, (U8 *)&result_unsigned, data_directive_size);

                                        if (!fits)
                                        {
                                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                diagnostic->kind      = Diagnostic_Kind__Warning;
                                                diagnostic->location  = expression->location_range.v[0];
                                                diagnostic->message   = String8__literal("value too large, truncated");
                                                diagnostic->ranges[0] = expression->location_range;
                                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                }

                                B32 break_should_directive =  cursor->source_index >= cursor->source->count
                                                           || cursor->current.kind == Token_Kind__Newline;
                                if (break_should_directive)
                                {
                                        break;
                                }

                                if (cursor->current.kind == Token_Kind__Comma)
                                {
                                        token_next(cursor, diagnostics, arena);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        }
                } break;
                case Directive_Kind__String: {} // fallthrough
                case Directive_Kind__Asciz:  { null_terminated_string = 1; } // fallthrough
                case Directive_Kind__Ascii:
                {
                        token_next(cursor, diagnostics, arena);
                        if (cursor->current.kind != Token_Kind__String)
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->location = cursor->current.location;
                                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        // Can be of the form `"\nhello\n", so with quotes and optional escaped characters.
                        String8 text = Token_Cursor__text(cursor);
                        text = String8__skip(text, 1);
                        text = String8__chop(text, 1);
                        U32 size_escaped = String8__escaped_size(text) + !!null_terminated_string;

                        U8 *data = Fragment_List__fixed(&section->fragment_list, section->arena, cursor->current.location, size_escaped);
                        bytes_escaped_fill(text, data, size_escaped);

                        token_next(cursor, diagnostics, arena);
                } break;
                case Directive_Kind__Section:
                {
                        // Syntax: `.section name [, "flags"[, @type[, argument...]]]`
                        token_next(cursor, diagnostics, arena);
                        String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
                        Section *section_new = Sections_Table__get_or_default(sections_table, name, cursor->current.location);

                        token_next(cursor, diagnostics, arena);
                        if (cursor->current.kind == Token_Kind__Comma)
                        {
                                // Read flags.
                                token_next(cursor, diagnostics, arena);
                                if (cursor->current.kind != Token_Kind__String)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                String8 text    = Token_Cursor__text(cursor);
                                String8 content = String8__skip_chop(text);
                                ELF_Section_Header_Flags flags = ELF_Section_Header_Flags__parse(content);

                                if (flags == ELF_Section_Header_Flags__Invalid)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind       = Diagnostic_Kind__Error;
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("invalid section flags, expected: " ELF_Section_Header_Flags__cstring);
                                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                // TODO: are they ORed? Or overwritten?
                                section_new->flags = flags;
                                token_next(cursor, diagnostics, arena);
                        }

                        if (cursor->current.kind == Token_Kind__Comma)
                        {
                                // Parse type.
                                token_next(cursor, diagnostics, arena);
                                if (cursor->current.kind != Token_Kind__At)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("invalid section type syntax, expected @type");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                token_next(cursor, diagnostics, arena);
                                String8 text    = Token_Cursor__text(cursor);
                                String8 content = String8__skip_chop(text);
                                ELF_Section_Header_Type type = ELF_Section_Header_Type__from_String8(content);
                                if (type == ELF_Section_Header_Type__Invalid)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind       = Diagnostic_Kind__Error;
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("invalid section type");
                                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                section_new->type = type;
                                token_next(cursor, diagnostics, arena);
                        }


                        section = section_new;
                } break;
                case Directive_Kind__Local:
                {
                        binding_set(arena, cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Local);
                } break;
                case Directive_Kind__Weak:
                {
                        binding_set(arena, cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Weak);
                } break;
                case Directive_Kind__Globl: {} // fallthrough
                case Directive_Kind__Global:
                {
                        binding_set(arena, cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Global);
                } break;
                // TODO: implement .eqv or .equiv which are more picky about re-definitions and forward references.
                // Lastly, support for `<identifier> = <expr>` could be added by jumping here.
                case Directive_Kind__Set: {} // fallthrough
                case Directive_Kind__Equality:
                {
                        S32 mode = 0;
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                sections_table,
                                mode
                        );
                } break;
                case Directive_Kind__Equiv:
                {
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                sections_table,
                                Set_Mode__Strict
                        );
                } break;
                case Directive_Kind__Eqv:
                {
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                sections_table,
                                Set_Mode__Strict_Forward
                        );
                } break;
                case Directive_Kind__Zero:
                {
                        // Equavalent to .fill repeat, 1, 0
                        fill_pattern     = 0;
                        fill_pattern_set = 1;
                } // fallthrough
                case Directive_Kind__Space:
                {
                        // Equavalent to .fill repeat, 1, value
                } // fallthrough
                case Directive_Kind__Skip:
                {
                        // Equavalent to .fill repeat, 1, value
                        fill_size     = 1;
                        fill_size_set = 1;
                } // fallthrough
                case Directive_Kind__Fill:
                {
                        // .fill repeat [, size [, value ]]. See GNU as `s_fill` in `read.c`.
                        token_next(cursor, diagnostics, arena);
                        U64 location_begin = cursor->current.location;

                        Expression_Node *repeat_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);

                        if (cursor->current.kind == Token_Kind__Comma && !fill_size_set)
                        {
                                // Read size
                                token_next(cursor, diagnostics, arena);
                                Expression_Node *size_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                fill_size = expression_evaluate(expressions, size_expression->index);
                                if (size_expression->evaluation != Expression_Kind__Constant)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("constant expression expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                if (fill_size <= 0)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind      = Diagnostic_Kind__Warning;
                                        diagnostic->location  = size_expression->location_range.v[0];
                                        diagnostic->ranges[0] = size_expression->location_range;
                                        // TODO: nicer diagnostic with vsnprintf support in String8
                                        diagnostic->message   = String8__literal("non-positive fill size, ensuring it is zero");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        fill_size = 0;
                                }
                                if (fill_size > 8)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind     = Diagnostic_Kind__Warning;
                                        diagnostic->location  = size_expression->location_range.v[0];
                                        diagnostic->ranges[0] = size_expression->location_range;
                                        diagnostic->message  = String8__literal("capping fill size to 8 bytes");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        fill_size = 8;
                                }
                        }

                        if (cursor->current.kind == Token_Kind__Comma && !fill_pattern_set)
                        {
                                // Read value
                                token_next(cursor, diagnostics, arena);
                                Expression_Node *value_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                fill_pattern = expression_evaluate(expressions, value_expression->index);
                                if (value_expression->evaluation != Expression_Kind__Constant)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("constant expression expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        }
                        Fragment_List__fill(&section->fragment_list, section->arena, location_begin, repeat_expression->index, fill_pattern, fill_size);
                } break;
                case Directive_Kind__Align:
                {
                        // .align <size> [, <pattern> [, <max_bytes>]]
                        //
                        // TODO: support omitting some values, e.g. .align 2, , 8
                        //
                        // .align is implementation-defined, in this case we interpret the next expression as a power of
                        // two. See also .p2align.
                        // For this expression, note that a label difference is allowed but there should be no expansion
                        // between them. Probably a good way to check is making sure both are defined within the same
                        // fragment
                        U32 location_begin = cursor->current.location;
                        U8  pattern   = 0;
                        U8  bytes_max = 0;

                        token_next(cursor, diagnostics, arena);
                        Expression_Node *alignment_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                        expression_evaluate(expressions, alignment_expression->index);

                        if (alignment_expression->evaluation != Expression_Kind__Constant)
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->message   = String8__literal("constant expression expected");
                                diagnostic->location  = alignment_expression->location_range.v[0];
                                diagnostic->ranges[0] = alignment_expression->location_range;
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        if (cursor->current.kind == Token_Kind__Comma)
                        {
                                // Read pattern
                                token_next(cursor, diagnostics, arena);

                                Expression_Node *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);

                                S64 pattern_evaluation = expression_evaluate(expressions, pattern_expression->index);
                                if (pattern_expression->evaluation != Expression_Kind__Constant)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message  = String8__literal("constant expression expected");
                                        diagnostic->location  = pattern_expression->location_range.v[0];
                                        diagnostic->ranges[0] = pattern_expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                // TODO: check if between 0 and 255 instead?
                                pattern = (U8)pattern_evaluation;
                                if ((S64)pattern != pattern_evaluation)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message  = String8__literal("expression result isn't a unsigned 8 bit integer");
                                        diagnostic->location  = pattern_expression->location_range.v[0];
                                        diagnostic->ranges[0] = pattern_expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        }

                        if (cursor->current.kind == Token_Kind__Comma)
                        {
                                // Read bytes_max
                                token_next(cursor, diagnostics, arena);
                                Expression_Node *bytes_max_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                S64 bytes_max_evaluation = expression_evaluate(expressions, bytes_max_expression->index);
                                if (bytes_max_expression->evaluation != Expression_Kind__Constant)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message  = String8__literal("constant expression expected");
                                        diagnostic->location  = bytes_max_expression->location_range.v[0];
                                        diagnostic->ranges[0] = bytes_max_expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                bytes_max = (U8)bytes_max_evaluation;

                                if (bytes_max_evaluation <= 0)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind     = Diagnostic_Kind__Warning;
                                        diagnostic->message  = String8__literal("non-positive max bytes size, ensuring it is zero");
                                        diagnostic->location  = bytes_max_expression->location_range.v[0];
                                        diagnostic->ranges[0] = bytes_max_expression->location_range;
                                        // TODO: nicer diagnostic with vsnprintf support in String8
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        bytes_max = 0;
                                }
                                // NOTE: I don't know what should be an upper limit but there should be one probably.
                                // GNU as allows you to pass zero to NOT provide one which I think can be risky.
                                if (bytes_max_evaluation > 64)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind     = Diagnostic_Kind__Warning;
                                        diagnostic->message  = String8__literal("capping fill size to 64 bytes");
                                        diagnostic->location  = bytes_max_expression->location_range.v[0];
                                        diagnostic->ranges[0] = bytes_max_expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        bytes_max = 8;
                                }
                        }

                        Fragment_List__align(&section->fragment_list, section->arena, location_begin, alignment_expression->index, pattern, bytes_max);
                } break;
                default: {} break;
                }

                // Find end of line junk
                U32 junk_location_begin = cursor->current.location;
                U32 junk_location_end   = 0;
                for (;;)
                {
                        Token_Kind kind = cursor->current.kind;
                        B32 break_should = empty_line
                                || kind == Token_Kind__None
                                || kind == Token_Kind__Newline
                                || kind == Token_Kind__Semicolon;
                        if (break_should)
                        {
                                break;
                        }
                        junk_location_end = cursor->current.location + cursor->current.size;
                        token_next(cursor, diagnostics, arena);
                }

                if (junk_location_end)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location   = junk_location_begin;
                        diagnostic->message    = String8__literal("junk found at the end of line");
                        diagnostic->ranges[0]  = (Range1_U32){{ junk_location_begin, junk_location_end }};
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
        }


        return;
}
