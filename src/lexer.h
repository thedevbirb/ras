#ifndef LEXER_H
#define LEXER_H

// @section EBNF grammar for RV64I assembly (ISO 14977)
//
// (* Notation:                                                *)
// (*   =       defines a rule                                 *)
// (*   ,       concatenation                                  *)
// (*   |       alternation (or)                               *)
// (*   [ ... ] optional      (zero or one)                    *)
// (*   { ... } repetition    (zero or more)                   *)
// (*   ( ... ) grouping                                       *)
// (*   ' ... ' terminal literal                               *)
// (*   ? ... ? special sequence (described in prose)          *)
// (*   ;       end of rule                                    *)
//
//
// (* Syntax rules *)
//
// program        = { line } ;
// line           = [ statement ], [ comment ], newline ;
// statement      = label
//               | directive
//               | instruction
//               | label, instruction ;
// label          = identifier, ':'
//               | '.', identifier, ':' ;
// directive      = '.', identifier, [ directive args ] ;
// directive args = ? directive-specific, see parser ? ;
// instruction    = mnemonic, [ operand list ] ;
// mnemonic       = identifier ;
// operand list   = operand, { ',', operand } ;
// operand        = register
//               | immediate
//               | memory
//               | symbol ;
// register       = identifier ;
// immediate      = [ '+' | '-' ], number ;
// memory         = immediate, '(', register, ')' ;
// symbol         = identifier, [ ( '+' | '-' ), immediate ] ;
//
//
// (* Lexical rules *)
//
// letter         = 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I'
//               | 'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R'
//               | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z'
//               | 'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i'
//               | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r'
//               | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' ;
// digit          = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' ;
// hex digit      = digit
//               | 'a' | 'b' | 'c' | 'd' | 'e' | 'f'
//               | 'A' | 'B' | 'C' | 'D' | 'E' | 'F' ;
// bin digit      = '0' | '1' ;
// identifier     = ( letter | '_' ), { letter | digit | '_' } ;
// number         = digit, { digit }
//               | '0', 'x', hex digit, { hex digit }
//               | '0', 'b', bin digit, { bin digit } ;
// comment        = '#', { ? any character except newline ? } ;
// newline        = ? line feed character, U+000A ? ;
//
//
// (* Instruction format constraints *)
//
// (* R-type:  mnemonic, register, ',', register, ',', register        *)
// (* I-type:  mnemonic, register, ',', register, ',', immediate       *)
// (* S-type:  mnemonic, register, ',', immediate, '(', register, ')'  *)
// (* B-type:  mnemonic, register, ',', register, ',', symbol          *)
// (* U-type:  mnemonic, register, ',', immediate                      *)
// (* J-type:  mnemonic, register, ',', symbol                         *)

internal U8 Input_count_extra = 8;

// Input is a String8 where we assume ZII, so we can ensure to not panic on out of bounds access. To do so, Input is
// slighly over-allocated. In practice, when an input is allocated `Input_count_extra` elements are reserved.
//
// This simplifies a lot of code because no particular branching is needed for checking out of bounds.
typedef struct Input Input;
struct Input
{
	U8 *data;
	U64 count;
};

internal Input
Input_new(U64 count, Arena *arena)
{
	assert_always_m(count < U64_max && "cannot allocate U64_max bytes");
	U64 count_extra = count + Input_count_extra;
	U8 *data = Arena_push_array_m(arena, U8, count_extra);

	Input input =
	{
		.data = data,
		.count = count,
	};

	return input;
}

typedef enum Lexing_Error_Kind
{
	Lexer_Error_Kind__None,
	Lexer_Error_Kind__String_Multiline_Unsupported,
	Lexer_Error_Kind__String_Literal_Unterminated,
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
	Lexer_Error_Kind__Label_Directive_Invalid,
	Lexer_Error_Kind__COUNT,
}
Lexer_Error_Kind;

global const char *lexer_error_kind_messages[Lexer_Error_Kind__COUNT] =
{
	[Lexer_Error_Kind__None]                                    = "",
	[Lexer_Error_Kind__String_Multiline_Unsupported]            = "multiline strings are not supported",
	[Lexer_Error_Kind__String_Literal_Unterminated]             = "string literal unterminated",
	[Lexer_Error_Kind__Character_Literal_Multiline_Unsupported] = "multiline character literals are not supported",
	[Lexer_Error_Kind__Character_Literal_Empty]                 = "empty character literal",
	[Lexer_Error_Kind__Character_Literal_Multiple]              = "character literal contains multiple characters",
	[Lexer_Error_Kind__Character_Literal_Escape_Invalid]        = "character literal contains invalid escape",
	[Lexer_Error_Kind__Character_Literal_Unterminated]          = "character literal untermindated",
	[Lexer_Error_Kind__Numeric_Literal_Invalid]                 = "numerical literal is invalid",
	[Lexer_Error_Kind__Numeric_Hex_Literal_Invalid]             = "numerical hex literal is invalid",
	[Lexer_Error_Kind__Numeric_Octal_Literal_Invalid]           = "numerical octal literal is invalid",
	[Lexer_Error_Kind__Numeric_Binary_Literal_Invalid]          = "numerical binary literal is invalid",
	[Lexer_Error_Kind__Label_Directive_Invalid]                 = "invalid label or directive",
	[Lexer_Error_Kind__Character_Unexpected]                    = "unexpected character",
};

typedef struct Lexer_Error Lexer_Error;
struct Lexer_Error
{
	Lexer_Error_Kind kind;

	U32 row_index;
	U32 column_begin_index;
	U32 column_end_index;
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
// assert_static_m(sizeof(struct Token) == 20, size_of_Token);

typedef struct Token_Array Token_Array;
struct Token_Array
{
	Token      *tokens;
	U32        *line_start_indexes; // FIX: missing count for this?
	U32         token_count;
	Lexer_Error error;
};

#endif // LEXER_H

