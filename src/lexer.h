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

typedef enum Token_Kind
{
	Token_Kind__None = 0,

	Token_Kind__Comma,
	Token_Kind__Colon,

	Token_Kind__Left_Parenthesis,
	Token_Kind__Right_Parenthesis,

	Token_Kind__Plus,
	Token_Kind__Minus,
	Token_Kind__Star,
	Token_Kind__Slash,

	Token_Kind__Pipe,
	Token_Kind__Ampersand,
	Token_Kind__Tilde,
	Token_Kind__Caret,

	Token_Kind__Percentage,
	Token_Kind__Newline,

	Token_Kind__Major,
	Token_Kind__Minor,
	Token_Kind__Shift_Left,
	Token_Kind__Shift_Right,

	Token_Kind__Label,
	Token_Kind__Directive,

	Token_Kind__Char_Literal,
	Token_Kind__String_Literal,

	Token_Kind__Identifier,
	Token_Kind__Number_Literal,

	Token_Kind__EOF,

	Token_Kind__COUNT
}
Token_Kind;

typedef enum Lexing_Error_Kind
{
	Lexer_Error_Kind__None,
	Lexer_Error_Kind__String_Multiline_Unsupported,
	Lexer_Error_Kind__Character_Literal_Multiline_Unsupported,
	Lexer_Error_Kind__Character_Literal_Empty,
	Lexer_Error_Kind__Character_Literal_Multiple,
	Lexer_Error_Kind__Character_Unexpected,
	Lexer_Error_Kind__Label_Directive_Invalid,
	Lexer_Error_Kind__COUNT,
}
Lexer_Error_Kind;

global const char *lexer_error_kind_messages[Lexer_Error_Kind__COUNT] = {
	[Lexer_Error_Kind__None]                                    = "",
	[Lexer_Error_Kind__String_Multiline_Unsupported]            = "multiline strings are not supported",
	[Lexer_Error_Kind__Character_Literal_Multiline_Unsupported] = "multiline character literals are not supported",
	[Lexer_Error_Kind__Character_Literal_Empty]                 = "empty character literal",
	[Lexer_Error_Kind__Character_Literal_Multiple]              = "character literal contains multiple characters",
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
	U32        *line_start_indexes;
	U32         token_count;
	Lexer_Error error;
};

#endif // LEXER_H

