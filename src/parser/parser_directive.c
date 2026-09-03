Directive_Kind
Directive_Kind__from_String8(String8 source)
{
        Directive_Kind result = Directive_Kind__None;
        U32 index = Directive_Kind__None;
        B32 found = 0;

        for (;;)
        {
                B32 break_should = found || index >= Directive_Kind__COUNT;
                if (break_should)
                {
                        break;
                }

                const String8 target = Directive_Kind__String8_table[index];
                found = source.count == target.count && memory_match(source.data, target.data, source.count) == 0;
                if (found)
                {
                        result = index;
                }

                index += 1;
        }
        return result;
}

// Handles .local, .weak, .global directive. Those simply try to set the binding of a symbol, and nothing else. It is
// created if missing.
internal void
directive_binding
(
        Token_Cursor            *cursor,
        Diagnostics             *diagnostics,
        Arena                   *arena,
        Symbols_Table           *symbols_table,
        ELF_Symbol_Binding       binding
)
{
        token_next(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__Identifier)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Identifier_Expected);
                diagnostic->location   = cursor->current.location;
        }

        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name, arena);
        if (symbol->section == &Section__undefined)
        {
                // Still give a preliminary location for it so that we can show diagnostics.
                symbol->location = cursor->current.location;
        }

        U8 binding_old = symbol->binding;
        B32 demoted = binding < binding_old && symbol->binding != ELF_Symbol_Binding__Weak;
        if (demoted)
        {
                Diagnostics__symbol_redefined(diagnostics, symbol, cursor->current);
        }

        if (symbol->binding != ELF_Symbol_Binding__Weak)
        {
                symbol->binding = binding;
        }

        token_next(cursor, diagnostics);
}

internal void
directive_set_like
(
        Arena          *arena,
        Token_Cursor   *cursor,
        Diagnostics    *diagnostics,
        Symbols_Table  *symbols_table,
        Set_Mode        mode
)
{
        // Some words on `.equ` vs `.eqv`.
        //
        // `.equ` creates volatile, redefinable symbols. When a symbol is redefined via `.equ`, it is kept as symbols
        // table data for references, but removed from trie lookup. Getting the same symbol name would return its latest
        // definition.
        //
        // `.eqv` should be thought of as an expression blueprint. It creates an expression-defined symbol that, on
        // every reference, is _deep-cloned_.
        //
        // Example assembly:
        // ```asm
        // .eqv OFF, B
        // .set B, 16
        // .set OFF_1, OFF # 16
        // .set B, 32
        // .set OFF_2, OFF # 32
        // # OFF equals 16 at the end.
        // ```
        //
        // The first `B` assignments modifies the original `OFF` expression. The first usage of `OFF` performs a
        // reference deep-clone while making symbols table lookups, meaning that it will contain a reference to the
        // current definition of `B`.
        // The second `B` assignment, due to `.equ` semantics, creates a new symbol. The second usage of `OFF`
        // performs again a reference deep-clone, with the latest definition of `B` which equals 32.

        token_next(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__Identifier)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Identifier_Expected);
                diagnostic->location   = cursor->current.location;
        }

        // What if you set the name of a section or a register? Well, for a section, you can't since a section is a
        // symbol. For registers, it is indeed possible and could be error prone, but in practice the assembler is
        // always able to discriminate between a register and a symbol named in the same way due to its position in the
        // instruction.
        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name, arena);

        B32 already_defined_or_equated = symbol->section != &Section__undefined || symbol->expression;
        if (already_defined_or_equated)
        {
                B32 frozen = mode != Set_Mode__Override
                          || !(symbol->flags & Symbol_Flags__Volatile);
                if (frozen)
                {
                        Diagnostics__symbol_redefined(diagnostics, symbol, cursor->current);
                }

                symbol = Symbols_Table__create(symbols_table, name, arena);
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

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                token_next(cursor, diagnostics);
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Comma_Expected);
                diagnostic->location   = cursor->current.location;
        }

        symbol->flags |= Symbol_Flags__Resolving;
        Expression *expression = expression_parse_with_flags
        (
                arena,
                cursor,
                symbols_table,
                diagnostics,
                expression_flags
        );
        symbol->expression = expression;
        symbol->flags &= ~Symbol_Flags__Resolving;

        if (mode != Set_Mode__Strict_Forward)
        {
                S64 result = expression_evaluate(expression);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        symbol->section = &Section__absolute;
                        symbol->value   = result;
                }

                // If we have an equation to a "real" symbol, i.e. defined on a concrete section, then we also inherit
                // its attributes.
                else if (expression->evaluation == Expression_Kind__Symbol && expression->symbol)
                {
                        Symbol_Ref *target = expression->symbol;
                        B32 target_section_real_is = Section__normal_is(target->section);
                        if (target_section_real_is)
                        {
                                symbol->section         = target->section;
                                // TODO(low): maybe setting the value already can be omitted?
                                symbol->value           = target->value + expression->integer_value;
                                symbol->fragment        = target->fragment;
                                symbol->type            = target->type;
                                symbol->size_expression = target->size_expression;
                        }
                }
        }

        return;
}

