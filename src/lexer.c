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
		next = &string->data[index];
	}
	return next;
}

typedef struct Lexer_Cursor Lexer_Cursor;
struct Lexer_Cursor
{
	U8  *text;
	U32 *line_start_indexes;
	U32  text_size;
	U32  index;
	U32  index_before;
	U32  column_index;
	U32  column_index_before;
	U32  row_index;
	B32  end_of_file_reached;
	U8   character;
};

internal Lexer_Cursor
Lexer_Cursor_new(Arena *arena, String8 *input)
{
	U8 *text  = input->data;
	U32 text_size = U32_cast_safe(input->count);

	Lexer_Cursor cursor = {0};
	cursor.text = text;
	cursor.text_size = text_size;
	cursor.line_start_indexes = Arena_push_array_m(arena, U32, text_size);
	cursor.end_of_file_reached = 0 >= text_size;

	if (!cursor.end_of_file_reached)
	{
		cursor.character = text[0];
	}

	return cursor;
}

internal void
Lexer_Cursor_advance(Lexer_Cursor *cursor)
{
	cursor->index += 1;
	cursor->column_index += 1;
	cursor->end_of_file_reached = cursor->index >= cursor->text_size;

	if (!cursor->end_of_file_reached)
	{
		cursor->character = cursor->text[cursor->index];
	}
}

internal void
Lexer_Cursor_advance_newline(Lexer_Cursor *cursor)
{
	cursor->index += 1;
	cursor->row_index += 1;
	cursor->column_index = 0;
	cursor->end_of_file_reached = cursor->index >= cursor->text_size;

	if (!cursor->end_of_file_reached)
	{
		cursor->character = cursor->text[cursor->index];
		cursor->line_start_indexes[cursor->row_index] = cursor->index;
	}
}

internal U8 *
Lexer_Cursor_peek_next(Lexer_Cursor *cursor)
{
	U8 *next = 0;
	if (cursor->index + 1 < cursor->text_size)
	{
		next = &cursor->text[cursor->index + 1];
	}
	return next;
}

internal Lexer_Error
Lexer_Error_new(Lexer_Error_Kind kind, Lexer_Cursor *cursor)
{
	Lexer_Error error =
	{
		.kind               = kind,
		.row_index          = cursor->row_index,
		.column_begin_index = cursor->column_index_before,
		.column_end_index   = cursor->column_index,
	};

	return error;
}

