internal void
statement_read
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostics             *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        RISCV_Options           *options
)
{

        U32 source_index_start = cursor->source_index;
        token_next(cursor, diagnostics);

        B32 progress = 1;
        B32 error =  0;

        for (;;)
        {
                Directive_Kind directive_kind         = 0;
                U32            instruction_hash       = 0;
                B32            null_terminated_string = 0;
                B32            empty_line             = 0;
                B32            label_found            = 0;

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
                        token_next(cursor, diagnostics);
                        empty_line = 1;
                } break;
                case Token_Kind__Semicolon:
                {
                        token_next(cursor, diagnostics);
                        empty_line = 1;
                } break;
                case Token_Kind__Number:
                {
                        Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);

                        // Should be a numeric label definition, e.g. 1:
                        U32 number = U32_cast_safe(cursor->current.numerical_value);
                        token_next(cursor, diagnostics);
                        label_found = cursor->current.kind == Token_Kind__Colon;
                        if (label_found)
                        {
                                token_next(cursor, diagnostics);
                        }
                        else
                        {
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message    = String8__literal("expected ':' for numeric label declaration");
                                diagnostic->location   = cursor->current.location;
                                diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                        }

                        Symbol_Numeric symbol_numeric = Symbols_Table__get_or_default_numeric(symbols_table, number, 1);
                                       symbol_numeric.label->instances += 1;

                        assert_always_m(symbol_numeric.symbol->section == &Section__undefined && "numeric label created previously");

                        symbol_numeric.symbol->location  = cursor->current.location;
                        // symbol_numeric.symbol->flags    |= Symbol_Flags__Used;
                        Symbol_Ref__update_section(symbol_numeric.symbol, symbols_table->section_current);

                        Arena__scratch_end_m(scratch);
                } break;
                case Token_Kind__Identifier:
                {
                        // Instructions, directives and label start with an identifier. We have to discriminate further.

                        String8 identifier = Token_Cursor__text(cursor);

                        B32 dot_start = identifier.data[0] == '.';
                        if (dot_start)
                        {
                                // TODO(high): common section directive like `.text`, `.data` are unsupported!
                                directive_kind = Directive_Kind__from_String8(identifier);
                        }

                        Token next = token_peek(cursor, diagnostics);
                        label_found = next.kind == Token_Kind__Colon;
                        if (label_found)
                        {
                                Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, identifier);
                                if (symbol->section != &Section__undefined)
                                {
                                        // NOTE: GNU as accepts the case where the fragment is the same AND same offset.
                                        // It also accepts defining the symbol via `.set`, and then as a label.
                                        {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Label_Duplicate];
                                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                        }
                                        {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->kind       = Diagnostic_Kind__Note;
                                        diagnostic->location   = symbol->location;
                                        diagnostic->message    = Diagnostic__previous_declaration_String8;
                                        diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + identifier.count }};
                                        }
                                }
                                symbol->location       = cursor->current.location;
                                // symbol->flags         |= Symbol_Flags__Used;
                                Symbol_Ref__update_section(symbol, symbols_table->section_current);

                                token_next(cursor, diagnostics);
                                token_next(cursor, diagnostics);
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
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Line_Invalid];
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                        error = 1;
                } break;
                }

                if (instruction_hash)
                {
                        U16               relocation  =  0;
                        RISCV_Instruction instruction = {0};

                        Expression *expression_parsed = 0;
                        RISCV_Instruction__parse
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                instruction_hash,
                                &relocation,
                                &instruction,
                                &expression_parsed
                        );

                        if (instruction.opcode->info & INSN_MACRO)
                        {
                                RISCV_instruction_pseudo_append
                                (
                                        symbols_table->section_current,
                                        expressions,
                                        symbols_table,
                                        options,
                                        &instruction,
                                        expression_parsed,
                                        relocation
                                );
                        }
                        else
                        {
                                RISCV_Instruction__append
                                (
                                        symbols_table->arena,
                                        symbols_table->section_current,
                                        options,
                                        &instruction,
                                        expression_parsed,
                                        relocation
                                );
                        }
                }

                switch (directive_kind)
                {
                case Directive_Kind__None: {} break;

                case Directive_Kind__Option:
                {
                        directive_option(cursor, diagnostics, options);
                } break;
                case Directive_Kind__Size:
                {
                        directive_size(arena, cursor, diagnostics, expressions, symbols_table);
                } break;
                case Directive_Kind__Word_Double:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, 8);
                } break;
                case Directive_Kind__Word:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, 4);
                } break;
                case Directive_Kind__Word_Half:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, 2);
                } break;
                case Directive_Kind__Byte:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, 1);
                } break;
                case Directive_Kind__String: {} // fallthrough
                case Directive_Kind__Asciz:  { null_terminated_string = 1; } // fallthrough
                case Directive_Kind__Ascii:
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
                        U32 size_escaped = String8__escaped_size(content) + !!null_terminated_string;

                        U8 *data = Fragments__push(&symbols_table->section_current->fragments, cursor->current.location, size_escaped);
                        bytes_escaped_fill(text, data, size_escaped);

                        token_next(cursor, diagnostics);
                } break;
                case Directive_Kind__Text:    {} // fallthrough
                case Directive_Kind__Data:    {} // fallthrough
                case Directive_Kind__BSS:
                {
                        String8 section_name = Directive_Kind__String8_table[directive_kind];
                        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, section_name);
                        if (symbol->section == &Section__undefined)
                        {
                                Symbols_Table__create_section(symbols_table, symbol);
                        }

                        symbols_table->section_current = symbol->section;

                        token_next(cursor, diagnostics);
                        break;
                }
                case Directive_Kind__Type:
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
                } break;
                case Directive_Kind__Section:
                {
                        // Syntax: `.section name [, "flags"[, @type[, argument...]]]`
                        token_next(cursor, diagnostics);
                        String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
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

                                if (symbol->section->special)
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
                                if (symbol->section->special)
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
                } break;
                case Directive_Kind__Local:
                {
                        binding_set(cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Local);
                } break;
                // case Directive_Kind__Weak:
                // {
                //         binding_set(cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Weak);
                // } break;
                case Directive_Kind__Globl: {} // fallthrough
                case Directive_Kind__Global:
                {
                        binding_set(cursor, diagnostics, symbols_table, ELF_Symbol_Binding__Global);
                } break;
                // TODO(low): support for `<identifier> = <expr>` could be added by jumping here.
                case Directive_Kind__Set: {} // fallthrough
                case Directive_Kind__Equality:
                {
                        Set_Mode mode = Set_Mode__Override;
                        directive_set_like
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
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
                                Set_Mode__Strict_Forward
                        );
                } break;
                case Directive_Kind__Zero:
                {
                        // Equavalent to .fill repeat, 1, 0
                        B32 size_can_be_parsed = 0;
                        B32 pattern_can_be_parsed = 0;
                        directive_fill
                        (
                                 arena,
                                 cursor,
                                 diagnostics,
                                 expressions,
                                 symbols_table,
                                 size_can_be_parsed,
                                 pattern_can_be_parsed
                        );
                } break;
                case Directive_Kind__Space:
                {
                        // Equavalent to .fill repeat, 1, value
                        B32 size_can_be_parsed = 0;
                        B32 pattern_can_be_parsed = 1;
                        directive_fill
                        (
                                 arena,
                                 cursor,
                                 diagnostics,
                                 expressions,
                                 symbols_table,
                                 size_can_be_parsed,
                                 pattern_can_be_parsed
                        );
                } break;
                case Directive_Kind__Skip:
                {
                        // Equavalent to .fill repeat, 1, value
                        B32 size_can_be_parsed = 0;
                        B32 pattern_can_be_parsed = 1;
                        directive_fill
                        (
                                 arena,
                                 cursor,
                                 diagnostics,
                                 expressions,
                                 symbols_table,
                                 size_can_be_parsed,
                                 pattern_can_be_parsed
                        );
                } break;
                case Directive_Kind__Fill:
                {
                        B32 size_can_be_parsed = 1;
                        B32 pattern_can_be_parsed = 1;
                        directive_fill
                        (
                                 arena,
                                 cursor,
                                 diagnostics,
                                 expressions,
                                 symbols_table,
                                 size_can_be_parsed,
                                 pattern_can_be_parsed
                        );
                } break;
                // TODO(medium): this should be clearly refactored a bit. Let's see once all the major components are
                // extracted into a context struct.
                case Directive_Kind__Align:
                {
                        B32 power_of_two_exponent = 1;
                        U8 pattern_size           = 1;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
                        );
                } break;
                case Directive_Kind__P2_Align:
                {
                        B32 power_of_two_exponent = 1;
                        U8 pattern_size           = 1;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
                        );
                } break;
                case Directive_Kind__P2_Align_W:
                {
                        B32 power_of_two_exponent = 1;
                        U8 pattern_size           = 2;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
                        );
                } break;
                case Directive_Kind__P2_Align_L:
                {
                        B32 power_of_two_exponent = 1;
                        U8 pattern_size           = 4;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
                        );
                } break;
                case Directive_Kind__B_Align:
                {
                        B32 power_of_two_exponent = 0;
                        U8 pattern_size           = 1;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
                        );
                } break;
                case Directive_Kind__B_Align_W:
                {
                        B32 power_of_two_exponent = 0;
                        U8 pattern_size           = 2;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
                        );
                } break;
                case Directive_Kind__B_Align_L:
                {
                        B32 power_of_two_exponent = 0;
                        U8 pattern_size           = 4;
                        directive_align
                        (
                                arena,
                                cursor,
                                diagnostics,
                                expressions,
                                symbols_table,
                                power_of_two_exponent,
                                pattern_size
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
                                || label_found
                                || kind == Token_Kind__None
                                || kind == Token_Kind__Error
                                || kind == Token_Kind__Newline
                                || kind == Token_Kind__Semicolon;
                        if (break_should)
                        {
                                break;
                        }
                        junk_location_end = cursor->current.location + cursor->current.size;
                        token_next(cursor, diagnostics);
                }

                if (junk_location_end)
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location   = junk_location_begin;
                        diagnostic->message    = String8__literal("junk found at the end of line");
                        diagnostic->ranges[0]  = (Range1_U32){{ junk_location_begin, junk_location_end }};
                }
        }


        return;
}