internal void
directive_data
(
        Arena          *arena,
        Token_Cursor   *cursor,
        Diagnostics    *diagnostics,
        Expressions    *expressions,
        Symbols_Table  *symbols_table,
        U8              data_directive_size
)
{
        // Format: .byte|half|word|dword <expr_1> , ..., <expr_n>.
        //
        // Advance to reach the first expression token.
        U32 location_begin = cursor->current.location;
        token_next(cursor, diagnostics);
        U8 bit_size = data_directive_size * 8;
        assert_always_m(bit_size <= 64);
        U32 expressions_count = 0;
        for (;;)
        {
                Expression *expression = expression_parse_with_flags(arena, cursor, symbols_table, diagnostics, Expression_Flags__Data_Directive);
                SLL_queue_push_m(expressions->first, expressions->last, expression);
                expressions_count += 1;
                S64 result = expression_evaluate(expression);

                U8 *data = Fragments__push(&symbols_table->section_current->fragments, cursor->current.location, data_directive_size);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        // NOTE: some carefulness here of UB for shifting more than bit size.
                        U64 overflow_mask = bit_size >= sizeof(U64) ? 0 : ~(U64)0 << bit_size;
                        U64 usage_mask    = ~overflow_mask;

                        // Check that the absolute value fits into the required number of bytes.
                        B32 fits = ((U64)result & overflow_mask) == 0 || ((U64)(-result) & overflow_mask) == 0;

                        U64 data_to_be_written = (U64)result & usage_mask;
                        memory_copy(data, (U8 *)&data_to_be_written, data_directive_size);

                        if (!fits)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Value_Too_Large_Truncated);
                                diagnostic->location   = expression->location_range.v[0];
                                diagnostic->ranges[0]  = expression->location_range;
                        }
                }
                else
                {
                        Fixup *fixup = Arena__push_struct_m(arena, Fixup);
                        fixup->expression           = expression;
                        fixup->fragment             = symbols_table->section_current->fragments.last;
                        fixup->offset               = symbols_table->section_current->fragments.last->data_size - data_directive_size;
                        fixup->fragment_write_size  = data_directive_size;
                        fixup->relocation_type      = bit_size == 8  ? Fixup__8_Bit
                                                    : bit_size == 16 ? Fixup__16_Bit
                                                    : bit_size == 32 ? Relocation_RISC_V__32_Bit
                                                    : Relocation_RISC_V__64_Bit;

                        Section *section = symbols_table->section_current;
                        DLL_push_back_m(section->fixups.first, section->fixups.last, fixup);
                }

                B32 break_should_directive =  cursor->source_index >= cursor->source->count
                                           || cursor->current.kind == Token_Kind__Newline;
                if (break_should_directive)
                {
                        break;
                }

                if (cursor->current.kind == Token_Kind__Comma)
                {
                        token_next(cursor, diagnostics);
                }
        }

        // Validate that on code section the directive respects alignment. Catching this up early provides better
        // diagnostics than doing it later.
        Section *section         = symbols_table->section_current;
        B32 section_code_is      = section->elf.flags & ELF_Section_Header_Flags__EXECINSTR;
        U64 directive_total_size = expressions_count * data_directive_size;
        B32 padding_invalid      = (directive_total_size % section->elf.alignment) != 0;
        if (section_code_is && padding_invalid)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Data_Directive_Disrupts_Alignment);
                diagnostic->message    = String8__format(arena, "data directive total size (%u bytes) disrupts alignment (%u bytes) in code section", directive_total_size, section->elf.alignment);
                diagnostic->location   = location_begin;
                diagnostic->ranges[0]  = (Range1_U32){{ location_begin, cursor->current.location }};
        }
}

internal void
directive_string
(
        Token_Cursor *cursor,
        Diagnostics  *diagnostics,
        Section      *section,

        B32 null_terminated
)
{
        token_next(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__String)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__String_Literal_Expected);
                diagnostic->location   = cursor->current.location;
        }

        // Can be of the form "\nhello\n", so with quotes and optional escaped characters.
        String8 text     = Token_Cursor__text(cursor);
        String8 content  = String8__skip_chop(text);
        U64 size_escaped = String8__escaped_size(content) + !!null_terminated;

        U8 *data = Fragments__push(&section->fragments, cursor->current.location, size_escaped);
        U32 error_index = bytes_escaped_fill(content, data, size_escaped - !!null_terminated);
        if (error_index)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Directive_Escape_Sequence_Invalid);
                diagnostic->location   = cursor->current.location + error_index;
                diagnostic->ranges[0]  = (Range1_U32){{ diagnostic->location, diagnostic->location + 1 }};
        }

        token_next(cursor, diagnostics);
}

