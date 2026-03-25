#ifndef EXPRESSION_H
#define EXPRESSION_H

// After the cursor processes an item, it ALWAYS advances.

// TODO: validate relocation names.
global char const *relocation_names[] =
{
	"hi", "lo", "pcrel_hi", "pcrel_lo", "got_pcrel_hi",
	"tprel_hi", "tprel_lo", "tprel_add",
	"tls_ie_pcrel_hi", "tls_gd_pcrel_hi", NULL,
};

typedef enum Expression_Error_Kind
{
	Expression_Error_Kind__None,
	Expression_Error_Kind__Unexpected_Token,
	Expression_Error_Kind__Unexpected_End,
	Expression_Error_Kind__Expected_Right_Parenthesis,
	Expression_Error_Kind__Relocation_Syntax_Invalid,
	Expression_Error_Kind__Expected_Relocation_Left_Parenthesis,
	Expression_Error_Kind__Expected_Relocation_Right_Parenthesis,
	Expression_Error_Kind__COUNT,
}
Expression_Error_Kind;

global const char *Expression_Error_Kind_messages[Expression_Error_Kind__COUNT] =
{
	[Expression_Error_Kind__None]                                  = "",
	[Expression_Error_Kind__Unexpected_Token]                      = "unexpected token in expression",
	[Expression_Error_Kind__Unexpected_End]                        = "unexpected end of tokens in expression",
	[Expression_Error_Kind__Expected_Right_Parenthesis]            = "expected ')' to close parenthesized expression",
	[Expression_Error_Kind__Relocation_Syntax_Invalid]             = "invalid relocation syntax, expected %<relocation>(<expression>)",
};

typedef struct Expression_Error Expression_Error;
struct Expression_Error
{
	Expression_Error_Kind kind;
	U32 row_index;
	U32 column_begin_index;
	U32 column_end_index;
};

internal Expression_Error
Expression_Error_new(Expression_Error_Kind kind, Token token)
{
	assert_always_m(kind);
	assert_always_m(token.kind);

	Expression_Error error =
	{
		.kind               = kind,
		.row_index          = token.row_index,
		.column_begin_index = token.column_index,
		.column_end_index   = token.column_index + token.size - 1,
	};

	return error;
}

typedef enum Expression_Kind
{
	Expression_Kind__None,

	// Leaf nodes
	Expression_Kind__Number_Literal,
	Expression_Kind__Char_Literal,
	Expression_Kind__Identifier,
	Expression_Kind__Current_Address,   // .
	Expression_Kind__Relocation,        // %hi(expr), %lo(expr), etc.

	// Unary operators
	Expression_Kind__Negate,            // -x
	Expression_Kind__Bitwise_Not,       // ~x
	Expression_Kind__Logical_Not,       // !x

	// Binary arithmetic
	Expression_Kind__Add,               // +
	Expression_Kind__Subtract,          // -
	Expression_Kind__Multiply,          // *
	Expression_Kind__Divide,            // /
	Expression_Kind__Modulo,            // %

	// Binary bitwise
	Expression_Kind__Bitwise_Or,        // |
	Expression_Kind__Bitwise_Xor,       // ^
	Expression_Kind__Bitwise_And,       // &
	Expression_Kind__Shift_Left,        // <<
	Expression_Kind__Shift_Right,       // >>

	// Binary comparison
	Expression_Kind__Equal,             // ==
	Expression_Kind__Not_Equal,         // !=
	Expression_Kind__Less_Than,         // <
	Expression_Kind__Less_Equal,        // <=
	Expression_Kind__Greater_Than,      // >
	Expression_Kind__Greater_Equal,     // >=

	// Binary logical
	Expression_Kind__Logical_And,       // &&
	Expression_Kind__Logical_Or,        // ||

	Expression_Kind__COUNT,
}
Expression_Kind;

typedef struct Expression_Node Expression_Node;
struct Expression_Node
{
	Expression_Node *left;
	Expression_Node *right;
	Expression_Kind  kind;
	union
	{
		U64 integer_value;
		struct
		{
			U32 index;
			U32 size;
		} source;
	} value;
};

// Binding power levels for Pratt parsing, ordered lowest to highest.
// Even numbers: gaps allow left/right binding power distinction if needed.
typedef enum Binding_Power
{
	Binding_Power__None           =   0,
	Binding_Power__Logical_Or     =   2,
	Binding_Power__Logical_And    =   4,
	Binding_Power__Bitwise_Or     =   6,
	Binding_Power__Bitwise_Xor    =   8,
	Binding_Power__Bitwise_And    =  10,
	Binding_Power__Equality       =  12,
	Binding_Power__Comparison     =  14,
	Binding_Power__Shift          =  16,
	Binding_Power__Additive       =  18,
	Binding_Power__Multiplicative =  20,
	Binding_Power__Unary          = 100,
}
Binding_Power;

