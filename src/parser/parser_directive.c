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

        U8 binding_old = symbol->binding;
        B32 demoted = binding < binding_old;
        if (demoted)
        {
                {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->kind       = Diagnostic_Kind__Warning;
                diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Symbol_Demoted];
                diagnostic->location   = cursor->current.location;
                diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
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
        else
        {
                symbol->binding = binding;
        }

        token_next(cursor, diagnostics, arena);
}

internal void
directive_set_like
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Sections_Table          *sections_table,
        Set_Mode                 mode
)
{
        // TODO(medium): check no conflicts with section names and register names. GNU as doesn't seem to error on using a
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

        B32 already_defined_or_equated = symbol->section->index || symbol->expression;
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

        Section *section_maybe = Sections_Table__get(sections_table, name);
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

        Expression *expression = expression_parse_with_flags
        (
                arena,
                cursor,
                expressions,
                symbols_table,
                sections_table,
                diagnostics,
                expression_flags
        );
        symbol->expression = expression;

        if (mode != Set_Mode__Strict_Forward)
        {

                S64 result = expression_evaluate(expression);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        symbol->section->index = ELF_Section_Index__Absolute;
                        symbol->value          = result;
                }
        }

        return;
}

internal void
directive_data
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Sections_Table          *sections_table,
        Fixups                  *fixups,
        U8                       data_directive_size
)
{
        // Format: .byte|half|word|dword <expr_1> , ..., <expr_n>.
        //
        // Advance to reach the first expression token.
        token_next(cursor, diagnostics, arena);
        U8 bit_size = data_directive_size * 8;
        for (;;)
        {
                Expression *expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                // We explicitly convert it to an unsigned value since this is how it's treated as.
                //
                // TODO: not very clear behaviour when in case of signed overflow.
                S64 result = expression_evaluate(expression);
                U64 result_unsigned = (U64)result;

                U8 *data = Fragments__push(&sections_table->current->fragments, cursor->current.location, data_directive_size);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        B32 fits = bit_size == 64 ? 1 : (result_unsigned < ((U64)1 << (data_directive_size * 8)));
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
                else
                {
                        U32 encoding_offset = sections_table->current->fragments.last->data_size - data_directive_size;

                        Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
                        fixup->expression = expression;
                        fixup->fragment         = sections_table->current->fragments.last;
                        fixup->encoding_offset  = encoding_offset;
                        fixup->size             = data_directive_size;

                        SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
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
}

internal void
directive_align
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Diagnostic_List *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        Sections_Table  *sections_table,

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

        token_next(cursor, diagnostics, arena);
        Expression *alignment_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
        expression_evaluate(alignment_expression);

        if (alignment_expression->evaluation == Expression_Kind__Constant)
        {
                U64 value = (U64)alignment_expression->integer_value;
                // By treating the value as unsigned (which we should anyway), with the next condition we check that
                // both very large values and negative ones are unsupported.
                if ((value > 32 && power_of_two_exponent) || value > U32_max)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->message    = String8__literal("alignment larger than 2^32 bytes");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        value = 0;
                }
                B32 bytes_boundary_invalid = !power_of_two_exponent && !pow_2_is_m(value) && !value;
                if (bytes_boundary_invalid)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("alignment boundary not a power of two");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        value = 0;
                }

                alignment.boundary = power_of_two_exponent ? (1UL << value) : value;
        }
        else
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

                if (cursor->current.kind != Token_Kind__Comma)
                {
                        Expression *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);

                        U64 pattern_evaluation = (U64)expression_evaluate(pattern_expression);
                        if (pattern_expression->evaluation != Expression_Kind__Constant)
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->message  = String8__literal("constant expression expected");
                                diagnostic->location  = pattern_expression->location_range.v[0];
                                diagnostic->ranges[0] = pattern_expression->location_range;
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        U64 pattern = pattern_evaluation >> (64 - pattern_size);
                        if (pattern != pattern_evaluation)
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->message  = String8__literal("alignment pattern larger than pattern size");
                                diagnostic->location  = pattern_expression->location_range.v[0];
                                diagnostic->ranges[0] = pattern_expression->location_range;
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        alignment.pattern = pattern;
                }
        }

        if (cursor->current.kind == Token_Kind__Comma)
        {
                // Read bytes_max
                token_next(cursor, diagnostics, arena);
                Expression *write_size_max_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                S64 write_size_max = expression_evaluate(write_size_max_expression);
                if (write_size_max_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->message  = String8__literal("constant expression expected");
                        diagnostic->location  = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = write_size_max_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
                if (write_size_max <= 0)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind     = Diagnostic_Kind__Warning;
                        diagnostic->message  = String8__literal("non-positive max alignment write size, ensuring it is zero");
                        diagnostic->location  = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = write_size_max_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        write_size_max = 0;
                }
                // NOTE: I don't know what should be an upper limit but there should be one probably.
                // GNU as allows you to pass zero to NOT provide one which I think can be risky.
                if (write_size_max > U32_max)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind     = Diagnostic_Kind__Warning;
                        diagnostic->message  = String8__literal("capping fill size to 2^31 bytes");
                        diagnostic->location  = write_size_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = write_size_max_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        write_size_max = U32_max;
                }
                alignment.write_size_max = (U32)write_size_max;
        }



        Fragments__align(&sections_table->current->fragments, location_begin, alignment);
}

internal void
directive_fill
(
        Arena           *arena,
        Token_Cursor    *cursor,
        Diagnostic_List *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        Sections_Table  *sections_table,

        B32              size_can_be_parsed,
        B32              pattern_can_be_parsed
)
{
        // .fill repeat [, size [, value ]]. See GNU as `s_fill` in `read.c`.
        token_next(cursor, diagnostics, arena);
        U64 location_begin = cursor->current.location;
        Fill fill = { .pattern_size = 1 };

        Expression *repeat_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
        expression_evaluate(repeat_expression);

        if (cursor->current.kind == Token_Kind__Comma && size_can_be_parsed)
        {
                // Read size
                token_next(cursor, diagnostics, arena);
                Expression *size_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                S64 fill_size = expression_evaluate(size_expression);
                B32 constant = size_expression->evaluation == Expression_Kind__Constant;
                S64 fill_capped = max_m(fill_size, 1);
                    fill_capped = min_m(fill_capped, 8);

                if (fill_size != fill_capped || !constant)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location = cursor->current.location;
                        diagnostic->message  = String8__literal("expected constant size expression between 1 and 8 included");
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
                fill.pattern_size = fill_capped;
        }

        if (cursor->current.kind == Token_Kind__Comma && pattern_can_be_parsed)
        {
                // Read value
                token_next(cursor, diagnostics, arena);
                Expression *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                U64 fill_pattern = (U64)expression_evaluate(pattern_expression);
                if (pattern_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location = cursor->current.location;
                        diagnostic->message  = String8__literal("constant expression expected");
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
                fill.pattern = fill_pattern;
        }
        Fragments__fill(&sections_table->current->fragments, location_begin, fill);
}
