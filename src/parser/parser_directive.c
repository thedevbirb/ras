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

        U8 binding_old = ELF_Symbol_bind_m(symbol->elf.type_and_binding);
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
                U8 type_and_binding = ELF_Symbol_info_m(binding, ELF_Symbol_type_m(symbol->elf.type_and_binding));
                symbol->elf.type_and_binding = type_and_binding;
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

        B32 already_defined_or_equated = symbol->elf.section_index || symbol->expression_node;
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

        Expression_Node *expression = expression_parse_with_flags
        (
                arena,
                cursor,
                expressions,
                symbols_table,
                sections_table,
                diagnostics,
                expression_flags
        );
        symbol->expression_node = expression;

        if (mode != Set_Mode__Strict_Forward)
        {

                S64 result = expression_evaluate(expression);
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        symbol->elf.section_index = ELF_Section_Index__Absolute;
                        symbol->elf.value         = result;
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
                Expression_Node *expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                // We explicitly convert it to an unsigned value since this is how it's treated as.
                //
                // TODO: not very clear behaviour when in case of signed overflow.
                S64 result = expression_evaluate(expression);
                U64 result_unsigned = (U64)result;

                U8 *data = Fragment_List__fixed(&sections_table->current->fragment_list, sections_table->current->arena, cursor->current.location, data_directive_size);
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
                        U32 encoding_offset = sections_table->current->fragment_list.last->size_fixed - data_directive_size;

                        Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
                        fixup->expression_node = expression;
                        fixup->fragment         = sections_table->current->fragment_list.last;
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
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Sections_Table          *sections_table
)
{
        // .align <size> [, <pattern> [, <max_bytes>]]
        //
        // TODO(low): support omitting some values, e.g. .align 2, , 8
        //
        // .align is implementation-defined, in this case we interpret the next expression as a power of
        // two. See also .p2align.
        // For this expression, note that a label difference is allowed but there should be no expansion
        // between them. Probably a good way to check is making sure both are defined within the same
        // fragment
        U32 location_begin = cursor->current.location;
        U8  pattern   = 0;
        U32 bytes_max = 0;

        token_next(cursor, diagnostics, arena);
        Expression_Node *alignment_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
        expression_evaluate( alignment_expression);

        if (alignment_expression->evaluation == Expression_Kind__Constant)
        {
                if (alignment_expression->integer_value < 0)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("negative alignment, converted to one");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        alignment_expression->integer_value = 1;
                }
                if (alignment_expression->integer_value > (S64)(2UL << 31))
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("alignment larger than 2^31, capping it");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        alignment_expression->integer_value = (2UL << 31);
                }
                B32 power_of_two = (alignment_expression->integer_value & ~(alignment_expression->integer_value)) == 0;
                if (!power_of_two)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__literal("alignment not power of two, setting it to next one");
                        diagnostic->location   = alignment_expression->location_range.v[0];
                        diagnostic->ranges[0]  = alignment_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        alignment_expression->integer_value = align_pow_2_m(alignment_expression->integer_value, 2);
                }
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

                Expression_Node *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);

                S64 pattern_evaluation = expression_evaluate( pattern_expression);
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
                Expression_Node *bytes_max_expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                S64 bytes_max_evaluation = expression_evaluate( bytes_max_expression);
                if (bytes_max_expression->evaluation != Expression_Kind__Constant)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->message  = String8__literal("constant expression expected");
                        diagnostic->location  = bytes_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = bytes_max_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
                bytes_max = (U32)bytes_max_evaluation;

                if (bytes_max_evaluation <= 0)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind     = Diagnostic_Kind__Warning;
                        diagnostic->message  = String8__literal("non-positive max bytes size, ensuring it is zero");
                        diagnostic->location  = bytes_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = bytes_max_expression->location_range;
                        // TODO(low): nicer diagnostic with vsnprintf support in String8
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        bytes_max = 0;
                }
                // NOTE: I don't know what should be an upper limit but there should be one probably.
                // GNU as allows you to pass zero to NOT provide one which I think can be risky.
                if (bytes_max_evaluation > (S64)(1UL << 31))
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->kind     = Diagnostic_Kind__Warning;
                        diagnostic->message  = String8__literal("capping fill size to 2^31 bytes");
                        diagnostic->location  = bytes_max_expression->location_range.v[0];
                        diagnostic->ranges[0] = bytes_max_expression->location_range;
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        bytes_max = (1UL << 31);
                }
        }

        if ((sections_table->current->elf.flags & ELF_Section_Header_Flags__EXECINSTR) != 0)
        {
                // TODO(low): notify that pattern is ignored in case of .align code?
                Fragment_List__align_code(&sections_table->current->fragment_list, sections_table->current->arena, location_begin, alignment_expression->integer_value, bytes_max);
        }
        else
        {
                Fragment_List__align(&sections_table->current->fragment_list, sections_table->current->arena, location_begin, alignment_expression->integer_value, pattern, bytes_max);
        }
}