// Implementation follows https://www.sourceware.org/binutils/docs/as.html#g_t_002ebase64-_0022string_0022_005b_002c-_002e_002e_002e_005d
internal void
directive_base64
(
        Token_Cursor *cursor,
        Diagnostics  *diagnostics,
        Section      *section
)
{
        token_next(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__String)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__String_Literal_Expected);
                diagnostic->location   = cursor->current.location;
        }

        String8 text    = Token_Cursor__text(cursor);
        String8 content = String8__skip_chop(text);

        B32 replace = 0;
        U8  replacement_data[4] = {0};
        if (content.count % 4 != 0)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Base64_Length_Multiple_4);
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);

                replace = 1;
        }
        else if (content.count == 0)
        {
                replace = 1;
        }

        if (replace)
        {
                content.data = replacement_data;
                content.count = array_count_m(replacement_data);
        }

        // Decode in place: 4 base64 chars -> 3 bytes (RFC 4648, with '=' padding).
        U64 output_size = (content.count / 4) * 3;
        assert_always_m(output_size > 0);

        // Account for at most two bytes of padding
        if (content.data[content.count - 1] == '=')
        {
                output_size   -= 1;
                content.count -= 1;
        }
        if (content.data[content.count - 1] == '=')
        {
                output_size   -= 1;
                content.count -= 1;
        }

        U8 *data = Fragments__push(&section->fragments, cursor->current.location, output_size);

        U64 input_index  = 0;
        U64 output_index = 0;

        for (;;)
        {
                B32 break_should = input_index >= content.count;
                if (break_should)
                {
                        break;
                }

                U32 accumulator = 0;
                U8  valid_bits  = 0;
                U8  index       = 0;
                for (;;)
                {
                        B32 break_should_group = index >= 4 || input_index >= content.count;
                        if (break_should_group)
                        {
                                break;
                        }

                        U8 character = content.data[input_index];
                        input_index += 1;

                        U8 value = base64_table[character];
                        if (value == 0xFF)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Base64_Character_Invalid);
                                diagnostic->location   = cursor->current.location + input_index;
                        }

                        accumulator = (accumulator << 6) | (value & 0x3F);
                        valid_bits += 6;
                        index += 1;
                }

                for (;;)
                {
                        if (valid_bits < 8)
                        {
                                break;
                        }

                        valid_bits -= 8;
                        data[output_index] = (U8)(accumulator >> valid_bits);
                        output_index += 1;
                }
        }

        token_next(cursor, diagnostics);
}

internal void
directive_ident
(
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Arena           *arena,
        Symbols_Table   *symbols_table
)
{
        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__String)
        {
                String8 text    = Token_Cursor__text(cursor);
                String8 content = String8__skip_chop(text);

                // GNU as collects all `.ident` strings into the `.comment` section,
                // each entry prefixed by a NULL byte and NULL-terminated.
                Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, section_name_comment, arena);
                if (symbol->section == &Section__undefined)
                {
                        Symbols_Table__create_section(symbols_table, symbol, arena, Arena_Parameters__default);
                        symbol->section->elf.entry_size = 1; // GNU as: `.comment` is MS with entsize 1.
                        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol->section);
                }

                // Leading NULL + content + trailing NULL.
                U8 *data = Fragments__push(&symbol->section->fragments, cursor->current.location, text.count);
                memory_copy(data + 1, content.data, content.count);
        }

        token_next(cursor, diagnostics);
}

// `.section`, `.pushsection`, `.popsection`, `.previous` tracking specification
//
// We keep track of the following sections:
//
// - `current`  - the section code is emitted into.
// - `previous` - the section `.previous` jumps to.
// - `stack`    - a LIFO stack of `(current, previous)` snapshots.
//
// Initial: `current = .text`, `previous = 0`, stack = []`.
//
// Here are the rules of the transitions:
//
// `section NAME`:
//
// 1. `previous := current`
// 2. `current := NAME`
//
// `pushsection NAME`:
//
// 1. `stack.push((current, previous))`
// 2. `previous := current`
// 3. `current := NAME`
//
// `popsection`:
//
// - If `stack` is empty: error, no change.
// - Else `(current, previous) := stack.pop()` - a restoration, not a transition.
//
// `previous`:
// - If `previous = 0` error, no change.
// - Else swap: `(current, previous) := (previous, current)`.
//
// Properties:
//
// 1. Push/pop are exact inverses: `pop(push)` restores `(current, previous)` to their pre-push values, regardless of
//    intervening `.section`/`.previous`.
// 2. Two consecutives uses of `previous` result in a no-op.
// 3. `previous` follows transitions: it targets the section that was current before the most recent
//    `.section`/`.pushsection`/`.previous`. Since `.popsection` restores `previous` directly, a `.previous` after a pop
//    goes to the pre-push `previous`, not the popped-from section.

