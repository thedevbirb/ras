// Symbol trie utilities.

#include "core_symbol.h"
#include "core_expression.h"

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

internal Symbols_Trie *
symbols_trie_get_or_default(Arena *arena, Symbols_Trie **root, U64 hash, String8 name)
{
        B32 initialized = 0;
        B32 match = 0;

        Symbols_Trie **trie_current = root;
        U64 hash_shifted = hash;
        for (;;)
        {
                if (*trie_current == 0)
                {
                        Symbols_Trie *trie_new = Arena__push_struct_m(arena, Symbols_Trie);
                        String8 name_duplicated = String8__duplicate_null_terminated(arena, name);

                        trie_new->name        = name_duplicated;
                        trie_new->symbol.name = &trie_new->name;
                        memory_zero_array(trie_new->children);

                        *trie_current = trie_new;
                        initialized = 1;
                }

                match = !initialized
                         && String8__match_exact((*trie_current)->name, name)
                         // Get the latest definition of the symbol.
                         && !((*trie_current)->symbol.flags & Symbol_Flags__Redefined);

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

// Adds the provided `Symbols_Trie`, by marking an definition as `Redefined` if it exist, without dropping it.
internal void
symbols_trie_add(Symbols_Trie **root, Symbols_Trie *trie, U64 hash)
{
        B32 initialized = 0;

        Symbols_Trie **trie_current = root;
        U64 hash_shifted = hash;
        for (;;)
        {
                if (*trie_current == 0)
                {
                        *trie_current = trie;
                        initialized = 1;
                }

                if (!initialized && String8__match_exact((*trie_current)->name, trie->name))
                {
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

        return;
}

// Label numeric utilities

internal String8
label_numeric_string(Arena *arena, Label_Numeric label)
{
        String8 result = String8__format(arena, "%s%u\x02%u", INTERNAL_SYMBOL_PREFIX, label.number, label.instances);
        return result;
}

// Symbols Table API

internal Symbol_Ref *
Symbols_Table__create(Symbols_Table *symbols_table, String8 name)
{
        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *trie              = Arena__push_struct_m(symbols_table->arena, Symbols_Trie);
                      trie->name        = String8__duplicate_null_terminated(symbols_table->arena, name);
                      trie->symbol.name = &trie->name;
        symbols_trie_add(&symbols_table->root, trie, hash);
        symbols_table->count += 1;
        return &trie->symbol;
}

internal Symbol_Ref *
Symbols_Table__clone(Symbols_Table *symbols_table, Symbol_Ref *symbol)
{
        Symbol_Ref *clone = Symbols_Table__create(symbols_table, *symbol->name);
                   *clone = *symbol;
        if (clone->binding == ELF_Symbol_Binding__Local)
        {
                DLL_push_back_m(symbols_table->local_first, symbols_table->local_last, clone);
        }
        else
        {
                DLL_push_back_m(symbols_table->global_first, symbols_table->global_last, clone);
        }
        return clone;
}

internal Symbol_Ref *
Symbols_Table__get(Symbols_Table *symbols_table, String8 name)
{

        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node = symbols_trie_get(symbols_table->root, hash, name);

        Symbol_Ref *result = node ? &node->symbol : 0;
        return result;
}

// Get or create a default symbol given its name, attaching it to the undefined section if it doesn't exist.
internal Symbol_Ref *
Symbols_Table__get_or_default(Symbols_Table *symbols_table, String8 name)
{
        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node   = symbols_trie_get_or_default(symbols_table->arena, &symbols_table->root, hash, name);
        Symbol_Ref   *symbol = &node->symbol;

        if (!symbol->section)
        {
                symbol->section  = &Section__undefined;
                symbol->fragment = Section__undefined.fragments.first;
                DLL_push_back_m(symbols_table->local_first, symbols_table->local_last, symbol);
                symbols_table->count += 1;
        }

        return symbol;
}

internal Symbol_Numeric
Symbols_Table__get_or_default_numeric(Symbols_Table *symbols_table, U32 number, B32 forward)
{
        Label_Numeric *current = symbols_table->label_numeric_first;
        Label_Numeric *match   = 0;

        Symbol_Numeric result = {0};

        for (;;)
        {
                B32 break_should = match || current == 0;
                if (break_should)
                {
                        break;
                }
                match = current->number == number ? current : 0;
                current = current->next;
        }


        if (!match)
        {
                match         = Arena__push_struct_m(symbols_table->arena, Label_Numeric);
                match->number = number;
                SLL_queue_push_m(symbols_table->label_numeric_first, symbols_table->label_numeric_last, match);
        }
        result.label         =  match;
        Label_Numeric target = *match;

        // TODO(medium): I don't like to use a TLS scratch here, but otherwise everytime we allocate a name
        if (forward)
        {
                target.instances += 1;
        }

        Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);
        String8 name = label_numeric_string(scratch.arena, target);
        result.symbol = Symbols_Table__get_or_default(symbols_table, name);
        Arena__scratch_end_m(scratch);

        return result;
}

// Create a section associated to the provided symbol.
//
// Special sections have already their attributes set in, according to https://gabi.xinuos.com/v42/elf/03-sheader.html#special-sections
internal void
Symbols_Table__create_section(Symbols_Table *symbols_table, Symbol_Ref *symbol)
{
        Section   *section    = Arena__push_struct_m(symbols_table->arena, Section);
        // TODO(low): configurable
        Arena     *arena      = Arena__allocate_m();
        Fragments  fragments  = { .arena = arena, .first = &Fragment__nil, .last = &Fragment__nil };

        assert_always_m(symbol->name);
        symbol->section = section;
        symbol->type    = STT_SECTION;

        section->symbol    = symbol;
        section->fragments = fragments;

        Section_Descriptor const *lookup = Section_Descriptor__lookup(*symbol->name);
        section->special   = lookup != 0;
        section->elf.type  = lookup ? lookup->type  : ELF_Section_Header_Type__default;
        section->elf.flags = lookup ? lookup->flags : ELF_Section_Header_Flags__default;

        symbols_table->sections_count += 1;
}

internal Symbol_Ref *
Symbols_Table__create_section_riscv_attributes(Symbols_Table *symbols_table)
{
        // TODO(low): hardcoded at the moment, will be configurable later.
        U8 data[] =
        {
                // format-version 'A'
                'A',
                // subsection length = 25
                0x19, 0x00, 0x00, 0x00,
                'r', 'i', 's', 'c', 'v', 0x00,
                // Tag_File
                0x01,
                // file_tag_data_length = 15
                0x0F, 0x00, 0x00, 0x00,
                // Tag_RISCV_arch = 5
                0x05,
                // "rv64i2p1\0"
                'r', 'v', '6', '4', 'i', '2', 'p', '1', 0x00,
        };

        String8 name = String8__literal(".riscv.attributes");
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);
        Symbols_Table__create_section(symbols_table, symbol);
        symbol->section->elf.alignment = 1;

        U32 location = 0;
        U8 *destination = Fragments__push(&symbol->section->fragments, location, sizeof(data));
        memory_copy(destination, data, sizeof(data));

        return symbol;
}

// Ensures the undefined symbol and sections are added.
internal void
Symbols_Table__ensure_undefined_present(Symbols_Table *symbols_table)
{
        if (symbols_table->local_first != &Symbol_Ref__undefined)
        {
                DLL_push_front_m(symbols_table->local_first, symbols_table->local_last, &Symbol_Ref__undefined);
                symbols_table->count += 1;
        }

        if (symbols_table->section_first != &Section__undefined)
        {
                DLL_push_front_m(symbols_table->section_first, symbols_table->section_last, &Section__undefined);
                symbols_table->sections_count += 1;
        }

        return;
}

internal Symbols_Trie *
Symbols_Table__dot(Symbols_Table *symbols_table)
{

        Symbols_Trie *result = symbols_trie_get_or_default(symbols_table->arena, &symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_string);
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

// Whether a symbol should be kept or not in the final symbols table
internal B32
Symbol_Ref__keep(Symbol_Ref *symbol_ref)
{
        B32 keep = !(symbol_ref->flags & Symbol_Flags__Skip)
                &&
                (
                           symbol_ref->type == STT_SECTION
                        || symbol_ref->flags & Symbol_Flags__Relocation
                );

        return keep;
}

internal Label_Numeric *
Symbols_Table__label_numeric_get_or_default(Symbols_Table *symbols_table, U32 number)
{

        B32 found = 0;
        Label_Numeric *first   = symbols_table->label_numeric_first;
        Label_Numeric *last    = symbols_table->label_numeric_last;
        Label_Numeric *current = first;
        Label_Numeric *result  = 0;

        for (;;)
        {
                B32 break_should = found || current == 0;
                if (break_should)
                {
                        break;
                }
                found = current->number == number;
                current = current->next;
        }

        if (!found)
        {
                result = Arena__push_struct_m(symbols_table->arena, Label_Numeric);
                result->number = number;
                SLL_queue_push_m(first, last, result);
        }
        return result;
}

internal B32
Symbol_Ref__internal_is(Symbol_Ref *symbol)
{
        String8 target        = String8__new((U8 *)INTERNAL_SYMBOL_PREFIX, sizeof(INTERNAL_SYMBOL_PREFIX));
        String8 substring     = String8__substring(*symbol->name, sizeof(INTERNAL_SYMBOL_PREFIX));
        B32 internal_name_has = String8__match_exact(target, substring);

        B32 result = symbol->expression == 0 && internal_name_has;
        return result;
}

internal Symbol_Ref *
Symbols_Table__create_internal(Symbols_Table *symbols_table, Section *section)
{
        String8 name = String8__literal(FAKE_LABEL_NAME);
        Symbol_Ref *result = Symbols_Table__create(symbols_table, name);
        DLL_push_back_m(symbols_table->local_first, symbols_table->local_last, result);
        Symbol_Ref__update_section(result, section);
        return result;
}

// Kinda based on GNU `as` `resolve_symbol_value`, although with different assumptions.
//
// 1. Labels don't have an expression. Their value can be read straight into `Symbol_Ref.value`.
// 2. `Symbol_Flags__Finalized` means the simplification pass reached an end, and the value can be read from
//    `Symbol_Ref.value`. Undefined symbols and similar should have value zero.
//
// NOTE that this will be called on every symbol during the finalization process.
internal S64
Symbol_Ref__resolve(Symbol_Ref *symbol, Diagnostics *diagnostics, Resolve_Level level)
{
        assert_always_m(level < Resolve_Level__Finalize || diagnostics && "finalization requires diagnostics");
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

        // Resolution works by interleaving symbol frames and expression frames. That is, in a frame we are resolving
        // the value of a symbol. If a symbol has an expression associated to it, then we create a frame to evaluate
        // that expression.
        //
        // Every time a codepath finishes to process a frame, it is popped. Once we have no more frames, we're done.
        typedef struct Frame Frame;
        struct Frame
        {
                Frame      *next;

                Symbol_Ref *symbol;
                Expression *expression;

                Frame_State state;
        };

        Frame *frame = Arena__push_struct_m(scratch.arena, Frame);
        frame->symbol = symbol;

        // Should be updated before popping every frame. The result before popping the last frame is what we need to
        // return.
        S64 result = 0;

        B32 traverse = level >= Resolve_Level__Traverse;
        B32 finalize = level >= Resolve_Level__Finalize;

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
                                result += frame->symbol->fragment->object_file_offset;
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
                                frame->state         |= Frame_State__Evaluated;
                                // We're evaluating a symbol expression in a dedicated frame, which will NOT be marked
                                // as `Frame_State__Evaluated` yet, for the following reason: if this inner
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
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->message    = String8__literal("recursive symbolic expression found");
                                        diagnostic->location   = symbol->location;
                                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + symbol->name->count }};
                                        }
                                        SLL_stack_pop_m(frame);
                                }
                        }

                        U16 index_inner = 0;
                        for (;;)
                        {
                                // NOTE: An `Expression_Kind__Symbol` should be also checked for its
                                // `Expression.integer_value` due to folding of symbols and constant required by
                                // correctly expressing canonical relocations of the format `<symbol> + <addend>`.

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

                                        // This should always be set.
                                        node->evaluation = node->kind;

                                        assert_always_m(left->evaluation);
                                        assert_always_m(right->evaluation);

                                        if (left->evaluation == Expression_Kind__Constant && right->evaluation == Expression_Kind__Constant)
                                        {
                                                result = operation_evaluate(node->kind, left->integer_value, right->integer_value);
                                                node->integer_value = result;
                                                node->evaluation    = Expression_Kind__Constant;
                                        }
                                        else if (left->evaluation == Expression_Kind__Symbol && right->evaluation == Expression_Kind__Symbol)
                                        {
                                                // A lot of checks are needed to understand whether an operation between
                                                // symbols can be performed.
                                                B32 same_fragment = left->symbol->fragment == right->symbol->fragment;

                                                B32 left_relocation_needed  = left->symbol->section  == &Section__undefined || left->symbol->section  == &Section__common;
                                                B32 right_relocation_needed = right->symbol->section == &Section__undefined || right->symbol->section == &Section__common;
                                                B32 relocation_needed       = left_relocation_needed || right_relocation_needed;
                                                // NOTE: label difference shouldn't be erased when across _code_ fragments,
                                                // even after finalization. The linker might further shrink some
                                                // instructions (e.g. a `call` might become a 1-instruction jump).
                                                B32 code_section_present    = left->symbol->section->elf.flags  & ELF_Section_Header_Flags__EXECINSTR
                                                                           || right->symbol->section->elf.flags & ELF_Section_Header_Flags__EXECINSTR;
                                                B32 same_section_no_relocation_needed  = (left->symbol->section  == right->symbol->section)
                                                                                          && !relocation_needed;
                                                B32 subtract_is   = node->kind == Expression_Kind__Subtract;
                                                B32 equality_is   = Expression_Kind__equality_is(node->kind);
                                                B32 comparison_is = Expression_Kind__comparison_is(node->kind);

                                                B32 valid = (equality_is && !relocation_needed)
                                                         || ((subtract_is || comparison_is) && same_section_no_relocation_needed);
                                                // NOTE: constant folding can always happen within the same fragment.
                                                // However, since finalization is assumed to happen only after
                                                // relaxation, which fixes the addresses of fragments, it is safe to
                                                // perform such operations inter-fragments. Moreover, in previous frames
                                                // we've already resolved their values.
                                                B32 constant_folding_allowed = valid && (same_fragment || (same_section_no_relocation_needed && !code_section_present && finalize));

                                                if (valid)
                                                {
                                                        // Both symbols might have some offsets to take into account.
                                                        S64 left_value  = left->symbol->value  + left->integer_value;
                                                        S64 right_value = right->symbol->value + right->integer_value;
                                                        result = operation_evaluate(node->kind, left_value, right_value);
                                                }

                                                if (constant_folding_allowed)
                                                {
                                                        node->evaluation     = Expression_Kind__Constant;
                                                        node->integer_value  = result;
                                                        node->symbol         = 0;
                                                }

                                                if (finalize && !valid)
                                                {
                                                        // TODO(medium): needs printf with section info style.
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                        diagnostic->message    = String8__literal("unsupported binary operator on this expression");
                                                        diagnostic->location   = node->location;
                                                        diagnostic->ranges[0]  = node->location_range;
                                                }
                                        }
                                        else
                                        {
                                                // We _might_ have a `(<symbol> + <offset>) + <constant>`. In such case,
                                                // we can fold it.
                                                Symbol_Ref *symbol_inner = 0;
                                                S64 integer_value  = 0;

                                                if (left->evaluation == Expression_Kind__Symbol && right->evaluation == Expression_Kind__Constant)
                                                {
                                                        symbol_inner  = left->symbol;
                                                        integer_value = operation_evaluate(node->kind, left->integer_value, right->integer_value);
                                                }
                                                else if (right->evaluation == Expression_Kind__Symbol && left->evaluation == Expression_Kind__Constant)
                                                {
                                                        symbol_inner  = right->symbol;
                                                        integer_value = operation_evaluate(node->kind, right->integer_value, left->integer_value);
                                                }

                                                if (symbol_inner)
                                                {
                                                        // Folding logic
                                                        node->symbol        = symbol_inner;
                                                        node->integer_value = integer_value;
                                                        node->evaluation    = Expression_Kind__Symbol;
                                                }

                                                if (finalize && (node->kind != Expression_Kind__Subtract && node->kind != Expression_Kind__Add))
                                                {
                                                        // We have to nitpick with the possible operations.
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                        diagnostic->message    = String8__literal("unsupported binary operator on non-constant symbols");
                                                        diagnostic->location   = node->location;
                                                        diagnostic->ranges[0]  = node->location_range;
                                                }
                                        }
                                        SLL_stack_pop_m(frame);
                                }
                                else if (node->right)
                                {
                                        Expression *right = node->right;
                                        assert_always_m(Expression_Kind__unary_is(node->kind) && "parsing internal error");

                                        if (right->evaluation == Expression_Kind__Constant)
                                        {
                                                result = unary_evaluate(node->kind, right->integer_value);
                                                node->integer_value = result;
                                                node->evaluation    = Expression_Kind__Constant;
                                                node->symbol        = 0;
                                        }
                                        else if (finalize && node->kind != Expression_Kind__Logical_Not)
                                        {
                                                // TODO(low): report symbol sections with format?
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
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

                                        B32 absolute_is =  symbol_inner->section == &Section__absolute
                                                       || (symbol_inner->expression && symbol_inner->expression->evaluation == Expression_Kind__Constant);
                                        if (absolute_is)
                                        {
                                                symbol_inner->section = &Section__absolute;
                                                node->evaluation    = Expression_Kind__Constant;
                                                node->integer_value = symbol_inner->value;

                                                // TODO(medium): is this correct here?
                                                if (finalize)
                                                {
                                                        symbol_inner->flags |= Symbol_Flags__Finalized;
                                                }

                                                result = node->integer_value;
                                                SLL_stack_pop_m(frame);
                                        }
                                        else if (frame->state & Frame_State__Symbol_Resolved)
                                        {
                                                SLL_stack_pop_m(frame);
                                        }
                                        else if (traverse)
                                        {
                                                frame->state |= Frame_State__Symbol_Resolved;
                                                Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
                                                frame_new->symbol = symbol_inner;
                                                SLL_stack_push_m(frame, frame_new);
                                        }
                                        else
                                        {
                                                SLL_stack_pop_m(frame);
                                        }
                                }
                                index_inner += 1;
                        }

                }
                else
                {
                        assert_always_m(frame->symbol && frame->symbol->expression && "internal logic bug");

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
                index += 1;
        }
        Arena__scratch_end_m(scratch);

        return result;
}
