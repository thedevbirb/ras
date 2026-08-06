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
//
// TODO(check-gas): should I just set the binding or in case of a promotion should I "delete" the other symbol and create a new
// one? This
internal void
binding_set
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
                {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->kind       = Diagnostic_Kind__Warning;
                diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Symbol_Demoted];
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                }
                {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->kind       = Diagnostic_Kind__Note;
                diagnostic->message    = Diagnostic__previous_declaration_String8;
                diagnostic->location   = symbol->location;
                diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + name.count }};
                }
        }

        symbol->binding = binding;
        token_next(cursor, diagnostics);
}

internal void
directive_set_like
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostics         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Set_Mode                 mode
)
{
        // TODO(medium): check no conflicts with section names and register names. GNU as doesn't seem to error on using a
        // register name like `sp` though, which I think can be quite confusing/error prone.

        token_next(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__Identifier)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
        }

        String8 name = Token_Cursor__text(cursor);
        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

        B32 already_defined_or_equated = symbol->section != &Section__undefined || symbol->expression;
        if (already_defined_or_equated)
        {
                B32 frozen = mode != Set_Mode__Override || !(symbol->flags & Symbol_Flags__Volatile);
                if (frozen)
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
                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + name.count }};
                        }
                }

                symbol = Symbols_Table__clone(symbols_table, symbol);
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
                diagnostic->location = cursor->current.location;
                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
        }

        Expression *expression = expression_parse_with_flags
        (
                arena,
                cursor,
                expressions,
                symbols_table,
                diagnostics,
                expression_flags
        );
        symbol->expression = expression;

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
                Expression *expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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
        U64 size         = content.count;
        U64 size_escaped = String8__escaped_size(content) + !!null_terminated;

        U8 *data = Fragments__push(&section->fragments, cursor->current.location, size_escaped);
        bytes_escaped_fill(content, data, size);

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
        Expression *alignment_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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
                        Expression *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);

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
                Expression *write_size_max_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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

        Expression *repeat_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
        expression_evaluate(repeat_expression);
        fill.repeat = repeat_expression;

        if (cursor->current.kind == Token_Kind__Comma && size_can_be_parsed)
        {
                // Read size
                token_next(cursor, diagnostics);
                Expression *size_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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
                Expression *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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
                Expression *expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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