internal Token_Array
LE_tokenize(String8 *input, Arena *arena)
{
	Lexer_Error error   = {0};
	Lexer_Cursor cursor = Lexer_Cursor_new(arena, input);

	// We overestimate using the file size. Consider doing at the start of the program and not here.
	Token *tokens   = Arena_push_array_m(arena, Token, cursor.text_size);
	U32 token_index = 0;

	Token_Kind token_kind = Token_Kind__None;
	for (;;)
	{
		B32 break_should = error.kind || cursor.end_of_file_reached;
		if (break_should)
		{
			break;
		}

		// Snapshot indexes before processing a token to determine its size.
		cursor.index_before = cursor.index;
		cursor.column_index_before = cursor.column_index;

		// NOTE: should maintain the logical order of Token_Kind enumeration.
		switch (cursor.character)
		{
		case ' ' : { Lexer_Cursor_advance(&cursor); } break;
		case '\t': { Lexer_Cursor_advance(&cursor); } break;

		// NOTE: no multi-line comment support (yet).
		case '#':
		{
			B32 break_should = 0;
			for (;;)
			{
				Lexer_Cursor_advance(&cursor);

				// We don't want to count extra newline tokens because of comments.
				B32 newline_reached = cursor.character == '\n';
				if (newline_reached)
				{
					Lexer_Cursor_advance_newline(&cursor);
				}

				break_should = newline_reached || cursor.end_of_file_reached;
				if (break_should)
				{
					break;
				}
			}

		} break;

		case ',' : { token_kind = Token_Kind__Comma;             Lexer_Cursor_advance(&cursor); } break;

		case '(' : { token_kind = Token_Kind__Left_Parenthesis;  Lexer_Cursor_advance(&cursor); } break;
		case ')' : { token_kind = Token_Kind__Right_Parenthesis; Lexer_Cursor_advance(&cursor); } break;

		case '+' : { token_kind = Token_Kind__Plus;              Lexer_Cursor_advance(&cursor); } break;
		case '-' : { token_kind = Token_Kind__Minus;             Lexer_Cursor_advance(&cursor); } break;
		case '*' : { token_kind = Token_Kind__Star;              Lexer_Cursor_advance(&cursor); } break;
		case '/' : { token_kind = Token_Kind__Slash;             Lexer_Cursor_advance(&cursor); } break;

		case '%' : { token_kind = Token_Kind__Percentage;        Lexer_Cursor_advance(&cursor); } break;

		case '\n': { token_kind = Token_Kind__Newline;           Lexer_Cursor_advance_newline(&cursor); } break;

		case '>':
		{
			Lexer_Cursor_advance(&cursor);
			U8 *next = Lexer_Cursor_peek_next(&cursor);
			if (next && *next == '>')
			{
				Lexer_Cursor_advance(&cursor);
				token_kind = Token_Kind__Shift_Right;
			}
			else
			{
				token_kind = Token_Kind__Major;
			}
		} break;
		case '<':
		{
			Lexer_Cursor_advance(&cursor);
			U8 *next = Lexer_Cursor_peek_next(&cursor);
			if (next && *next == '<')
			{
				Lexer_Cursor_advance(&cursor);
				token_kind = Token_Kind__Shift_Left;
			}
			else
			{
				token_kind = Token_Kind__Minor;
			}
		} break;

		case '.':
		{	// This could be either a label or a directive.
			B32 invalid = 0;
			U8 character_last = '.';
			for (;;)
			{
				B32 break_should = invalid || cursor.end_of_file_reached;
				if (break_should)
				{
					break;
				}
				Lexer_Cursor_advance(&cursor);
				character_last = cursor.character;
				invalid = !LE_U8_identifier_is(character_last);
			}

			if (character_last == ':')
			{
				token_kind = Token_Kind__Label;
			}
			else if (character_last == ' ' || character_last == '\n' || cursor.end_of_file_reached)
			{
				token_kind = Token_Kind__Directive;
			}
			else
			{
				error = Lexer_Error_new(Lexer_Error_Kind__Label_Directive_Invalid, &cursor);
			}

		} break;

		case '\'':
		{
			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8 escaping_started   = 0;
			U8 quote_ending_found = 0;
			B32 break_should      = 0;
			// NOTE: This could be simplified with a peek_next_n method.
			for (;;)
			{
				Lexer_Cursor_advance(&cursor);
				break_should = quote_ending_found || cursor.end_of_file_reached || error.kind;
				if (break_should)
				{
					break;
				}

				U8 character = cursor.character;

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
					error = Lexer_Error_new(Lexer_Error_Kind__Character_Literal_Multiline_Unsupported, &cursor);
				}

				B32 char_invalid = quote_ending_found && cursor.index != cursor.index_before + 2;
				if (quote_ending_found && char_invalid)
				{
					Lexer_Error_Kind kind =
						cursor.index - cursor.index_before == 1 ?
						Lexer_Error_Kind__Character_Literal_Empty :
						Lexer_Error_Kind__Character_Literal_Multiple;
					error = Lexer_Error_new(kind, &cursor);
				}
			}

			token_kind = Token_Kind__Char_Literal;
		} break;
		case '\"':
		{
			U8 character_quote = cursor.character;

			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8 escaping_started   = 0;
			U8 quote_ending_found = 0;
			B32 break_should      = 0;
			for (;;)
			{
				Lexer_Cursor_advance(&cursor);
				break_should = quote_ending_found || cursor.end_of_file_reached || error.kind;
				if (break_should)
				{
					break;
				}

				U8 character = cursor.character;

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
				{	// NOTE: it may make sense to introduce a flag that changes this behaviour.
					error = Lexer_Error_new(Lexer_Error_Kind__String_Multiline_Unsupported, &cursor);
				}
			}

			token_kind = Token_Kind__String_Literal;
		} break;
		default:
		{
			if (LE_U8_identifier_start_is(cursor.character))
			{
				B32 break_should = 0;
				B32 invalid = 0;
				for (;;)
				{
					break_should = invalid || cursor.end_of_file_reached;
					if (break_should)
					{
						break;
					}
					Lexer_Cursor_advance(&cursor);
					invalid = !LE_U8_identifier_is(cursor.character);
				}
			}
			// TODO: support float (hex float?), literal hex, literal octal, literal binary.
			else if (U8_ascii_digit_is(cursor.character))
			{
				B32 break_should = 0;
				B32 invalid = 0;
				for (;;)
				{
					break_should = invalid || cursor.end_of_file_reached;
					if (break_should)
					{
						break;
					}
					Lexer_Cursor_advance(&cursor);
					invalid = !U8_ascii_digit_is(cursor.character);
				}
			}
			else
			{
				// NOTE: decide on whether erroring after a while bunch on invalid tokens are read.
				error = Lexer_Error_new(Lexer_Error_Kind__Character_Unexpected, &cursor);
			}


		} break;
		}

		B32 loop_infinite_avoided = cursor.index_before < cursor.index || error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop edge case");

		if (token_kind != 0)
		{
			// Update phase
			tokens[token_index] = (Token)
			{
				.index        = cursor.index_before,
				.row_index    = cursor.row_index,
				.column_index = cursor.column_index,
				.size         = (U32)(cursor.index - cursor.index_before),
				.kind         = token_kind,
			};

			token_index += 1;
		}
	}

	Token_Array token_array =
	{
		.tokens             = tokens,
		.line_start_indexes = cursor.line_start_indexes,
		.token_count        = token_index,
		.error              = error,
	};

	return token_array;
}
