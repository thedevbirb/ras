#ifndef LANGUAGE_TOKEN_H
#define LANGUAGE_TOKEN_H

typedef enum Token_Kind
{
	Token_Kind__None = 0,

	Token_Kind__Dot,
	Token_Kind__Comma,
	Token_Kind__Semicolon,

	Token_Kind__Parenthesis_Left,
	Token_Kind__Parenthesis_Right,

	Token_Kind__Plus,
	Token_Kind__Minus,
	Token_Kind__Star,
	Token_Kind__Slash,
	Token_Kind__Tilde,
	Token_Kind__Caret,

	Token_Kind__Newline,

	Token_Kind__Shift_Right,
	Token_Kind__Greater_Equal,
	Token_Kind__Greater_Than,

	Token_Kind__Shift_Left,
	Token_Kind__Less_Equal,
	Token_Kind__Less_Than,

	Token_Kind__Equal,  // '=='
	Token_Kind__Assign, // '='

	Token_Kind__Equal_Not,
	Token_Kind__Bang,

	Token_Kind__Logical_Or,
	Token_Kind__Pipe,
	Token_Kind__Logical_And,
	Token_Kind__Ampersand,

	Token_Kind__Relocation_Prefix, // %
	Token_Kind__Percentage,

	Token_Kind__Label,
	Token_Kind__Label_Numeric,                    // e.g. 1:
	Token_Kind__Label_Numeric_Reference_Forward,  // e.g. 1f
	Token_Kind__Label_Numeric_Reference_Backward, // e.g. 1b
	Token_Kind__Directive,

	Token_Kind__Char_Literal,
	Token_Kind__String_Literal,

	Token_Kind__Identifier,
	Token_Kind__Number_Literal,

	Token_Kind__EOF,

	Token_Kind__COUNT
}
Token_Kind;

#endif // LANGUAGE_TOKEN_H