internal void
directive_section
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        B32              push
)
{
        // Syntax: `.[push]section name [, "flags"[, @type[, argument...]]]`

        U32 location_start = cursor->current.location;

        // We essentially accept anything that is not an end of statement or a comma.
        U8      ending_bytes_set_data[] = {' ', ',', ';', '\n'};
        String8 ending_bytes_set        = String8__new(ending_bytes_set_data, array_count_m(ending_bytes_set_data));
        B32     skip_initial_whitespace = 1;

        Token_Cursor__read_raw_identifier_until(cursor, ending_bytes_set, skip_initial_whitespace);
        String8 name = Token_Cursor__text(cursor);
        if (!name.count)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Section_Name_Empty);
                diagnostic->location   = location_start;
                diagnostic->ranges[0]  = (Range1_U32){{ location_start, cursor->current.location + cursor->current.size }};
        }

        Symbol_Ref *symbol = Symbols_Table__get(symbols_table, name);
        B32 switching_only_is         = symbol && symbol->type == ELF_Symbol_Type__Section;
        B32 section_should_be_created = !switching_only_is;
        B32 can_create_section        = !symbol
                                     || (symbol->section == &Section__undefined && symbol->type == ELF_Symbol_Type__None)
                                     || (symbol->flags & Symbol_Flags__Volatile);

        if (section_should_be_created)
        {
                if (!can_create_section)
                {
                        Diagnostics__symbol_redefined(diagnostics, symbol, cursor->current);
                }
                else
                {
                        symbol = Symbols_Table__create(symbols_table, name, arena);
                        symbol->location = cursor->current.location;
                        Symbols_Table__create_section(symbols_table, symbol, arena, Arena_Parameters__default);
                        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol->section);
                }
        }

        // TODO(medium): some missing validations:
        // 1. Some section types requires mandatory checks or arguments. See https://www.sourceware.org/binutils/docs/as.html#ELF-Version

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                // Error if already defined.
                if (switching_only_is)
                {
                        Diagnostics__symbol_redefined(diagnostics, symbol, cursor->previous);
                }

                // Read flags.
                token_next(cursor, diagnostics);
                if (cursor->current.kind != Token_Kind__String)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__String_Literal_Expected);
                        diagnostic->location   = cursor->current.location;
                }
                String8 text    = Token_Cursor__text(cursor);
                String8 content = String8__skip_chop(text);
                ELF_Section_Header_Flags flags = ELF_Section_Header_Flags__parse(content);

                if (flags == ELF_Section_Header_Flags__Invalid)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Section_Flags_Invalid);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }

                if (symbol->section->special && symbol->section->elf.flags != flags)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Flags_Redefinition_Ignored);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }
                else
                {
                        symbol->section->elf.flags = flags;
                }
                token_next(cursor, diagnostics);
        }

        // TODO(low): section groups.

        if (cursor->current.kind == Token_Kind__Comma)
        {
                // Parse type.
                token_next(cursor, diagnostics);
                if (cursor->current.kind != Token_Kind__At)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Section_Type_Syntax_Invalid);
                        diagnostic->location   = cursor->current.location;
                }
                token_next(cursor, diagnostics);
                String8 content = Token_Cursor__text(cursor);
                ELF_Section_Header_Type type = ELF_Section_Header_Type__from_String8(content);
                if (type == ELF_Section_Header_Type__Invalid)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Section_Type_Invalid);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                }
                if (symbol->section->special && symbol->section->elf.type != type)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Type_Redefinition_Ignored);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                }
                else
                {
                        symbol->section->elf.type = type;
                }
                token_next(cursor, diagnostics);
        }

        // Optional trailing arguments: `,entsize`. For example used on MERGE/STRINGS sections
        // (e.g. `.rodata.str1.8,"aMS",@progbits,1`).
        if (cursor->current.kind == Token_Kind__Comma)
        {
                token_next(cursor, diagnostics);
                Expression *entry_size_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                SLL_queue_push_m(expressions->first, expressions->last, entry_size_expression);
                expression_evaluate(entry_size_expression);
                if (entry_size_expression->evaluation == Expression_Kind__Constant)
                {
                        S64 entsize = entry_size_expression->integer_value;
                        if (entsize < 0)
                        {
                                Diagnostics__expression(diagnostics, entry_size_expression, DG__Entry_Size_Invalid_Ignored);
                        }
                        else
                        {
                                symbol->section->elf.entry_size = (U64)entsize;
                        }
                }
                else
                {
                        Diagnostics__expression(diagnostics, entry_size_expression, DG__Constant_Expression_Expected_Lower);
                }
        }

        if (push)
        {
                Section_Stack *element = Arena__push_struct_m(arena, Section_Stack);
                               element->previous = symbols_table->section_previous;
                               element->current  = symbols_table->section_current;
                SLL_stack_push_m(symbols_table->section_stack, element);
        }

        symbols_table->section_previous = symbols_table->section_current;
        symbols_table->section_current  = symbol->section;
}

