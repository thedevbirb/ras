internal Token_Cursor
Token_Cursor_new(Input *input, Token_Array *token_array)
{
	Token_Cursor cursor =
	{
		.input       = input,
		.token_array = token_array,
		.end_reached = 0 >= token_array->token_count,
		.current     = token_array->tokens[0],
	};

	return cursor;
}

// It is a no-op if the end has been reached already.
internal void
Token_Cursor_advance(Token_Cursor *cursor)
{
	cursor->end_reached = cursor->index + 1 == cursor->token_array->token_count;
	cursor->index      += !cursor->end_reached;
	cursor->current     = cursor->token_array->tokens[cursor->index];

	return;
}

internal Token *
Token_Cursor_peek_next(Token_Cursor *cursor)
{
	Token *next = &cursor->token_array->tokens[cursor->index + 1];
	return next;
}

internal String8
Token_Cursor_substring(Token_Cursor *cursor)
{
	String8 string =
	{
		.data  = cursor->input->data + cursor->current.index,
		.count = (U64)cursor->current.size,
	};
	return string;
}

internal Parser_Error
Parser_Error_new(Parser_Error_Kind kind, Token_Cursor *cursor)
{
	Parser_Error error =
	{
		.kind               = kind,
		.row_index          = cursor->current.row_index,
		.column_begin_index = cursor->current.column_index,
		.column_end_index   = cursor->current.column_index + cursor->current.size - 1,
	};

	assert_always_m(error.column_begin_index <= error.column_end_index && "index bug");

	return error;
}

internal Directive_Kind
Directive_Kind__from_String8(String8 string)
{
	Directive_Kind kind = Directive_Kind__None;

	U32 index = 0;
	B32 found = 0;
	for (;;)
	{
		B32 break_should = found || index >= Directive_Kind__COUNT;
		if (break_should)
		{
			break;
		}

		const char *target = Directive_Kind_strings[index];

		U32 index_match = 0;
		B32 mismatch = 0;
		for (;;)
		{
			B32 break_should = mismatch || index_match >= string.count || target[index_match] == '\0';
			if (break_should)
			{
				break;
			}

			mismatch = string.data[index_match] != target[index_match];
			index_match += 1;
		}

		found = !mismatch && index_match == string.count && target[index_match] == '\0';
		if (found)
		{
			kind = index;
		}
		else
		{
			index += 1;
		}
	}

	return kind;
}

// TODO: how am I transforming output after this stage?
internal Parser_Result
PA_parse(Input *input, Token_Array *token_array, Object_File_Section *sections, Arena *arena)
{
	Token_Cursor cursor = Token_Cursor_new(input, token_array);
	Parser_Error error  = {0};
	Parser_Result result = {0};

	Symbol_Hashmap symbol_hashmap = {0};
	Symbol_Hashmap_initialize(&symbol_hashmap, arena);

	// By default, the section is `.text`.
	Object_File_Section section = sections[ELF64_Section__Text];
	Object_File_Section section_string_table = sections[ELF64_Section__String_Table];

	for (;;)
	{
		B32 break_should = cursor.end_reached || error.kind;
		if (break_should)
		{
			break;
		}

		U32 index_before = cursor.index;

		switch (cursor.current.kind)
		{
		case Token_Kind__Newline: { Token_Cursor_advance(&cursor); } break;
		case Token_Kind__Label:
		{
			String8 key = Token_Cursor_substring(&cursor);
			key.count -= key.count > 0; // Remove the colon.

			U32 offset = Object_File_Section_write(&section_string_table, key.data, key.count);
			ELF64_Symbol symbol =
			{
				.string_table_offset = offset,
				.value = section.offset,
				.section_index = section.section_index
			};
			B32 overridden = Symbol_Hashmap_put(&symbol_hashmap, key, symbol);
			if (overridden)
			{
				error = Parser_Error_new(Parser_Error_Kind__Label_Duplicate, &cursor);
			}

			Token_Cursor_advance(&cursor);
		} break;
		case Token_Kind__Directive:
		{
			String8 substring = Token_Cursor_substring(&cursor);
			Directive_Kind directive_kind = Directive_Kind__from_String8(substring);

			switch (directive_kind)
			{
			case Directive_Kind__None:
			{
				error = Parser_Error_new(Parser_Error_Kind__Directive_Unknown, &cursor);
			} break;
			case Directive_Kind__Section:
			{
				Token_Cursor_advance(&cursor);
				String8 substring = Token_Cursor_substring(&cursor);
				Directive_Kind directive_kind = Directive_Kind__from_String8(substring);
				ELF64_Section section_index = ELF64_Section_from_Directive_Kind[directive_kind];
				section = sections[section_index];

				if (!section_index)
				{
					error = Parser_Error_new(Parser_Error_Kind__Directive_Section_Argument_Invalid, &cursor);
				}

				Token_Cursor_advance(&cursor);
			} break;
			case Directive_Kind__Align:
			{
				Token *token_next = Token_Cursor_peek_next(&cursor);
				if (!token_next || token_next->kind == Token_Kind__Newline)
				{
					error = Parser_Error_new(Parser_Error_Kind__Directive_Align_Argument_Missing, &cursor);
				}
				// Actually, here I should grab all the tokens until newline, and try to parse the
				// expression.
				else if (token_next->kind == Token_Kind__Number_Literal)
				{
					// TODO: parse the number
					Token_Cursor_advance(&cursor);
				}
				else
				{
					Token_Cursor_advance(&cursor);
					error = Parser_Error_new(Parser_Error_Kind__Directive_Align_Argument_Invalid, &cursor);
				}
			} break;
			default:
			{
				ELF64_Section section_kind = ELF64_Section_from_Directive_Kind[directive_kind];
				assert_always_m(section_kind && "unhandled directive");

				section = sections[section_kind];
				Token_Cursor_advance(&cursor);
			} break;
			}
		} break;
		default:
		{
			error = Parser_Error_new(Parser_Error_Kind__Line_Invalid, &cursor);
		} break;
		}

		B32 loop_infinite_avoided = cursor.index > index_before || error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop in parser");
	}

	result.error = error;

	return result;
}
