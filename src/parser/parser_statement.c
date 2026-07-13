internal void
statement_read
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Section                 *section,
        Sections_Table          *sections_table,
        Fixups                  *fixups
)
{

        U32 source_index_start = cursor->source_index;
        token_next(cursor, diagnostics, arena);

        B32 progress = 1;
        B32 error =  0;

        for (;;)
        {
                Directive_Kind directive_kind         = 0;
                U32            instruction_hash       = 0;
                B32            null_terminated_string = 0;
                B32            empty_line             = 0;

                progress = source_index_start < cursor->source_index;
                B32 break_should_outer = cursor->current.kind == Token_Kind__None
                                      || cursor->current.kind == Token_Kind__Error
                                      || error;
                assert_always_m((progress || break_should_outer) && "infinite loop detected");

                if (break_should_outer)
                {
                        break;
                }

                switch (cursor->current.kind)
                {
                // no-op, continue;
                case Token_Kind__Newline:
                {
                        token_next(cursor, diagnostics, arena);
                        empty_line = 1;
                } break;
                case Token_Kind__Semicolon:
                {
                        token_next(cursor, diagnostics, arena);
                        empty_line = 1;
                } break;
                case Token_Kind__Number:
                {
                        Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);

                        // Should be a numeric label definition, e.g. 1:
                        U32 number = U32_cast_safe(cursor->current.numerical_value);
                        token_next(cursor, diagnostics, arena);
                        B32 label_definition = cursor->current.kind == Token_Kind__Colon;
                        if (label_definition)
                        {
                                token_next(cursor, diagnostics, arena);
                        }
                        else
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->message    = String8__literal("expected ':' for numeric label declaration");
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        Label_Numeric *label_numeric = Symbols_Table__label_numeric_get_or_default(symbols_table, number);
                                       label_numeric->instances += 1;
                        String8        label_name    = label_numeric_string(scratch.arena, *label_numeric);
                        Symbol_Ref    *label         = Symbols_Table__get_or_default(symbols_table, label_name);

                        assert_always_m(label->elf.section_index == 0 && "numeric label created previously");
                        Symbol_Ref__update_section(label, section);

                        Arena__scratch_end_m(scratch);
                } break;
                // Instructions, directives and label start with an identifier. We have to discriminate further.
                case Token_Kind__Identifier:
                {

                        String8 identifier = Token_Cursor__text(cursor);

                        B32 dot_start = identifier.data[0] == '.';
                        if (dot_start)
                        {
                                directive_kind = Directive_Kind__from_String8(identifier);
                        }

                        Token next = token_peek(cursor, diagnostics, arena);
                        B32 label_found = next.kind == Token_Kind__Colon;
                        if (label_found)
                        {
                                // Label declaration!
                                Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, identifier);
                                if (symbol->fragment)
                                {
                                        // NOTE: GNU as accepts the case where the fragment is the same AND same offset.
                                        {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Label_Duplicate];
                                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                        {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind       = Diagnostic_Kind__Note;
                                        diagnostic->location   = symbol->location;
                                        diagnostic->message    = Diagnostic__previous_declaration_String8;
                                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + identifier.count }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                }
                                symbol->flags            |= Symbol_Flags__Declared;
                                symbol->location          = cursor->current.location;
                                symbol->fragment          = section->fragment_list.last;
                                symbol->elf.value         = section->fragment_list.last->size_fixed;
                                symbol->elf.section_index = section->index;

                                token_next(cursor, diagnostics, arena);
                                token_next(cursor, diagnostics, arena);
                        }

                        B32 instruction_expected = !label_found && !directive_kind;
                        if (instruction_expected)
                        {
                                instruction_hash = FNV_hash_U32(identifier);
                        }
                } break;
                default:
                {
                        // Sort of catch-all
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Line_Invalid];
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        error = 1;
                } break;
                }

                if (instruction_hash)
                {
                        U16               relocation  =  0;
                        RISCV_Instruction instruction = {0};

                        Expression_Node *expression_node_parsed = 0;
                        RISCV_Instruction__parse
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                instruction_hash,
                                &relocation,
                                &instruction,
                                &expression_node_parsed
                        );

                        if (instruction.opcode->info & INSN_MACRO)
                        {
                                RISCV_instruction_pseudo_append
                                (
                                        arena,
                                        section,
                                        fixups,
                                        expressions,
                                        symbols_table,
                                        &instruction,
                                        expression_node_parsed,
                                        relocation
                                );
                        }
                        else
                        {
                                RISCV_Instruction__append
                                (
                                        section,
                                        fixups,
                                        &instruction,
                                        expression_node_parsed,
                                        relocation
                                );
                        }
                }

                S64  fill_size           = 0;
                B32  fill_size_set       = 0;
                S64  fill_pattern        = 0;
                B32  fill_pattern_set    = 0;

                switch (directive_kind)
                {
                case Directive_Kind__None: {} break;

                case Directive_Kind__Word_Double:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, section, fixups, 8);
                } break;
                case Directive_Kind__Word:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, section, fixups, 4);
                } break;
                case Directive_Kind__Word_Half:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, section, fixups, 2);
                } break;
                case Directive_Kind__Byte:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, section, fixups, 1);
                } break;
                case Directive_Kind__String: {} // fallthrough
                case Directive_Kind__Asciz:  { null_terminated_string = 1; } // fallthrough
                case Directive_Kind__Ascii:
                {
                        token_next(cursor, diagnostics, arena);
                        if (cursor->current.kind != Token_Kind__String)
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->location = cursor->current.location;
                                diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        // Can be of the form `"\nhello\n", so with quotes and optional escaped characters.
                        String8 text = Token_Cursor__text(cursor);
                        text = String8__skip(text, 1);
                        text = String8__chop(text, 1);
                        U32 size_escaped = String8__escaped_size(text) + !!null_terminated_string;

                        U8 *data = Fragment_List__fixed(&section->fragment_list, section->arena, cursor->current.location, size_escaped);
                        bytes_escaped_fill(text, data, size_escaped);

                        token_next(cursor, diagnostics, arena);
                } break;
                case Directive_Kind__Section:
                {
                        // Syntax: `.section name [, "flags"[, @type[, argument...]]]`
                        token_next(cursor, diagnostics, arena);
                        String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
                        Section *section_new = Sections_Table__get_or_default(sections_table, name, cursor->current.location);

                        token_next(cursor, diagnostics, arena);
                        if (cursor->current.kind == Token_Kind__Comma)
                        {
                                // Read flags.
                                token_next(cursor, diagnostics, arena);
                                if (cursor->current.kind != Token_Kind__String)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                String8 text    = Token_Cursor__text(cursor);
                                String8 content = String8__skip_chop(text);
                                ELF_Section_Header_Flags flags = ELF_Section_Header_Flags__parse(content);

                                if (flags == ELF_Section_Header_Flags__Invalid)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind       = Diagnostic_Kind__Error;
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("invalid section flags, expected: " ELF_Section_Header_Flags__cstring);
                                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                // TODO(medium, check-gas): are they ORed? Or overwritten?
                                section_new->elf.flags = flags;
                                token_next(cursor, diagnostics, arena);
                        }

                        if (cursor->current.kind == Token_Kind__Comma)
                        {
                                // Parse type.
                                token_next(cursor, diagnostics, arena);
                                if (cursor->current.kind != Token_Kind__At)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("invalid section type syntax, expected @type");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                token_next(cursor, diagnostics, arena);
                                String8 text    = Token_Cursor__text(cursor);
                                String8 content = String8__skip_chop(text);
                                ELF_Section_Header_Type type = ELF_Section_Header_Type__from_String8(content);
                                if (type == ELF_Section_Header_Type__Invalid)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind       = Diagnostic_Kind__Error;
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("invalid section type");
                                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                section_new->elf.type = type;
                                token_next(cursor, diagnostics, arena);
                        }


                        section = section_new;
                } break;
                case Directive_Kind__Local:
                {
                        binding_set(arena, cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Local);
                } break;
                case Directive_Kind__Weak:
                {
                        binding_set(arena, cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Weak);
                } break;
                case Directive_Kind__Globl: {} // fallthrough
                case Directive_Kind__Global:
                {
                        binding_set(arena, cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Global);
                } break;
                // TODO(low): support for `<identifier> = <expr>` could be added by jumping here.
                case Directive_Kind__Set: {} // fallthrough
                case Directive_Kind__Equality:
                {
                        S32 mode = 0;
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                sections_table,
                                mode
                        );
                } break;
                case Directive_Kind__Equiv:
                {
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                sections_table,
                                Set_Mode__Strict
                        );
                } break;
                case Directive_Kind__Eqv:
                {
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section,
                                sections_table,
                                Set_Mode__Strict_Forward
                        );
                } break;
                case Directive_Kind__Zero:
                {
                        // Equavalent to .fill repeat, 1, 0
                        fill_pattern     = 0;
                        fill_pattern_set = 1;
                } // fallthrough
                case Directive_Kind__Space:
                {
                        // Equavalent to .fill repeat, 1, value
                } // fallthrough
                case Directive_Kind__Skip:
                {
                        // Equavalent to .fill repeat, 1, value
                        fill_size     = 1;
                        fill_size_set = 1;
                } // fallthrough
                case Directive_Kind__Fill:
                {
                        // .fill repeat [, size [, value ]]. See GNU as `s_fill` in `read.c`.
                        token_next(cursor, diagnostics, arena);
                        U64 location_begin = cursor->current.location;

                        Expression_Node *repeat_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);

                        if (cursor->current.kind == Token_Kind__Comma && !fill_size_set)
                        {
                                // Read size
                                token_next(cursor, diagnostics, arena);
                                Expression_Node *size_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                fill_size = expression_evaluate(size_expression);
                                if (size_expression->evaluation != Expression_Kind__Constant)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("constant expression expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                                if (fill_size <= 0)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind      = Diagnostic_Kind__Warning;
                                        diagnostic->location  = size_expression->location_range.v[0];
                                        diagnostic->ranges[0] = size_expression->location_range;
                                        // TODO(low): nicer diagnostic with vsnprintf support in String8
                                        diagnostic->message   = String8__literal("non-positive fill size, ensuring it is zero");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        fill_size = 0;
                                }
                                if (fill_size > 8)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->kind     = Diagnostic_Kind__Warning;
                                        diagnostic->location  = size_expression->location_range.v[0];
                                        diagnostic->ranges[0] = size_expression->location_range;
                                        diagnostic->message  = String8__literal("capping fill size to 8 bytes");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        fill_size = 8;
                                }
                        }

                        if (cursor->current.kind == Token_Kind__Comma && !fill_pattern_set)
                        {
                                // Read value
                                token_next(cursor, diagnostics, arena);
                                Expression_Node *value_expression = expression_parse(arena, cursor, expressions, symbols_table, section, diagnostics);
                                fill_pattern = expression_evaluate(value_expression);
                                if (value_expression->evaluation != Expression_Kind__Constant)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("constant expression expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        }
                        Fragment_List__fill(&section->fragment_list, section->arena, location_begin, repeat_expression, 0, fill_pattern, fill_size);
                } break;
                case Directive_Kind__Align:
                {
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                section
                        );
                } break;
                default: {} break;
                }

                // Find end of line junk
                U32 junk_location_begin = cursor->current.location;
                U32 junk_location_end   = 0;
                for (;;)
                {
                        Token_Kind kind = cursor->current.kind;
                        B32 break_should = empty_line
                                || kind == Token_Kind__None
                                || kind == Token_Kind__Error
                                || kind == Token_Kind__Newline
                                || kind == Token_Kind__Semicolon;
                        if (break_should)
                        {
                                break;
                        }
                        junk_location_end = cursor->current.location + cursor->current.size;
                        token_next(cursor, diagnostics, arena);
                }

                if (junk_location_end)
                {
                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                        diagnostic->location   = junk_location_begin;
                        diagnostic->message    = String8__literal("junk found at the end of line");
                        diagnostic->ranges[0]  = (Range1_U32){{ junk_location_begin, junk_location_end }};
                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                }
        }


        return;
}