internal void
directive_section_pop
(
        Token_Cursor  *cursor,
        Diagnostics   *diagnostics,
        Symbols_Table *symbols_table
)
{
        if (symbols_table->section_stack)
        {
                symbols_table->section_current  = symbols_table->section_stack->current;
                symbols_table->section_previous = symbols_table->section_stack->previous;
                SLL_stack_pop_m(symbols_table->section_stack);
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Section_Pop_Unmatched);
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
        }

        token_next(cursor, diagnostics);
}

internal void
directive_section_previous
(
        Token_Cursor  *cursor,
        Diagnostics   *diagnostics,
        Symbols_Table *symbols_table
)
{
        if (symbols_table->section_previous)
        {
                // Swap them
                Section *temporary              = symbols_table->section_current;
                symbols_table->section_current  = symbols_table->section_previous;
                symbols_table->section_previous = temporary;
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Section_Previous_Undefined);
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
        }

        token_next(cursor, diagnostics);
}

internal void
directive_section_current
(
        Token_Cursor  *cursor,
        Diagnostics   *diagnostics,
        Symbols_Table *symbols_table,
        Directive_Kind directive_kind
)
{
        String8 section_name = Directive_Kind__String8_table[directive_kind];
        Symbol_Ref *symbol = Symbols_Table__get(symbols_table, section_name);
        if (symbol)
        {
                symbols_table->section_current = symbol->section;
        }

        token_next(cursor, diagnostics);
}

internal void
directive_align
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Diagnostics *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,

        B32              power_of_two_exponent,
        U8               pattern_size
)
{
        pattern_size = max_m(pattern_size, 1);
        pattern_size = min_m(pattern_size, 64);
        assert_always_m(pow_2_is_m(pattern_size) && "invalid pattern size");

        // .[p2|b]align[wl] <size> [, <pattern> [, <max_bytes>]]
        U32 location_begin = cursor->current.location;
        Alignment alignment = { .pattern_size = 1 };

        token_next(cursor, diagnostics);
        Expression *alignment_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
        SLL_queue_push_m(expressions->first, expressions->last, alignment_expression);

        expression_evaluate(alignment_expression);

        if (alignment_expression->evaluation == Expression_Kind__Constant)
        {
                U64 value = (U64)alignment_expression->integer_value;
                // By treating the value as unsigned (which we should anyway), with the next condition we check that
                // both very large values and negative ones are unsupported.
                if ((value > 32 && power_of_two_exponent) || value > U32_max)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Alignment_Larger_2_32);
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        value = 0;
                }
                B32 bytes_boundary_invalid = !power_of_two_exponent && !pow_2_is_m(value) && !value;
                if (bytes_boundary_invalid)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Alignment_Not_Power_Of_Two);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        value = 0;
                }

                alignment.boundary = power_of_two_exponent ? (1UL << value) : value;

                // Track the highest alignment requested for the section.
                if (alignment.boundary > symbols_table->section_current->elf.alignment)
                {
                        symbols_table->section_current->elf.alignment = alignment.boundary;
                }
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Constant_Expression_Expected_Lower);
                diagnostic->location   = alignment_expression->location_range.v[0];
                diagnostic->ranges[0]  = alignment_expression->location_range;
        }

        if (cursor->current.kind == Token_Kind__Comma)
        {
                // Read pattern
                token_next(cursor, diagnostics);

                if (cursor->current.kind != Token_Kind__Comma)
                {
                        Expression *pattern_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                        SLL_queue_push_m(expressions->first, expressions->last, pattern_expression);

                        U64 pattern_evaluation = (U64)expression_evaluate(pattern_expression);
                        if (pattern_expression->evaluation != Expression_Kind__Constant)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Constant_Expression_Expected_Lower);
                                diagnostic->location   = pattern_expression->location_range.v[0];
                                diagnostic->ranges[0]  = pattern_expression->location_range;
                        }

                        U64 pattern = pattern_evaluation >> (64 - pattern_size);
                        if (pattern != pattern_evaluation)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Alignment_Pattern_Too_Large);
                                diagnostic->location   = pattern_expression->location_range.v[0];
                                diagnostic->ranges[0]  = pattern_expression->location_range;
                        }

                        alignment.pattern = pattern;
                }
        }

        if (cursor->current.kind == Token_Kind__Comma)
        {
                // Read bytes_max
                token_next(cursor, diagnostics);
                Expression *write_size_max_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                SLL_queue_push_m(expressions->first, expressions->last, write_size_max_expression);

                S64 write_size_max = expression_evaluate(write_size_max_expression);
                if (write_size_max_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Constant_Expression_Expected_Lower);
                        diagnostic->location   = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0]  = write_size_max_expression->location_range;
                }
                if (write_size_max <= 0)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Alignment_Max_Non_Positive);
                        diagnostic->location   = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0]  = write_size_max_expression->location_range;
                        write_size_max = 0;
                }
                // NOTE: I don't know what should be an upper limit but there should be one probably.
                // GNU as allows you to pass zero to NOT provide one which I think can be risky.
                if (write_size_max > U32_max)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Fill_Size_Capping);
                        diagnostic->location   = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0]  = write_size_max_expression->location_range;
                        write_size_max = U32_max;
                }
                alignment.write_size_max = (U32)write_size_max;
        }

        Fragments__align(&symbols_table->section_current->fragments, location_begin, alignment);
}

