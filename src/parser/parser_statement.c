internal void
statement_read
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostics         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Sections_Table          *sections_table,
        Fixups                  *fixups
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

                        Label_Numeric *label_numeric = Symbols_Table__label_numeric_get_or_default(symbols_table, number);
                                       label_numeric->instances += 1;
                        String8        label_name    = label_numeric_string(scratch.arena, *label_numeric);
                        Symbol_Ref    *label         = Symbols_Table__get_or_default(symbols_table, label_name, sections_table->undefined);

                        assert_always_m(label->section->index == 0 && "numeric label created previously");
                        Symbol_Ref__update_section(label, sections_table->current);

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
                                Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, identifier, sections_table->undefined);
                                if (symbol->section->index != 0)
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
                                symbol->fragment       = sections_table->current->fragments.last;
                                symbol->value          = sections_table->current->fragments.last->data_size;
                                symbol->section        = sections_table->current;

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
                                sections_table,
                                instruction_hash,
                                &relocation,
                                &instruction,
                                &expression_parsed
                        );

                        if (instruction.opcode->info & INSN_MACRO)
                        {
                                RISCV_instruction_pseudo_append
                                (
                                        arena,
                                        sections_table->current,
                                        fixups,
                                        expressions,
                                        symbols_table,
                                        &instruction,
                                        expression_parsed,
                                        relocation
                                );
                        }
                        else
                        {
                                RISCV_Instruction__append
                                (
                                        sections_table->current,
                                        fixups,
                                        &instruction,
                                        expression_parsed,
                                        relocation
                                );
                        }
                }

                switch (directive_kind)
                {
                case Directive_Kind__None: {} break;

                case Directive_Kind__Word_Double:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, sections_table, fixups, 8);
                } break;
                case Directive_Kind__Word:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, sections_table, fixups, 4);
                } break;
                case Directive_Kind__Word_Half:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, sections_table, fixups, 2);
                } break;
                case Directive_Kind__Byte:
                {
                        directive_data(arena, cursor, diagnostics, expressions, symbols_table, sections_table, fixups, 1);
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

                        // Can be of the form `"\nhello\n", so with quotes and optional escaped characters.
                        String8 text = Token_Cursor__text(cursor);
                        text = String8__skip(text, 1);
                        text = String8__chop(text, 1);
                        U32 size_escaped = String8__escaped_size(text) + !!null_terminated_string;

                        U8 *data = Fragments__push(&sections_table->current->fragments, cursor->current.location, size_escaped);
                        bytes_escaped_fill(text, data, size_escaped);

                        token_next(cursor, diagnostics);
                } break;
                case Directive_Kind__Section:
                {
                        // Syntax: `.section name [, "flags"[, @type[, argument...]]]`
                        token_next(cursor, diagnostics);
                        String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
                        Section *section_new = Sections_Table__get_or_default(sections_table, name, cursor->current.location);

                        token_next(cursor, diagnostics);
                        if (cursor->current.kind == Token_Kind__Comma)
                        {
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
                                        diagnostic->location = cursor->current.location;
                                        diagnostic->message  = String8__literal("invalid section flags, expected: " ELF_Section_Header_Flags__cstring);
                                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                }

                                // TODO(medium, check-gas): are they ORed? Or overwritten?
                                section_new->elf.flags = flags;
                                token_next(cursor, diagnostics);
                        }

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
                                String8 text    = Token_Cursor__text(cursor);
                                String8 content = String8__skip_chop(text);
                                ELF_Section_Header_Type type = ELF_Section_Header_Type__from_String8(content);
                                if (type == ELF_Section_Header_Type__Invalid)
                                {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->kind       = Diagnostic_Kind__Error;
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("invalid section type");
                                        diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                }
                                section_new->elf.type = type;
                                token_next(cursor, diagnostics);
                        }


                        sections_table->current = section_new;
                } break;
                case Directive_Kind__Local:
                {
                        binding_set(cursor, diagnostics, symbols_table, sections_table, ELF_Symbol_Binding__Local);
                } break;
                // case Directive_Kind__Weak:
                // {
                //         binding_set(cursor, diagnostics, symbols_table, sections_table, ELF_Symbol_Binding__Weak);
                // } break;
                case Directive_Kind__Globl: {} // fallthrough
                case Directive_Kind__Global:
                {
                        binding_set(cursor, diagnostics, symbols_table, sections_table, ELF_Symbol_Binding__Global);
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
                                sections_table,
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
                                 sections_table,
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
                                 sections_table,
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
                                 sections_table,
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
                                 sections_table,
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
                                sections_table,
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
                                sections_table,
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
                                sections_table,
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
                                sections_table,
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
                                sections_table,
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
                                sections_table,
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
                                sections_table,
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

