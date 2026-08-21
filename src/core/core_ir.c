//-----------------------------------------------------------------------------
// @Expression
//-----------------------------------------------------------------------------
internal B32
Expression_Kind__unary_is(Expression_Kind kind)
{
        B32 result = kind == Expression_Kind__Negate
                  || kind == Expression_Kind__Bitwise_Not
                  || kind == Expression_Kind__Logical_Not;
        return result;
}

internal B32
Expression_Kind__equality_is(Expression_Kind kind)
{
        B32 result = kind == Expression_Kind__Equal
                  || kind == Expression_Kind__Not_Equal;
        return result;
}

internal B32
Expression_Kind__comparison_is(Expression_Kind kind)
{
        B32 result = kind == Expression_Kind__Not_Equal
                  || kind == Expression_Kind__Less_Than
                  || kind == Expression_Kind__Less_Equal
                  || kind == Expression_Kind__Greater_Than
                  || kind == Expression_Kind__Greater_Equal;
        return result;
}

internal Expression_Kind
Expression_Kind__from_Token_Kind_binary(Token_Kind kind)
{
        Expression_Kind result = Expression_Kind__None;

        switch (kind)
        {
        case Token_Kind__Plus:            { result = Expression_Kind__Add;           } break;
        case Token_Kind__Minus:           { result = Expression_Kind__Subtract;      } break;
        case Token_Kind__Star:            { result = Expression_Kind__Multiply;      } break;
        case Token_Kind__Slash:           { result = Expression_Kind__Divide;        } break;
        case Token_Kind__Percentage:      { result = Expression_Kind__Modulo;        } break;
        case Token_Kind__Pipe:            { result = Expression_Kind__Bitwise_Or;    } break;
        case Token_Kind__Caret:           { result = Expression_Kind__Bitwise_Xor;   } break;
        case Token_Kind__Ampersand:       { result = Expression_Kind__Bitwise_And;   } break;
        case Token_Kind__Less_2:          { result = Expression_Kind__Shift_Left;    } break;
        case Token_Kind__Greater_2:       { result = Expression_Kind__Shift_Right;   } break;
        case Token_Kind__Equal_2:         { result = Expression_Kind__Equal;         } break;
        case Token_Kind__Equal_Bang:      { result = Expression_Kind__Not_Equal;     } break;
        case Token_Kind__Less:            { result = Expression_Kind__Less_Than;     } break;
        case Token_Kind__Less_Equal:      { result = Expression_Kind__Less_Equal;    } break;
        case Token_Kind__Greater:         { result = Expression_Kind__Greater_Than;  } break;
        case Token_Kind__Greater_Equal:   { result = Expression_Kind__Greater_Equal; } break;
        case Token_Kind__Ampersand_2:     { result = Expression_Kind__Logical_And;   } break;
        case Token_Kind__Pipe_2:          { result = Expression_Kind__Logical_Or;    } break;
        default:                          {} break;
        }

        return result;
}

internal Expression_Kind
Expression_Kind__from_Token_Kind_unary(Token_Kind kind)
{
        Expression_Kind result = Expression_Kind__None;

        switch (kind)
        {
        case Token_Kind__Minus: { result = Expression_Kind__Negate;      } break;
        case Token_Kind__Tilde: { result = Expression_Kind__Bitwise_Not; } break;
        case Token_Kind__Bang:  { result = Expression_Kind__Logical_Not; } break;
        default:                {} break;
        }

        return result;
}


// TODO(low): maybe this should be done on U64 so we don't have UB. And division by zero is zero.
internal S64
operation_evaluate(Expression_Kind kind, S64 a, S64 b)
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

        default: { unreachable_m(); } break;
        }

        return result;
}

internal Binding_Power
Binding_Power_from_Token_Kind(Token_Kind kind)
{
        Binding_Power result = Binding_Power__None;

        switch (kind)
        {
        case Token_Kind__Pipe_2:         { result = Binding_Power__Logical_Or;     } break;
        case Token_Kind__Ampersand_2:    { result = Binding_Power__Logical_And;    } break;
        case Token_Kind__Pipe:           { result = Binding_Power__Bitwise_Or;     } break;
        case Token_Kind__Caret:          { result = Binding_Power__Bitwise_Xor;    } break;
        case Token_Kind__Ampersand:      { result = Binding_Power__Bitwise_And;    } break;
        case Token_Kind__Equal_2:
        case Token_Kind__Equal_Bang:     { result = Binding_Power__Equality;       } break;
        case Token_Kind__Less:
        case Token_Kind__Greater:
        case Token_Kind__Less_Equal:
        case Token_Kind__Greater_Equal:  { result = Binding_Power__Comparison;     } break;
        case Token_Kind__Less_2:
        case Token_Kind__Greater_2:      { result = Binding_Power__Shift;          } break;
        case Token_Kind__Plus:
        case Token_Kind__Minus:          { result = Binding_Power__Additive;       } break;
        case Token_Kind__Star:
        case Token_Kind__Slash:
        case Token_Kind__Percentage:     { result = Binding_Power__Multiplicative; } break;
        default:                         {} break;
        }

        return result;
}

internal S64
unary_evaluate(Expression_Kind kind, S64 a)
{
        S64 result = 0;

        switch (kind)
        {
        case Expression_Kind__Negate:        { result = a == S64_min ? a : -a; } break;
        case Expression_Kind__Logical_Not:   { result = !a; } break;
        case Expression_Kind__Bitwise_Not:   { result = ~a; } break;

        default: { unreachable_m(); } break;
        }

        return result;
}

// I think this can be dropped and we can use Expression_Kind in the evaluation for that.
typedef enum Evaluation_Frame_State
{
        Evaluation_Frame_State__None             = 0 << 0,
        Evaluation_Frame_State__Right_Evaluated  = 1 << 0,
        Evaluation_Frame_State__Left_Evaluated   = 1 << 1,
        Evaluation_Frame_State__COUNT,
}
Evaluation_Frame_State;


