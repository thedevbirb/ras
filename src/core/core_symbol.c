// Symbol trie utilities.

#include "core_symbol.h"

#include "core_expression.h"

internal Symbols_Trie *
symbols_trie_chunk_list_push(Arena *arena, Symbols_Trie_Chunk_List *chunks, U64 capacity)
{
        if (chunks->last == 0 || chunks->last->count >= chunks->last->capacity)
        {
                Symbols_Trie_Chunk *chunk_new = Arena__push_struct_m(arena, Symbols_Trie_Chunk);
                chunk_new->nodes = Arena__push_array_m(arena, Symbols_Trie, capacity);
                chunk_new->capacity = capacity;

                SLL_queue_push_m(chunks->first, chunks->last, chunk_new);
                chunks->count += 1;
        }

        Symbols_Trie_Chunk *chunk_last = chunks->last;
        Symbols_Trie *result = &chunk_last->nodes[chunk_last->count];
        chunk_last->count += 1;

        return result;
}

// TODO: check whether get, get_or_default and create can be unified in a single implementation with "modes".

internal Symbols_Trie *
symbols_trie_get(Symbols_Trie *trie, U64 hash, String8 name)
{
        Symbols_Trie *result = 0;
        Symbols_Trie *trie_current = trie;
        U64 hash_shifted = hash;
        for (;;)
        {
                B32 trie_current_zero = trie_current == 0;
                B32 found = !trie_current_zero && String8__match_exact(trie_current->name, name);
                if (found)
                {
                        result = trie_current;
                }

                B32 break_should = trie_current_zero || found;
                if (break_should)
                {
                        break;
                }

                trie_current = trie_current->children[(hash_shifted >> 62)];
                hash_shifted = hash_shifted << 2;
        }

        return result;
}

// NOTE: we need a reference to the root pointer so that in case it's null we can change it.
internal Symbols_Trie *
symbols_trie_get_or_default(Arena *arena, Symbols_Trie_Chunk_List *chunks, Symbols_Trie **root, U64 hash, String8 name)
{
        B32 initialized = 0;
        B32 match = 0;

        Symbols_Trie **trie_current = root;
        U64 hash_shifted = hash;
        for (;;)
        {
                if (*trie_current == 0)
                {
                        Symbols_Trie *trie_new = symbols_trie_chunk_list_push(arena, chunks, Symbols_Trie_Chunk__capacity_default);
                        String8 name_duplicated = String8__duplicate(arena, name);
                        trie_new->name        = name_duplicated;
                        trie_new->symbol.name = &trie_new->name;
                        memory_zero_array(trie_new->children);
                        *trie_current = trie_new;
                        initialized = 1;
                }

                if (!initialized && String8__match_exact((*trie_current)->name, name))
                {
                        match = 1;
                }

                B32 break_should = initialized || match;
                if (break_should)
                {
                        break;
                }

                trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
                hash_shifted = hash_shifted << 2;
        }

        return *trie_current;
}

// Always create a new symbol, by marking a current definition as `Redefined` if it exist, without dropping it.
internal Symbols_Trie *
symbols_trie_create(Arena *arena, Symbols_Trie_Chunk_List *chunks, Symbols_Trie **root, U64 hash, String8 name)
{
        B32 initialized = 0;

        Symbols_Trie **trie_current = root;
        U64 hash_shifted = hash;
        for (;;)
        {
                if (*trie_current == 0)
                {
                        Symbols_Trie *trie_new = symbols_trie_chunk_list_push(arena, chunks, Symbols_Trie_Chunk__capacity_default);
                        trie_new->name = name;
                        memory_zero_array(trie_new->children);
                        *trie_current = trie_new;
                        initialized = 1;
                }

                if (!initialized && String8__match_exact((*trie_current)->name, name))
                {
                        // We've found an existing definition. Ensure we don't write it in the object file.
                        (*trie_current)->symbol.flags |= Symbol_Flags__Redefined;
                }

                B32 break_should = initialized;
                if (break_should)
                {
                        break;
                }

                trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
                hash_shifted = hash_shifted << 2;
        }

        return *trie_current;
}

// Label numeric utilities

internal Label_Numeric *
label_numeric_chunk_list_push(Arena *arena, Label_Numeric_Chunk_List *chunks, U64 capacity)
{
        if (chunks->last == 0 || chunks->last->count >= chunks->last->capacity)
        {
                Label_Numeric_Chunk *chunk_new = Arena__push_struct_m(arena, Label_Numeric_Chunk);
                chunk_new->nodes = Arena__push_array_m(arena, Label_Numeric, capacity);
                chunk_new->capacity = capacity;

                SLL_queue_push_m(chunks->first, chunks->last, chunk_new);
                chunks->count += 1;
        }

        Label_Numeric_Chunk *chunk_last = chunks->last;
        Label_Numeric *result = &chunk_last->nodes[chunk_last->count];
        chunk_last->count += 1;

        return result;
}

