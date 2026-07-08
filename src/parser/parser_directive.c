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
// TODO: should I just set the binding or in case of a promotion should I "delete" the other symbol and create a new
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
                diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
                {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->kind       = Diagnostic_Kind__Note;
                diagnostic->message    = Diagnostic__previous_declaration_String8;
                diagnostic->location   = symbol->location;
                diagnostic->ranges[0] = (Range1_U32){{ symbol->location, symbol->location + name.count }};
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
        Section                 *section,
        Sections_Table          *section_table,
        Set_Mode                 mode
)
{
        // TODO: check no conflicts with section names and register names. GNU as doesn't seem to error on using a
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

        B32 already_defined_or_equated = symbol->elf.section_index || symbol->expression_index;
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

        Section *section_maybe = Sections_Table__get(section_table, name);
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
                section,
                diagnostics,
                expression_flags
        );
        symbol->expression_index = expression->index;

        if (mode != Set_Mode__Strict_Forward)
        {

                S64 result = expression_evaluate(expressions, expression->index);
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
        Section                 *section,
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
                Expression_Node *expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                // We explicitly convert it to an unsigned value since this is how it's treated as.
                //
                // TODO: not very clear behaviour when in case of signed overflow.
                S64 result = expression_evaluate(expressions, expression->index);
                U64 result_unsigned = (U64)result;

                U8 *data = Fragment_List__fixed(&section->fragment_list, section->arena, cursor->current.location, data_directive_size);
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
                        U32 encoding_offset = section->fragment_list.last->size_fixed - data_directive_size;

                        Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
                        fixup->expression_index = expression->index;
                        fixup->fragment         = section->fragment_list.last;
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