internal void
directive_fill
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,

        B32              size_can_be_parsed,
        B32              pattern_can_be_parsed
)
{
        // .fill repeat [, size [, value ]]. See GNU as `s_fill` in `read.c`.
        token_next(cursor, diagnostics);
        U64 location_begin = cursor->current.location;
        Fill fill = { .pattern_size = 1 };

        Expression *repeat_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
        SLL_queue_push_m(expressions->first, expressions->last, repeat_expression);

        expression_evaluate(repeat_expression);
        fill.repeat = repeat_expression;

        if (cursor->current.kind == Token_Kind__Comma && size_can_be_parsed)
        {
                // Read size
                token_next(cursor, diagnostics);
                Expression *size_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                SLL_queue_push_m(expressions->first, expressions->last, size_expression);

                S64 fill_size = expression_evaluate(size_expression);
                B32 constant = size_expression->evaluation == Expression_Kind__Constant;
                S64 fill_capped = max_m(fill_size, 1);
                    fill_capped = min_m(fill_capped, 8);

                if (fill_size != fill_capped || !constant)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Fill_Size_1_8);
                        diagnostic->location   = cursor->current.location;
                }
                fill.pattern_size = fill_capped;
        }

        if (cursor->current.kind == Token_Kind__Comma && pattern_can_be_parsed)
        {
                // Read value
                token_next(cursor, diagnostics);
                Expression *pattern_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                SLL_queue_push_m(expressions->first, expressions->last, pattern_expression);

                U64 fill_pattern = (U64)expression_evaluate(pattern_expression);
                if (pattern_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Constant_Expression_Expected_Lower);
                        diagnostic->location   = cursor->current.location;
                }
                fill.pattern = fill_pattern;
        }
        Fragments__fill(&symbols_table->section_current->fragments, location_begin, fill);
}

// Reference: s_riscv_option
internal void
directive_option(Token_Cursor *cursor, Diagnostics *diagnostics, Arena *arena, Options *options)
{
        token_next(cursor, diagnostics);
        String8 option_text = Token_Cursor__text(cursor);

        if (String8__match_exact(option_text, String8__literal("pic")))
        {
                options->position_indipendent_code = 1;
        }
        else if (String8__match_exact(option_text, String8__literal("nopic")))
        {
                options->position_indipendent_code = 0;
        }
        else if (String8__match_exact(option_text, String8__literal("relax")))
        {
                options->relax = 1;
        }
        else if (String8__match_exact(option_text, String8__literal("norelax")))
        {
                options->relax = 0;
        }
        else if (String8__match_exact(option_text, String8__literal("push")))
        {
                Options *snapshot = Arena__push_struct_m(arena, Options);
                *snapshot = *options;
                snapshot->extensions.data = Arena__push_array_m(arena, RISCV_Extension, snapshot->extensions.count);
                memory_copy(snapshot->extensions.data, options->extensions.data, sizeof(RISCV_Extension) * options->extensions.count);
                SLL_stack_push_m(options->next, snapshot);
        }
        else if (String8__match_exact(option_text, String8__literal("pop")))
        {
                Options *top = options->next;
                if (top)
                {
                        SLL_stack_pop_m(options->next);
                        *options = *top;
                }
                else
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Option_Pop_Unmatched);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                }
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Option_Unknown);
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
        }

        token_next(cursor, diagnostics);

        return;
}

internal void
directive_size
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table
)
{
        token_next(cursor, diagnostics);
        String8 symbol_name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, symbol_name, arena);

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                token_next(cursor, diagnostics);
                Expression *expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                SLL_queue_push_m(expressions->first, expressions->last, expression);

                symbol->size_expression = expression;
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Comma_Expected);
                diagnostic->location   = cursor->current.location;
        }

        return;
}

internal void
directive_file
(
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Arena           *arena,
        Symbols_Table   *symbols_table
)
{
        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__String)
        {
                String8 text    = Token_Cursor__text(cursor);
                String8 content = String8__skip_chop(text);

                // We create it manually and explicitly omit it from the table so it can't be searched for.
                //
                // TODO(medium): this is a symbol creation which is outside the API and should be monitored, since it's
                // a delicate process and can be error prone. Ideally these cases should be taken into account in the
                // symbols table API.
                String8 *name = Arena__push_struct_m(arena, String8);
                *name = String8__duplicate_null_terminated(arena, content);
                Symbol_Ref *symbol = Arena__push_struct_m(arena, Symbol_Ref);
                SLL_queue_push_m(symbols_table->first, symbols_table->last, symbol);

                symbol->name = name;
                // I don't know precisely why, but that's what GNU as does.
                symbol->section  = &Section__absolute;
                symbol->fragment = Section__absolute.fragments.first;
                symbol->type     = ELF_Symbol_Type__File;

                token_next(cursor, diagnostics);
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__String_File_Expected);
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
        }
}