internal Label_Numeric *
label_numeric_get_or_default(Arena *arena, Label_Numeric_Chunk_List *chunks, U64 capacity, U32 label)
{
        B32 found = 0;
        Label_Numeric_Chunk *current = chunks->first;
        Label_Numeric *result = 0;

        for (;;)
        {
                B32 break_should = found || current == 0;
                if (break_should)
                {
                        break;
                }

                U32 index = 0;
                for (;;)
                {
                        B32 break_should_inner = found || index >= current->count;
                        if (break_should_inner)
                        {
                                break;
                        }
                        result = &(current->nodes[index]);
                        found = result->number == label;

                        index += 1;
                }

                current = current->next;
        }

        if (!found)
        {
                result = label_numeric_chunk_list_push(arena, chunks, capacity);
        }

        return result;
}

internal String8
label_numeric_string(Arena *arena, Label_Numeric label)
{
        String8 result = String8__format(arena, ".L%u\x02%u", label.number, label.instances);
        return result;
}

// Symbols Table API

internal Symbols_Trie *
Symbols_Table__last(Symbols_Table *symbols_table)
{
        Symbols_Trie *result = 0;
        if (symbols_table->root)
        {
                Symbols_Trie_Chunk *chunk_last = symbols_table->chunks->last;
                // Valid because there is at least the root.
                result = &chunk_last->nodes[chunk_last->count - 1];
        }

        return result;
}

// Get or create a default symbol given its name.
internal Symbol_Ref *
Symbols_Table__get(Symbols_Table *symbols_table, String8 name)
{

        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node = symbols_trie_get(symbols_table->root, hash, name);

        Symbol_Ref *result = node ? &node->symbol : 0;
        return result;
}

// Get or create a default symbol given its name.
internal Symbol_Ref *
Symbols_Table__get_or_default(Symbols_Table *symbols_table, String8 name, Section *undefined)
{

        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node = symbols_trie_get_or_default(symbols_table->arena, symbols_table->chunks, &symbols_table->root, hash, name);
        Symbol_Ref *symbol = &node->symbol;
        if (!symbol->section)
        {
                symbol->section  = undefined;
                symbol->fragment = undefined->fragments.first;
        }

        return symbol;
}

// Return the global dot symbol trie.
internal Symbols_Trie *
Symbols_Table__dot(Symbols_Table *symbols_table)
{

        Symbols_Trie *result = symbols_trie_get(symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_string);
        return result;
}

internal void
Symbol_Ref__update_section(Symbol_Ref *symbol, Section *section)
{
        symbol->fragment      = section->fragments.last;
        symbol->value         = section->fragments.last->data_size;
        symbol->section       = section;

        return;
}

internal Symbols_Trie *
Symbols_Table__create_trie(Symbols_Table *symbols_table, String8 name)
{
        U64 hash = FNV_hash_U64(name);
        // Symbols_Trie *last = Symbols_Table__last(symbols_table);
        Symbols_Trie *result = symbols_trie_create(symbols_table->arena, symbols_table->chunks, &symbols_table->root, hash, name);
        return result;
}

internal Symbol_Ref *
Symbols_Table__create(Symbols_Table *symbols_table, String8 name)
{
        Symbols_Trie *node = Symbols_Table__create_trie(symbols_table, name);
        return &node->symbol;
}

internal Symbol_Ref *
Symbols_Table__clone(Symbols_Table *symbols_table, Symbol_Ref *symbol, String8 name)
{
        Symbols_Trie *clone = Symbols_Table__create_trie(symbols_table, name);
        clone->name   = String8__duplicate(symbols_table->arena, name);
        clone->symbol = *symbol;
        return &clone->symbol;
}

internal Label_Numeric *
Symbols_Table__label_numeric_get_or_default(Symbols_Table *symbols_table, U32 number)
{
        Label_Numeric *label = label_numeric_get_or_default(symbols_table->arena, symbols_table->chunks_label, Label_Numeric_Chunk__capacity_default, number);
        return label;
}

