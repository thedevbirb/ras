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

// internal U8 *
// Lexer_peek_next(Lexer *lexer)
// {
// 	U8 *result = 0;
// 	if (!lexer->end_reached)
// 	{
// 		result = &lexer->source->input.data[lexer->index + 1];
// 	}
// 	return result;
// }
//
// // It is a no-op if the end has been reached already.
// internal void
// Lexer_advance(Lexer *lexer)
// {
// 	B32                  = lexer->source->input.count == 0 || lexer->index >= lexer->source->input.count;
// 	lexer->index        += !lexer->end_reached;
// 	lexer->current       = lexer->source->input.data[lexer->index];
//
// 	// An index pointing out of bounds is of no-one's help.
// 	assert_always_m(lexer->index < lexer->source->input.count);
//
// 	return;
// }
//
// internal void
// Lexer_expect(Lexer *lexer, B32 condition, Lexer_Error_Kind error_kind)
// {
// 	if (!condition && !lexer->error.kind)
// 	{
// 		lexer->error = (Lexer_Error)
// 		{
// 			.index = lexer->index,
// 			.kind  = error_kind,
// 		};
// 	}
// 	return;
// }

// INVARIANT
//
// Assumes extra 8 bytes of zero after source->count.
//
// We don't want annoying codepaths because I read past one, so I assume the past one is zero.
// This simplifies a lot of check, where the last character being zero is already enough of a check.
internal Token_2
token_peek
(
	Source          *source,
	U32              index_current,
	Diagnostic_List *diagnostics,
	Arena           *arena
)
{
	U8  *data  = source->data;
	U64  count = source->count;
	for (U32 i = 0; i < 8; i++)
	{
		assert_always_m(data[count + i] == 0);
	}

	Token_2 token = {0};

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
			if (data[index] == '>')
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
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Literal_Unterminated];
				diagnostic->location = source->start_offset_logical + index;
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);

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
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Escape_Sequence_Invalid];
						diagnostic->location = source->start_offset_logical + error_index;
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
					result = result * 16 + first;
					index += 1;
				}
				else if (octal_prefix)
				{
					// We read up to two more characters, and the current digit counts.
					result = digit;

					index += 1;
					if (data[index] - '0' < 8)
					{
						result = result * 8 + (data[index] - '0');
						index += 1;
					}

					if (data[index] - '0' < 8)
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
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Escape_Sequence_Invalid];
						diagnostic->location = source->start_offset_logical + index;
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
					index += 1;
				}
			}
			else if (data[index] == '\n')
			{
				token.kind = Token_Kind__Error;
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Literal_Multiline_Unsupported];
				diagnostic->location = source->start_offset_logical + index;
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
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
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Literal_Unterminated];
				diagnostic->location = source->start_offset_logical + index;
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
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
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Escape_Sequence_Invalid];
						diagnostic->location = source->start_offset_logical + index;
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
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
							Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
							diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Hex_Literal_Invalid];
							diagnostic->location = source->start_offset_logical + error_index;
							SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
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
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__String_Multiline_Unsupported];
					diagnostic->location = source->start_offset_logical + index;
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
			}

			if (!quote_ending_found)
			{
					token.kind = Token_Kind__Error;
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__String_Literal_Unterminated];
					diagnostic->location = source->start_offset_logical + index;
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
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
			else if (U8_ascii_digit_is(data[index]))
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
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Hex_Literal_Invalid];
						diagnostic->location = source->start_offset_logical + index;
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
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
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Binary_Literal_Invalid];
						diagnostic->location = source->start_offset_logical + index;
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
				}
				else if (digit == '0' && U8_ascii_digit_is(next))
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
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Numeric_Octal_Literal_Invalid];
						diagnostic->location = source->start_offset_logical + index;
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
				}
				else
				{	// Base 10 number
					for (;;)
					{
						U8 value = (U8)(data[index] - '0');
						if (value >= 10)
						{
							break;
						}
						token.numerical_value = token.numerical_value * 10 + value;
						index += 1;
					}
				}
			}
			else
			{
				// NOTE: decide on whether erroring after a while bunch on invalid tokens are read.
				token.kind = Token_Kind__Error;
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->message  = lexer_error_kind_messages[Lexer_Error_Kind__Character_Unexpected];
				diagnostic->location = source->start_offset_logical + index;
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}


		} break;
		}

		assert_always_m(start_index <= index);
		B32 loop_infinite_avoided = start_index < index || index >= count || token.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop edge case");

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

	return token;
}

internal void
token_next
(
	Token_Cursor    *cursor,
	Diagnostic_List *diagnostics,
	Arena           *arena
)
{
	cursor->current = token_peek(cursor->source, cursor->source_index, diagnostics, arena);
	if (cursor->current.kind != Token_Kind__None)
	{
		cursor->source_index = cursor->current.index + cursor->current.size;
	}
}
