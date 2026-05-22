// After the lexer processes an item, it ALWAYS advances.

internal B32
LE_U8_identifier_start_is(U8 character)
{
	B32 result = U8_ascii_letter_is(character) || character == '_' || character == '.';
	return result;
}

internal B32
LE_U8_identifier_is(U8 character)
{
	B32 result = U8_ascii_letter_is(character) || character == '_' || character == '.' || U8_ascii_digit_is(character);
	return result;
}

internal B32
LE_U8_number_character_is(U8 character)
{
	B32 result = U8_ascii_digit_is(character);
	return result;
}

// It is a no-op if the end has been reached already.
internal void
Lexer_advance(Lexer *lexer, const String8 *input)
{
	B32 newline_reached = lexer->current == '\n';

	lexer->end_reached   = input->count == 0 || lexer->index + 1 == input->count;
	lexer->index        += !lexer->end_reached;
	lexer->row_index    += !lexer->end_reached && newline_reached;
	lexer->column_index += !lexer->end_reached;
	lexer->current       = input->data[lexer->index];

	if (newline_reached && !lexer->end_reached)
	{
		lexer->column_index = 0;
		lexer->line_start_index = lexer->index;
	}

	// An index pointing out of bounds is of no-one's help.
	assert_always_m(lexer->index < input->count);
	assert_always_m(lexer->end_reached == 0 || lexer->end_reached == 1);

	return;
}

internal void
Lexer_advance_to_newline(Lexer *lexer, const String8 *input)
{
	for (;;)
	{
		B32 break_should = lexer->current == '\n' || lexer->end_reached;
		Lexer_advance(lexer, input);
		if (break_should)
		{
			break;
		}
	}
}

internal U8 *
Lexer_peek_next(Lexer *lexer, const String8 *input)
{
	U8 *result = 0;
	if (!lexer->end_reached)
	{
		result = &input->data[lexer->index + 1];
	}
	return result;
}

internal String8
Lexer_string_under_cursor(Lexer *lexer, const String8 *input)
{
	String8 string =
	{
		.data  = input->data  + lexer->index_before,
		.count = lexer->index - lexer->index_before,
	};
	return string;
}

internal void
Lexer_diagnostic_fill(Lexer *lexer, Diagnostic *diagnostic, Lexer_Error_Kind kind)
{
	diagnostic->message_kind       = lexer_error_kind_messages[kind];
	diagnostic->row_index          = lexer->row_index;
	diagnostic->column_index_begin = lexer->column_index_before;
	diagnostic->column_index_end   = lexer->column_index;
	diagnostic->input_index_start  = lexer->line_start_index;
	diagnostic->variant            = (U32)kind;

	return;
}

internal void
Lexer_expect(Lexer *lexer, B32 condition, Lexer_Error_Kind error_kind, Diagnostic *diagnostic)
{
	if (!condition && diagnostic->variant == 0)
	{
		Lexer_diagnostic_fill(lexer, diagnostic, error_kind);
	}
	return;
}