internal void
directive_type
(
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Arena           *arena,
        Symbols_Table   *symbols_table
)
{
        token_next(cursor, diagnostics);
        String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name, arena);

        // There are various syntaxes: https://www.sourceware.org/binutils/docs/as.html#g_t_002etype
        // We support `.type <name>,@<type>`, as emitted by GCC

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                token_next(cursor, diagnostics);
                if (cursor->current.kind == Token_Kind__At)
                {
                        token_next(cursor, diagnostics);
                        String8 string_type = Token_Cursor__text(cursor);
                        U8 type = ELF_Symbol_Type__from_String8(string_type);
                        symbol->type = type;

                        token_next(cursor, diagnostics);
                }
                else
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Type_Syntax_Expected);
                        diagnostic->location   = cursor->current.location;
                }
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Comma_Expected);
                diagnostic->location   = cursor->current.location;
        }
}

internal void
directive_attribute(Token_Cursor *cursor, Diagnostics *diagnostics, Arena *arena, Options *options, Section *section_first)
{
        token_next(cursor, diagnostics);

        String8 text  = Token_Cursor__text(cursor);
        RISCV_Tag tag = RISCV_Tag__find(text);

        if (tag == RISCV_Tag__None)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Attribute_Unknown);
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
        }
        else
        {
                switch (tag)
                {
                default: {} break;
                case RISCV_Tag__Architecture:
                {
                        B32 assembly_started = 0;
                        Section *section = section_first;
                        for (;;)
                        {
                                if (assembly_started || !section)
                                {
                                        break;
                                }

                                assembly_started = section->elf.flags & ELF_Section_Header_Flags__EXECINSTR
                                                && section->fragments.first != &Fragment__nil;

                                section = section->next;
                        }

                        if (assembly_started)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Attribute_After_Assembly);
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                        }


                } break;
                }

                token_next(cursor, diagnostics);
                if (cursor->current.kind == Token_Kind__Comma)
                {
                        token_next(cursor, diagnostics);
                        Token_Kind token_expected = RISCV_Tag__is_ntbs(tag) ? Token_Kind__String : Token_Kind__Number;
                        Token token = cursor->current;

                        String8 value_text    = Token_Cursor__text(cursor);
                        String8 value_content = String8__skip_chop(value_text);

                        if (token.kind == token_expected)
                        {
                                switch (tag)
                                {
                                        // TODO(architecture, medium): should be validated.
                                        case RISCV_Tag__Architecture:
                                        {
                                                String8 error = Options__architecture_parse(options, value_content, diagnostics->arena);
                                                if (error.count)
                                                {
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Architecture_Parse);
                                                        diagnostic->message    = error;
                                                        diagnostic->location   = cursor->current.location;
                                                        diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                                                }
                                                String8 duplicate = String8__duplicate(arena, value_content);
                                                options->attributes.architecture = duplicate;
                                        } break;
                                        case RISCV_Tag__Stack_Alignment:
                                        {
                                                options->attributes.stack_alignment = token.numerical_value;
                                        } break;
                                        case RISCV_Tag__Unaligned_Access:
                                        {
                                                if (token.numerical_value == 0 || token.numerical_value == 1)
                                                {
                                                        options->attributes.unaligned_access = token.numerical_value;
                                                }
                                                else
                                                {
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Attribute_Value_0_1);
                                                        diagnostic->location   = cursor->current.location;
                                                        diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                                                }
                                        } break;
                                }

                                token_next(cursor, diagnostics);
                        }
                        else
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Number_Or_String_Expected);
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = Range1_U32_m(cursor->current.location, cursor->current.size);
                        }
                }
                else
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Comma_Expected);
                        diagnostic->location   = cursor->current.location;
                }

        }
}

