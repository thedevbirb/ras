internal void
statements_read
(
        Arena             *arena,
        Token_Cursor      *cursor,
        Diagnostics       *diagnostics,
        Expressions       *expressions,
        Symbols_Table     *symbols_table,
        Options           *options
)
{
        B32 progress = 1;
        for (;;)
        {
                B32            label_found            = 0;
                B32            empty_line             = 0;

                U32 source_index_start = cursor->source_index;
                B32 break_should_outer = !progress
                                      || cursor->current.kind == Token_Kind__Error;
                assert_always_m((progress || break_should_outer) && "infinite loop detected");

                if (break_should_outer)
                {
                        break;
                }

                switch (cursor->current.kind)
                {
                // no-op, continue;
                case Token_Kind__None:      { token_next(cursor, diagnostics); empty_line = 1; } break;
                case Token_Kind__Newline:   { token_next(cursor, diagnostics); empty_line = 1; } break;
                case Token_Kind__Semicolon: { token_next(cursor, diagnostics); empty_line = 1; } break;
                case Token_Kind__Number:
                {
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

                        Symbol_Numeric symbol_numeric = Symbols_Table__get_or_default_numeric(symbols_table, number, 1, arena);
                                       symbol_numeric.label->instances += 1;

                        assert_always_m(symbol_numeric.symbol->section == &Section__undefined && "numeric label created previously");

                        symbol_numeric.symbol->location  = cursor->current.location;
                        Symbol_Ref__update_section(symbol_numeric.symbol, symbols_table->section_current);
                } break;
                case Token_Kind__Identifier:
                {
                        // Instructions, directives and label start with an identifier. We have to discriminate further.
                        String8 identifier = Token_Cursor__text(cursor);
                        Token next = token_peek(cursor, diagnostics);
                        label_found = next.kind == Token_Kind__Colon;
                        B32 dot_start = identifier.data[0] == '.';

                        if (label_found)
                        {
                                // Label found
                                Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, identifier, arena);
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
                                Symbol_Ref__update_section(symbol, symbols_table->section_current);

                                token_next(cursor, diagnostics);
                                token_next(cursor, diagnostics);
                        }
                        else if (dot_start)
                        {
                                Directive_Kind directive_kind = Directive_Kind__from_String8(identifier);

                                switch (directive_kind)
                                {
                                case Directive_Kind__None:
                                {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Directive_Unknown];
                                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                } break;

                                case Directive_Kind__Attribute: { directive_attribute(cursor, diagnostics, arena, options, symbols_table->section_first); } break;

                                case Directive_Kind__Option: { directive_option(cursor, diagnostics, options);                         } break;
                                case Directive_Kind__File:   { directive_file(cursor, diagnostics, arena, symbols_table);              } break;
                                case Directive_Kind__Type:   { directive_type(cursor, diagnostics, arena, symbols_table);              } break;
                                case Directive_Kind__Ident:  { directive_ident(cursor, diagnostics, arena, symbols_table);             } break;
                                case Directive_Kind__Size:   { directive_size(arena, cursor, diagnostics, expressions, symbols_table); } break;

                                case Directive_Kind__Word_Double: { directive_data(arena, cursor, diagnostics, expressions, symbols_table, 8); } break;
                                case Directive_Kind__Word:        { directive_data(arena, cursor, diagnostics, expressions, symbols_table, 4); } break;
                                case Directive_Kind__Word_Half:   { directive_data(arena, cursor, diagnostics, expressions, symbols_table, 2); } break;
                                case Directive_Kind__Byte:        { directive_data(arena, cursor, diagnostics, expressions, symbols_table, 1); } break;

                                case Directive_Kind__String: { directive_string(cursor, diagnostics, symbols_table->section_current, 1); } break;
                                case Directive_Kind__Asciz:  { directive_string(cursor, diagnostics, symbols_table->section_current, 1); } break;
                                case Directive_Kind__Ascii:  { directive_string(cursor, diagnostics, symbols_table->section_current, 0); } break;
                                case Directive_Kind__Base64: { directive_base64(cursor, diagnostics, symbols_table->section_current);    } break;

                                case Directive_Kind__Text: { directive_section_current(cursor, diagnostics, symbols_table, directive_kind); } break;
                                case Directive_Kind__Data: { directive_section_current(cursor, diagnostics, symbols_table, directive_kind); } break;
                                case Directive_Kind__BSS:  { directive_section_current(cursor, diagnostics, symbols_table, directive_kind); } break;

                                case Directive_Kind__Section: { directive_section(arena, cursor, diagnostics, expressions, symbols_table); } break;

                                case Directive_Kind__Local:   { directive_binding(cursor, diagnostics, arena, symbols_table, ELF_Symbol_Binding__Local);  } break;
                                case Directive_Kind__Weak:    { directive_binding(cursor, diagnostics, arena, symbols_table, ELF_Symbol_Binding__Weak);   } break;
                                case Directive_Kind__Globl:   { directive_binding(cursor, diagnostics, arena, symbols_table, ELF_Symbol_Binding__Global); } break;
                                case Directive_Kind__Global:  { directive_binding(cursor, diagnostics, arena, symbols_table, ELF_Symbol_Binding__Global); } break;

                                case Directive_Kind__Comm:    { directive_common(cursor, diagnostics, arena, symbols_table, options->xlen); } break;
                                case Directive_Kind__Common:  { directive_common(cursor, diagnostics, arena, symbols_table, options->xlen); } break;

                                // TODO(low): support for `<identifier> = <expr>` could be added by jumping here.
                                case Directive_Kind__Set:      { directive_set_like(arena, cursor, diagnostics, symbols_table, Set_Mode__Override);        } break;
                                case Directive_Kind__Equality: { directive_set_like(arena, cursor, diagnostics, symbols_table, Set_Mode__Override);        } break;
                                case Directive_Kind__Equiv:    { directive_set_like(arena, cursor, diagnostics, symbols_table, Set_Mode__Strict);          } break;
                                case Directive_Kind__Eqv:      { directive_set_like(arena, cursor, diagnostics, symbols_table, Set_Mode__Strict_Forward);  } break;
                                case Directive_Kind__Zero:
                                {
                                        // Equavalent to .fill repeat, 1, 0
                                        B32 size_can_be_parsed = 0;
                                        B32 pattern_can_be_parsed = 0;
                                        directive_fill(arena, cursor, diagnostics, expressions, symbols_table, size_can_be_parsed, pattern_can_be_parsed);
                                } break;
                                case Directive_Kind__Space:
                                {
                                        // Equavalent to .fill repeat, 1, value
                                        B32 size_can_be_parsed = 0;
                                        B32 pattern_can_be_parsed = 1;
                                        directive_fill(arena, cursor, diagnostics, expressions, symbols_table, size_can_be_parsed, pattern_can_be_parsed);
                                } break;
                                case Directive_Kind__Skip:
                                {
                                        // Equavalent to .fill repeat, 1, value
                                        B32 size_can_be_parsed = 0;
                                        B32 pattern_can_be_parsed = 1;
                                        directive_fill(arena, cursor, diagnostics, expressions, symbols_table, size_can_be_parsed, pattern_can_be_parsed);
                                } break;
                                case Directive_Kind__Fill:
                                {
                                        B32 size_can_be_parsed = 1;
                                        B32 pattern_can_be_parsed = 1;
                                        directive_fill(arena, cursor, diagnostics, expressions, symbols_table, size_can_be_parsed, pattern_can_be_parsed);
                                } break;
                                // TODO(medium): this should be clearly refactored a bit. Let's see once all the major components are
                                // extracted into a context struct.
                                case Directive_Kind__Align:
                                {
                                        B32 power_of_two_exponent = 1;
                                        U8 pattern_size           = 1;
                                        directive_align(arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                case Directive_Kind__P2_Align:
                                {
                                        B32 power_of_two_exponent = 1;
                                        U8 pattern_size           = 1;
                                        directive_align(arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                case Directive_Kind__P2_Align_W:
                                {
                                        B32 power_of_two_exponent = 1;
                                        U8 pattern_size           = 2;
                                        directive_align(arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                case Directive_Kind__P2_Align_L:
                                {
                                        B32 power_of_two_exponent = 1;
                                        U8 pattern_size           = 4;
                                        directive_align (arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                case Directive_Kind__B_Align:
                                {
                                        B32 power_of_two_exponent = 0;
                                        U8 pattern_size           = 1;
                                        directive_align(arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                case Directive_Kind__B_Align_W:
                                {
                                        B32 power_of_two_exponent = 0;
                                        U8 pattern_size           = 2;
                                        directive_align(arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                case Directive_Kind__B_Align_L:
                                {
                                        B32 power_of_two_exponent = 0;
                                        U8 pattern_size           = 4;
                                        directive_align(arena, cursor, diagnostics, expressions, symbols_table, power_of_two_exponent, pattern_size);
                                } break;
                                default: {} break;
                                }

                        }
                        else
                        {
                                U32 instruction_hash = FNV_hash_U32(identifier);

                                Instruction_Parsed instruction =
                                        RISCV_Instruction__parse(arena, cursor, diagnostics, expressions, symbols_table, options, instruction_hash);

                                if (instruction.data.opcode->info & INSN_MACRO)
                                {
                                        RISCV_instruction_pseudo_append(arena, symbols_table->section_current, expressions, symbols_table, options, &instruction);
                                }
                                else
                                {
                                        RISCV_Instruction__append(arena, symbols_table->section_current, options, &instruction);
                                }
                        }
                } break;
                default:
                {
                        // Sort of catch-all
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->location   = cursor->current.location;
                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Line_Invalid];
                        diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                } break;
                }

                // Find end of line junk
                U32 junk_location_begin = cursor->current.location;
                U32 junk_location_end   = 0;
                for (;;)
                {
                        Token_Kind kind = cursor->current.kind;
                        B32 break_should = empty_line
                                || label_found
                                || Token_Kind__end_of_statement(cursor->current.kind)
                                || kind == Token_Kind__Error;
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

                progress = source_index_start < cursor->source_index;
        }


        return;
}