internal void
Lexer_tokenize(Lexer *lexer, Arena *arena, Lexer_Context *context)
{
	const String8  *input       = context->input;
	const char     *filename    = context->filename;
	Token_Xar      *tokens      = context->tokens;
	Diagnostics    *diagnostics = context->diagnostics;

	lexer->end_reached = 0 >= input->count;
	if (!lexer->end_reached)
	{
		lexer->current = input->data[0];
	}

	Token_Kind token_kind = Token_Kind__None;
	U64 numerical_value = 0;

	Diagnostic error = { .filename = filename };

	for (;;)
	{
		B32 break_should_lexer = lexer->end_reached || diagnostics->errors_count == DIAGNOSTICS_ERRORS_MAX;
		if (break_should_lexer)
		{
			break;
		}

		// Snapshot indexes before processing a token to determine its size.
		lexer->index_before        = lexer->index;
		lexer->column_index_before = lexer->column_index;

		// NOTE: should maintain the logical order of Token_Kind enumeration.
		switch (lexer->current)
		{
		case ' ' : { Lexer_advance(lexer, input); } break;
		case '\t': { Lexer_advance(lexer, input); } break;

		// NOTE: no multi-line comment support (yet).
		case '#':
		{
			for (;;)
			{
				// We don't want to count extra newline tokens because of comments.
				B32 newline_reached = lexer->current == '\n';
				B32 break_should_comment = newline_reached || lexer->end_reached;
				if (break_should_comment)
				{
					break;
				}

				Lexer_advance(lexer, input);
			}

		} break;

		case ',' : { token_kind = Token_Kind__Comma;             Lexer_advance(lexer, input); } break;
		case ';' : { token_kind = Token_Kind__Semicolon;         Lexer_advance(lexer, input); } break;
		case '(' : { token_kind = Token_Kind__Parenthesis_Left;  Lexer_advance(lexer, input); } break;
		case ')' : { token_kind = Token_Kind__Parenthesis_Right; Lexer_advance(lexer, input); } break;
		case '+' : { token_kind = Token_Kind__Plus;              Lexer_advance(lexer, input); } break;
		case '-' : { token_kind = Token_Kind__Minus;             Lexer_advance(lexer, input); } break;
		case '*' : { token_kind = Token_Kind__Star;              Lexer_advance(lexer, input); } break;
		case '/' : { token_kind = Token_Kind__Slash;             Lexer_advance(lexer, input); } break;
		case '~' : { token_kind = Token_Kind__Tilde;             Lexer_advance(lexer, input); } break;
		case '^' : { token_kind = Token_Kind__Caret;             Lexer_advance(lexer, input); } break;
		case '\n': { token_kind = Token_Kind__Newline;           Lexer_advance(lexer, input); } break;

		case '>':
		{
			token_kind = Token_Kind__Greater_Than;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached)
			{
				if (lexer->current == '>')
				{
					Lexer_advance(lexer, input);
					token_kind = Token_Kind__Shift_Right;
				}
				else if (lexer->current == '=')
				{
					Lexer_advance(lexer, input);
					token_kind = Token_Kind__Greater_Equal;
				}
			}
		} break;
		case '<':
		{
			token_kind = Token_Kind__Less_Than;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached)
			{
				if (lexer->current == '>')
				{
					Lexer_advance(lexer, input);
					token_kind = Token_Kind__Shift_Left;
				}
				else if (lexer->current == '=')
				{
					Lexer_advance(lexer, input);
					token_kind = Token_Kind__Less_Equal;
				}
			}
		} break;

		case '=':
		{
			token_kind = Token_Kind__Assign;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached && lexer->current == '=')
			{
				token_kind = Token_Kind__Equal;
				Lexer_advance(lexer, input);
			}
		} break;

		case '!':
		{
			token_kind = Token_Kind__Bang;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached && lexer->current == '=')
			{
				token_kind = Token_Kind__Equal_Not;
				Lexer_advance(lexer, input);
			}
		} break;

		case '|':
		{
			token_kind = Token_Kind__Pipe;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached && lexer->current == '|')
			{
				token_kind = Token_Kind__Logical_Or;
				Lexer_advance(lexer, input);
			}
		} break;
		case '&':
		{
			token_kind = Token_Kind__Ampersand;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached && lexer->current == '&')
			{
				token_kind = Token_Kind__Logical_And;
				Lexer_advance(lexer, input);
			}
		} break;

		case '%':
		{
			token_kind = Token_Kind__Percentage;
			Lexer_advance(lexer, input);
			if (!lexer->end_reached && U8_ascii_letter_is(lexer->current))
			{
				token_kind = Token_Kind__Relocation_Prefix;
			}
		} break;
		case ':':
		{       // We check the previous. This is to avoid including the colon into the label token.
			U32 token_index_previous = tokens->header.count - (tokens->header.count > 0);
			Token *token_previous    = xar_get_m(tokens, token_index_previous);

			Lexer_expect(lexer, token_previous->kind == Token_Kind__Label, Lexer_Error_Kind__Character_Unexpected, &error);
			Lexer_advance(lexer, input);
		} break;

		case '\'':
		{
			B32 result = 0;

			Lexer_advance(lexer, input);
			Lexer_expect(lexer, !lexer->end_reached, Lexer_Error_Kind__Character_Literal_Unterminated, &error);

			if (lexer->current == '\\')
			{
				Lexer_advance(lexer, input);
				U8 character = lexer->current;

				B32 hex_prefix = character == 'x';
				U8 digit = character - '0';
				B32 octal_prefix = digit <= 3;

				if (hex_prefix)
				{
					// We read up to two more characters.
					Lexer_advance(lexer, input);
					U8 value = hex_table[lexer->current];
					Lexer_expect(lexer, value != hex_table_sentinel_invalid, Lexer_Error_Kind__Escape_Sequence_Invalid, &error);
					result = value;

					Lexer_advance(lexer, input);
					value = hex_table[lexer->current];
					if (value != hex_table_sentinel_invalid)
					{
						result = result * 16 + value;
						Lexer_advance(lexer, input);
					}
				}
				else if (octal_prefix)
				{
					// We read up to two more characters, and the current digit counts.
					result = digit;

					Lexer_advance(lexer, input);
					if (lexer->current - '0' < 8)
					{
						result = result * 8 + (lexer->current - '0');
						Lexer_advance(lexer, input);
					}

					if (lexer->current - '0' < 8)
					{
						result = result * 8 + (lexer->current - '0');
						Lexer_advance(lexer, input);
					}
				}
				else
				{
					result = escape_table[character];
					Lexer_expect(lexer, result != escape_value_invalid, Lexer_Error_Kind__Escape_Sequence_Invalid, &error);
					Lexer_advance(lexer, input);
				}
			}
			else if (lexer->current == '\n')
			{
				Lexer_diagnostic_fill(lexer, &error, Lexer_Error_Kind__Character_Literal_Multiline_Unsupported);
			}
			else
			{
				result = lexer->current;
				Lexer_advance(lexer, input);
			}

			Lexer_expect(lexer, lexer->current == '\'', Lexer_Error_Kind__Character_Literal_Unterminated, &error);
			Lexer_advance(lexer, input);

			token_kind = Token_Kind__Char_Literal;
			numerical_value = result;
		} break;
		case '\"':
		{
			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8 escaping_started             = 0;
			U8 quote_ending_found           = 0;
			B32 break_should_double_quote   = 0;
			U32 escape_started_column_index = 0;
			for (;;)
			{
				break_should_double_quote = quote_ending_found || lexer->end_reached || error.variant;
				if (break_should_double_quote)
				{
					break;
				}
				Lexer_advance(lexer, input);

				U8 character = lexer->current;

				if (escaping_started)
				{
					escaping_started = 0;

					U8 valid = escape_valid_table[character];
					Lexer_expect(lexer, valid, Lexer_Error_Kind__Escape_Sequence_Invalid, &error);

					B32 hex_prefix = character == 'x';

					// We don't parse it but we ensure at least there is another following character.
					if (hex_prefix)
					{
						U8 *next = Lexer_peek_next(lexer, input);
						B32 invalid = next && hex_table[*next] == hex_table_sentinel_invalid;

						// We create a raw version for better diagnostic of an invalid escape
						// sequence within a longer string.
						if (invalid)
						{
							Lexer_diagnostic_fill(lexer, &error, Lexer_Error_Kind__Escape_Sequence_Invalid);
							error.column_index_begin = escape_started_column_index;
							error.column_index_end   = lexer->column_index + 1;
						}

					}
				}
				else if (character == '\\')
				{
					escaping_started = 1;
					escape_started_column_index = lexer->column_index;
				}
				else if (character == '\"')
				{
					quote_ending_found = 1;
				}
				else if (character == '\n')
				{	// NOTE: it may make sense to introduce a flag that changes this behaviour.
					Lexer_diagnostic_fill(lexer, &error, Lexer_Error_Kind__String_Multiline_Unsupported);
				}
			}

			Lexer_expect(lexer, quote_ending_found, Lexer_Error_Kind__String_Literal_Unterminated, &error);

			token_kind = Token_Kind__String_Literal;
		} break;
		default:
		{
			if (LE_U8_identifier_start_is(lexer->current))
			{
				U8 character_start = lexer->current;
				B32 break_should_identifier = 0;
				B32 invalid = 0;
				for (;;)
				{
					break_should_identifier = invalid || lexer->end_reached;
					if (break_should_identifier)
					{
						break;
					}
					Lexer_advance(lexer, input);
					invalid = !LE_U8_identifier_is(lexer->current);
				}

				String8 token_string = Lexer_string_under_cursor(lexer, input);
				Directive_Kind directive_kind = Directive_Kind__from_String8(token_string);

				// This gives a good hint that lexing assembly comes with some pain.
				if (token_string.count == 1 && character_start == '.')
				{
					token_kind = Token_Kind__Dot;
				}
				else if (lexer->current == ':')
				{
					token_kind = Token_Kind__Label;
				}
				else if (directive_kind)
				{
					token_kind = Token_Kind__Directive;
					numerical_value = directive_kind;
				}
				else
				{
					token_kind = Token_Kind__Identifier;
				}
			}
			// TODO: support float (hex float?)
			else if (U8_ascii_digit_is(lexer->current))
			{
				U8 digit = lexer->current;
				U8 *next = Lexer_peek_next(lexer, input);

				token_kind = Token_Kind__Number_Literal;

				if (digit == '0' && next && *next == 'x')
				{
					Lexer_advance(lexer, input);
					U64 result = 0;
					for (;;)
					{
						Lexer_advance(lexer, input);
						U8 value = hex_table[lexer->current];
						if (lexer->end_reached || value >= 16)
						{
							break;
						}
						result = result * 16 + value;
					}

					B32 valid = lexer->end_reached || numeric_suffix_table[lexer->current];
					Lexer_expect(lexer, valid, Lexer_Error_Kind__Numeric_Hex_Literal_Invalid, &error);
					numerical_value = result;
				}
				else if (digit == '0' && next && *next == 'b')
				{
					Lexer_advance(lexer, input);
					U64 result = 0;
					for (;;)
					{
						Lexer_advance(lexer, input);
						U8 value = (U8)(lexer->current - '0');
						if (lexer->end_reached || value >= 2)
						{
							break;
						}
						result = result * 2 + value;
					}

					B32 valid = lexer->end_reached || numeric_suffix_table[lexer->current];
					Lexer_expect(lexer, valid, Lexer_Error_Kind__Numeric_Binary_Literal_Invalid, &error);
					numerical_value = result;
				}
				else if (digit == '0' && next && U8_ascii_digit_is(*next))
				{	// Octal
					Lexer_advance(lexer, input);
					U64 result = 0;
					for (;;)
					{
						Lexer_advance(lexer, input);
						U8 value = (U8)(lexer->current - '0');
						if (lexer->end_reached || value >= 8)
						{
							break;
						}
						result = result * 8 + value;
					}

					B32 valid = lexer->end_reached || numeric_suffix_table[lexer->current];
					Lexer_expect(lexer, valid, Lexer_Error_Kind__Numeric_Octal_Literal_Invalid, &error);
					numerical_value = result;
				}
				else
				{	// Base 10 number
					U64 result = 0;
					for (;;)
					{
						U8 value = (U8)(lexer->current - '0');
						if (lexer->end_reached || value >= 10)
						{
							break;
						}
						result = result * 10 + value;
						numerical_value = result;
						Lexer_advance(lexer, input);
					}

					// This also gives a good hint that lexing assembly comes with some pain.
					if (lexer->current == ':')
					{
						token_kind = Token_Kind__Label_Numeric;
						Lexer_advance(lexer, input);
					}
					else if (lexer->current == 'f')
					{
						token_kind = Token_Kind__Label_Numeric_Reference_Forward;
						Lexer_advance(lexer, input);
					}
					else if (lexer->current == 'b')
					{
						token_kind = Token_Kind__Label_Numeric_Reference_Backward;
						Lexer_advance(lexer, input);
					}
					else
					{
						B32 valid = lexer->end_reached || numeric_suffix_table[lexer->current];
						Lexer_expect(lexer, valid, Lexer_Error_Kind__Numeric_Literal_Invalid, &error);
					}
				}
			}
			else
			{
				// NOTE: decide on whether erroring after a while bunch on invalid tokens are read.
				Lexer_diagnostic_fill(lexer, &error, Lexer_Error_Kind__Character_Unexpected);
			}


		} break;
		}

		B32 loop_infinite_avoided = lexer->index_before < lexer->index || lexer->end_reached || error.variant;
		assert_always_m(loop_infinite_avoided && "infinite loop edge case");

		if (token_kind && !error.variant)
		{


			Token *token = xar_push_m(tokens, arena);
			// Update phase
			*token = (Token)
			{
				.numerical_value = numerical_value,
				.index           = lexer->index_before,
				.row_index       = lexer->row_index,
				.column_index    = lexer->column_index_before,
				.size            = (U32)(lexer->index - lexer->index_before),
				.kind            = token_kind,
			};

			token_kind        = 0;
			numerical_value   = 0;
		}

		if (error.variant)
		{
			Diagnostics__push_error(diagnostics, &error);
			Lexer_advance_to_newline(lexer, input);
			error = (Diagnostic){ .filename = filename };
		}
	}

	return;
}