typedef struct Evaluation_Frame Evaluation_Frame;
struct Evaluation_Frame
{
        Expression  *node;
        Evaluation_Frame *next;
        Evaluation_Frame_State state;
};

internal S64
expression_evaluate(Expression *node_root)
{
        Symbol_Ref symbol = { .expression = node_root };
        S64 result = Symbol_Ref__resolve(&symbol, 0, Resolve_Level__None);
        return result;
}

Expression *
Expression__push_constant(Arena *arena, S64 constant)
{
        Expression *result = Arena__push_struct_m(arena, Expression);

        result->integer_value = constant;
        result->kind          = Expression_Kind__Constant;
        result->evaluation    = Expression_Kind__Constant;

        return result;
}

internal Expression *
Expression__push_symbol(Arena *arena, Symbol_Ref *symbol)
{
        Expression *result = Arena__push_struct_m(arena, Expression);

        result->symbol     = symbol;
        result->kind       = Expression_Kind__Symbol;
        result->evaluation = Expression_Kind__Symbol;
        return result;
}

// Evaluate all expressions while finalizing symbols. See `Symbol_Ref__finalize`/`Symbols_Table__finalize`.
//
// After this is called expressions cannot be reduced further, since now symbols are frozen.
internal void
Expressions__finalize(Expressions *expressions, Diagnostics *diagnostics)
{
        for each_node_m(expressions->first, expression)
        {
                Symbol_Ref symbol_expression = { .expression = expression };
                S64 result = Symbol_Ref__resolve(&symbol_expression, diagnostics, Resolve_Level__Finalize);
                expression->integer_value = result;
        }
        return;
}

internal Diagnostic *
Diagnostics__expression(Diagnostics *diagnostics, Expression *expression, String8 message)
{
        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
        diagnostic->message    = message;
        diagnostic->location   = expression->location;
        diagnostic->ranges[0]  = expression->location_range;
        return diagnostic;
}

//-----------------------------------------------------------------------------
// @Symbol
//-----------------------------------------------------------------------------

// Symbol trie utilities.

