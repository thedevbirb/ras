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
	Token_Kind__Caret,

	Token_Kind__Percentage,
	Token_Kind__Newline,

	Token_Kind__Major,
	Token_Kind__Minor,
	Token_Kind__Shift_Left,
	Token_Kind__Shift_Right,

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

	Lexer_Error_Kind__COUNT,


}
Lexer_Error_Kind;

typedef struct Token_Array Token_Array;
struct Token_Array
{
	U32        *positions;
	U32        *sizes;
	Token_Kind *tokens;
	U32         count;

	U32	    row_index;
	U32         column_index;

	Lexer_Error_Kind error;
};

internal B32
LE_U8_identifier_start_is(U8 character)
{
	B32 result = U8_ascii_letter_is(character) || character == '_';
	return result;
}

internal B32
LE_U8_identifier_is(U8 character)
{
	B32 result = U8_ascii_letter_is(character) || character == '_' || U8_ascii_digit_is(character) ;
	return result;
}

internal B32
LE_U8_number_character_is(U8 character)
{
	B32 result = U8_ascii_digit_is(character);
	return result;
}

internal U8 *
LE_String8_get(String8 *string, U64 index)
{
	U8 *next = 0;
	if (index < string->count)
	{
		next = &string->data[index + 1];
	}
	return next;
}

internal void
LE_advance(U64 *index, U32 *column_index)
{
	*index        += 1;
	*column_index += 1;
}

internal void
LE_advance_newline(U64 *index, U32 *column_index, U32 *row_index)
{
	*index        += 1;
	*row_index    += 1;
	*column_index  = 1;
}

internal Token_Array
LE_tokenize(String8 *input, Arena *arena)
{
	U32 row_index    = 0;
	U32 column_index = 0;
	U32 token_index  = 0;

	U8 *input_data  = input->data;
	U64 input_count = input->count;

	Lexer_Error_Kind error = 0;

	// We overastimate using the file size. Consider doing at the start of the program and not here.
	U32 *positions = Arena_push_array_m(arena, U32, input_count);
	U32 *sizes     = Arena_push_array_m(arena, U32, input_count);
	U32 *tokens    = Arena_push_array_m(arena, Token_Kind, input_count);

	Token_Kind token_kind = Token_Kind__None;

	U64 index = 0;
	for (;;)
	{
		U64 index_before = index;

		// NOTE: should maintain the logical order of Token_Kind enumeration.
		switch (input_data[index])
		{
		case ' ' : { LE_advance(&index, &column_index); } break;
		case '\t': { LE_advance(&index, &column_index); } break;

		// NOTE: no multi-line comment support (yet).
		case '#':
		{
			B32 break_should = 0;
			for (;;)
			{
				LE_advance(&index, &column_index);

				break_should = input_data[index] == '\n' || index >= input_count;
				if (break_should)
				{
					break;
				}
			}

		} break;

		case '.' : { token_kind = Token_Kind__Dot;               LE_advance(&index, &column_index); } break;
		case ',' : { token_kind = Token_Kind__Comma;             LE_advance(&index, &column_index); } break;
		case ':' : { token_kind = Token_Kind__Colon;             LE_advance(&index, &column_index); } break;

		case '(' : { token_kind = Token_Kind__Left_Parenthesis;  LE_advance(&index, &column_index); } break;
		case ')' : { token_kind = Token_Kind__Right_Parenthesis; LE_advance(&index, &column_index); } break;

		case '+' : { token_kind = Token_Kind__Plus;              LE_advance(&index, &column_index); } break;
		case '-' : { token_kind = Token_Kind__Minus;             LE_advance(&index, &column_index); } break;
		case '*' : { token_kind = Token_Kind__Star;              LE_advance(&index, &column_index); } break;
		case '/' : { token_kind = Token_Kind__Slash;             LE_advance(&index, &column_index); } break;

		case '%' : { token_kind = Token_Kind__Percentage;        LE_advance(&index, &column_index); } break;

		case '\n': { token_kind = Token_Kind__Newline;           LE_advance_newline(&index, &column_index, &row_index); } break;

		case '>':
		{
			LE_advance(&index, &column_index);
			U8 *next = LE_String8_get(input, index);
			if (next && *next == '>')
			{
				LE_advance(&index, &column_index);
				token_kind = Token_Kind__Shift_Right;
			}
			else
			{
				token_kind = Token_Kind__Major;
			}
		} break;
		case '<':
		{
			LE_advance(&index, &column_index);
			U8 *next = LE_String8_get(input, index);
			if (next && *next == '<')
			{
				LE_advance(&index, &column_index);
				token_kind = Token_Kind__Shift_Left;
			}
			else
			{
				token_kind = Token_Kind__Minor;
			}
		} break;

		case '\'':
		case '\"':
		{
			U8  character_quote = input_data[index];

			// We only check for ending quotes: handling of bytes values and parsing of
			// escapes is done at a later stage.
			U8  escaping_started   = 0;
			U8  quote_ending_found = 0;
			B32 break_should       = 0;
			for (;;)
			{
				LE_advance(&index, &column_index);
				U8  character = input_data[index];

				if (escaping_started)
				{
					escaping_started = 0;
				}
				else if (character == '\\')
				{
					escaping_started = 1;
				}
				else if (character == character_quote)
				{
					quote_ending_found = 1;
				}
				else if (character == '\n')
				{
					error = Lexer_Error_Kind__String_Multiline_Unsupported;
				}

				break_should = quote_ending_found || error || index >= input_count;
				if (break_should)
				{
					break;
				}

			}

			if (character_quote == '\"')
			{
				token_kind = Token_Kind__String_Literal;
			}
			else if (character_quote == '\'')
			{
				token_kind = Token_Kind__Char_Literal;
			}
			else
			{
				assert_always_m(0 && "unreachable");
			}

		} break;
		default:
		{
			if (LE_U8_identifier_start_is(input_data[index]))
			{
				B32 break_should = 0;
				for (;;)
				{
					LE_advance(&index, &column_index);
					B32 invalid = !LE_U8_identifier_is(input_data[index]);

					break_should = invalid || index >= input_count;
					if (break_should)
					{
						break;
					}
				}
			}

			// TODO: support float (hex float?), literal hex, literal octal, literal binary.
			if (U8_ascii_digit_is(input_data[index]))
			{
				B32 break_should = 0;
				for (;;)
				{
					LE_advance(&index, &column_index);
					B32 invalid = !U8_ascii_digit_is(input_data[index]);

					break_should = invalid || index >= input_count;
					if (break_should)
					{
						break;
					}
				}
			}
		} break;
		}

		assert_always_m(index_before < index && "infinite loop edge case");

		if (token_kind != 0)
		{
			// Update phase
			positions[token_index] = index_before;
			tokens   [token_index] = token_kind;
			sizes    [token_index] = index - index_before;

			token_index += 1;
		}

		B32 break_should = error || index >= input_count;
		if (break_should)
		{
			break;
		}
	}

	Token_Array token_array =
	{
		.positions    = positions,
		.sizes        = sizes,
		.tokens       = tokens,
		.count        = token_index,

		.row_index    = row_index,
		.column_index = column_index,

		.error        = error
	};

	return token_array;
}

#endif // LEXER_H

