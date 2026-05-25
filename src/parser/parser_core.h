#ifndef PARSER_CORE_H
#define PARSER_CORE_H

#define expression_recursion_max 8


typedef enum Parser_Error_Kind
{
	Parser_Error_Kind__None,
	Parser_Error_Kind__Line_Invalid,
	Parser_Error_Kind__Line_Extra_Content,
	Parser_Error_Kind__Identifier_Expected,
	Parser_Error_Kind__Comma_Expected,
	Parser_Error_Kind__Parenthesis_Left_Expected,
	Parser_Error_Kind__Parenthesis_Right_Expected,
	Parser_Error_Kind__Directive_Unknown,
	Parser_Error_Kind__Directive_Section_Argument_Missing,
	Parser_Error_Kind__Directive_Section_Argument_Invalid,
	Parser_Error_Kind__Directive_Align_Argument_Missing,
	Parser_Error_Kind__Directive_Align_Argument_Invalid,
	Parser_Error_Kind__Directive_Data_Invalid,
	Parser_Error_Kind__Directive_Data_Value_Size_Invalid,
	Parser_Error_Kind__Directive_Argument_Invalid,
	Parser_Error_Kind__Label_Duplicate,
	Parser_Error_Kind__Label_Numeric_Large,
	Parser_Error_Kind__Expression_Unexpected_Token,
	Parser_Error_Kind__Expression_Unexpected_End,
	Parser_Error_Kind__Expression_Parenthesis_Right_Expected,
	Parser_Error_Kind__Expression_Parenthesis_Left_Expected,
	Parser_Error_Kind__Expression_Identifier_Undefined,
	Parser_Error_Kind__Expression_Relocation_Syntax_Invalid,
	Parser_Error_Kind__Expression_Kind_Unknown,
	Parser_Error_Kind__Expression_Recursion_Max,
	Parser_Error_Kind__Relocation_Operator_Invalid,
	Parser_Error_Kind__Relocation_Instruction_Missing,
	Parser_Error_Kind__Relocation_Instruction_Invalid,
	Parser_Error_Kind__Relocation_Symbol_Missing,
	Parser_Error_Kind__Register_Invalid,
	Parser_Error_Kind__Immediate_Invalid,
	Parser_Error_Kind__Instruction_Unknown,
	Parser_Error_Kind__String_Literal_Expected,
	Parser_Error_Kind__Symbol_Demoted,
	Parser_Error_Kind__Symbol_Duplicate,
	Parser_Error_Kind__Symbol_Context_Invalid,
	Parser_Error_Kind__Fence_Operand_Invalid,
	Parser_Error_Kind__Option_Invalid,
	Parser_Error_Kind__Relocation_Operator_Multiple,

	Parser_Error_Kind__COUNT,
}
Parser_Error_Kind;

global const char *Parser_Error_Kind_messages[Parser_Error_Kind__COUNT] =
{
	[Parser_Error_Kind__None]                                  = "",
	[Parser_Error_Kind__Line_Invalid]                          = "line can only start with a directive, label or instruction",
	[Parser_Error_Kind__Line_Extra_Content]                    = "line ends with unexpected tokens",
	[Parser_Error_Kind__Identifier_Expected]                   = "expected identifier",
	[Parser_Error_Kind__Comma_Expected]                        = "comma expected",
	[Parser_Error_Kind__Parenthesis_Left_Expected]             = "left parenthesis expected",
	[Parser_Error_Kind__Parenthesis_Right_Expected]            = "right parenthesis expected",
	[Parser_Error_Kind__Directive_Unknown]                     = "unknown directive found",
	[Parser_Error_Kind__Directive_Section_Argument_Missing]    = "section directive is missing the argument",
	[Parser_Error_Kind__Directive_Section_Argument_Invalid]    = "section directive argument is invalid",
	[Parser_Error_Kind__Directive_Align_Argument_Missing]      = "align directive is missing the argument",
	[Parser_Error_Kind__Directive_Align_Argument_Invalid]      = "align directive argument is not a number literal",
	[Parser_Error_Kind__Directive_Data_Invalid]                = "invalid data directive syntax",
	[Parser_Error_Kind__Directive_Data_Value_Size_Invalid]     = "directive data expression value doesn't fit",
	[Parser_Error_Kind__Directive_Argument_Invalid]            = "directive argument invalid",
	[Parser_Error_Kind__Label_Duplicate]                       = "duplicate label found",
	[Parser_Error_Kind__Label_Numeric_Large]                   = "numerical label must be less than " stringify_m(label_numeric_max),
	[Parser_Error_Kind__Expression_Unexpected_Token]           = "unexpected token in expression",
	[Parser_Error_Kind__Expression_Unexpected_End]             = "unexpected end of tokens in expression",
	[Parser_Error_Kind__Expression_Parenthesis_Right_Expected] = "expected ')' to close parenthesized expression",
	[Parser_Error_Kind__Expression_Parenthesis_Left_Expected]  = "expected '(' to open parenthesized expression",
	[Parser_Error_Kind__Expression_Identifier_Undefined]       = "undefined identifier are not allowed in this expression",
	[Parser_Error_Kind__Expression_Relocation_Syntax_Invalid]  = "invalid relocation syntax, expected %<relocation>(<expression>)",
	[Parser_Error_Kind__Expression_Kind_Unknown]               = "unknown expression kind",
	[Parser_Error_Kind__Expression_Recursion_Max]              = "max recursion reached for expression: " stringify_m(expression_recursion_max),
	[Parser_Error_Kind__Relocation_Operator_Invalid]           = "relocation operator invalid",
	[Parser_Error_Kind__Relocation_Instruction_Missing]        = "relocation operator can be used only within an instruction",
	[Parser_Error_Kind__Relocation_Instruction_Invalid]        = "relocation operator cannot be with this instruction",
	[Parser_Error_Kind__Relocation_Symbol_Missing]             = "relocation operator used without a symbol",
	[Parser_Error_Kind__Register_Invalid]                      = "register invalid",
	[Parser_Error_Kind__Immediate_Invalid]                     = "immediate invalid",
	[Parser_Error_Kind__Instruction_Unknown]                   = "instruction unknown",
	[Parser_Error_Kind__String_Literal_Expected]               = "string literal expected",
	[Parser_Error_Kind__Symbol_Demoted]                        = "demoted symbol from global/weak to local",
	[Parser_Error_Kind__Symbol_Duplicate]                      = "duplicated symbol",
	[Parser_Error_Kind__Symbol_Context_Invalid]                = "invalid symbol context: must be either a data directive, a supported branch/jump instruction or inside relocation operator",
	[Parser_Error_Kind__Fence_Operand_Invalid]                 = "fence operand invalid",
	[Parser_Error_Kind__Option_Invalid]                        = "invalid option",
	[Parser_Error_Kind__Relocation_Operator_Multiple]          = "multiple relocation operators in the same statement is invalid",
};

