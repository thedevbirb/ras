internal B32
LE_U8_identifier_start_is(U8 character)
{
        B32 result = U8__ascii_letter_is(character) || character == '_' || character == '.';
        return result;
}

internal B32
LE_U8_identifier_is(U8 character)
{
        B32 result = U8__ascii_letter_is(character) || character == '_' || character == '.' || U8__ascii_digit_is(character);
        return result;
}

internal B32
LE_U8_number_character_is(U8 character)
{
        B32 result = U8__ascii_digit_is(character);
        return result;
}

internal String8
Token_Cursor__text(Token_Cursor *cursor)
{
        String8 result =
        {
                .data  = &cursor->source->data[cursor->current.index],
                .count =  cursor->current.size
        };
        return result;
}

// INVARIANTS:
//
// 1. Assumes extra 4 bytes of zero after source->count.
//    I don't want annoying codepaths because I read past one, so I assume the past one is zero. This simplifies a lot
//    checks, where the last character being zero is already enough of a check.
// 2. Unless the returned token is `Token_Kind__None`, `Token.size` is guaranteed to be greater than zero. Rationale is
//    that zero-sized tokens should not exist, even in case of `Token_Kind__Error`, as an error character has still size
//    one.
internal Token
lex_at
(
        const Source *source,
        U32           index_current,
        Diagnostics  *diagnostics
)
{
        U8  *data  = source->data;
        U64  count = source->count;

        Token token = {0};

        U32 start_index = min_m(index_current, count);
        U64 index = start_index;

        // Read until we find a token, or we end the input i.e. index >= count;
        // Newlines mark the end of a token.

        for (;;)
        {
                start_index = index;
                U8 start = data[start_index];
                switch (start)
                {
                // A token cannot start with a zero byte.
                case  0  : {} break;

                case ' ' : { index += 1; } break;
                case '\t': { index += 1; } break;

                // NOTE: no multi-line comment support (yet).
                case '#':
                {
                        for (;;)
                        {
                                index += 1;
                                B32 break_should = index >= count || data[index] == '\n';
                                if (break_should)
                                {
                                        break;
                                }
                        }
                } break;

                case '%' : { token.kind = Token_Kind__Percentage;        index += 1; } break;
                case ',' : { token.kind = Token_Kind__Comma;             index += 1; } break;
                case ':' : { token.kind = Token_Kind__Colon;             index += 1; } break;
                case ';' : { token.kind = Token_Kind__Semicolon;         index += 1; } break;
                case '(' : { token.kind = Token_Kind__Parenthesis_Left;  index += 1; } break;
                case ')' : { token.kind = Token_Kind__Parenthesis_Right; index += 1; } break;
                case '+' : { token.kind = Token_Kind__Plus;              index += 1; } break;
                case '-' : { token.kind = Token_Kind__Minus;             index += 1; } break;
                case '*' : { token.kind = Token_Kind__Star;              index += 1; } break;
                case '/' : { token.kind = Token_Kind__Slash;             index += 1; } break;
                case '~' : { token.kind = Token_Kind__Tilde;             index += 1; } break;
                case '^' : { token.kind = Token_Kind__Caret;             index += 1; } break;
                case '@' : { token.kind = Token_Kind__At;                index += 1; } break;
                case '\n': { token.kind = Token_Kind__Newline;           index += 1; } break;

                case '>':
                {
                        token.kind = Token_Kind__Greater;
                        if (data[index] == '>')
                        {
                                index += 1;
                                token.kind = Token_Kind__Greater_2;
                        }
                        else if (data[index] == '=')
                        {
                                index += 1;
                                token.kind = Token_Kind__Greater_Equal;
                        }
                } break;
                case '<':
                {
                        token.kind = Token_Kind__Less;
                        index += 1;
                        if (data[index] == '<')
                        {
                                index += 1;
                                token.kind = Token_Kind__Less_2;
                        }
                        else if (data[index] == '=')
                        {
                                index += 1;
                                token.kind = Token_Kind__Less_Equal;
                        }
                } break;
                case '=':
                {
                        token.kind = Token_Kind__Equal;
                        index += 1;
                        if (data[index] == '=')
                        {
                                token.kind = Token_Kind__Equal_2;
                                index += 1;
                        }
                } break;
                case '!':
                {
                        token.kind = Token_Kind__Bang;
                        index += 1;
                        if (data[index] == '=')
                        {
                                token.kind = Token_Kind__Equal_Bang;
                                index += 1;
                        }
                } break;
                case '|':
                {
                        token.kind = Token_Kind__Pipe;
                        index += 1;
                        if (data[index] == '|')
                        {
                                token.kind = Token_Kind__Pipe_2;
                                index += 1;
                        }
                } break;
                case '&':
                {
                        token.kind = Token_Kind__Ampersand;
                        index += 1;
                        if (data[index] == '&')
                        {
                                token.kind = Token_Kind__Ampersand_2;
                                index += 1;
                        }
                } break;
                case '\'':
                {
                        U64 result = 0;
                        token.kind = Token_Kind__Number;

                        index += 1;
                        if (index >= count)
                        {
                                token.kind = Token_Kind__Error;
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Literal_Unterminated];
                                diagnostic->location = source->start_offset_logical + index;

                        }

                        if (data[index] == '\\')
                        {
                                index += 1;

                                B32 hex_prefix   = data[index] == 'x';
                                U8  digit        = data[index] - '0';
                                B32 octal_prefix = digit <= 3;

                                if (hex_prefix)
                                {
                                        U32 error_index = 0;
                                        // We read up to two more characters.
                                        index += 1;
                                        U8 first  = hex_table[data[index]];
                                        error_index = first == hex_table_invalid ? index : 0;

                                        index += 1;
                                        U8 second = hex_table[data[index]];
                                        error_index = second == hex_table_invalid ? index : 0;

                                        if (error_index)
                                        {
                                                token.kind = Token_Kind__Error;
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Escape_Sequence_Invalid];
                                                diagnostic->location = source->start_offset_logical + error_index;
                                        }
                                        result = result * 16 + first;
                                        index += 1;
                                }
                                else if (octal_prefix)
                                {
                                        // We read up to two more characters, and the current digit counts.
                                        result = digit;

                                        index += 1;
                                        if ('0' <= data[index] && data[index] < '8')
                                        {
                                                result = result * 8 + (data[index] - '0');
                                                index += 1;
                                        }

                                        if ('0' <= data[index] && data[index] < '8')
                                        {
                                                result = result * 8 + (data[index] - '0');
                                                index += 1;
                                        }
                                }
                                else
                                {
                                        result = escape_table[data[index]];
                                        if (result == escape_value_invalid)
                                        {
                                                token.kind = Token_Kind__Error;
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Escape_Sequence_Invalid];
                                                diagnostic->location = source->start_offset_logical + index;
                                        }
                                        index += 1;
                                }
                        }
                        else if (data[index] == '\n')
                        {
                                token.kind = Token_Kind__Error;
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Literal_Multiline_Unsupported];
                                diagnostic->location = source->start_offset_logical + index;
                        }
                        else
                        {
                                result = data[index];
                                index += 1;
                        }

                        if (data[index] != '\'')
                        {
                                // Could add hint here?
                                token.kind = Token_Kind__Error;
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Literal_Unterminated];
                                diagnostic->location = source->start_offset_logical + index;
                        }

                        index += 1;
                        token.numerical_value = result;
                } break;
                case '\"':
                {
                        token.kind = Token_Kind__String;

                        // We only check for ending quotes: handling of bytes values and parsing of
                        // escapes is done at a later stage.
                        U8 escaping_started             = 0;
                        U8 quote_ending_found           = 0;
                        B32 break_should_double_quote   = 0;
                        for (;;)
                        {
                                index += 1;
                                break_should_double_quote = quote_ending_found || index >= count || token.kind == Token_Kind__Error;
                                if (break_should_double_quote)
                                {
                                        break;
                                }

                                if (escaping_started)
                                {
                                        escaping_started = 0;

                                        U8 valid = escape_valid_table[data[index]];
                                        if (!valid)
                                        {
                                                token.kind = Token_Kind__Error;
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Escape_Sequence_Invalid];
                                                diagnostic->location = source->start_offset_logical + index;
                                        }

                                        B32 hex_prefix = data[index] == 'x';

                                        // We don't parse it but we ensure the next two characters are valid.
                                        if (hex_prefix)
                                        {
                                                U32 error_index = 0;
                                                index += 1;
                                                B32 invalid_first = hex_table[data[index]] == hex_table_invalid;
                                                error_index = invalid_first  ? index : error_index;
                                                B32 invalid_second = hex_table[data[index]] == hex_table_invalid;
                                                error_index = invalid_second ? index : error_index;

                                                if (error_index)
                                                {
                                                        token.kind = Token_Kind__Error;
                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                        diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Hex_Literal_Invalid];
                                                        diagnostic->location = source->start_offset_logical + error_index;
                                                }
                                        }
                                }
                                else if (data[index] == '\\')
                                {
                                        escaping_started = 1;
                                }
                                else if (data[index] == '\"')
                                {
                                        quote_ending_found = 1;
                                }
                                else if (data[index] == '\n')
                                {
                                        token.kind = Token_Kind__Error;
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__String_Multiline_Unsupported];
                                        diagnostic->location = source->start_offset_logical + index;
                                }
                        }

                        if (!quote_ending_found && token.kind != Token_Kind__Error)
                        {
                                        token.kind = Token_Kind__Error;
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__String_Literal_Unterminated];
                                        diagnostic->location = source->start_offset_logical + index;
                        }
                } break;
                default:
                {
                        if (LE_U8_identifier_start_is(data[index]))
                        {
                                token.kind = Token_Kind__Identifier;
                                B32 break_should_identifier = 0;
                                B32 invalid = 0;
                                for (;;)
                                {
                                        break_should_identifier = invalid || index >= count;
                                        if (break_should_identifier)
                                        {
                                                break;
                                        }
                                        index += 1;
                                        invalid = !LE_U8_identifier_is(data[index]);
                                }
                        }
                        // TODO: support float (hex float?)
                        else if (U8__ascii_digit_is(data[index]))
                        {
                                U8 digit = data[index];
                                U8 next  = data[index + 1];

                                token.kind = Token_Kind__Number;

                                if (digit == '0' && next == 'x')
                                {
                                        index += 2;
                                        for (;;)
                                        {
                                                U8 value = hex_table[data[index]];
                                                if (value >= 16)
                                                {
                                                        break;
                                                }
                                                token.numerical_value = token.numerical_value * 16 + value;
                                                index += 1;
                                        }

                                        B32 valid = numeric_suffix_table[data[index]];
                                        if (!valid)
                                        {
                                                token.kind = Token_Kind__Error;
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Hex_Literal_Invalid];
                                                diagnostic->location = source->start_offset_logical + index;
                                        }
                                }
                                else if (digit == '0' && next == 'b')
                                {
                                        index += 2;
                                        for (;;)
                                        {
                                                U8 value = hex_table[data[index]];
                                                if (value >= 2)
                                                {
                                                        break;
                                                }
                                                token.numerical_value = token.numerical_value * 2 + value;
                                                index += 1;
                                        }

                                        B32 valid = numeric_suffix_table[data[index]];
                                        if (!valid)
                                        {
                                                token.kind = Token_Kind__Error;
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Binary_Literal_Invalid];
                                                diagnostic->location = source->start_offset_logical + index;
                                        }
                                }
                                else if (digit == '0' && U8__ascii_digit_is(next))
                                {
                                        // Octal
                                        index += 1;
                                        for (;;)
                                        {
                                                U8 value = hex_table[data[index]];
                                                if (value >= 8)
                                                {
                                                        break;
                                                }
                                                token.numerical_value = token.numerical_value * 8 + value;
                                                index += 1;
                                        }

                                        B32 valid = numeric_suffix_table[data[index]];
                                        if (!valid)
                                        {
                                                token.kind = Token_Kind__Error;
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Octal_Literal_Invalid];
                                                diagnostic->location = source->start_offset_logical + index;
                                        }
                                }
                                else
                                {       // Base 10 number
                                        for (;;)
                                        {
                                                B32 break_should = data[index] < '0' || '9' < data[index];
                                                if (break_should)
                                                {
                                                        break;
                                                }
                                                U8 value = data[index] - '0';
                                                token.numerical_value = token.numerical_value * 10 + value;
                                                index += 1;
                                        }
                                }
                        }
                        else
                        {
                                // NOTE: decide on whether erroring after a while bunch on invalid tokens are read.
                                token.kind = Token_Kind__Error;
                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Unexpected];
                                diagnostic->location = source->start_offset_logical + index;

                                index += 1;
                        }
                } break;
                }

                B32 loop_infinite_avoided = start_index < index || index >= count || token.kind;
                assert_always_m(loop_infinite_avoided && "lexer infinite loop");

                if (token.kind)
                {
                        token.index    = start_index;
                        token.location = source->start_offset_logical + start_index;
                        token.size     = index - start_index;
                }

                if (token.kind || index >= count)
                {
                        break;
                }
                else
                {
                        start_index = index;
                }
        }

        assert_always_m(token.size > 0 || token.kind == Token_Kind__None && "zero-sized token");

        return token;
}