internal void
directive_common(Token_Cursor *cursor, Diagnostics *diagnostics, Arena *arena, Symbols_Table *symbols_table, U8 xlen)
{
        // Syntax: .comm symbol, size, [,align]
        token_next(cursor, diagnostics);
        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get(symbols_table, name);
        B32 should_be_placed_in_bss = symbol
                                   && symbol->binding == ELF_Symbol_Binding__Local
                                   && symbol->section == &Section__undefined;
        if (!symbol)
        {
                symbol = Symbols_Table__get_or_default(symbols_table, name, arena);
        }

        B32 replace_needed = (symbol->section != &Section__undefined || symbol->expression)
                        && symbol->section != &Section__common;
        B32 clonable = symbol->flags & Symbol_Flags__Volatile
                    && symbol->binding != ELF_Symbol_Binding__Weak;

        if (replace_needed)
        {
                if (clonable)
                {
                        Symbol_Ref *clone = Symbols_Table__clone(symbols_table, symbol, arena);
                                    clone->flags &= ~Symbol_Flags__Volatile;
                                    clone->expression = 0;
                        Symbol_Ref__update_section(clone, &Section__undefined);
                        should_be_placed_in_bss |= symbol->binding == ELF_Symbol_Binding__Local;

                        // TODO(low): perhaps a code smell that it should be handled better.
                        symbol->flags |= Symbol_Flags__Skip;
                        symbol = clone;
                }
                else
                {
                        Diagnostics__symbol_redefined(diagnostics, symbol, cursor->current);
                }
        }

        symbol->type = ELF_Symbol_Type__Object;

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                token_next(cursor, diagnostics);
                Expression *size_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                expression_evaluate(size_expression);
                S64 size = size_expression->integer_value;

                if (!symbol->size_expression)
                {
                        symbol->size_expression = Expression__push_constant(arena, size);
                }

                if (size_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostics__expression(diagnostics, size_expression, DG__Size_Expression_Not_Constant);
                }
                else if (size <= 0)
                {
                        Diagnostics__expression(diagnostics, size_expression, DG__Size_Expression_Not_Positive);
                }
                else if (xlen == XLEN_32 && (U64)size > U32_max)
                {
                        Diagnostics__expression(diagnostics, size_expression, DG__Size_Expression_Exceeds_32);
                }
                else if (symbol->size_expression && symbol->size_expression->integer_value != size)
                {
                        // Fix one size per object file, the linker will pick the largest among them.
                        Diagnostics__expression(diagnostics, size_expression, DG__Size_Already_Set);
                }

                U64 alignment_boundary = 1;
                for (;;)
                {
                        B32 break_should = alignment_boundary >= (U64)size || alignment_boundary >= (1 << 4);
                        if (break_should)
                        {
                                break;
                        }
                        alignment_boundary <<= 1;
                }
                if (cursor->current.kind == Token_Kind__Comma)
                {
                        token_next(cursor, diagnostics);
                        // Read alignment_boundary
                        Expression *alignment_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                        expression_evaluate(alignment_expression);
                        alignment_boundary = (U64)alignment_expression->integer_value;

                        if (alignment_expression->evaluation != Expression_Kind__Constant)
                        {
                                Diagnostics__expression(diagnostics, alignment_expression, DG__Alignment_Boundary_Not_Constant);
                        }
                        else if ((S64)alignment_boundary <= 0)
                        {
                                Diagnostics__expression(diagnostics, alignment_expression, DG__Alignment_Boundary_Not_Positive);
                        }
                        else if (!pow_2_is_m(alignment_boundary))
                        {
                                Diagnostics__expression(diagnostics, alignment_expression, DG__Alignment_Boundary_Not_Power_Of_Two);
                        }
                        else
                        {
                                symbol->value = alignment_boundary;
                        }
                }

                if (should_be_placed_in_bss)
                {
                        Symbol_Ref *symbol_bss = Symbols_Table__get(symbols_table, section_name_bss);
                        Section    *section    = symbol_bss->section;

                        // For a .bss symbol the value field is irrelevant, we use it to mark its size.
                        // symbol->value = size;

                        Alignment alignment = { .boundary = alignment_boundary, .write_size_max = 0, .pattern_size = 1 };
                        Fill fill = { .repeat = size_expression, .pattern = 0, .pattern_size = 1 };

                        if (section->elf.alignment < alignment_boundary)
                        {
                                section->elf.alignment = alignment_boundary;
                        }

                        U32 location = 0;
                        Fragments__align(&section->fragments, location, alignment);
                        Symbol_Ref__update_section(symbol, section);
                        Fragments__fill(&section->fragments, location, fill);
                }
                else
                {
                        symbol->section = &Section__common;
                        symbol->value   = alignment_boundary;
                        symbol->binding = ELF_Symbol_Binding__Global;
                        if (!symbol->size_expression)
                        {
                                symbol->size_expression = Expression__push_constant(arena, size);
                        }
                }
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Comma_Expected);
                diagnostic->location   = cursor->current.location;
        }
}

internal void
directive_ignored(Token_Cursor *cursor, Diagnostics *diagnostics)
{
        Token_Cursor backup = *cursor;

        for (;;)
        {
                B32 break_should = Token_Kind__end_of_statement(cursor->current.kind) || cursor->current.kind == Token_Kind__Error;
                if (break_should)
                {
                        break;
                }
                token_next(cursor, diagnostics);
        }

        Diagnostic *diagnostic = Diagnostics__push(diagnostics, DG__Directive_Unsupported);
        diagnostic->kind       = Diagnostic_Kind__Warning;
        diagnostic->location   = backup.current.location;
        diagnostic->ranges[0]  = Range1_U32_m(backup.current.location, backup.current.size);

        return;
}