typedef struct Parser_Error Parser_Error;
struct Parser_Error
{
	Parser_Error_Kind kind;

	U32 row_index;
	U32 column_index_begin;
	U32 column_index_end;
};

typedef struct Parser_2 Parser_2;
struct Parser_2
{
	String8                  filename;
	Diagnostics             *diagnostics;
	Lexer                   *lexer;
	Symbols_Trie            *symbols_trie;
	Symbols_Trie_Chunk_List *symbols_trie_chunk_list;
};

// The goal of the parser is to drive the lexer until it has to yield again, while filling as much information as
// possible.
// It will yield back to the caller when the input has ended, or when an #include directive or similar has been found,
// and another input must be provided again.

internal void
Parser_2__parse(Parser_2 *parser, String8 *input, Arena *arena, Statements_Xar *statements);


// // Implemented as a cursor over tokens.
// typedef struct Parser Parser;
// struct Parser
// {
// 	Arena          *arena;
// 	String8        *input;
// 	Token          *tokens;
// 	Statements     *statements;
// 	Expressions    *expressions;
// 	Symbols_Table  *symbols_table;
//
// 	// The current statement being built
// 	Statement      *statement_context;
//
// 	U16 section_current_index;
//
// 	Token         token_current;
//
// 	U32	      token_count;
// 	U32	      token_index_before;
// 	U32	      token_index;
// 	B32           end_reached;
//
// 	Statement_Flags flags;
//
// 	Parser_Error error;
// };
//
// internal Parser
// Parser_new(Arena *arena, String8 *input, Token *tokens, U32 token_count);
//
// // It is a no-op if the end has been reached already.
// internal void
// Parser_advance(Parser *parser);
//
// internal Token *
// Parser_peek_next(Parser *parser);
//
// internal String8
// Parser_token_string(Parser *parser);
//
// internal Parser_Error
// Parser_Error_new(Parser_Error_Kind kind, Parser *parser);
//
// // Expect the provided condition to hold, setting the error if undefined.
// internal void
// Parser_expect(Parser *parser, B32 condition, Parser_Error_Kind error_kind);
//
// internal void
// Parser_expect_token(Parser *parser, Token_Kind token_kind, Parser_Error_Kind error_kind);
//
// internal U8
// Parser_register(Parser *parser);
//
// internal U8
// Parser_expect_register(Parser *parser);
//
// // TODO: deprecate expression flags from these methods.
//
// // Core Pratt parser loop. Parses an expression where all binary operators
// // must have binding power strictly greater than binding_power_minimum.
// // All operators are left-associative (the <= comparison ensures this).
// internal Expression_Node *
// Parser_expression_parse_inner(Parser *parser, Binding_Power binding_power_minimum);
//
// // Null denotation: handles prefix positions (atoms, unary operators,
// // parenthesized groups, relocations). The token has already been consumed
// // from the parser before this call.
// internal Expression_Node *
// Parser_parse_null_denotation(Parser *parser);
//
//
// // Entry point. Parses an expression starting at the token_current parser position.
// // Advances the parser past consumed tokens. On error, error->kind is nonzero.
// //
// // Moves the cursor further until a non-valid token for an expression is met.
// Expression_Node *
// Parser_expression_parse(Parser *parser);
//
// // Create an expression consisting of an immediate value.
// Expression_Node *
// Parser_expression_immediate_create(Parser *parser, U64 immediate);
//
//
// // Create a barebone, incomplete statement for an instruction.
// internal Statement *
// Parser_statement_instruction_create(Parser *parser);
//
// void
// Parser_parse(Parser *parser);

#endif // PARSER_CORE_H
