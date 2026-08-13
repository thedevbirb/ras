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

internal void
Diagnostics__symbol_redefined(Diagnostics *diagnostics, Symbol_Ref *symbol, Token_Cursor *cursor)
{
        {
        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
        diagnostic->message    = String8__literal("symbol cannot be redefined");
        diagnostic->location   = cursor->current.location;
        diagnostic->ranges[0]  = Token__range(cursor->current);
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

internal Diagnostic *
Diagnostics__expression(Diagnostics *diagnostics, Expression *expression, String8 message)
{
        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
        diagnostic->message    = message;
        diagnostic->location   = expression->location;
        diagnostic->ranges[0]  = expression->location_range;
        return diagnostic;
}

// Handles .local, .weak, .global directive. Those simply try to set the binding of a symbol, and nothing else. It is
// created if missing.
//
// TODO(check-gas): should I just set the binding or in case of a promotion should I "delete" the other symbol and create a new
// one? This
internal void
directive_binding
(
        Token_Cursor            *cursor,
        Diagnostics             *diagnostics,
        Symbols_Table           *symbols_table,
        ELF_Symbol_Binding       binding
)
{
        token_next(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__Identifier)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
        }

        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);
        if (symbol->section == &Section__undefined)
        {
                // Still give a preliminary location for it so that we can show diagnostics.
                symbol->location = cursor->current.location;
        }

        U8 binding_old = symbol->binding;
        B32 demoted = binding < binding_old;
        if (demoted)
        {
                Diagnostics__symbol_redefined(diagnostics, symbol, cursor);
        }

        symbol->binding = binding;
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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
        }

        // What if you set the name of a section or a register? Well, for a section, you can't since a section is a
        // symbol. For registers, it is indeed possible and could be error prone, but in practice the assembler is
        // always able to discriminate between a register and a symbol named in the same way due to its position in the
        // instruction.
        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

        B32 already_defined_or_equated = symbol->section != &Section__undefined || symbol->expression;
        if (already_defined_or_equated)
        {
                B32 frozen = mode != Set_Mode__Override
                          || !(symbol->flags & Symbol_Flags__Volatile);
                if (frozen)
                {
                        Diagnostics__symbol_redefined(diagnostics, symbol, cursor);
                }

                symbol = Symbols_Table__create(symbols_table, name);
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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location   = cursor->current.location;
                diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
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
                Expression *expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                SLL_queue_push_m(expressions->first, expressions->last, expression);
                expressions_count += 1;
                // We explicitly convert it to an unsigned value since this is how it's treated as.
                //
                // TODO: not very clear behaviour when in case of signed overflow.
                S64 result = expression_evaluate(expression);
                U64 result_unsigned = (U64)result;

                U8 *data = Fragments__push(&symbols_table->section_current->fragments, cursor->current.location, data_directive_size);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        B32 fits = bit_size == 64 ? 1 : (result_unsigned < ((U64)1 << (data_directive_size * 8)));
                        memory_copy(data, (U8 *)&result_unsigned, data_directive_size);

                        if (!fits)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->kind      = Diagnostic_Kind__Warning;
                                diagnostic->location  = expression->location_range.v[0];
                                diagnostic->message   = String8__literal("value too large, truncated");
                                diagnostic->ranges[0] = expression->location_range;
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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
        }

        // Can be of the form "\nhello\n", so with quotes and optional escaped characters.
        String8 text     = Token_Cursor__text(cursor);
        String8 content  = String8__skip_chop(text);
        U64 size_escaped = String8__escaped_size(content) + !!null_terminated;

        U8 *data = Fragments__push(&section->fragments, cursor->current.location, size_escaped);
        bytes_escaped_fill(content, data, size_escaped - !!null_terminated);

        token_next(cursor, diagnostics);
}

