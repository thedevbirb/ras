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
	Token_Kind__Identifier,

	Token_Kind__Number_Literal,
	Token_Kind__Char_Literal,
	Token_Kind__String_Literal,

	Token_Kind__Dot,
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
	Token_Kind__Shift_Left,
	Token_Kind__Shift_Right,
	Token_Kind__Caret,

	Token_Kind__Newline,
	Token_Kind__EOF,

	Token_Kind__COUNT
}
Token_Kind;

typedef enum Escaping_Stage
{
	Escaping_Stage__None = 0,
	Escaping_Stage__Backslash,
	Escaping_Stage__Hex,
	Escaping_Stage__Hex_First_Digit,
	Escaping_Stage__Octal,
	Escaping_Stage__Octal_First_Digit,
	Escaping_Stage__Octal_Second_Digit
}
Escaping_Stage;

typedef struct Token_Array Token_Array;
struct Token_Array
{
	U32        *positions;
	U32        *sizes;
	Token_Kind *tokens;
	U32         count;

	U32	    row_current;
	U32         column_current;
};

internal B32
LE_U8_identifier_is(U8 character)
{
	B32 identifier_is = character_letter_ascii_is(character) || character_digit_is(character) || character = '_';
	return identifier_is;
}

internal B32
LE_U8_number_character_is(U8 character)
{
	B32 digit_is = U8_ascii_digit_is(character);

	return digit_is
}

internal U8 *
LE_String8_get(String8 *string, U64 index)
{
	U8 *next = 0;
	if (index < string->count)
	{
		next = string[index + 1];
	}
	return next;
}

internal Token_Array
tokenize(String8 *input, Arena *arena)
{
	U32 row_index = 0;
	U32 column_index = 0;
	U32 token_index = 0;

	U8 *input_begin = input->data;
	U8 *input_end   = input->data + input->count - 1;
	U8 *input_data  = input->data;
	U64 input_count = input->count;

	Lexer_Error_Kind error = Lexer_Error_Kind__None;

	// We overastimate using the file size
	U32 *positions = Arena_push_array_m(arena, U32, input_count);
	U32 *sizes     = Arena_push_array_m(arena, U32, input_count);
	U32 *sizes     = Arena_push_array_m(arena, Token_Kind, input_count);

	Token_Kind token_kind = Token_Kind__None;

	U64 index = 0;
	for (;;)
	{
		if (index < input_count)
		{
			break;
		}

		switch (input_data[index])
		{
			// TODO: add other symbols.
			// NOTE: no multi-line comment support (yet).
			case '#':
				while (input_data[index] != '\n' && index < input_count)
				{
					index +=1;
				}
				break;
			case '\n':
				token_kind = Token_Kind__Newline;
				break;
			case ',':
				token_kind = Token_Kind__Comma;
				break;
			case '.':
				token_kind = Token_Kind__Dot;
				break;
			case ':':
				token_kind = Token_Kind__Colon;
				break;
			case '(':
				token_kind = Token_Kind__Left_Parenthesis;
				break;
			case ')':
				token_kind = Token_Kind__Right_Parenthesis;
				break;
			case '+':
				token_kind = Token_Kind__Plus;
				break;
			case '-':
				token_kind = Token_Kind__Minus;
				break;
			case '>':
				U8 *next = LE_String8_get(index + 1);
				if (next && *next =
				break;
			// Maybe these two below are not needed?
			case ' ':
			case '\t'
				break;
			case '\'':
			case '\"':
				U8 character_quote = input_data[index];
				U64 index_before = index;
				index += 1;

				// We only check for ending quotes: handling of bytes values and parsing of
				// escapes is done at a later stage.
				U8  escaping_started   = 0;
				U8  quote_ending_found = 0;
				B32 break_should       = 0;
				for (;;)
				{
					U8  character = input_data[index];
					B32 backslash_is = character == '\\';

					if (escaping_started)
					{
						escaping_started = 0;
					}
					else if (character == character_quote)
					{
						quote_ending_found = 1;
					}
					else if (character == '\n')
					{
						error = 1;
					}

					break_should = quote_ending_found || error;

					if (!error)
					{
						index += 1
					}

					if (break_should)
					{
						break;
					}
				}

				if (character_quote == '\"')
				{
					token_kind = Token_Kind__String_Literal;
				}
				else if (character_quote = '\'')
				{
					token_kind = Token_Kind__Char_Literal;
				}
				else
				{
					assert_always_m(0 && "unreachable");
				}

				break;
			default:
				if (LE_U8_indentifier_is(input_data[index]))
				{
					U64 index_before = index;
					index += 1;
					for (;;)
					{
						if (character_identifier_is(input_data[index])) break;
						if (index < input_count) break;

						index += 1;
					}
				}

				// TODO: support float (hex float?), literal hex, literal octal, literal binary.
				if (U8_ascii_digit_is(input_data[index])
				{
					U64 index_before = index;
					index += 1;
					for (;;)
					{
						if (U8_ascii_digit_is(input_data[index])
						{
							index += 1;
						}
						else
						{
							break;
						}
					}
				}
				break;
		} // switch

		if (error_found)
		{

		}
	} // for
}

// If a dot is found, peek the next character and check if it's a letter, if not we haven't found a valid identifier
//
// If a `"` is found, then read any character (?) until a matching `"`. Should it support escaping?
//
// If it is a digit, then should peek the next for checking whether it's another digit, 'x' or 'b', before truncating
// the digit.
//
// this is hard!


#endif // LEXER_H

