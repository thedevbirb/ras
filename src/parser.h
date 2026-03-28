#ifndef PARSER_H
#define PARSER_H

typedef struct InputSlice InputSlice;
struct InputSlice
{
	U32 index_begin;
	U32 index_end;
};

typedef enum Parser_Error_Kind
{
	Parser_Error_Kind__None,
	Parser_Error_Kind__Line_Invalid,
	Parser_Error_Kind__Identifier_Expected,
	Parser_Error_Kind__Comma_Expected,
	Parser_Error_Kind__Directive_Unknown,
	Parser_Error_Kind__Directive_Section_Argument_Missing,
	Parser_Error_Kind__Directive_Section_Argument_Invalid,
	Parser_Error_Kind__Directive_Align_Argument_Missing,
	Parser_Error_Kind__Directive_Align_Argument_Invalid,
	Parser_Error_Kind__Directive_Data_Invalid,
	Parser_Error_Kind__Directive_Data_Value_Size_Invalid,
	Parser_Error_Kind__Label_Duplicate,
	Parser_Error_Kind__Expression_Unexpected_Token,
	Parser_Error_Kind__Expression_Unexpected_End,
	Parser_Error_Kind__Expression_Parenthesis_Right_Expected,
	Parser_Error_Kind__Expression_Parenthesis_Left_Expected,
	Parser_Error_Kind__Expression_Identifier_Undefined,
	Parser_Error_Kind__Expression_Relocation_Syntax_Invalid,
	Parser_Error_Kind__Expression_Kind_Unknown,
	Parser_Error_Kind__Expression_Recursion_Max,
	Parser_Error_Kind__Register_Invalid,
	Parser_Error_Kind__Immediate_Invalid,

	Parser_Error_Kind__COUNT,
}
Parser_Error_Kind;

global const char *Parser_Error_Kind_messages[Parser_Error_Kind__COUNT] =
{
	[Parser_Error_Kind__None]                                  = "",
	[Parser_Error_Kind__Line_Invalid]                          = "line can only start with a directive, label or instruction",
	[Parser_Error_Kind__Identifier_Expected]                   = "expected identifier",
	[Parser_Error_Kind__Comma_Expected]                        = "comma expected",
	[Parser_Error_Kind__Directive_Unknown]                     = "unknown directive found",
	[Parser_Error_Kind__Directive_Section_Argument_Missing]    = "section directive is missing the argument",
	[Parser_Error_Kind__Directive_Section_Argument_Invalid]    = "section directive argument is invalid",
	[Parser_Error_Kind__Directive_Align_Argument_Missing]      = "align directive is missing the argument",
	[Parser_Error_Kind__Directive_Align_Argument_Invalid]      = "align directive argument is not a number literal",
	[Parser_Error_Kind__Directive_Data_Invalid]                = "invalid data directive syntax",
	[Parser_Error_Kind__Directive_Data_Value_Size_Invalid]     = "directive data expression value doesn't fit",
	[Parser_Error_Kind__Label_Duplicate]                       = "duplicate label found",
	[Parser_Error_Kind__Expression_Unexpected_Token]           = "unexpected token in expression",
	[Parser_Error_Kind__Expression_Unexpected_End]             = "unexpected end of tokens in expression",
	[Parser_Error_Kind__Expression_Parenthesis_Right_Expected] = "expected ')' to close parenthesized expression",
	[Parser_Error_Kind__Expression_Parenthesis_Left_Expected]  = "expected '(' to open parenthesized expression",
	[Parser_Error_Kind__Expression_Identifier_Undefined]       = "undefined identifier are not allowed in this expression",
	[Parser_Error_Kind__Expression_Relocation_Syntax_Invalid]  = "invalid relocation syntax, expected %<relocation>(<expression>)",
	[Parser_Error_Kind__Expression_Kind_Unknown]               = "unknown expression kind",
	[Parser_Error_Kind__Expression_Recursion_Max]              = "max recursion reached for expression: " stringify_m(expression_recursion_max),
	[Parser_Error_Kind__Register_Invalid]                      = "register invalid",
	[Parser_Error_Kind__Immediate_Invalid]                     = "immediate invalid",
};

typedef struct Parser_Error Parser_Error;
struct Parser_Error
{
	Parser_Error_Kind kind;

	U32 row_index;
	U32 column_begin_index;
	U32 column_end_index;
};

typedef struct Parser Parser;
struct Parser
{
	Arena         *arena;
	Input	      *input;
	Token	      *tokens;
	Symbols_Table *symbols_table;

	Object_File_Section *sections;
	Object_File_Section *section_current;
	Object_File_Section *section_string_table;

	Token         token_current;
	U32	      token_count;
	U32	      token_index;
	B32           end_reached;

	Parser_Error error;
};

internal Parser
Parser_new(Arena *arena, Input *input, Token *tokens, U32 token_count);

// It is a no-op if the end has been reached already.
internal void
Parser_advance(Parser *parser);

internal Token *
Parser_peek_next(Parser *parser);

internal String8
Parser_token_string(Parser *parser);

internal Parser_Error
Parser_Error_new(Parser_Error_Kind kind, Parser *parser);

// Expect the provided condition to hold, setting the error if undefined.
internal void
Parser_expect(Parser *parser, B32 condition, Parser_Error_Kind error_kind);

internal void
Parser_expect_token(Parser *parser, Token_Kind token_kind, Parser_Error_Kind error_kind);

internal Directive_Kind
Directive_Kind__from_String8(String8 string);

typedef struct Parser_Result Parser_Result;
struct Parser_Result
{
	Parser_Error error;
};

// TODO: how am I transforming output after this stage?
internal void
Parser_parse(Parser *parser);

typedef struct Fixup Fixup;
struct Fixup
{
	U32 section_index;
	U32 section_offset;
	Instruction_Kind instruction_kind;
};

#endif // PARSER_H