// Creates a new symbols table with a dedicated arena allocator and by creating the global dot symbol.
internal Symbols_Table *
Symbols_Table__new(void)
{
        Arena *arena_symbols_table = Arena__allocate_m();
        Symbols_Table *symbols_table = Arena__push_struct_m(arena_symbols_table, Symbols_Table);

        symbols_table->arena        = arena_symbols_table;
        symbols_table->chunks_label = Arena__push_struct_m(arena_symbols_table, Label_Numeric_Chunk_List);
        symbols_table->chunks       = Arena__push_struct_m(arena_symbols_table, Symbols_Trie_Chunk_List);

        // Initialization steps

        // 1. Create global dot
        String8 dot_symbol_name = String8__duplicate(symbols_table->arena, dot_symbol_string);
        symbols_trie_create(symbols_table->arena, symbols_table->chunks, &symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_name);
        // 2. Place numeric labels 0..9 at beginning of chunks.
        U8 index = 0;
        for (;;)
        {
                if (index == 10)
                {
                        break;
                }
                Label_Numeric *label = label_numeric_chunk_list_push(symbols_table->arena, symbols_table->chunks_label, Label_Numeric_Chunk__capacity_default);
                label->number = index;

                index += 1;
        }


        return symbols_table;
}

internal Symbol_Ref *
Symbols_Table__internal_label(Symbols_Table *symbols_table, Section *section)
{
        String8 name = String8__literal(FAKE_LABEL_NAME);
        Symbol_Ref *result = Symbols_Table__create(symbols_table, name);
        Symbol_Ref__update_section(result, section);
        return result;
}

