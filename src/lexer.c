// After the lexer processes an item, it ALWAYS advances.

// TODO: handle local numeric labels.

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

typedef struct Lexer Lexer;
struct Lexer
{
	U8  *text;
	U32 *line_start_indexes;
	U32  text_size;
	U32  index;
	U32  index_before;
	U32  column_index;
	U32  column_index_before;
	U32  row_index;
	B32  end_reached;
	U8   current;
};

internal Lexer
Lexer_new(Arena *arena, Input *input)
{
	U8 *text  = input->data;
	U32 text_size = U32_cast_safe(input->count);
	B32 end_reached = 0 >= text_size;

	Lexer lexer =
	{
		.text = text,
		.text_size = text_size,
		.line_start_indexes = Arena_push_array_m(arena, U32, text_size),
		.end_reached = end_reached,
	};


	if (!end_reached)
	{
		lexer.current = text[0];
	}

	return lexer;
}

// It is a no-op if the end has been reached already.
internal void
Lexer_advance(Lexer *lexer)
{
	lexer->index        += !lexer->end_reached;
	lexer->column_index += !lexer->end_reached;
	lexer->current       = lexer->text[lexer->index - lexer->end_reached];
	lexer->end_reached   = lexer->text_size == 0 || lexer->index + 1 == lexer->text_size;

	// An index pointing out of bounds is of no-one's help.
	assert_always_m(lexer->index < lexer->text_size);

	return;
}

// It is a no-op if the end has been reached already.
internal void
Lexer_advance_newline(Lexer *lexer)
{
	assert_always_m(lexer->text_size > 0 && "cannot have read newline in zero-sized input");

	lexer->end_reached  = lexer->index + 1 == lexer->text_size;
	lexer->index       += !lexer->end_reached;
	lexer->row_index   += !lexer->end_reached;
	lexer->column_index = ~(lexer->end_reached - 1) & lexer->column_index;
	lexer->current      = lexer->text[lexer->index];

	lexer->line_start_indexes[lexer->row_index] = lexer->index;

	// An index pointing out of bounds is of no-one's help.
	assert_always_m(lexer->index < lexer->text_size);

	return;
}

internal U8 *
Lexer_peek_next(Lexer *lexer)
{
	// See invariants; we overallocate and assume ZII. As such the returned pointer is always valid.
	U8 *result = &lexer->text[lexer->index + 1];
	return result;
}

internal String8
Lexer_string(Lexer *lexer)
{
	String8 string =
	{
		.data =  lexer->text + lexer->index_before,
		.count = lexer->index - lexer->index_before,
	};
	return string;
}

internal Lexer_Error
Lexer_Error_new(Lexer_Error_Kind kind, Lexer *lexer)
{
	Lexer_Error error =
	{
		.kind               = kind,
		.row_index          = lexer->row_index,
		.column_begin_index = lexer->column_index_before,
		.column_end_index   = lexer->column_index,
	};

	return error;
}