internal Symbols_Trie *
symbols_trie_get(Symbols_Trie *trie, U64 hash, String8 name)
{
        Symbols_Trie *result = 0;
        Symbols_Trie *trie_current = trie;
        U64 hash_shifted = hash;
        for (;;)
        {
                B32 trie_current_zero = trie_current == 0;
                B32 found = !trie_current_zero && String8__match_exact(trie_current->name, name)
                         // Get the latest definition of the symbol.
                         && !(trie_current->symbol.flags & Symbol_Flags__Redefined);
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
Symbols_Table__create(Symbols_Table *symbols_table, String8 name, Arena *arena)
{
        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *trie              = Arena__push_struct_m(arena, Symbols_Trie);
                      trie->name        = String8__duplicate_null_terminated(arena, name);
                      trie->symbol.name = &trie->name;
        symbols_trie_add(&symbols_table->root, trie, hash);

        SLL_queue_push_m(symbols_table->first, symbols_table->last, &trie->symbol);
        symbols_table->count += 1;

        return &trie->symbol;
}

internal Symbol_Ref *
Symbols_Table__clone(Symbols_Table *symbols_table, Symbol_Ref *symbol, Arena *arena)
{
        Symbol_Ref *clone = Symbols_Table__create(symbols_table, *symbol->name, arena);
                   *clone = *symbol;
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
Symbols_Table__get_or_default(Symbols_Table *symbols_table, String8 name, Arena *arena)
{
        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node   = symbols_trie_get_or_default(arena, &symbols_table->root, hash, name);
        Symbol_Ref   *symbol = &node->symbol;

        if (!symbol->section)
        {
                symbol->section  = &Section__undefined;
                symbol->fragment = Section__undefined.fragments.first;
                SLL_queue_push_m(symbols_table->first, symbols_table->last, symbol);
                symbols_table->count += 1;
        }

        return symbol;
}

internal Symbol_Numeric
Symbols_Table__get_or_default_numeric(Symbols_Table *symbols_table, U32 number, B32 forward, Arena *arena)
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
                match         = Arena__push_struct_m(arena, Label_Numeric);
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
        result.symbol = Symbols_Table__get_or_default(symbols_table, name, arena);
        Arena__scratch_end_m(scratch);

        return result;
}

internal void
Symbols_Table__create_section(Symbols_Table *symbols_table, Symbol_Ref *symbol, Arena *arena, Arena_Parameters arena_parameters)
{
        Section   *section    = Arena__push_struct_m(arena, Section);
        Arena *arena_fragments = Arena__allocate_m
        (
                .reserve_size = arena_parameters.reserve_size,
                .commit_size  = arena_parameters.commit_size,
                .flags        = arena_parameters.flags
        );
        Fragments  fragments  = { .arena = arena_fragments, .first = &Fragment__nil, .last = &Fragment__nil };

        assert_always_m(symbol->name);
        symbol->section  = section;
        symbol->fragment = fragments.first;
        symbol->type     = ELF_Symbol_Type__Section;

        section->symbol    = symbol;
        section->fragments = fragments;

        Section_Descriptor const *lookup = Section_Descriptor__lookup(*symbol->name);
        section->special       = lookup != 0;
        section->elf.type      = lookup ? lookup->type  : ELF_Section_Header_Type__default;
        section->elf.flags     = lookup ? lookup->flags : ELF_Section_Header_Flags__default;

        section->elf.alignment = 1;

        symbols_table->sections_count += 1;
}

internal Symbol_Ref *
Symbols_Table__create_section_riscv_attributes(Symbols_Table *symbols_table, RISCV_Attributes *attributes, Arena *arena)
{
        // According to the specification, small tags can be written as just a byte:
        // https://riscv-non-isa.github.io/riscv-elf-psabi-doc/#_attributes
        //
        // Previous hardcoded example, still helpful to read.
        // U8 data[] =
        // {
        //         // format-version 'A'
        //         'A',
        //
        //         // The mandatory "riscv" sub-section
        //         // subsection length = 25
        //         0x19, 0x00, 0x00, 0x00,
        //         'r', 'i', 's', 'c', 'v', 0x00,
        //
        //         //  sub-sub-section contents
        //
        //         // Tag File, relates to the whole object file
        //         0x01,
        //         // file_tag_data_length = 15
        //         0x0F, 0x00, 0x00, 0x00,
        //         // Tag_RISCV_arch = 5
        //         0x05,
        //         // "rv64i2p1\0"
        //         'r', 'v', '6', '4', 'i', '2', 'p', '1', 0x00,
        // };
        U8  tag_size = 1;
        U32 sub_sub_section_file_data_size = tag_size + (attributes->architecture.count + 1)
                                           + (attributes->stack_alignment  ? tag_size + ULEB128__encoded_size(attributes->stack_alignment)  : 0)
                                           + (attributes->unaligned_access ? tag_size + ULEB128__encoded_size(attributes->unaligned_access) : 0);

        U32 sub_sub_section_file_size = tag_size + sizeof(U32) + sub_sub_section_file_data_size;

        const char riscv_cstring[] = "riscv";
        U32 sub_section_riscv_size = sizeof(U32)
                                   + sizeof(riscv_cstring)
                                   + sub_sub_section_file_size;

        U8 format_version = 'A';
        U32 total_size = sizeof(format_version) + sub_section_riscv_size;

        String8 name = String8__literal(".riscv.attributes");
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name, arena);
        Symbols_Table__create_section(symbols_table, symbol, arena, Arena_Parameters__default);
        symbol->section->elf.alignment = 1;

        U32 location = 0;
        U8 *data = Fragments__push(&symbol->section->fragments, location, total_size);

        String8 data_cursor = String8__new(data, total_size);

        String8__serial_write_m(&data_cursor, &format_version);
        String8__serial_write_m(&data_cursor, &sub_section_riscv_size);
        String8__serial_write(&data_cursor, (U8 *)riscv_cstring, sizeof(riscv_cstring));

        // Sub-sub section contents
        U8 tag_file = 1;
        String8__serial_write_m(&data_cursor, &tag_file);
        String8__serial_write_m(&data_cursor, &sub_sub_section_file_size);

        if (attributes->stack_alignment)
        {
                U8 tag_stack_alignment = RISCV_Tag__Stack_Alignment;
                U8 encoding[U32_ULEB128_encoding_size_max] = {0};
                U8 encoding_size = ULEB128__from_U32(attributes->stack_alignment, encoding);

                String8__serial_write_m(&data_cursor, &tag_stack_alignment);
                String8__serial_write(&data_cursor, encoding, encoding_size);
        }

        U8 tag_architecture = RISCV_Tag__Architecture;
        String8__serial_write_m(&data_cursor, &tag_architecture);
        String8__serial_write(&data_cursor, attributes->architecture.data, attributes->architecture.count);
        U8 null_termination = 0;
        String8__serial_write_m(&data_cursor, &null_termination);

        if (attributes->unaligned_access)
        {
                U8 tag_unaligned_access = RISCV_Tag__Unaligned_Access;
                U8 encoding[U32_ULEB128_encoding_size_max] = {0};
                U8 encoding_size = ULEB128__from_U32(attributes->unaligned_access, encoding);

                String8__serial_write_m(&data_cursor, &tag_unaligned_access);
                String8__serial_write(&data_cursor, encoding, encoding_size);
        }
        assert_always_m(data_cursor.count == 0 && "internal logic bug while filling riscv.attributes");

        return symbol;
}

// Ensures the undefined symbol and sections are added.
internal void
Symbols_Table__ensure_undefined_present(Symbols_Table *symbols_table)
{
        if (symbols_table->first != &Symbol_Ref__undefined)
        {
                SLL_queue_push_front_m(symbols_table->first, symbols_table->last, &Symbol_Ref__undefined);
                symbols_table->count += 1;
        }

        if (symbols_table->section_first != &Section__undefined)
        {
                DLL_push_front_m(symbols_table->section_first, symbols_table->section_last, &Section__undefined);
                symbols_table->sections_count += 1;
        }

        return;
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
Symbol_Ref__keep(Symbol_Ref *symbol)
{
        B32 prerequisites = !(symbol->flags & Symbol_Flags__Skip)
                         && symbol->name != &dot_symbol_string;

        B32 section_or_other_is       = symbol->type != ELF_Symbol_Type__None;
        B32 relocation_usage_has      = symbol->flags & Symbol_Flags__Relocation;
        B32 non_redefined_constant_is = symbol->section == &Section__absolute && !(symbol->flags & Symbol_Flags__Redefined);

        B32 global_non_alias_is = (symbol->section == &Section__undefined || symbol->section == &Section__common)
                               && symbol->binding == ELF_Symbol_Binding__Global
                               && !symbol->expression;

        B32 label_in_section_is         = !symbol->expression && Section__normal_is(symbol->section);
        B32 label_non_internal_is       = label_in_section_is
                                       && !Symbol_Ref__internal_is(symbol);
        B32 label_internal_relocated_is = label_in_section_is
                                       && Symbol_Ref__internal_is(symbol)
                                       && relocation_usage_has;

        B32 set_defined_in_section_is = symbol->expression
                && Section__normal_is(symbol->section)
                && !Symbol_Ref__internal_is(symbol);
        B32 undefined_symbol_is = symbol == &Symbol_Ref__undefined;

        B32 condition = section_or_other_is
                     || relocation_usage_has
                     || non_redefined_constant_is
                     || global_non_alias_is
                     || label_non_internal_is
                     || label_internal_relocated_is
                     || set_defined_in_section_is
                     || undefined_symbol_is;

        B32 keep = prerequisites && condition;

        return keep;
}

// True if the symbol name belongs to the internal `.L` family used for local labels or it is a dot.
internal B32
Symbol_Ref__internal_name_is(Symbol_Ref *symbol)
{
        B32 dot_is        = symbol->name->count == 1 && symbol->name->data[0] == '.';
        B32 l_prefixed_is = String8__match_prefix(*symbol->name, String8__literal(INTERNAL_SYMBOL_PREFIX));
        B32 result = dot_is || l_prefixed_is;
        return result;
}

internal B32
Symbol_Ref__internal_is(Symbol_Ref *symbol)
{
        return symbol->expression == 0 && Symbol_Ref__internal_name_is(symbol);
}

internal Symbol_Ref *
Symbols_Table__create_internal(Symbols_Table *symbols_table, Section *section, Arena *arena)
{
        String8 name = String8__literal(FAKE_LABEL_NAME);
        Symbol_Ref *result = Symbols_Table__create(symbols_table, name, arena);
        Symbol_Ref__update_section(result, section);
        return result;
}

internal S64
Symbol_Ref__resolve_label(Symbol_Ref *symbol, Resolve_Level level)
{
        // A symbol without an expression can be either a label definition, or some undefined symbol.
        // In any case, its value depends on its position on the fragment.
        S64 result = symbol->value;
        if (!(symbol->flags & Symbol_Flags__Finalized))
        {
                result += symbol->fragment->object_file_offset;

                if (level == Resolve_Level__Finalize)
                {
                        symbol->value = result;
                        symbol->flags |= Symbol_Flags__Finalized;
                }
        }

        return result;
}

// Kinda based on GNU `as` `resolve_symbol_value`, although with different assumptions.
//
// 1. Labels don't have an expression. Their value can be read straight into `Symbol_Ref.value`.
// 2. `Symbol_Flags__Finalized` means the simplification pass reached an end, and the value can be read from
//    `Symbol_Ref.value`. Undefined symbols and similar should have value zero.
//
// NOTE that this will be called on every symbol during the finalization process and every jump target used in
// relaxation.
internal S64
Symbol_Ref__resolve(Symbol_Ref *symbol, Diagnostics *diagnostics, Resolve_Level level)
{
        // TODO(low): I'm not a fan of early returns, but this is extremely common for labels.
        if (!symbol->expression)
        {
                return Symbol_Ref__resolve_label(symbol, level);
        }

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

        Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);
        Frame frame_base = {0};
        Frame *frame     = &frame_base;
        frame->symbol    = symbol;

        // Should be updated before popping every frame. The result before popping the last frame is what we need to
        // return.
        S64 result = 0;

        B32 traverse = level >= Resolve_Level__Traverse;
        B32 finalize = level >= Resolve_Level__Finalize;

        U16 index = 0;
        for (;;)
        {
                if (frame->symbol && frame->symbol->flags & Symbol_Flags__Finalized)
                {
                        // The simplest case. The symbol is already finalized.
                        result = frame->symbol->value;
                        SLL_stack_pop_m(frame);
                }
                else if (frame->symbol && !frame->symbol->expression)
                {
                        // A symbol without an expression can be either a label definition, or some undefined symbol.
                        // In any case, its value depends on its position on the fragment.
                        result = Symbol_Ref__resolve_label(frame->symbol, level);
                        SLL_stack_pop_m(frame);
                }
                else if (!(frame->state & Frame_State__Evaluated))
                {
                        if (frame->symbol)
                        {                                B32 loop_detected = frame->symbol->flags & Symbol_Flags__Resolving;
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

                                if (node->kind == Expression_Kind__Constant || node->evaluation == Expression_Kind__Constant)
                                {
                                        // Ensure it is set.
                                        node->evaluation = Expression_Kind__Constant;
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
                                                        node->integer_value  = result;
                                                }

                                                if (constant_folding_allowed)
                                                {
                                                        node->evaluation     = Expression_Kind__Constant;
                                                        node->symbol         = 0;
                                                }

                                                if (finalize && !valid)
                                                {
                                                        // TODO(medium): needs printf with section info style.
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                        diagnostic->message    = String8__literal("expression cannot be fully resolved and finalized");
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
                                        // The logic of this function should ensure this is a leaf reached.
                                        assert_always_m(node->left == 0);
                                        // Previous code ensure constants are check before, so this should NOT be a
                                        // constant. As such, it must be either a symbol, or nothing!
                                        assert_always_m(node->kind == Expression_Kind__None || node->kind == Expression_Kind__Symbol);

                                        Symbol_Ref *symbol_inner = node->symbol;
                                        node->evaluation = node->kind;

                                        B32 absolute_is = symbol_inner
                                                       &&
                                                       (
                                                                symbol_inner->section == &Section__absolute
                                                            || (symbol_inner->expression && symbol_inner->expression->evaluation == Expression_Kind__Constant)
                                                       );
                                        if (absolute_is)
                                        {
                                                symbol_inner->section = &Section__absolute;
                                                symbol_inner->value   = symbol_inner->expression->integer_value;

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
                                        else if (symbol_inner && traverse)
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
                                Expression *expression = frame->symbol->expression;
                                S64 value = expression->integer_value;
                                if (expression->evaluation == Expression_Kind__Symbol && expression->symbol)
                                {
                                        value += expression->symbol->value;
                                }
                                frame->symbol->value = value;
                                frame->symbol->flags |= Symbol_Flags__Finalized;
                        }

                        if (frame->symbol->expression->evaluation == Expression_Kind__Constant)
                        {
                                frame->symbol->section = &Section__absolute;
                        }

                        result = frame->symbol->expression->integer_value;
                        SLL_stack_pop_m(frame);
                }

                if (!frame)
                {
                        break;
                }
                else
                {
                        index += 1;
                        assert_always_m(index < U16_max && "infinite loop");
                }
        }
        Arena__scratch_end_m(scratch);

        return result;
}

internal void
Diagnostics__symbol_redefined(Diagnostics *diagnostics, Symbol_Ref *symbol, Token token)
{
        {
        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
        diagnostic->message    = String8__literal("symbol cannot be redefined");
        diagnostic->location   = token.location;
        diagnostic->ranges[0]  = Token__range(token);
        }
        {
        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
        diagnostic->kind       = Diagnostic_Kind__Note;
        diagnostic->message    = Diagnostic__previous_declaration_String8;
        diagnostic->location   = symbol->location;
        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + symbol->name->count }};
        }

        return;
}

//-----------------------------------------------------------------------------
// @Fragment
//-----------------------------------------------------------------------------

internal Fragment *
Fragments__push_empty_fragment(Fragments *fragments, U32 location)
{
        Fragment *fragment = Arena__push_struct_m(fragments->arena, Fragment);
        fragment->location = location;

        SLL_queue_push_z_m(&Fragment__nil, fragments->first, fragments->last, fragment);
        fragments->count += 1;

        return fragment;
}

// TODO(medium): still very NOT happy with this equivalent to `frag_grow`, and it is duplicated, delicate logic of the
// `Fragments__push`.
internal void
Fragments__ensure(Fragments *fragments, U32 size)
{
        U64  capacity_left             = fragments->arena->reserved_size - fragments->arena->offset;
        B32  arena_block_new_needed    = capacity_left < size;

        assert_always_m(capacity_left < fragments->arena->reserved_size && "underflow");
        if (arena_block_new_needed)
        {
                // Fill the capacity left of the arena block, so that we're sure to have a new block later.
                Arena *block_before = fragments->arena->current;
                Arena__push_array_m(fragments->arena, U8, capacity_left);
                assert_always_m(fragments->arena->current == block_before);
        }
        return;
}

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size.
internal U8 *
Fragments__push(Fragments *fragments, U32 location, U32 size)
{
        U8  *result                    = 0;
        U64  capacity_left             = fragments->arena->reserved_size - fragments->arena->offset;
        B32  arena_block_new_needed    = capacity_left < size;
        B32  fragment_seal_last_needed = arena_block_new_needed && fragments->last;
        B32  fragment_new_needed       = arena_block_new_needed || fragments->last == &Fragment__nil;
        B32  buffer_new_needed         = 0;

        assert_always_m(capacity_left < fragments->arena->reserved_size && "underflow");
        assert_always_m(fragments->last->relax_state == Relax_State__None && "cannot push to sealed fragment");

        if (arena_block_new_needed)
        {
                // Fill the capacity left of the arena block, so that we're sure to have a new block later.
                Arena *block_before = fragments->arena->current;
                Arena__push_array_m(fragments->arena, U8, capacity_left);
                assert_always_m(fragments->arena->current == block_before);
        }

        if (fragment_seal_last_needed)
        {
                // We have to "seal" the current fragment, and switch to another arena block.
                // We that also the new fragment header is on the new arena.
                Fragment__wane(fragments->last);
        }

        if (fragment_new_needed)
        {
                Fragments__push_empty_fragment(fragments, location);
                buffer_new_needed = 1;
        }

        buffer_new_needed |= fragments->last->data == 0;
        if (buffer_new_needed)
        {
                result = Arena__push_array_m(fragments->arena, U8, size);
                fragments->last->data = result;
        }
        else
        {
                // `Arena__push_array_m` ensure data has a minimum of 8 byte alignment.
                // In this case we want to keep appending into an already aligned byte array, so we need a 1-byte
                // alignment to ensure contiguity.
                result = Arena__push_array_aligned_m(fragments->arena, U8, size, cc_align_of(U8));
        }

        fragments->last->data_size += size;

        return result;
}

internal Fragment *
Fragments__variable
(
        Fragments   *fragments,
        U32          location,
        Relax_Info   relax_info,
        Relax_State  relax_state,
        U8          *data_variable,
        U8           data_variable_size
)
{
        if (fragments->first == &Fragment__nil)
        {
                Fragments__push_empty_fragment(fragments, location);
        }

        data_variable_size = min_m(data_variable_size, Fragment__data_variable_size_max);
        Fragment *sealed = fragments->last;

        memory_copy(sealed->data_variable, data_variable, data_variable_size);
        sealed->data_variable_size  = data_variable_size;
        sealed->relax_info          = relax_info;
        sealed->relax_state         = relax_state;

        // We have to create another fragment since variable data seal it.
        Fragments__push_empty_fragment(fragments, location);

        return sealed;
}

// Seal the current fragment with a fill pattern.
internal void
Fragments__fill(Fragments *fragments, U32 location, Fill fill)
{
        Relax_Info relax_info = { .fill_expression = fill.repeat };
        U8 *pattern = (U8 *)&fill.pattern;
        Fragments__variable
        (
                fragments,
                location,
                relax_info,
                Relax_State__Fill,
                pattern,
                fill.pattern_size
        );
        return;
}

internal void
Fragments__align(Fragments *fragments, U32 location, Alignment alignment)
{
        assert_always_m(pow_2_is_m(alignment.boundary) || !alignment.boundary);

        // TODO(check-gas): GNU as does some special handling of the absolute section. Since no variable-sized data exist on the
        // absolute section, it can be expanded to match the required alignment right away.

        Relax_Info relax_info =
        {
                .alignment =
                {
                        .boundary = alignment.boundary,
                        .write_size_max = alignment.write_size_max
                }
        };

        U8 *pattern = (U8 *)&alignment.pattern;
        Fragments__variable
        (
                fragments,
                location,
                relax_info,
                Relax_State__Align,
                pattern,
                alignment.pattern_size
        );

        return;
}

internal void
Fragment__wane(Fragment *fragment)
{
        assert_always_m(fragment != &Fragment__nil);
        assert_always_m(fragment->relax_state == Relax_State__None);
        fragment->data_variable_size = 0;
        fragment->relax_state        = Relax_State__Fill;
        fragment->relax_info         = (Relax_Info){0};
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
                Expression *fill_expression = Expression__push_constant(arena, repeat_count);
                SLL_queue_push_m(expressions->first, expressions->last, fill_expression);
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
                                // Keep rs1/rs2 from the original branch, invert its funct3
                                // (beq<->bne, blt<->bge, bltu<->bgeu), and skip over the following jal.

                                // A bit raw logic here, but it does the job.
                                U32 original_instruction = *(U32 *)fragment->data_variable;
                                U32 inverted_funct3      = ((original_instruction >> OP_SH_FUNCT3) & OP_MASK_FUNCT3) ^ 0x1;
                                U32 opcode_and_registers = original_instruction
                                                      & (OP_MASK_OP | (OP_MASK_RS1 << OP_SH_RS1) | (OP_MASK_RS2 << OP_SH_RS2));
                                U32 instruction_1 = opcode_and_registers | (inverted_funct3 << OP_SH_FUNCT3) | encode_immediate_b_m(8);
                                U32 instruction_2 = MATCH_JAL;

                                // Adjust associated fixup information
                                relax_info->jump.fixup->offset              = fragment->data_size + sizeof(instruction_1);
                                relax_info->jump.fixup->fragment_write_size = sizeof(instruction_2);
                                relax_info->jump.fixup->relocation_type     = Relocation_RISC_V__JAL;

                                memory_copy(fragment->data_variable,                         (U8 *)&instruction_1, sizeof(instruction_1));
                                memory_copy(fragment->data_variable + sizeof(instruction_1), (U8 *)&instruction_2, sizeof(instruction_2));

                        }
                        else if (instructions_total_size == 4)
                        {
                                U16 relocation_type = relax_info->jump.unconditional_is ? Relocation_RISC_V__JAL : Relocation_RISC_V__Branch;
                                // Adjust associated fixup information
                                relax_info->jump.fixup->offset              = fragment->data_size;
                                relax_info->jump.fixup->fragment_write_size = instructions_total_size;
                                relax_info->jump.fixup->relocation_type     = relocation_type;
                        }
                        else
                        {
                                unreachable_m();
                        }

                        Expression *repeat_expression = Expression__push_constant(arena, 1);
                        fragment->relax_info  = (Relax_Info){ .fill_expression = repeat_expression };
                        fragment->relax_state = Relax_State__Fill;
                }
        } break;
        }

        assert_always_m(data_size_before          == fragment->data_size);
        assert_always_m(data_variable_size_before == fragment->data_variable_size);

        return;
}

