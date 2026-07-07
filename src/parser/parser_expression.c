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
                        Token   peek      = token_peek(cursor, diagnostics, arena);
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
        Arena                    *arena,
        Token_Cursor             *cursor,
        Expressions              *expressions,
        Symbols_Table            *symbols_table,
        Section                  *section,
        Diagnostic_List          *diagnostics,
        // Machine-dependent
        U16                      *relocation_out,
        Relocation_Operator_List  relocation_match_list
)
{

        if (cursor->current.kind == Token_Kind__Percentage)
        {
                assert_always_m(relocation_out && "relocation_out should be set");
                *relocation_out = 0;

                // Parse relocation
                token_next(cursor, diagnostics, arena);
                String8 text = Token_Cursor__text(cursor);

                U64 index = 0;
                for (;;)
                {
                        B32 break_should = *relocation_out || index >= relocation_match_list.count;
                        if (break_should)
                        {
                                break;
                        }
                        Relocation_Operator operator = relocation_match_list.data[index];
                        B32 found = String8__match_exact(operator.text, text);
                        *relocation_out = found ? operator.relocation : 0;
                        index += 1;
                }

                if (!(*relocation_out))
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                        diagnostic->message    = String8__literal("invalid relocation operator for instruction");
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }

                token_next(cursor, diagnostics, arena);
        }
        Expression_Node *result = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
        return result;
}