internal Binding_Power
Binding_Power_from_Token_Kind(Token_Kind kind)
{
	Binding_Power result = Binding_Power__None;

	switch (kind)
	{
	case Token_Kind__Logical_Or:    { result = Binding_Power__Logical_Or;     } break;
	case Token_Kind__Logical_And:   { result = Binding_Power__Logical_And;    } break;
	case Token_Kind__Pipe:          { result = Binding_Power__Bitwise_Or;     } break;
	case Token_Kind__Caret:         { result = Binding_Power__Bitwise_Xor;    } break;
	case Token_Kind__Ampersand:     { result = Binding_Power__Bitwise_And;    } break;
	case Token_Kind__Equal:
	case Token_Kind__Equal_Not:     { result = Binding_Power__Equality;       } break;
	case Token_Kind__Less_Than:
	case Token_Kind__Greater_Than:
	case Token_Kind__Less_Equal:
	case Token_Kind__Greater_Equal: { result = Binding_Power__Comparison;     } break;
	case Token_Kind__Shift_Left:
	case Token_Kind__Shift_Right:   { result = Binding_Power__Shift;          } break;
	case Token_Kind__Plus:
	case Token_Kind__Minus:         { result = Binding_Power__Additive;       } break;
	case Token_Kind__Star:
	case Token_Kind__Slash:
	case Token_Kind__Percentage:    { result = Binding_Power__Multiplicative; } break;
	default:                        {} break;
	}

	return result;
}

internal Expression_Kind
Expression_Kind_from_binary_Token_Kind(Token_Kind kind)
{
	Expression_Kind result = Expression_Kind__None;

	switch (kind)
	{
	case Token_Kind__Plus:          { result = Expression_Kind__Add;           } break;
	case Token_Kind__Minus:         { result = Expression_Kind__Subtract;      } break;
	case Token_Kind__Star:          { result = Expression_Kind__Multiply;      } break;
	case Token_Kind__Slash:         { result = Expression_Kind__Divide;        } break;
	case Token_Kind__Percentage:    { result = Expression_Kind__Modulo;        } break;
	case Token_Kind__Pipe:          { result = Expression_Kind__Bitwise_Or;    } break;
	case Token_Kind__Caret:         { result = Expression_Kind__Bitwise_Xor;   } break;
	case Token_Kind__Ampersand:     { result = Expression_Kind__Bitwise_And;   } break;
	case Token_Kind__Shift_Left:    { result = Expression_Kind__Shift_Left;    } break;
	case Token_Kind__Shift_Right:   { result = Expression_Kind__Shift_Right;   } break;
	case Token_Kind__Equal:         { result = Expression_Kind__Equal;         } break;
	case Token_Kind__Equal_Not:     { result = Expression_Kind__Not_Equal;     } break;
	case Token_Kind__Less_Than:     { result = Expression_Kind__Less_Than;     } break;
	case Token_Kind__Less_Equal:    { result = Expression_Kind__Less_Equal;    } break;
	case Token_Kind__Greater_Than:  { result = Expression_Kind__Greater_Than;  } break;
	case Token_Kind__Greater_Equal: { result = Expression_Kind__Greater_Equal; } break;
	case Token_Kind__Logical_And:   { result = Expression_Kind__Logical_And;   } break;
	case Token_Kind__Logical_Or:    { result = Expression_Kind__Logical_Or;    } break;
	default:                        {} break;
	}

	return result;
}

internal Expression_Kind
Expression_Kind_from_unary_Token_Kind(Token_Kind kind)
{
	Expression_Kind result = Expression_Kind__None;

	switch (kind)
	{
	case Token_Kind__Minus: { result = Expression_Kind__Negate;      } break;
	case Token_Kind__Tilde: { result = Expression_Kind__Bitwise_Not; } break;
	case Token_Kind__Bang:  { result = Expression_Kind__Logical_Not; } break;
	default:                {} break;
	}

	return result;
}

typedef struct Expression_Parser Expression_Parser;
struct Expression_Parser
{
	Token_Cursor     *cursor;
	Arena            *arena;
	Symbol_Table     *table;
	Expression_Error *error;
}
Expression_Parser;

internal void
Expression_Parser_expect(Expression_Parser *parser, B32 condition, Expression_Error_Kind error_kind)
{

}

internal void
EX_expect(B32 condition, Expression_Error *error, Expression_Error_Kind error_kind, Token_Cursor *cursor)
{
	if (condition && !error->kind)
	{
		*error = Expression_Error_new(error_kind, cursor->current);
	}
	return;
}

internal void
EX_expect_token(Token_Kind token_kind, Expression_Error *error, Expression_Error_Kind error_kind, Token_Cursor *cursor)
{
	EX_expect(cursor->current.kind == token_kind);
	return;
}


// Forward declaration for mutual recursion.
internal Expression_Node *
EX_parse_expression(Token_Cursor *cursor, Binding_Power binding_power_minimum,
                    Arena *arena, Expression_Error *error);