internal U8
Fragment__jump_instructions_total_size(Fragment *fragment, Section *section, Diagnostics *diagnostics)
{
        U8 size = 0;
        if (fragment->relax_state == Relax_State__Jump)
        {
                // NOTE: assume jumps are in range; the linker will catch any that aren't.
                // For branches, assume worst size and then fix it.
                size = fragment->relax_info.jump.unconditional_is ? 4 : 8;
                Symbol_Ref *symbol_target_jump = fragment->relax_info.jump.expression->symbol;

                B32 symbol_defined_is    = symbol_target_jump && symbol_target_jump->section != &Section__undefined;
                B32 symbol_weak_is       = symbol_target_jump && symbol_target_jump->binding  == ELF_Symbol_Binding__Weak;
                B32 section_same_is      = symbol_defined_is && symbol_target_jump->section   == section;
                B32 size_can_be_computed = symbol_defined_is && !symbol_weak_is && section_same_is;

                // If we have no symbol, e.g. `j 4` or `beq zero, zero, 4`, then we default the worst expansion.
                if (size_can_be_computed)
                {
                        S64 jump_target_offset = Symbol_Ref__resolve(symbol_target_jump, diagnostics, Resolve_Level__Traverse);
                        S64 distance = jump_target_offset - (fragment->object_file_offset + fragment->data_size);

                        // TODO(compressed, check-gas): compressed range
                        //
                        // Check that `distance` fits a signed `RISCV_BRANCH_REACH`, i.e.
                        // `[-RISCV_BRANCH_REACH/2, RISCV_BRANCH_REACH/2)`.
                        // if (compressed && range compressed blah blah)
                        B32 within_branch_range_is = (S64)(-(S64)RISCV_BRANCH_REACH / 2) <= distance
                                                  && distance < (S64)RISCV_BRANCH_REACH / 2;
                        if (within_branch_range_is)
                        {
                                size = 4;
                        }
                        // else if (!unconditional && compressed) then this is 6.
                }
        }

        return size;
}