internal Token
token_peek
(
        Token_Cursor const *cursor,
        Diagnostics        *diagnostics
)
{
        Token result = lex_at(cursor->source, cursor->source_index, diagnostics);
        return result;
}

// TODO: maybe return the token AND update the cursor?
internal void
token_next
(
        Token_Cursor *cursor,
        Diagnostics  *diagnostics
)
{
        cursor->previous = cursor->current;
        cursor->current  = token_peek(cursor, diagnostics);
        if (cursor->current.kind != Token_Kind__None)
        {
                cursor->source_index = cursor->current.index + cursor->current.size;
        }

#ifdef RAS_DEBUG_TOKEN_DUMP
        const char *token_string = Token_Kind_strings[cursor->current.kind];
        String8 text = Token_Cursor__text(cursor);
        printf("token: %10s   content: %.*s\n", token_string, (int)text.count, text.data);
#endif
}

// TODO(medium): perhaps it makes sense to merge part of this logic inside `lex_at`?
internal void
Token_Cursor__read_raw_identifier_until(Token_Cursor *cursor, String8 ending_bytes_set, B32 skip_initial_whitespace)
{
        B32 ending_found           = 0;
        B32 skipped_all_whitespace = 0;

        for (;;)
        {
                B32 break_should = !skip_initial_whitespace || skipped_all_whitespace || cursor->source_index >= cursor->source->count;
                if (break_should)
                {
                        break;
                }

                U8 byte = cursor->source->data[cursor->source_index];
                skipped_all_whitespace = byte != ' ' && byte != '\t';
                cursor->source_index += (U32)!skipped_all_whitespace;
        }

        U32 start_index = cursor->source_index;
        for (;;)
        {
                ending_found |= cursor->source_index >= cursor->source->count;
                if (!ending_found)
                {
                        // Read one byte from the source and compare it with the ending set.
                        U8 byte = cursor->source->data[cursor->source_index];
                        U64 index = 0;
                        for (;;)
                        {
                                B32 break_should = ending_found || index >= ending_bytes_set.count;
                                if (break_should)
                                {
                                        break;
                                }

                                ending_found |= byte == ending_bytes_set.data[index];
                                index += 1;
                        }
                }

                if (ending_found)
                {
                        break;
                }

                cursor->source_index += 1;
        }

        assert_always_m(start_index <= cursor->source_index);

        Token result_token =
        {
                .index    = start_index,
                .location = cursor->source->start_offset_logical + start_index,
                .size     = cursor->source_index - start_index,
                .kind     = Token_Kind__Identifier
        };
        cursor->previous = cursor->current;
        cursor->current  = result_token;

        return;
}