internal Token_Array
LE_tokenize(Input *input, Arena *arena)
{
	Lexer_Error error   = {0};
	Lexer lexer = Lexer_new(arena, input);

	// We overestimate using the file size. Consider doing at the start of the program and not here.
	// Also here +1 to avoid index out of bounds, assuming ZII
	Token *tokens   = Arena_push_array_m(arena, Token, lexer.text_size + 1);
	U32 token_index = 0;

	Token_Kind token_kind = Token_Kind__None;
	U64 numerical_value = 0;
	for (;;)
	{
		B32 break_should = error.kind || lexer.end_reached;
		if (break_should)
		{
			break;
		}

		// Snapshot indexes before processing a token to determine its size.
		lexer.index_before = lexer.index;
		lexer.column_index_before = lexer.column_index;

		// NOTE: should maintain the logical order of Token_Kind enumeration.
		switch (lexer.current)
		{
		case ' ' : { Lexer_advance(&lexer); } break;
		case '\t': { Lexer_advance(&lexer); } break;

		// NOTE: no multi-line comment support (yet).
		case '#':
		{
			B32 break_should = 0;
			for (;;)
			{
				Lexer_advance(&lexer);

				// We don't want to count extra newline tokens because of comments.
				B32 newline_reached = lexer.current == '\n';
				if (newline_reached)
				{
					Lexer_advance_newline(&lexer);
				}

				break_should = newline_reached || lexer.end_reached;
				if (break_should)
				{
					break;
				}
			}

		} break;

		case ',' : { token_kind = Token_Kind__Comma;             Lexer_advance(&lexer); } break;
		case '(' : { token_kind = Token_Kind__Left_Parenthesis;  Lexer_advance(&lexer); } break;
		case ')' : { token_kind = Token_Kind__Right_Parenthesis; Lexer_advance(&lexer); } break;
		case '+' : { token_kind = Token_Kind__Plus;              Lexer_advance(&lexer); } break;
		case '-' : { token_kind = Token_Kind__Minus;             Lexer_advance(&lexer); } break;
		case '*' : { token_kind = Token_Kind__Star;              Lexer_advance(&lexer); } break;
		case '/' : { token_kind = Token_Kind__Slash;             Lexer_advance(&lexer); } break;
		case '~' : { token_kind = Token_Kind__Tilde;             Lexer_advance(&lexer); } break;
		case '^' : { token_kind = Token_Kind__Caret;             Lexer_advance(&lexer); } break;

		case '\n': { token_kind = Token_Kind__Newline;           Lexer_advance_newline(&lexer); } break;

		case '>':
		{
			token_kind = Token_Kind__Greater_Than;
			U8 *next = Lexer_peek_next(&lexer);
			if (*next == '>')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Shift_Right;
			}
			else if (*next == '=')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Greater_Equal;
			}
		} break;
		case '<':
		{
			Lexer_advance(&lexer);
			U8 next = lexer.current;
			if (next == '<')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Shift_Left;
			}
			else if (next == '=')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Less_Equal;
			}
		} break;

		case '=':
		{
			token_kind = Token_Kind__Assign;
			Lexer_advance(&lexer);
			U8 next = lexer.current;
			if (next == '=')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Equal;
			}
		} break;

		case '!':
		{
			token_kind = Token_Kind__Bang;
			Lexer_advance(&lexer);
			U8 next = lexer.current;
			if (next == '=')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Equal_Not;
			}
		} break;

		case '|':
		{
			token_kind = Token_Kind__Pipe;
			Lexer_advance(&lexer);
			U8 next = lexer.current;
			if (next == '|')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Logical_Or;
			}
		} break;
		case '&':
		{
			token_kind = Token_Kind__Ampersand;
			Lexer_advance(&lexer);
			U8 next = lexer.current;
			if (next == '&')
			{
				Lexer_advance(&lexer);
				token_kind = Token_Kind__Logical_And;
			}
		} break;

		case '%':
		{
			token_kind = Token_Kind__Percentage;
			Lexer_advance(&lexer);
			U8 next = lexer.current;
			if (U8_ascii_letter_is(next))
			{
				token_kind = Token_Kind__Relocation_Prefix;
			}
		} break;
		case ':':
		{       // We check the previous. This is to avoid including the colon into the label token.
			U32 token_index_previous = token_index - (token_index > 0);
			Token token_previous     = tokens[token_index_previous];

			if (token_previous.kind != Token_Kind__Label)
			{
				error = Lexer_Error_new(Lexer_Error_Kind__Character_Unexpected, &lexer);
			}
			Lexer_advance(&lexer);
		} break;

		case '\'':
		{
			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8 escaping_started   = 0;
			U8 escaped            = 0;
			U8 quote_ending_found = 0;
			B32 break_should      = 0;
			B32 value             = 0;

			for (;;)
			{
				Lexer_advance(&lexer);
				break_should = quote_ending_found || lexer.end_reached || error.kind;
				if (break_should)
				{
					break;
				}

				U8 character = lexer.current;

				if (escaping_started)
				{
					escaping_started = 0;
					value = escape_table[character];
					if (value == escape_value_invalid)
					{
						error = Lexer_Error_new(Lexer_Error_Kind__Character_Literal_Escape_Invalid, &lexer);
					}
				}
				else if (character == '\\')
				{
					escaping_started = 1;
					escaped = 1;
				}
				else if (character == '\'')
				{
					quote_ending_found = 1;
				}
				else if (character == '\n')
				{
					error = Lexer_Error_new(Lexer_Error_Kind__Character_Literal_Multiline_Unsupported, &lexer);
				}
				else
				{
					value = lexer.current;
				}
			}

			U32 characters_read_expected = 3 + escaped;
			U32 characters_read = lexer.index - lexer.index_before;

			if (characters_read_expected != characters_read)
			{
				error = Lexer_Error_new(Lexer_Error_Kind__Character_Literal_Multiple, &lexer);
			}
			if (!quote_ending_found && lexer.end_reached)
			{
				error = Lexer_Error_new(Lexer_Error_Kind__Character_Literal_Unterminated, &lexer);
			}

			token_kind = Token_Kind__Char_Literal;
			numerical_value = value;
		} break;
		case '\"':
		{
			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8 escaping_started   = 0;
			U8 quote_ending_found = 0;
			B32 break_should      = 0;
			for (;;)
			{
				Lexer_advance(&lexer);
				break_should = quote_ending_found || lexer.end_reached || error.kind;
				if (break_should)
				{
					break;
				}

				U8 character = lexer.current;

				if (escaping_started)
				{
					escaping_started = 0;
				}
				else if (character == '\\')
				{
					escaping_started = 1;
				}
				else if (character == '\"')
				{
					quote_ending_found = 1;
				}
				else if (character == '\n')
				{	// NOTE: it may make sense to introduce a flag that changes this behaviour.
					error = Lexer_Error_new(Lexer_Error_Kind__String_Multiline_Unsupported, &lexer);
				}
			}

			if (!quote_ending_found && lexer.end_reached)
			{
				error = Lexer_Error_new(Lexer_Error_Kind__String_Literal_Unterminated, &lexer);
			}

			token_kind = Token_Kind__String_Literal;
		} break;
		default:
		{
			if (LE_U8_identifier_start_is(lexer.current))
			{
				U8 character_start = lexer.current;
				B32 break_should = 0;
				B32 invalid = 0;
				for (;;)
				{
					break_should = invalid || lexer.end_reached;
					if (break_should)
					{
						break;
					}
					Lexer_advance(&lexer);
					invalid = !LE_U8_identifier_is(lexer.current);
				}

				String8 token_string = Lexer_string(&lexer);
				Directive_Kind directive_kind = Directive_Kind__from_String8(token_string);

				// This gives a good hint that lexing assembly comes with some pain.
				if (token_string.count == 1 && character_start == '.')
				{
					token_kind = Token_Kind__Dot;
				}
				else if (lexer.current == ':')
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
			else if (U8_ascii_digit_is(lexer.current))
			{
				U8 digit = lexer.current;
				U8 *next = Lexer_peek_next(&lexer);

				if (digit == '0' && next && *next == 'x')
				{
					Lexer_advance(&lexer);
					U64 result = 0;
					for (;;)
					{
						Lexer_advance(&lexer);
						U8 value = hex_table[lexer.current];
						if (lexer.end_reached || value >= 16)
						{
							break;
						}
						result = result * 16 + value;
					}

					B32 valid = lexer.end_reached || numeric_suffix_table[lexer.current];
					if (!valid)
					{
						error = Lexer_Error_new(Lexer_Error_Kind__Numeric_Hex_Literal_Invalid, &lexer);
					}
					numerical_value = result;
				}
				else if (digit == '0' && next && *next == 'b')
				{
					Lexer_advance(&lexer);
					U64 result = 0;
					for (;;)
					{
						Lexer_advance(&lexer);
						U8 value = (U8)(lexer.current - '0');
						if (lexer.end_reached || value >= 2)
						{
							break;
						}
						result = result * 2 + value;
					}

					B32 valid = lexer.end_reached || numeric_suffix_table[lexer.current];
					if (!valid)
					{
						error = Lexer_Error_new(Lexer_Error_Kind__Numeric_Binary_Literal_Invalid, &lexer);
					}
					numerical_value = result;
				}
				else if (digit == '0' && next && U8_ascii_digit_is(*next))
				{	// Octal
					Lexer_advance(&lexer);
					U64 result = 0;
					for (;;)
					{
						Lexer_advance(&lexer);
						U8 value = (U8)(lexer.current - '0');
						if (lexer.end_reached || value >= 8)
						{
							break;
						}
						result = result * 8 + value;
					}

					B32 valid = lexer.end_reached || numeric_suffix_table[lexer.current];
					if (!valid)
					{
						error = Lexer_Error_new(Lexer_Error_Kind__Numeric_Octal_Literal_Invalid, &lexer);
					}
					numerical_value = result;
				}
				else
				{	// Base 10 number
					U64 result = 0;
					for (;;)
					{
						U8 value = (U8)(lexer.current - '0');
						if (lexer.end_reached || value >= 10)
						{
							break;
						}
						result = result * 10 + value;
						Lexer_advance(&lexer);
					}

					// This also gives a good hint that lexing assembly comes with some pain.
					if (lexer.current == ':')
					{
						token_kind = Token_Kind__Label_Numeric;
					}
					else if (lexer.current == 'f')
					{
						token_kind = Token_Kind__Label_Numeric_Reference_Forward;
					}
					else if (lexer.current == 'b')
					{
						token_kind = Token_Kind__Label_Numeric_Reference_Backwards;
					}
					else
					{
						B32 valid = lexer.end_reached || numeric_suffix_table[lexer.current];
						if (!valid)
						{
							error = Lexer_Error_new(Lexer_Error_Kind__Numeric_Literal_Invalid, &lexer);
						}
						numerical_value = result;
					}
				}

				token_kind = Token_Kind__Number_Literal;
			}
			else
			{
				// NOTE: decide on whether erroring after a while bunch on invalid tokens are read.
				error = Lexer_Error_new(Lexer_Error_Kind__Character_Unexpected, &lexer);
			}


		} break;
		}

		B32 loop_infinite_avoided = lexer.index_before < lexer.index || error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop edge case");

		if (token_kind && !error.kind)
		{
			// Update phase
			tokens[token_index] = (Token)
			{
				.numerical_value = numerical_value,
				.index           = lexer.index_before,
				.row_index       = lexer.row_index,
				.column_index    = lexer.column_index_before,
				.size            = (U32)(lexer.index - lexer.index_before),
				.kind            = token_kind,
			};

			token_kind        = 0;
			numerical_value   = 0;
			token_index      += 1;
		}
	}

	assert_always_m(token_index < lexer.text_size && "too many tokens");

	Token_Array token_array =
	{
		.tokens             = tokens,
		.line_start_indexes = lexer.line_start_indexes,
		.token_count        = token_index,
		.error              = error,
	};

	return token_array;
}
