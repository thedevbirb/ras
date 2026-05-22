#ifndef LEXER_H
#define LEXER_H

global const U8 escape_valid_table[256] =
{
	['a']  = 1,  // bell
	['b']  = 1,  // backspace
	['t']  = 1,  // tab
	['n']  = 1,  // newline
	['v']  = 1,  // vertical tab
	['f']  = 1,  // form feed
	['r']  = 1,  // carriage return
	['e']  = 1,  // escape
	['\\'] = 1,  // backslash
	['\''] = 1,  // single quote
	['"']  = 1,  // double quote
	['0']  = 1,  // null or octal begin
	['1']  = 1,  // octal begin
	['2']  = 1,  // octal begin
	['3']  = 1,  // octal begin
	['x']  = 1,  // hex begin
};

typedef enum Lexing_Error_Kind
{
	Lexer_Error_Kind__None,
	Lexer_Error_Kind__String_Multiline_Unsupported,
	Lexer_Error_Kind__String_Literal_Unterminated,
	Lexer_Error_Kind__Escape_Sequence_Invalid,
	Lexer_Error_Kind__Character_Literal_Multiline_Unsupported,
	Lexer_Error_Kind__Character_Literal_Empty,
	Lexer_Error_Kind__Character_Literal_Multiple,
	Lexer_Error_Kind__Character_Literal_Escape_Invalid,
	Lexer_Error_Kind__Character_Literal_Unterminated,
	Lexer_Error_Kind__Numeric_Literal_Invalid,
	Lexer_Error_Kind__Numeric_Hex_Literal_Invalid,
	Lexer_Error_Kind__Numeric_Octal_Literal_Invalid,
	Lexer_Error_Kind__Numeric_Binary_Literal_Invalid,
	Lexer_Error_Kind__Character_Unexpected,
	Lexer_Error_Kind__Escape_Sequence_Unterminated,
	Lexer_Error_Kind__Label_Directive_Invalid,
	Lexer_Error_Kind__COUNT,
}
Lexer_Error_Kind;

global const char *lexer_error_kind_messages[Lexer_Error_Kind__COUNT] =
{
	[Lexer_Error_Kind__None]                                    = "",
	[Lexer_Error_Kind__String_Multiline_Unsupported]            = "multiline strings are not supported",
	[Lexer_Error_Kind__String_Literal_Unterminated]             = "string literal unterminated",
	[Lexer_Error_Kind__Escape_Sequence_Invalid]                 = "escape sequence invalid",
	[Lexer_Error_Kind__Character_Literal_Multiline_Unsupported] = "multiline character literals are not supported",
	[Lexer_Error_Kind__Character_Literal_Empty]                 = "empty character literal",
	[Lexer_Error_Kind__Character_Literal_Multiple]              = "character literal contains multiple characters",
	[Lexer_Error_Kind__Character_Literal_Escape_Invalid]        = "character literal contains invalid escape",
	[Lexer_Error_Kind__Character_Literal_Unterminated]          = "character literal untermindated",
	[Lexer_Error_Kind__Numeric_Literal_Invalid]                 = "numerical literal is invalid",
	[Lexer_Error_Kind__Numeric_Hex_Literal_Invalid]             = "numerical hex literal is invalid",
	[Lexer_Error_Kind__Numeric_Octal_Literal_Invalid]           = "numerical octal literal is invalid",
	[Lexer_Error_Kind__Numeric_Binary_Literal_Invalid]          = "numerical binary literal is invalid",
	[Lexer_Error_Kind__Escape_Sequence_Unterminated]            = "escape sequence unterminated",
	[Lexer_Error_Kind__Label_Directive_Invalid]                 = "invalid label or directive",
	[Lexer_Error_Kind__Character_Unexpected]                    = "unexpected character",
};

typedef struct Token Token;
struct Token
{
	U64        numerical_value; // No float support yet.
	U32        index;
	U32        row_index;
	U32        column_index;
	U32        size;
	Token_Kind kind;
};

#define Token_Xar__shift_amount 12
typedef struct Token_Xar Token_Xar;
struct Token_Xar
{
	Xar_Metadata metadata;
	Xar_Header   header;
	Token       *chunks[14];
};


// assert_static_m(sizeof(struct Token) == 20, size_of_Token);
//
typedef struct Lexer Lexer;
struct Lexer
{
	U32  index;
	U32  index_before;
	U32  column_index;
	U32  column_index_before;
	U32  line_start_index;
	U32  row_index;
	B32  end_reached;
	U8   current;
};

typedef struct Lexer_Context Lexer_Context;
struct Lexer_Context
{
	const String8 *input;
	const char    *filename;
	Token_Xar     *tokens;
	Diagnostics   *diagnostics;
};


// It is a no-op if the end has been reached already.
internal void
Lexer_advance(Lexer *lexer, const String8 *input);

internal U8 *
Lexer_peek_next(Lexer *lexer, const String8 *input);

internal String8
Lexer_string_under_cursor(Lexer *lexer, const String8 *input);

internal void
Lexer_error_set(Lexer *lexer, const char *filename, Diagnostics *diagnostic, Lexer_Error_Kind kind);

internal void
Lexer_expect(Lexer *lexer, B32 condition, Lexer_Error_Kind error_kind, Diagnostic *diagnostic);

internal void
Lexer_tokenize(Lexer *lexer, Arena *arena, Lexer_Context *context);

#endif // LEXER_H

