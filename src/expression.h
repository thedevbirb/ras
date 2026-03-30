#ifndef EXPRESSION_H
#define EXPRESSION_H

// After the parser processes an item, it ALWAYS advances.

// TODO: validate relocation names.
global char const *relocation_names[] =
{
	"hi",
	"lo",
	"pcrel_hi",
	"pcrel_lo",
	"got_pcrel_hi",

	"tprel_hi",
	"tprel_lo",
	"tprel_add",

	"tls_ie_pcrel_hi",
	"tls_gd_pcrel_hi",
	NULL,
};

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
	U32 index;
	U32 index_left;
	U32 index_right;

	U64 integer_value;

	// TODO: I don't know if this makes sense.
	U32 token_index;

	Expression_Kind  kind;
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


typedef struct Expression_Unevaluated Expression_Unevaluated;
struct Expression_Unevaluated
{
	Expression_Node *expression;
	U32 section_index;
	U32 section_offset;
};

list_declare_m(Expression_Unevaluated);
list_implement_m(Expression_Unevaluated);

// Assumption: the underlying arena is used only for storing expressions.
typedef struct Expressions Expressions;
struct Expressions
{
	Arena *arena;
	Expression_Node *data;
	U32 count;
};

void
Expressions_initialize(Expressions *expressions, Arena *arena)
{
	*expressions = (Expressions)
	{
		.arena = arena,
		.data  = (Expression_Node *)(Arena_push_zero_m(arena)),
		.count = 0,
	};
	return;
}

Expression_Node *
Expressions_push_empty(Expressions *expressions)
{
	Expression_Node *node = Arena_push_struct_m(expressions->arena, Expression_Node);
	node->index           = expressions->count;
	expressions->count   += 1;

	return node;
}

#endif // EXPRESSION_H
