#ifndef PARSER_H
#define PARSER_H

typedef enum Directive_Kind
{
	Directive_Kind__None,
	Directive_Kind__Section,
	Directive_Kind__Text,
	Directive_Kind__Data,
	Directive_Kind__Read_Only_Data,
	Directive_Kind__BSS,
	Directive_Kind__Globl,
	Directive_Kind__Word,
	Directive_Kind__Ascii,
	Directive_Kind__Asciz,
	Directive_Kind__Align,
	Directive_Kind__COUNT,
}
Directive_Kind;

global const char *Directive_Kind_strings[Directive_Kind__COUNT] =
{
	[Directive_Kind__None]            = "",
	[Directive_Kind__Section]         = ".section",
	[Directive_Kind__Text]            = ".text",
	[Directive_Kind__Data]            = ".data",
	[Directive_Kind__Read_Only_Data]  = ".rodata",
	[Directive_Kind__BSS]             = ".bss",
	[Directive_Kind__Globl]           = ".globl",
	[Directive_Kind__Word]            = ".word",
	[Directive_Kind__Ascii]           = ".ascii",
	[Directive_Kind__Align]           = ".align",
	[Directive_Kind__Asciz]           = ".asciz",
};

// typedef enum Directive_Argument_Kind Directive_Argument_Kind
// {
// }
// Directive_Argument_Kind;

typedef struct InputSlice InputSlice;
struct InputSlice
{
	U32 index_begin;
	U32 index_end;
};

typedef struct Token_Cursor Token_Cursor;
struct Token_Cursor
{
	Token_Array *token_array;
	Token current;
	U32 index;
	B32 end_reached;
};

internal Token_Cursor
Token_Cursor_new(Token_Array *token_array)
{
	Token_Cursor cursor = {0};
	cursor.token_array = token_array;
	cursor.end_reached = 0 >= token_array->token_count;
	if (!cursor.end_reached)
	{
		cursor.current = token_array->tokens[0];
	}

	return cursor;
}

// It is a no-op if the end has been reached already.
internal void
Token_Cursor_advance(Token_Cursor *cursor)
{
	cursor->index += !cursor->end_reached;
	cursor->end_reached = cursor->index >= cursor->token_array->token_count;
	if (!cursor->end_reached)
	{
		cursor->current = cursor->token_array->tokens[cursor->index];
	}
	return;
}

internal Token *
Token_Cursor_peek_next(Token_Cursor *cursor)
{
	Token *next = 0;
	if (cursor->index + 1 < cursor->token_array->token_count)
	{
		next = &cursor->token_array->tokens[cursor->index + 1];
	}

	return next;
}

typedef enum Parser_Error_Kind
{
	Parser_Error_Kind__None,
	Parser_Error_Kind__Directive_Unknown,
	Parser_Error_Kind__Directive_Section_Argument_Missing,
	Parser_Error_Kind__Directive_Section_Argument_Invalid,
	Parser_Error_Kind__Directive_Align_Argument_Missing,
	Parser_Error_Kind__Directive_Align_Argument_Invalid,

	Parser_Error_Kind__COUNT,
}
Parser_Error_Kind;

global const char *Parser_Error_Kind_messages[Parser_Error_Kind__COUNT] =
{
	[Parser_Error_Kind__None]                               = "",
	[Parser_Error_Kind__Directive_Unknown]                  = "unknown directive found",
	[Parser_Error_Kind__Directive_Section_Argument_Missing] = "section directive is missing the argument",
	[Parser_Error_Kind__Directive_Section_Argument_Invalid] = "section directive argument is invalid",
	[Parser_Error_Kind__Directive_Align_Argument_Missing]   = "align directive is missing the argument",
	[Parser_Error_Kind__Directive_Align_Argument_Invalid]   = "align directive argument is not a number literal",
};

typedef struct Parser_Error Parser_Error;
struct Parser_Error
{
	Parser_Error_Kind kind;

	U32 row_index;
	U32 column_begin_index;
	U32 column_end_index;
};

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

typedef struct Parser_Result Parser_Result;
struct Parser_Result
{
	Parser_Error error;
};

internal ELF64_Section
ELF64_Section_from_Directive_Kind(Directive_Kind kind)
{
	ELF64_Section section = ELF64_Section__Null;

	switch (kind)
	{
	case ELF64_Section__Text:           { section = ELF64_Section__Text;           } break;
	case ELF64_Section__Data:           { section = ELF64_Section__Data;           } break;
	case ELF64_Section__Read_Only_Data: { section = ELF64_Section__Read_Only_Data; } break;
	case ELF64_Section__BSS:            { section = ELF64_Section__BSS;            } break;
	default: {} break;
	}

	return section;
}


internal Parser_Result
PA_parse(Input *input, Token_Array *token_array, Object_File_Section *sections, Arena *arena)
{
	Token_Cursor cursor = Token_Cursor_new(token_array);
	Parser_Error error  = {0};
	Parser_Result result = {0};

	// By default, the section is `.text`.
	Object_File_Section section = sections[ELF64_Section__Text];

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
		case Token_Kind__Directive:
		{
			String8 substring =
			{
				.data  = input->data + cursor.current.index,
				.count = (U64)cursor.current.size
			};
			Directive_Kind directive_kind = Directive_Kind__from_String8(substring);

			switch (directive_kind)
			{
			case Directive_Kind__None:
			{
				error = Parser_Error_new(Parser_Error_Kind__Directive_Unknown, &cursor);
			} break;
			case Directive_Kind__Section:
			{
				Token *token_next = Token_Cursor_peek_next(&cursor);
				if (!token_next || token_next->kind == Token_Kind__Newline)
				{
					error = Parser_Error_new(Parser_Error_Kind__Directive_Section_Argument_Missing, &cursor);
				}
				else if (token_next->kind == Token_Kind__Directive)
				{
					Token_Cursor_advance(&cursor);
				}
				else
				{
					Token_Cursor_advance(&cursor);
					error = Parser_Error_new(Parser_Error_Kind__Directive_Section_Argument_Invalid, &cursor);
				}
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
				ELF64_Section section_kind = ELF64_Section_from_Directive_Kind(directive_kind);
				assert_always_m(section_kind && "unhandled directive");

				section = sections[section_kind];
				Token_Cursor_advance(&cursor);
			} break;
			}
		} break;
		default:
		{
			assert_always_m(0 && "unexpected token received");
		} break;
		}

		B32 loop_infinite_avoided = cursor.index > index_before || error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop in parser");
	}

	result.error = error;

	return result;
}

#endif // PARSER_H