internal void
directive_section
(
        Token_Cursor  *cursor,
        Diagnostics   *diagnostics,
        Symbols_Table *symbols_table
)
{
        // Syntax: `.section name [, "flags"[, @type[, argument...]]]`

        U32 location_start = cursor->current.location;

        // We essentially accept anything that is not an end of statement or a comma.
        U8      ending_bytes_set_data[] = {' ', ',', ';', '\n'};
        String8 ending_bytes_set        = String8__new(ending_bytes_set_data, array_count_m(ending_bytes_set_data));
        B32     skip_initial_whitespace = 1;

        Token_Cursor__read_raw_identifier_until(cursor, ending_bytes_set, skip_initial_whitespace);
        String8 name = Token_Cursor__text(cursor);
        if (!name.count)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location   = location_start;
                diagnostic->message    = String8__literal("section directive has empty name");
                diagnostic->ranges[0]  = (Range1_U32){{ location_start, cursor->current.location + cursor->current.size }};
        }

        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

        B32 new_is = symbol->section == &Section__undefined;
        if (new_is)
        {
                Symbols_Table__create_section(symbols_table, symbol);
                DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol->section);
        }

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                // Error if already defined.
                if (!new_is)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = String8__literal("cannot redefine section");
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }

                // Read flags.
                token_next(cursor, diagnostics);
                if (cursor->current.kind != Token_Kind__String)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location = cursor->current.location;
                        diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
                }
                String8 text    = Token_Cursor__text(cursor);
                String8 content = String8__skip_chop(text);
                ELF_Section_Header_Flags flags = ELF_Section_Header_Flags__parse(content);

                if (flags == ELF_Section_Header_Flags__Invalid)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Error;
                        diagnostic->message    = String8__literal("invalid section flags, expected: " ELF_Section_Header_Flags__cstring);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }

                if (symbol->section->special && symbol->section->elf.flags != flags)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("ignoring redefinition of flags for special section");
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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = String8__literal("invalid section type syntax, expected @type");
                }
                token_next(cursor, diagnostics);
                String8 content = Token_Cursor__text(cursor);
                ELF_Section_Header_Type type = ELF_Section_Header_Type__from_String8(content);
                if (type == ELF_Section_Header_Type__Invalid)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Error;
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = String8__literal("invalid section type");
                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }
                if (symbol->section->special && symbol->section->elf.type != type)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("ignoring redefinition of type for special section");
                        diagnostic->location   = cursor->current.location;
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }
                else
                {
                        symbol->section->elf.type = type;
                }
                token_next(cursor, diagnostics);
        }

        symbols_table->section_current = symbol->section;
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
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, section_name);
        if (symbol->section == &Section__undefined)
        {
                Symbols_Table__create_section(symbols_table, symbol);
        }

        symbols_table->section_current = symbol->section;

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
        Alignment alignment = {0};

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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->message    = String8__literal("alignment larger than 2^32 bytes");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        value = 0;
                }
                B32 bytes_boundary_invalid = !power_of_two_exponent && !pow_2_is_m(value) && !value;
                if (bytes_boundary_invalid)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("alignment boundary not a power of two");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        value = 0;
                }

                alignment.boundary = power_of_two_exponent ? (1UL << value) : value;
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message   = String8__literal("constant expression expected");
                diagnostic->location  = alignment_expression->location_range.v[0];
                diagnostic->ranges[0] = alignment_expression->location_range;
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
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message  = String8__literal("constant expression expected");
                                diagnostic->location  = pattern_expression->location_range.v[0];
                                diagnostic->ranges[0] = pattern_expression->location_range;
                        }

                        U64 pattern = pattern_evaluation >> (64 - pattern_size);
                        if (pattern != pattern_evaluation)
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message  = String8__literal("alignment pattern larger than pattern size");
                                diagnostic->location  = pattern_expression->location_range.v[0];
                                diagnostic->ranges[0] = pattern_expression->location_range;
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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->message  = String8__literal("constant expression expected");
                        diagnostic->location  = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = write_size_max_expression->location_range;
                }
                if (write_size_max <= 0)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind     = Diagnostic_Kind__Warning;
                        diagnostic->message  = String8__literal("non-positive max alignment write size, ensuring it is zero");
                        diagnostic->location  = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = write_size_max_expression->location_range;
                        write_size_max = 0;
                }
                // NOTE: I don't know what should be an upper limit but there should be one probably.
                // GNU as allows you to pass zero to NOT provide one which I think can be risky.
                if (write_size_max > U32_max)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind     = Diagnostic_Kind__Warning;
                        diagnostic->message  = String8__literal("capping fill size to 2^31 bytes");
                        diagnostic->location  = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = write_size_max_expression->location_range;
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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location = cursor->current.location;
                        diagnostic->message  = String8__literal("expected constant size expression between 1 and 8 included");
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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location = cursor->current.location;
                        diagnostic->message  = String8__literal("constant expression expected");
                }
                fill.pattern = fill_pattern;
        }
        Fragments__fill(&symbols_table->section_current->fragments, location_begin, fill);
}

// Reference: s_riscv_option
internal void
directive_option(Token_Cursor *cursor, Diagnostics *diagnostics, Options *options)
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
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__literal("unknown option");
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Token__range(cursor->current);
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
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, symbol_name);

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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location   = cursor->current.location;
                diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
        }

        return;
}

