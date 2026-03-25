#ifndef PARSER_H
#define PARSER_H

typedef struct InputSlice InputSlice;
struct InputSlice
{
	U32 index_begin;
	U32 index_end;
};

typedef struct Token_Cursor Token_Cursor;
struct Token_Cursor
{
	Input	     *input;
	Token_Array  *token_array;
	Token         current;
	U32           index;
	B32           end_reached;
};

internal Token_Cursor
Token_Cursor_new(Input *input, Token_Array *token_array);

// It is a no-op if the end has been reached already.
internal void
Token_Cursor_advance(Token_Cursor *cursor);

internal Token *
Token_Cursor_peek_next(Token_Cursor *cursor);

internal String8
Token_Cursor_substring(Token_Cursor *cursor);

typedef enum Parser_Error_Kind
{
	Parser_Error_Kind__None,
	Parser_Error_Kind__Line_Invalid,
	Parser_Error_Kind__Directive_Unknown,
	Parser_Error_Kind__Directive_Section_Argument_Missing,
	Parser_Error_Kind__Directive_Section_Argument_Invalid,
	Parser_Error_Kind__Directive_Align_Argument_Missing,
	Parser_Error_Kind__Directive_Align_Argument_Invalid,
	Parser_Error_Kind__Label_Duplicate,

	Parser_Error_Kind__COUNT,
}
Parser_Error_Kind;

global const char *Parser_Error_Kind_messages[Parser_Error_Kind__COUNT] =
{
	[Parser_Error_Kind__None]                               = "",
	[Parser_Error_Kind__Line_Invalid]                       = "line can only start with a directive, label or instruction",
	[Parser_Error_Kind__Directive_Unknown]                  = "unknown directive found",
	[Parser_Error_Kind__Directive_Section_Argument_Missing] = "section directive is missing the argument",
	[Parser_Error_Kind__Directive_Section_Argument_Invalid] = "section directive argument is invalid",
	[Parser_Error_Kind__Directive_Align_Argument_Missing]   = "align directive is missing the argument",
	[Parser_Error_Kind__Directive_Align_Argument_Invalid]   = "align directive argument is not a number literal",
	[Parser_Error_Kind__Label_Duplicate]                    = "duplicate label found",
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
Parser_Error_new(Parser_Error_Kind kind, Token_Cursor *cursor);

internal Directive_Kind
Directive_Kind__from_String8(String8 string);

typedef struct Parser_Result Parser_Result;
struct Parser_Result
{
	Parser_Error error;
};

// TODO: how am I transforming output after this stage?
internal Parser_Result
PA_parse(Input *input, Token_Array *token_array, Object_File_Section *sections, Arena *arena);

#endif // PARSER_H