// Null denotation: handles prefix positions (atoms, unary operators,
// parenthesized groups, relocations). The token has already been consumed
// from the cursor before this call.
internal Expression_Node *
EX_parse_null_denotation(Token_Cursor *cursor, Arena *arena, Expression_Error *error)
{
	Expression_Node *node = 0;
	Token token = cursor->current;

	switch (token)
	{
	case Token_Kind__Number_Literal:
	{
		node                      = Arena_push_struct_m(arena, Expression_Node);
		node->kind                = Expression_Kind__Number_Literal;
		node->value.integer_value = token.numerical_value;

		Token_Cursor_advance(&cursor);
	} break;

	case Token_Kind__Char_Literal:
	{
		node                      = Arena_push_struct_m(arena, Expression_Node);
		node->kind                = Expression_Kind__Char_Literal;
		node->value.integer_value = token.numerical_value;

		Token_Cursor_advance(&cursor);
	} break;

	case Token_Kind__Dot:
	{
		node                      = Arena_push_struct_m(arena, Expression_Node);
		node->kind                = Expression_Kind__Current_Address;

		Token_Cursor_advance(&cursor);
	} break;

	case Token_Kind__Identifier:
	{
		node                     = Arena_push_struct_m(arena, Expression_Node);
		node->kind               = Expression_Kind__Identifier;
		node->value.source.index = token.index;
		node->value.source.size  = token.size;

		Token_Cursor_advance(&cursor);
	} break;

	case Token_Kind__Minus:
	case Token_Kind__Tilde:
	case Token_Kind__Bang:
	{
		Expression_Node *operand = EX_parse_expression(cursor, Binding_Power__Unary, arena, error);
		node       = Arena_push_struct_m(arena, Expression_Node);
		node->kind = Expression_Kind_from_unary_Token_Kind(token.kind);
		node->left = operand;
		Token_Cursor_advance(&cursor);
	} break;

	case Token_Kind__Relocation_Prefix:
	{	// %reloc(expression)
		Token_Cursor_advance(&cursor);
		EX_expect_token(Token_Kind__Identifier, error, Expression_Error_Kind__Relocation_Syntax_Invalid, cursor);
		Token relocation_name = cursor->current;

		Token_Cursor_advance(cursor);
		EX_expect_token(Token_Kind__Left_Parenthesis, error, Expression_Error_Kind__Relocation_Syntax_Invalid, cursor);

		Token_Cursor_advance(cursor);
		Expression_Node *inner = EX_parse_expression(cursor, Binding_Power__None, arena, error);

		Token_Cursor_advance(cursor);
		EX_expect_token(Token_Kind__Right_Parenthesis, error, Expression_Error_Kind__Relocation_Syntax_Invalid, cursor);

		node                     = Arena_push_struct_m(arena, Expression_Node);
		node->kind               = Expression_Kind__Relocation;
		node->left               = inner;
		node->value.source.index = relocation_name.index;
		node->value.source.size  = relocation_name.size;

		Token_Cursor_advance(cursor);

	} break;

	case Token_Kind__Left_Parenthesis:
	{
		Token_Cursor_advance(cursor);
		Expression_Node *inner = EX_parse_expression(cursor, Binding_Power__None, arena, error);

		EX_expect_token(Token_Kind__Identifier, error, Expression_Error_Kind__Expected_Right_Parenthesis, cursor);
		Token_Cursor_advance(cursor);

		node = inner;
	} break;

	default:
	{
		*error = Expression_Error_new(Expression_Error_Kind__Unexpected_Token, token);
	} break;
	}

	return node;
}


// Core Pratt parser loop. Parses an expression where all binary operators
// must have binding power strictly greater than binding_power_minimum.
// All operators are left-associative (the <= comparison ensures this).
internal Expression_Node *
EX_parse_expression(Token_Cursor *cursor, Binding_Power binding_power_minimum,
                    Arena *arena, Expression_Error *error, Expression_Flags flags)
{
	Expression_Node *left = 0;
	EX_expect(!cursor->end_reached, error, Expression_Error_Kind__Unexpected_End, cursor);

	left = EX_parse_null_denotation(cursor, arena, error);
	for (;;)
	{
		Token_Kind operator_kind = cursor->current.kind;
		Binding_Power next_power = Binding_Power_from_Token_Kind(operator_kind);
		B32 break_should = next_power <= binding_power_minimum && !cursor->end_reached && !error.kind;
		if (break_should)
		{
			break;
		}

		Expression_Node *right = EX_parse_expression(cursor, next_power, arena, error);
		Expression_Node *node = Arena_push_struct_m(arena, Expression_Node);
		node->kind  = Expression_Kind_from_binary_Token_Kind(operator_kind);
		node->left  = left;
		node->right = right;
		left = node;
	}

	return left;
}


// Entry point. Parses an expression starting at the current cursor position.
// Advances the cursor past consumed tokens. On error, error->kind is nonzero.
internal Expression_Node *
EX_parse(Token_Cursor *cursor, Arena *arena, Expression_Error *error)
{
	Expression_Node *node = EX_parse_expression(cursor, Binding_Power__None, arena, error);
	return node;
}

#endif // EXPRESSION_H