// Kinda based on GNU `as` `resolve_symbol_value`, although with different assumptions.
//
// 1. Labels don't have an expression. Their value can be read straight into `Symbol_Ref.value`.
// 2. `Symbol_Flags__Finalized` means the simplification pass reached an end, and the value can be read from
//    `Symbol_Ref.value`. Undefined symbols and similar should have value zero.
//
// NOTE that this will be called on every symbol.
// TODO(low): replace `expression_evalute` with this, more general version, by wrapping an expression into a
// stack-allocated symbol, since the core evaluation logic is shared.
internal S64
Symbol_Ref__resolve(Symbol_Ref *symbol, Arena *arena, Diagnostic_List *diagnostics, Resolve_Level level)
{
        Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);

        typedef enum Frame_State
        {
                Frame_State__None                 = 0 << 0,
                Frame_State__Right_Evaluated      = 1 << 0,
                Frame_State__Left_Evaluated       = 1 << 1,
                Frame_State__Evaluated            = 1 << 2,
                Frame_State__Symbol_Resolved      = 1 << 4,
        }
        Frame_State;

        typedef struct Frame Frame;
        struct Frame
        {
                Frame      *next;

                // Contains either one of the two.
                Symbol_Ref *symbol;
                Expression *expression;

                B32 expression_depends_on_symbol;

                S64         result;
                Frame_State state;
        };

        Frame *frame = Arena__push_struct_m(scratch.arena, Frame);
        frame->symbol = symbol;
        S64 result = 0;

        B32 traverse = level >= Resolve_Level__Traverse;
        B32 finalize = level >= Resolve_Level__Finalize;

        // Notes
        //
        // Resolution works by interleaving symbol frames and expression frames. That is, in a frame we are resolving
        // the value of a symbol. If a symbol has an expression associated to it, then we create a frame to evaluate
        // that expression.
        //
        // Every time a codepath finishes to process a frame, it is popped. Once we have no more frames, we're done.

        U16 index = 0;
        for (;;)
        {
                assert_always_m(index < U16_max && "infinite loop");
                if (!frame)
                {
                        break;
                }
                else if (frame->symbol && frame->symbol->flags & Symbol_Flags__Finalized)
                {
                        // The simplest case. The symbol is already finalized.
                        result = frame->symbol->value;
                        SLL_stack_pop_m(frame);
                }
                else if (frame->symbol && !frame->symbol->expression)
                {
                        // A symbol without an expression can be either a label definition, or some undefined symbol.
                        result = frame->symbol->value;
                        if (!(frame->symbol->flags & Symbol_Flags__Finalized))
                        {
                                result += symbol->fragment->object_file_offset;
                        }

                        if (finalize)
                        {
                                frame->symbol->value = result;
                                frame->symbol->flags |= Symbol_Flags__Finalized;
                        }
                        SLL_stack_pop_m(frame);
                }
                else if (!(frame->state & Frame_State__Evaluated))
                {
                        if (frame->symbol)
                        {
                                B32 loop_detected = frame->symbol->flags & Symbol_Flags__Resolving;
                                frame->symbol->flags |= Symbol_Flags__Resolving;
                                frame->state |= Frame_State__Evaluated;
                                // We're evaluating a symbol expression in a dedicated frame, which will NOT be marked
                                // as `Frame_State__Evaluated` yet, for the following reason: if the
                                // expression does NOT contain symbols to resolve, the new frame will be popped and
                                // we're done. Otherwise, we have to create a new symbol frame. Once this latter symbol
                                // is resolved, we can go back to this expression frame, _now mark it as evaluated_
                                // because we have all information to return a value.
                                Frame *frame_expression = Arena__push_struct_m(scratch.arena, Frame);
                                frame_expression->expression = frame->symbol->expression;
                                SLL_stack_push_m(frame, frame_expression);

                                if (loop_detected)
                                {
                                        {
                                        // TODO(low): finding the previous definition here is NOT trivial.
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("recursive symbol definition found");
                                        diagnostic->location   = symbol->location;
                                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + symbol->name->count }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                        SLL_stack_pop_m(frame);
                                }
                        }

                        U16 index_inner = 0;
                        for (;;)
                        {
                                assert_always_m(index_inner < U16_max && "infinite loop");
                                if (!frame->expression)
                                {
                                        break;
                                }
                                Expression *node = frame->expression;

                                if (node->evaluation == Expression_Kind__Constant)
                                {
                                        result = node->integer_value;
                                        SLL_stack_pop_m(frame);
                                }
                                else if (node->right && !(frame->state & Frame_State__Right_Evaluated))
                                {
                                        // We have to evaluate the inner expression
                                        frame->state |= Frame_State__Right_Evaluated;
                                        Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
                                        frame_new->expression = node->right;
                                        SLL_stack_push_m(frame, frame_new);
                                }
                                else if (node->left && !(frame->state & Frame_State__Left_Evaluated))
                                {
                                        // We have to evaluate the inner expression
                                        frame->state |= Frame_State__Left_Evaluated;
                                        Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
                                        frame_new->expression = node->left;
                                        SLL_stack_push_m(frame, frame_new);
                                }
                                else if (node->right && node->left)
                                {
                                        Expression *left  = node->left;
                                        Expression *right = node->right;

                                        assert_always_m(left->evaluation);
                                        assert_always_m(right->evaluation);

                                        if (left->evaluation == Expression_Kind__Constant && right->evaluation == Expression_Kind__Constant)
                                        {
                                                S64 result_inner    = operation_evaluate(node->kind, left->integer_value, right->integer_value);
                                                node->integer_value = result_inner;
                                                node->evaluation    = Expression_Kind__Constant;
                                        }
                                        else if (left->evaluation == Expression_Kind__Symbol && right->evaluation == Expression_Kind__Symbol)
                                        {
                                                // Extra checks to ensure undefined symbols are not considered equal.
                                                B32 same_fragment = (left->symbol->fragment == right->symbol->fragment)
                                                                  && left->symbol->fragment && right->symbol->fragment;
                                                B32 no_undefined_sections = left->symbol->section->index &&  right->symbol->section->index;
                                                B32 same_section_not_undefined  = (left->symbol->section->index  == right->symbol->section->index)
                                                                                && no_undefined_sections;
                                                B32 subtract_is   = node->kind == Expression_Kind__Subtract;
                                                B32 equality_is   = Expression_Kind__equality_is(node->kind);
                                                B32 comparison_is = Expression_Kind__comparison_is(node->kind);

                                                B32 valid = (equality_is && no_undefined_sections)
                                                               || ((subtract_is || comparison_is) && same_section_not_undefined);
                                                if (valid && same_fragment)
                                                {
                                                        // Fold to constant.
                                                        node->evaluation = Expression_Kind__Constant;
                                                }

                                                if (valid)
                                                {
                                                        node->integer_value = operation_evaluate(node->kind, left->symbol->value, right->symbol->value);
                                                }
                                                else
                                                {
                                                        node->symbol         = left->symbol;
                                                        node->symbol_operand = right->symbol;
                                                }

                                                if (finalize && !valid)
                                                {
                                                        // TODO(medium): needs printf with section info style.
                                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                        diagnostic->message    = String8__literal("unsupported binary operator on this expression");
                                                        diagnostic->location   = node->location;
                                                        diagnostic->ranges[0]  = node->location_range;
                                                }
                                        }
                                        else
                                        {
                                                // We can still absorb something like `symbol_inner <operator> constant`
                                                Symbol_Ref *symbol_inner = 0;
                                                S64 integer_value  = 0;

                                                if (left->evaluation == Expression_Kind__Symbol && right->evaluation == Expression_Kind__Constant)
                                                {
                                                        symbol_inner  = left->symbol;
                                                        integer_value = right->integer_value;
                                                }
                                                else if (right->evaluation == Expression_Kind__Symbol && left->evaluation == Expression_Kind__Constant)
                                                {
                                                        symbol_inner  = right->symbol;
                                                        integer_value = left->integer_value;
                                                }

                                                if (symbol_inner)
                                                {
                                                        node->symbol  = symbol_inner;
                                                        node->integer_value = integer_value;
                                                }

                                                if (finalize && (node->kind != Expression_Kind__Subtract && node->kind != Expression_Kind__Add))
                                                {
                                                        // We have to nitpick with the possible operations.
                                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                        diagnostic->message    = String8__literal("unsupported binary operator on non-constant symbols");
                                                        diagnostic->location   = node->location;
                                                        diagnostic->ranges[0]  = node->location_range;
                                                }
                                        }
                                        result = node->integer_value;
                                        SLL_stack_pop_m(frame);
                                }
                                else if (node->right)
                                {
                                        Expression *right = node->right;
                                        assert_always_m(right->evaluation);
                                        assert_always_m(Expression_Kind__unary_is(node->kind) && "parsing internal error");

                                        if (right->evaluation == Expression_Kind__Constant)
                                        {
                                                S64 result_inner    = unary_evaluate(node->kind, right->integer_value);
                                                node->integer_value = result_inner;
                                                node->evaluation    = Expression_Kind__Constant;
                                        }
                                        else if (node->kind != Expression_Kind__Logical_Not && finalize)
                                        {
                                                // TODO(low): report symbol sections with format?
                                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                diagnostic->message    = String8__literal("unsupported unary operator on non-constant symbols");
                                                diagnostic->location   = node->location;
                                                diagnostic->ranges[0]  = node->location_range;
                                        }
                                        else
                                        {
                                                // Absorb it.
                                                node->evaluation = node->evaluation;
                                                node->symbol     = right->symbol;
                                        }

                                        result = node->integer_value;
                                        SLL_stack_pop_m(frame);
                                }
                                else
                                {
                                        // Leaf reached. Since constant are eagerly set to such evaluation at parse
                                        // time, this MUST be a symbol.
                                        assert_always_m(node->left == 0);
                                        assert_always_m(node->kind == Expression_Kind__Symbol);
                                        assert_always_m(node->symbol);
                                        Symbol_Ref *symbol_inner = node->symbol;

                                        node->evaluation = node->kind;

                                        if (symbol_inner->section->index == ELF_Section_Index__Absolute)
                                        {
                                                node->evaluation = Expression_Kind__Constant;
                                                node->integer_value = symbol_inner->value;

                                                if (finalize)
                                                {
                                                        symbol_inner->flags |= Symbol_Flags__Finalized;
                                                }

                                                result = node->integer_value;
                                                SLL_stack_pop_m(frame);
                                        }
                                        else if (frame->state & Frame_State__Symbol_Resolved)
                                        {
                                                // probably not integer_value
                                                node->integer_value = symbol_inner->expression->integer_value;
                                                SLL_stack_pop_m(frame);
                                        }
                                        else if (traverse && symbol_inner->expression)
                                        {
                                                frame->state |= Frame_State__Symbol_Resolved;
                                                Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
                                                frame_new->symbol = symbol_inner;
                                                SLL_stack_push_m(frame, frame_new);
                                        }
                                        else
                                        {
                                                result = node->integer_value;
                                                SLL_stack_pop_m(frame);
                                        }
                                }
                                index_inner += 1;
                        }

                }
                else if (frame->symbol && frame->symbol->expression)
                {
                        frame->symbol->flags &= ~Symbol_Flags__Resolving;
                        // After this loop, the inner expression has been evaluated, and subsequent symbols optionally
                        // resolved. The value of the symbol is whatever this expression evaluates to, and we can mark
                        // it as resolved.
                        if (finalize)
                        {
                                frame->symbol->value = frame->symbol->expression->integer_value;
                                frame->symbol->flags |= Symbol_Flags__Finalized;
                        }

                        result = frame->symbol->expression->integer_value;
                        SLL_stack_pop_m(frame);
                }
                else
                {
                        frame->symbol->flags &= ~Symbol_Flags__Resolving;
                        result = frame->symbol->value;
                        if (frame->symbol->flags & Symbol_Flags__Finalized)
                        {
                                result += symbol->fragment->object_file_offset;
                        }

                        if (finalize)
                        {
                                frame->symbol->value = result;
                                frame->symbol->flags |= Symbol_Flags__Finalized;
                        }
                }
                index += 1;
        }
        Arena__scratch_end_m(scratch);

        return result;
}