//-----------------------------------------------------------------------------
// @Fixup
//-----------------------------------------------------------------------------

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
Fixup__apply_jump(Fixup *fixup, U32(*encoding_callback)(S64), B32(*valid_immediate_callback)(S64), Options *options, Diagnostics *diagnostics)
{
        S64 target   = fixup->expression->integer_value;
        target      += fixup->expression->symbol ? fixup->expression->symbol->value : 0;
        // The distance is measured from the physical address of the patched
        // instruction, i.e. the fixup's write position.  For an in-range
        // branch the fixup sits at fragment start (fixup->offset == data_size),
        // but a relaxed branch repositions it to `data_size + 4` to point at
        // the emitted JAL, so we must use fixup->offset rather than data_size.
        S64 distance = target - (fixup->fragment->object_file_offset + fixup->offset);

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

        B32 internal_is = fixup->expression->symbol && Symbol_Ref__internal_is(fixup->expression->symbol);
        if (!options->relax && internal_is && valid_immediate)
        {
                fixup->flags |= Fixup_Flags__Done;
        }
}

internal void
Fixup__apply(Fixup *fixup, Section *section, Arena *arena, Options *options, Diagnostics *diagnostics)
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
                Symbol_Ref *symbol           = fixup->expression ? fixup->expression->symbol : 0;
                Expression *expression_inner = symbol            ? symbol->expression        : 0;
                Symbol_Ref *symbol_inner     = expression_inner  ? expression_inner->symbol  : 0;

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

        Expression *expression = fixup->expression;
        Symbol_Ref *symbol     = expression ? expression->symbol : 0;

        switch (fixup->relocation_type)
        {
        default: { unreachable_m(); }

        case Relocation_RISC_V__High_20:
        {
                // Remember that `%lo` is treated as a signed 12-bit field, so the pair must satisfy `x = %hi(x) << 12 +
                // signext(%lo(x))` with %lo in [-0x800, 0x7ff]. Round the value up to the nearest 0x1000 by re-adding
                // the 0x800 offset, so the low 12 bits stay signed-representable.
                S64 high_part = (expression->integer_value + 0x800) & ~(S64)0xfff;
                Fixup__apply_constant(fixup, encode_immediate_u_m(high_part));
                relaxable = 1;
        } break;
        case Relocation_RISC_V__Low_12_I_Type: { Fixup__apply_constant(fixup, encode_immediate_i_m(expression->integer_value)); relaxable = 1; } break;
        case Relocation_RISC_V__Low_12_S_Type: { Fixup__apply_constant(fixup, encode_immediate_s_m(expression->integer_value)); relaxable = 1; } break;

        case Relocation_RISC_V__GOT_High_20:
        {
                // R_RISCV_GOT_HI20 is resolved by the linker (which creates the GOT entry), so nothing is encoded here:
                // the `auipc` immediate is left as zero and the relocation is emitted as-is.
                //
                // R_RISCV_GOT_HI20 and the following R_RISCV_PCREL_LO12_I are relaxable only if they were created as a
                // result of the `la` or `lga` assembler macros. A hand-written `%got_pcrel_hi` is never relaxed
                // (matching GNU as).
                //
                // GNU as restricts the relaxation to macro expansions because the relaxation rewrites the whole pattern
                //
                // ```asm
                // auipc rd, %got_pcrel_hi(sym)
                // ld/lw rd, %pcrel_lo(.L0)(rd)
                // ```
                // And becomes
                // ```asm
                // auipc rd, %pcrel_hi(sym)
                // addi  rd, rd, %pcrel_lo(.L0)
                // ```
                // The rewrite is only semantically correct when the second instruction is exactly the canonical load
                // the macro emits (`ld` on RV64, `lw` on RV32). A hand-written sequence could use a different-width
                // load or no load at all, so relaxing it would silently produce wrong code. Hence the R_RISCV_RELAX
                // hint is a contract with the linker that is only made for a known-safe, canonical source (a macro).
                if (fixup->flags & Fixup_Flags__Macro)
                {
                        relaxable = 1;
                }
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
                if (expression->evaluation == Expression_Kind__Subtract)
                {
                        // The idea is that: since this can only be valid if it's a subtract,
                        // unpack it into an "add" and "sub" relocation by looking at the left and right subexpressions.

                        Fixup *fixup_sub = Arena__push_struct_m(arena, Fixup);
                              *fixup_sub = *fixup;

                        fixup_sub->expression = expression->right;
                        fixup->expression     = expression->left;
                        expression            = fixup->expression;
                        DLL_insert_m(section->fixups.first, section->fixups.last, fixup, fixup_sub);
                }
                else if (expression->evaluation == Expression_Kind__Constant)
                {
                        U8 size = min_m(fixup->fragment_write_size, sizeof(expression->integer_value));
                        U8 *write_area = Fixup__write_area(fixup);
                        memory_copy(write_area, (U8 *)&expression->integer_value, size);
                        fixup->flags |= Fixup_Flags__Done;
                }
                else if (fixup->relocation_type == Fixup__8_Bit || fixup->relocation_type == Fixup__16_Bit)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->message    = String8__literal("cannot represent an 8-bit or 16-bit relocation on RISC-V/ELF object file");
                        diagnostic->location   = expression->location_range.v[0];
                        diagnostic->ranges[0]  = expression->location_range;
                }
        } break;

        case Relocation_RISC_V__JAL:               { Fixup__apply_jump(fixup, encode_immediate_j,  validate_immediate_j,  options, diagnostics); } break;
        case Relocation_RISC_V__Branch:            { Fixup__apply_jump(fixup, encode_immediate_b,  validate_immediate_b,  options, diagnostics); } break;
        case Relocation_RISC_V__Jump_Compressed:   { Fixup__apply_jump(fixup, encode_immediate_cj, validate_immediate_cj, options, diagnostics); } break;
        case Relocation_RISC_V__Branch_Compressed: { Fixup__apply_jump(fixup, encode_immediate_cb, validate_immediate_cb, options, diagnostics); } break;

        case Relocation_RISC_V__PC_Relative_High_20:
        {
                B32 symbol_internal_is = symbol && Symbol_Ref__internal_is(symbol);
                B32 evaluatable = symbol_internal_is && symbol->section == section;
                if (evaluatable)
                {
                        S64 position = symbol->value;
                        S64 offset   = expression->integer_value;
                        S64 target   = (position + offset) - fixup->fragment->object_file_offset;

                        PC_Relative_High *pc_relative_high = Arena__push_struct_m(arena, PC_Relative_High);
                        pc_relative_high->section            = section;
                        pc_relative_high->object_file_offset = fixup->fragment->object_file_offset;
                        pc_relative_high->expression         = expression;

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
                                diagnostic->location   = expression->location;
                                diagnostic->ranges[0]  = expression->location_range;
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
                U64 object_file_offset = symbol->value + expression->integer_value;
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

        if (!(fixup->flags & Fixup_Flags__Done) && expression && expression->evaluation == Expression_Kind__Subtract)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__format
                        (
                                diagnostics->arena,
                                "Cannot resolve %.*s - %.*s", String8__varg(*(expression->left->symbol->name)), String8__varg(*(expression->right->symbol->name))
                        );
                diagnostic->location   = expression->location;
                diagnostic->ranges[0]  = expression->location_range;
        }

        B32 emit_relax_relocation = relaxable
                                 && (fixup->flags & Fixup_Flags__Relax)
                                 && expression
                                 && symbol;
        if (emit_relax_relocation)
        {
                Fixup *fixup_relax = Arena__push_struct_m(arena, Fixup);
                       fixup_relax->expression          = expression;
                       fixup_relax->offset              = fixup->offset;
                       fixup_relax->fragment            = fixup->fragment;
                       fixup_relax->relocation_type     = Relocation_RISC_V__Relax;
                DLL_insert_m(section->fixups.first, section->fixups.last, fixup, fixup_relax);
        }

        if (!(fixup->flags & Fixup_Flags__Done))
        {
                if (symbol && expression->evaluation != Expression_Kind__Constant)
                {
                        symbol->flags |= Symbol_Flags__Relocation;
                }
                section->fixups.unresolved += 1;
                section->symbol->flags |= Symbol_Flags__Relocation;
        }

        B32 addend_doesnt_fit = options->xlen == XLEN_32 && expression && expression->integer_value > (S64)U32_max;
        if (addend_doesnt_fit)
        {
                Diagnostics__expression(diagnostics, expression, String8__literal("relocation addend doesn't fit in 32 bits"));
        }

        return;
}