internal void
directive_file
(
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
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
                String8 *name = Arena__push_struct_m(symbols_table->arena, String8);
                *name = String8__duplicate_null_terminated(symbols_table->arena, content);
                Symbol_Ref *symbol = Arena__push_struct_m(symbols_table->arena, Symbol_Ref);
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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__literal("expected string file");
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Token__range(cursor->current);
        }
}

internal void
directive_type
(
        Token_Cursor    *cursor,
        Diagnostics     *diagnostics,
        Symbols_Table   *symbols_table
)
{
        token_next(cursor, diagnostics);
        String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = String8__literal("`.type <name>,@<type>` syntax expected");
                }
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location   = cursor->current.location;
                diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
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
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__literal("unknown attribute");
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = Token__range(cursor->current);
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
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message    = String8__literal("cannot set this attribute after assembly started");
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = Token__range(cursor->current);
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
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                        diagnostic->message    = String8__literal("invalid value for attributes, must be either 0 or 1");
                                                        diagnostic->location   = cursor->current.location;
                                                        diagnostic->ranges[0]  = Token__range(cursor->current);
                                                }
                                        } break;
                                }

                                token_next(cursor, diagnostics);
                        }
                        else
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message    = String8__literal("expected number or string, depending on attribute");
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = Token__range(cursor->current);
                        }
                }
                else
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->message    = String8__literal("comma expected");
                        diagnostic->location   = cursor->current.location;
                }

        }
}

internal void
directive_common(Token_Cursor *cursor, Diagnostics *diagnostics, Arena *arena, Symbols_Table *symbols_table)
{
        // Syntax: .comm symbol, size, [,align]
        token_next(cursor, diagnostics);
        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

        B32 replace_needed = (symbol->section != &Section__undefined || symbol->expression)
                        && symbol->section != &Section__common;
        B32 clonable = symbol->flags & Symbol_Flags__Volatile;
        if (replace_needed)
        {
                if (clonable)
                {
                        Symbol_Ref *clone = Symbols_Table__clone(symbols_table, symbol);
                                    clone->flags &= ~Symbol_Flags__Volatile;
                                    clone->expression = 0;
                        Symbol_Ref__update_section(clone, &Section__undefined);
                }
                else
                {
                        Diagnostics__symbol_redefined(diagnostics, symbol, cursor);
                }
        }

        symbol->type = ELF_Symbol_Type__Object;

        token_next(cursor, diagnostics);
        if (cursor->current.kind == Token_Kind__Comma)
        {
                // TODO(32-bit): check that size is in 32-bit range
                Expression *size_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                expression_evaluate(size_expression);
                S64 size = size_expression->integer_value;

                if (clonable)
                {
                        symbol->size = size;
                }

                if (size_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostics__expression(diagnostics, size_expression, String8__literal("size expression expected to have constant evaluation"));
                }
                else if (size <= 0)
                {
                        Diagnostics__expression(diagnostics, size_expression, String8__literal("size expression expected to have positive evaluation"));
                }
                else if ((U64)size != symbol->size)
                {
                        Diagnostic *diagnostic = Diagnostics__expression(diagnostics, size_expression, String8__literal("size already set, not changing it"));
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                }

                if (cursor->current.kind == Token_Kind__Comma)
                {
                        // Read alignment
                        Expression *alignment_expression = expression_parse(arena, cursor, symbols_table, diagnostics);
                        expression_evaluate(alignment_expression);
                        U64 alignment = (U64)alignment_expression->integer_value;

                        if (alignment_expression->evaluation != Expression_Kind__Constant)
                        {
                                Diagnostics__expression(diagnostics, alignment_expression, String8__literal("alignment expression expected to have constant evaluation"));
                        }
                        else if ((S64)alignment <= 0)
                        {
                                Diagnostics__expression(diagnostics, alignment_expression, String8__literal("alignment expression expected to have positive evaluation"));
                        }
                        else if (!pow_2_is_m(alignment))
                        {
                                Diagnostics__expression(diagnostics, alignment_expression, String8__literal("alignment is not a power of two"));
                        }
                        else
                        {
                                symbol->value = alignment;
                        }
                }

                B32 should_be_placed_in_bss = symbol->binding == ELF_Symbol_Binding__Local
                                          && symbol->section == &Section__undefined;
                if (should_be_placed_in_bss)
                {

                }
        }
        else
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->message    = String8__literal("comma expected");
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

        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
        diagnostic->kind       = Diagnostic_Kind__Warning;
        diagnostic->message    = String8__literal("directive unsupported, skipping");
        diagnostic->location   = backup.current.location;
        diagnostic->ranges[0]  = Token__range(backup.current);

        return;
}
