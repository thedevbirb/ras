internal Symbol_Ref *
expression_clone_forward_reference
(
        Arena              *arena,
        Symbols_Table      *symbols_table,
        Symbol_Ref         *target
)
{
        typedef struct Frame_State Frame_State;
        struct Frame_State
        {
                Frame_State *next;
                Expression  *target;
                Expression  *clone;
                B32 cloned_right;
                B32 cloned_left;
                B32 forward_reference_parsed;
        };

        Expression *expression_clone_root = Arena__push_struct_m(arena, Expression);
        Symbol_Ref *symbol_clone = Arena__push_struct_m(arena, Symbol_Ref);
        *symbol_clone = *target;
        // Otherwise its evaluation could be skipped
        symbol_clone->section = &Section__undefined;
        symbol_clone->expression = expression_clone_root;
        symbol_clone->expression->kind = target->expression->kind;

        target->flags |= Symbol_Flags__Resolving;

        Arena_Temporary scratch = Arena__scratch_begin_m(&arena, 1);

        Frame_State *frame_state = Arena__push_struct_m(scratch.arena, Frame_State);
        frame_state->target        = target->expression;
        frame_state->clone         = expression_clone_root;

        for (;;)
        {
                if (!frame_state)
                {
                        break;
                }
                else if (frame_state->target->kind == Expression_Kind__Constant || frame_state->target->evaluation == Expression_Kind__Constant)
                {
                        frame_state->clone->kind          = Expression_Kind__Constant;
                        frame_state->clone->evaluation    = Expression_Kind__Constant;
                        frame_state->clone->integer_value = frame_state->target->integer_value;
                        SLL_stack_pop_m(frame_state);
                }
                else if (frame_state->target->right && !frame_state->cloned_right)
                {
                        Expression *clone = Arena__push_struct_m(arena, Expression);
                        frame_state->clone->right = clone;
                        frame_state->cloned_right = 1;

                        Frame_State *inner = Arena__push_struct_m(scratch.arena, Frame_State);

                        inner->clone              = clone;
                        inner->target             = frame_state->target->right;

                        SLL_stack_push_m(frame_state, inner);
                }
                else if (frame_state->target->left && !frame_state->cloned_left)
                {
                        Expression *clone = Arena__push_struct_m(arena, Expression);
                        frame_state->clone->left = clone;
                        frame_state->cloned_left = 1;

                        Frame_State *inner = Arena__push_struct_m(scratch.arena, Frame_State);

                        inner->clone              = clone;
                        inner->target             = frame_state->target->left;

                        SLL_stack_push_m(frame_state, inner);
                }
                else if (frame_state->target->right)
                {
                        SLL_stack_pop_m(frame_state);
                }
                else if (frame_state->target->left)
                {
                        SLL_stack_pop_m(frame_state);
                }
                else
                {
                        // We've reached a leaf. It should be either a symbol, or nothing.
                        assert_always_m(frame_state->target->kind == Expression_Kind__Symbol || frame_state->target->kind == Expression_Kind__None);

                        frame_state->clone->kind = frame_state->target->kind;

                        if (frame_state->target->symbol)
                        {
                                String8    *name              = frame_state->target->symbol->name;
                                Symbol_Ref *latest_definition = Symbols_Table__get_or_default(symbols_table, *name, arena);

                                B32 expression_contains_non_looping_forward_reference =
                                                         latest_definition->flags & Symbol_Flags__Forward_Reference
                                                    && !(latest_definition->flags & Symbol_Flags__Resolving)
                                                    && !(frame_state->forward_reference_parsed);

                                if (expression_contains_non_looping_forward_reference)
                                {
                                        // In such case, we have repeat the process, and we will come back here again,
                                        // knowing that work has been done here already thanks to this state variable.
                                        frame_state->forward_reference_parsed = 1;
                                        latest_definition->flags |= Symbol_Flags__Resolving;

                                        Expression *clone = Arena__push_struct_m(arena, Expression);
                                        Symbol_Ref *symbol_clone_inner = Arena__push_struct_m(arena, Symbol_Ref);
                                        *symbol_clone_inner = *frame_state->target->symbol;
                                        // Otherwise its evaluation could be skipped
                                        symbol_clone_inner->section = &Section__undefined;
                                        symbol_clone_inner->expression->evaluation = Expression_Kind__None;
                                        symbol_clone_inner->expression = clone;
                                        symbol_clone_inner->expression->kind = frame_state->target->symbol->expression->kind;

                                        frame_state->clone->symbol = symbol_clone_inner;

                                        Frame_State *inner = Arena__push_struct_m(scratch.arena, Frame_State);
                                                     inner->target = frame_state->target->symbol->expression;
                                                     inner->clone  = clone;

                                        SLL_stack_push_m(frame_state, inner);
                                }
                                else
                                {
                                        latest_definition->flags &= ~Symbol_Flags__Resolving;
                                        if (!frame_state->forward_reference_parsed)
                                        {
                                                frame_state->clone->symbol = latest_definition;
                                        }

                                        SLL_stack_pop_m(frame_state);
                                }
                        }
                        else
                        {
                                // We've nothing here
                                SLL_stack_pop_m(frame_state);
                        }
                }
        }

        target->flags &= ~Symbol_Flags__Resolving;
        Arena_Temporary__end(scratch);
        return symbol_clone;
}