//-----------------------------------------------------------------------------
// @Section
//-----------------------------------------------------------------------------

// Add a fixed size instruction into a fragment. If there is a fixup associated to this function (fixup != 0),
// track the information of where this instruction has been placed.
internal void
Section__add_instruction_fixed
(
        Section *section,
        Fixup   *fixup,

        U32      encoding,
        U8       encoding_size,
        U32      location
)
{
        U8 *data = Fragments__push
        (
                 &section->fragments,
                 location,
                 encoding_size
        );

        // Track its precise location within the fragment. Important to do it _after_ we've written it
        // since it might have landed into another fragment because of low capacity of the previous.
        if (fixup)
        {
                Fragment *last = section->fragments.last;

                fixup->fragment            = last;
                fixup->offset              = last->data_size - encoding_size;
                fixup->fragment_write_size = encoding_size;
        }

        memory_copy(data, (U8 *)&encoding, encoding_size);
        return;
}

internal void
Section__finish(Section *section)
{
        // We ensure that every section ends with a [align][fill] layout, using different heuristics for the alignment
        // fragment size.
        U8 alignment_power = 0;

        if (section->elf.flags & ELF_Section_Header_Flags__EXECINSTR)
        {
                alignment_power = section->elf.alignment ? count_trailing_zeros(section->elf.alignment) : 0;
        }

        if ((section->elf.flags & (ELF_Section_Header_Flags__MERGE | ELF_Section_Header_Flags__STRINGS))
            && section->elf.entry_size)
        {
                U8 entry_alignment_power = section->elf.entry_size ? count_trailing_zeros(section->elf.entry_size) : 0;
                alignment_power = max_m(alignment_power, entry_alignment_power);
        }

        Alignment alignment =
        {
                .boundary = 1 << alignment_power,
        };
        U32 location = section->fragments.last == &Fragment__nil ? 0 : section->fragments.last->location;

        Fragments__align(&section->fragments, location, alignment);
        Fragment__wane(section->fragments.last);

        return;
}

