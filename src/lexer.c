internal B32
LE_U8_identifier_start_is(U8 character)
{
	B32 result = U8_ascii_letter_is(character) || character == '_';
	return result;
}

internal B32
LE_U8_identifier_is(U8 character)
{
	B32 result = U8_ascii_letter_is(character) || character == '_' || U8_ascii_digit_is(character) ;
	return result;
}

internal B32
LE_U8_number_character_is(U8 character)
{
	B32 result = U8_ascii_digit_is(character);
	return result;
}

internal U8 *
LE_String8_get(String8 *string, U32 index)
{
	U8 *next = 0;
	if (index < string->count)
	{
		next = &string->data[index + 1];
	}
	return next;
}

internal void
LE_advance(U32 *index, U32 *column_index)
{
	*index        += 1;
	*column_index += 1;
}

internal void
LE_advance_newline(U32 *index, U32 *column_index, U32 *row_index, U32 *line_start_indexes)
{
	*index        += 1;
	*row_index    += 1;
	*column_index  = 0;

	line_start_indexes[*row_index] = *index;
}

internal Token_Array
LE_tokenize(String8 *input, Arena *arena)
{
	U32 row_index    = 0;
	U32 column_index = 0;
	U32 token_index  = 0;

	U8 *input_data  = input->data;
	U32 input_count = U32_cast_safe(input->count);

	Lexer_Error error = {0};

	// We overestimate using the file size. Consider doing at the start of the program and not here.
	Token *tokens             = Arena_push_array_m(arena, Token, input_count);
	U32   *line_start_indexes = Arena_push_array_m(arena, U32, input_count);


	Token_Kind token_kind = Token_Kind__None;

	U32 index = 0;
	for (;;)
	{
		U32 index_before = index;

		// NOTE: should maintain the logical order of Token_Kind enumeration.
		switch (input_data[index])
		{
		case ' ' : { LE_advance(&index, &column_index); } break;
		case '\t': { LE_advance(&index, &column_index); } break;

		// NOTE: no multi-line comment support (yet).
		case '#':
		{
			B32 break_should = 0;
			for (;;)
			{
				LE_advance(&index, &column_index);

				break_should = input_data[index] == '\n' || index >= input_count;
				if (break_should)
				{
					break;
				}
			}

		} break;

		case '.' : { token_kind = Token_Kind__Dot;               LE_advance(&index, &column_index); } break;
		case ',' : { token_kind = Token_Kind__Comma;             LE_advance(&index, &column_index); } break;
		case ':' : { token_kind = Token_Kind__Colon;             LE_advance(&index, &column_index); } break;

		case '(' : { token_kind = Token_Kind__Left_Parenthesis;  LE_advance(&index, &column_index); } break;
		case ')' : { token_kind = Token_Kind__Right_Parenthesis; LE_advance(&index, &column_index); } break;

		case '+' : { token_kind = Token_Kind__Plus;              LE_advance(&index, &column_index); } break;
		case '-' : { token_kind = Token_Kind__Minus;             LE_advance(&index, &column_index); } break;
		case '*' : { token_kind = Token_Kind__Star;              LE_advance(&index, &column_index); } break;
		case '/' : { token_kind = Token_Kind__Slash;             LE_advance(&index, &column_index); } break;

		case '%' : { token_kind = Token_Kind__Percentage;        LE_advance(&index, &column_index); } break;

		case '\n': { token_kind = Token_Kind__Newline;           LE_advance_newline(&index, &column_index, &row_index, line_start_indexes); } break;

		case '>':
		{
			LE_advance(&index, &column_index);
			U8 *next = LE_String8_get(input, index);
			if (next && *next == '>')
			{
				LE_advance(&index, &column_index);
				token_kind = Token_Kind__Shift_Right;
			}
			else
			{
				token_kind = Token_Kind__Major;
			}
		} break;
		case '<':
		{
			LE_advance(&index, &column_index);
			U8 *next = LE_String8_get(input, index);
			if (next && *next == '<')
			{
				LE_advance(&index, &column_index);
				token_kind = Token_Kind__Shift_Left;
			}
			else
			{
				token_kind = Token_Kind__Minor;
			}
		} break;

		case '\'':
		{
			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8  escaping_started   = 0;
			U8  quote_ending_found = 0;
			B32 break_should       = 0;
			for (;;)
			{
				LE_advance(&index, &column_index);
				U8  character = input_data[index];

				if (escaping_started)
				{
					escaping_started = 0;
				}
				else if (character == '\\')
				{
					escaping_started = 1;
				}
				else if (character == '\'')
				{
					quote_ending_found = 1;
				}
				else if (character == '\n')
				{
					error = (Lexer_Error)
					{
						.kind               = Lexer_Error_Kind__Character_Literal_Multiline_Unsupported,
						.row_index          = row_index,
						.column_begin_index = column_index - (index - index_before),
						.column_end_index   = column_index,
					};
				}

				B32 char_invalid = quote_ending_found && index != index_before + 2;
				if (quote_ending_found && char_invalid)
				{
					Lexer_Error_Kind kind =
						index - index_before == 1 ?
						Lexer_Error_Kind__Character_Literal_Empty :
						Lexer_Error_Kind__Character_Literal_Multiple;
					error = (Lexer_Error)
					{
						.kind         = kind,
						.row_index    = row_index,
						.column_begin_index = column_index - (index - index_before),
						.column_end_index   = column_index,
					};
				}

				break_should = quote_ending_found || error.kind || index >= input_count;
				if (break_should)
				{
					break;
				}

			}

			token_kind = Token_Kind__Char_Literal;
		} break;
		case '\"':
		{
			U8  character_quote = input_data[index];

			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8  escaping_started   = 0;
			U8  quote_ending_found = 0;
			B32 break_should       = 0;
			for (;;)
			{
				LE_advance(&index, &column_index);
				U8  character = input_data[index];

				if (escaping_started)
				{
					escaping_started = 0;
				}
				else if (character == '\\')
				{
					escaping_started = 1;
				}
				else if (character == character_quote)
				{
					quote_ending_found = 1;
				}
				else if (character == '\n')
				{
					error = (Lexer_Error)
					{
						.kind         = Lexer_Error_Kind__String_Multiline_Unsupported,
						.row_index    = row_index,
						.column_begin_index = column_index - (index - index_before),
						.column_end_index   = column_index,
					};
				}

				break_should = quote_ending_found || error.kind || index >= input_count;
				if (break_should)
				{
					break;
				}

			}

			token_kind = Token_Kind__String_Literal;

		} break;
		default:
		{
			if (LE_U8_identifier_start_is(input_data[index]))
			{
				B32 break_should = 0;
				for (;;)
				{
					LE_advance(&index, &column_index);
					B32 invalid = !LE_U8_identifier_is(input_data[index]);

					break_should = invalid || index >= input_count;
					if (break_should)
					{
						break;
					}
				}
			}
			// TODO: support float (hex float?), literal hex, literal octal, literal binary.
			else if (U8_ascii_digit_is(input_data[index]))
			{
				B32 break_should = 0;
				for (;;)
				{
					LE_advance(&index, &column_index);
					B32 invalid = !U8_ascii_digit_is(input_data[index]);

					break_should = invalid || index >= input_count;
					if (break_should)
					{
						break;
					}
				}
			}
			else
			{
				error = (Lexer_Error)
				{
					.kind               = Lexer_Error_Kind__Character_Unexpected,
					.row_index          = row_index,
					.column_begin_index = column_index - (index - index_before),
					.column_end_index   = column_index,
				};
			}


		} break;
		}

		B32 loop_infinite_avoided = (index_before < index) || error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop edge case");

		if (token_kind != 0)
		{
			// Update phase
			tokens[token_index] = (Token)
			{
				.index        = index_before,
				.row_index    = row_index,
				.column_index = column_index,
				.size         = (U32)(index - index_before),
				.kind         = token_kind,
			};

			token_index += 1;
		}

		B32 break_should = error.kind || index >= input_count;
		if (break_should)
		{
			break;
		}
	}

	Token_Array token_array =
	{
		.tokens             = tokens,
		.line_start_indexes = line_start_indexes,
		.token_count        = token_index,
		.error              = error,
	};

	return token_array;
}