internal Expression *
expression_parse_with_flags
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Symbols_Table      *symbols_table,
        Diagnostics        *diagnostics,
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
                Expression       *node;
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

        Frame *frame = Arena__push_struct_m(scratch.arena, Frame);
        frame->node  = Arena__push_struct_m(arena, Expression);

        frame->node->location = cursor->current.location;
        frame->location_begin = cursor->current.location;

        // ZII node as initial result;
        Expression *result = frame->node;

        Parenthesis_Frame *parenthesis_frame = 0;

        U16 index = 0;
        for (;;)
        {
                assert_always_m(index < U16_max && "infinite loop detected");
                // Iterate until we pop the last frame.

                // Start by reading a null-denotation. We mark we've done this process be setting
                // `frame->null_denotation_parsed = 1`. When done, we shouldn't do it again for this frame.
                //
                // Every branch advances the current token since it has its own custom logic.

                if (frame->null_denotation_parsed == 0)
                {
                        switch (cursor->current.kind)
                        {
                        case Token_Kind__Number:
                        {
                                Token   peek      = token_peek(cursor, diagnostics);
                                String8 peek_text = Source__text_at(cursor->source, peek.location, peek.size);

                                B32 identifier_is    = peek.kind == Token_Kind__Identifier;
                                B32 single_letter_is = identifier_is && (peek_text.count == 1);
                                B32 forward          = single_letter_is && peek_text.data[0] == 'f';
                                B32 backward         = single_letter_is && peek_text.data[0] == 'b';

                                if (forward || backward)
                                {
                                        // TODO(medium): what happens here in case of `Expression_Flags__Defer_Dot` or
                                        // similar?
                                        U32 number = U32_cast_safe(cursor->current.numerical_value);

                                        Symbol_Numeric symbol_numeric = Symbols_Table__get_or_default_numeric(symbols_table, number, forward, arena);
                                        if (backward && symbol_numeric.symbol->section == &Section__undefined)
                                        {
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Label_Backward_Not_Found);
                                                diagnostic->location   = cursor->current.location;
                                                diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, peek.location + peek.size }};
                                        }

                                        assert_always_m(!forward || symbol_numeric.symbol->section == &Section__undefined);

                                        frame->node->kind       = Expression_Kind__Symbol;
                                        frame->node->symbol     = symbol_numeric.symbol;
                                        // Skip the letter
                                        token_next(cursor, diagnostics);
                                }
                                else
                                {
                                        frame->node->kind          = Expression_Kind__Constant;
                                        frame->node->evaluation    = Expression_Kind__Constant;
                                        frame->node->integer_value = cursor->current.numerical_value;
                                }

                                frame->node->location         = cursor->current.location;
                                frame->null_denotation_parsed = 1;

                                token_next(cursor, diagnostics);
                        } break;

                        case Token_Kind__Identifier:
                        {
                                String8 name = Token_Cursor__text(cursor);
                                B32 dot_is = name.count == 1 && name.data[0] == '.';

                                // Check if the symbol is the special dot_is. If so, and we're parsing a forward reference
                                // (`.eqv`, via `Expression_Flags__Defer_Dot`), we won't be cloning it. Otherwise, we
                                // must take a snapshot of it.
                                Symbol_Ref *symbol = 0;
                                if (dot_is)
                                {
                                        Symbol_Ref *dot = Symbols_Table__get_or_default(symbols_table, dot_symbol_string, arena);
                                        Symbol_Ref__update_section(dot, symbols_table->section_current);
                                        if (flags & Expression_Flags__Defer_Dot)
                                        {
                                                symbol = dot;
                                        }
                                        else
                                        {
                                                Symbol_Ref *clone = Symbols_Table__create_internal(symbols_table, symbols_table->section_current, arena);
                                                String8 *clone_name_backup = clone->name;
                                                *clone = *dot;
                                                clone->name = clone_name_backup;
                                                symbol = clone;
                                        }
                                }
                                else
                                {
                                        symbol = Symbols_Table__get_or_default(symbols_table, name, arena);
                                }

                                if (symbol->flags & Symbol_Flags__Forward_Reference && !(symbol->flags & Symbol_Flags__Resolving))
                                {
                                        if (flags & Expression_Flags__Defer_Dot)
                                        {
                                                // This expression is a forward reference definition. Make just a shallow
                                                // clone of the symbol.
                                                Symbol_Ref *clone = Arena__push_struct_m(arena, Symbol_Ref);
                                                *clone = *symbol;

                                                symbol = clone;
                                        }
                                        else
                                        {
                                                // We have to perform a deep clone of the symbol, that can be evaluated
                                                // individually.
                                                Symbol_Ref *clone = expression_clone_forward_reference(arena, symbols_table, symbol);
                                                symbol = clone;
                                        }
                                }

                                frame->node->kind             = Expression_Kind__Symbol;
                                frame->node->symbol           = symbol;
                                frame->node->location         = cursor->current.location;
                                frame->null_denotation_parsed = 1;

                                token_next(cursor, diagnostics);
                        } break;

                        case Token_Kind__Percentage:
                        case Token_Kind__Minus:
                        case Token_Kind__Tilde:
                        case Token_Kind__Bang:
                        {
                                // unary_operator <expression>
                                frame->node->kind = Expression_Kind__from_Token_Kind_unary(cursor->current.kind);
                                frame->null_denotation_parsed = 1;

                                Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);

                                frame_new->node = Arena__push_struct_m(arena, Expression);
                                frame_new->node->location = cursor->current.location;
                                frame_new->binding_power_minimum = Binding_Power__Unary;
                                frame_new->is_right_side_of_next = 1;

                                token_next(cursor, diagnostics);
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

                                frame->binding_power_minimum = Binding_Power__None;

                                token_next(cursor, diagnostics);
                                continue;
                        } break;

                        default:
                        {
                                // Don't pollute codepaths to exit: mark an empty node, which is safe, and mark the error with
                                // its diagnostic.
                                frame->node = Arena__push_struct_m(arena, Expression);
                                frame->null_denotation_parsed = 1;
                                frame->node->location = cursor->current.location;

                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Expression_Null_Denotation);
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);

                                token_next(cursor, diagnostics);
                        } break;
                        }
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
                // From this, we can understand that (right) parenthesis are simply tokens with zero binding power, used to
                // conclude expressions. If we had `(4 + 3) * 5`, reading the right parenthesis after 3, where the
                // former has zero binding power, would conclude reading the expression `4 + 3`. Left parenthesis behave
                // the same, e.g. in `4 - (3 + 5)` we want to force `3` to have no binding power, so that we parse the
                // `3 + 5` subexpression correctly.

                Binding_Power next_power = Binding_Power_from_Token_Kind(cursor->current.kind);
                B32 pop = next_power <= frame->binding_power_minimum || cursor->source_index >= cursor->source->count;

                // Minor housekeeping of parenthesis.
                if (cursor->current.kind == Token_Kind__Parenthesis_Right)
                {
                        if (!parenthesis_frame)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Parenthesis_Right_Unmatched);
                                diagnostic->location   = cursor->current.location;
                        }

                        SLL_stack_pop_m(parenthesis_frame);
                        token_next(cursor, diagnostics);
                }

                if (pop)
                {
                        if (frame->is_right_side_of_next)
                        {
                                assert_always_m(frame->next);
                                frame->next->node->right = frame->node;

                                // TODO(low): early folding. In this specific point, `frame->next` contains a complete
                                // subtree with a right and optionally (in case of a binary operator) a left node. As
                                // such, we could inspect what we have here and attempt an early folding. This depends
                                // also on whether we've some form of deferred mode or not. Moreover, it would be
                                // efficient only if `Expression`s were first allocated on a scratch arena (or
                                // stack), and then allocate on a persistent arena only what's necessary, and memory
                                // copy from one to the other.
                                //
                                // In practice we should:
                                //
                                // 1. Simplify the stack-allocated node altogether, modifying the tree in place
                                // 2. At the end of the whole parsing process, copy on permanent arena until we find a
                                // leaf node.
                        }

                        if (!frame->next)
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
                        Expression *left = frame->node;

                        // Set central node.
                        frame->node = Arena__push_struct_m(arena, Expression);
                        frame->node->kind = Expression_Kind__from_Token_Kind_binary(cursor->current.kind);
                        frame->node->left = left;
                        frame->node->location   = cursor->current.location;

                        token_next(cursor, diagnostics);

                        // Prepare new frame
                        Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);

                        frame_new->node = Arena__push_struct_m(arena, Expression);
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

                index += 1;
        }

        if (parenthesis_frame)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Parenthesis_Left_Unclosed);
                diagnostic->location   = parenthesis_frame->location;
        }

        Arena_Temporary__end(scratch);
        return result;
}

// NOTE: parsing an expression right now mixes machine-dependent and independent code. It would be nice to provide a
// common ground for it if it makes sense.
internal Expression *
expression_parse
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Symbols_Table   *symbols_table,
        Diagnostics     *diagnostics
)
{
        Expression *result = expression_parse_with_flags
        (
                arena,
                cursor,
                symbols_table,
                diagnostics,
                Expression_Flags__None
        );
        return result;
}

internal void
try_parse_relocation_prefix
(
        Token_Cursor             *cursor,
        Diagnostics              *diagnostics,
        // Machine-dependent
        U16                      *relocation_out,
        Relocation_Operator_List  relocation_match_list
)
{

        if (cursor->current.kind == Token_Kind__Percentage)
        {
                *relocation_out = 0;

                // Parse relocation
                token_next(cursor, diagnostics);
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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Relocation_Operator_Invalid_Instruction);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                }

                token_next(cursor, diagnostics);
        }

        return;
}