internal B32
Section__normal_is(Section *section)
{
        B32 result = section != &Section__undefined
                  && section != &Section__absolute
                  && section != &Section__common;
        return result;
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
                        U32 boundary = fragment->relax_info.alignment.boundary ? fragment->relax_info.alignment.boundary : 1;
                        assert_always_m(pow_2_is_m(boundary) || !boundary);

                        U64 address_aligned = align_pow_2_m(address, boundary);
                        U64 growth          = address_aligned - address;
                        U8 pattern_size     = fragment->data_variable_size;

                        U32 write_size_max = fragment->relax_info.alignment.write_size_max;
                        if (write_size_max && growth > write_size_max)
                        {
                                // Explicitly give up as alignment, as request by the user.
                                growth = 0;
                        }

                        if (growth % (pattern_size || 1) != 0)
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
                        Symbol_Ref *symbol = fragment->relax_info.jump.expression->symbol;
                        if (symbol)
                        {
                                Symbol_Ref__resolve(symbol, diagnostics, Resolve_Level__Traverse);
                        }

                        U8 size = Fragment__jump_instructions_total_size(fragment, section, diagnostics);
                        fragment->data_variable_size = size;
                        address += size;
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

                                                // Prevent this error from being repeated?
                                                fragment->relax_info.fill_expression = 0;
                                                expression = 0;
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
                                U32 boundary = fragment->relax_info.alignment.boundary ? fragment->relax_info.alignment.boundary : 1;
                                S64 align_offset_old = offset_was + fragment->data_size;
                                S64 align_offset     = offset     + fragment->data_size;

                                U64 align_offset_old_aligned = align_pow_2_m(align_offset_old, boundary);
                                U64 align_offset_aligned     = align_pow_2_m(align_offset,     boundary);

                                // Again, give up with above `fragment->relax_info.alignment.write_size_max`
                                U32 write_size_max = fragment->relax_info.alignment.write_size_max;
                                if (write_size_max)
                                {
                                        if (align_offset_old_aligned > fragment->relax_info.alignment.write_size_max) { align_offset_old_aligned = 0; }
                                        if (align_offset_aligned     > fragment->relax_info.alignment.write_size_max) { align_offset_aligned = 0; }
                                }

                                // The growth contributed by this align is the _change in padding_ compared to the
                                // previous iterations. The position itself already shifts by `stretch` (the cumulative
                                // growth of preceding fragments); counting that shift again here would count it twice.
                                S64 padding_old = (S64)align_offset_old_aligned - align_offset_old;
                                S64 padding     = (S64)align_offset_aligned     - align_offset;
                                growth = padding - padding_old;
                        } break;
                        case Relax_State__Jump:
                        {
                                // `riscv_relax_frag`
                                U8 size_old = fragment->relax_info.jump.instructions_total_size;
                                U8 size_new = Fragment__jump_instructions_total_size(fragment, section, diagnostics);
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
