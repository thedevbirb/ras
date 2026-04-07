#ifndef EXPRESSION_H
#define EXPRESSION_H

// After the parser processes an item, it ALWAYS advances.

typedef enum Expression_Kind
{
	Expression_Kind__None,

	// Leaf nodes
	Expression_Kind__Number_Literal,
	Expression_Kind__Char_Literal,
	Expression_Kind__Identifier,
	Expression_Kind__Label_Numeric_Reference_Forward,
	Expression_Kind__Label_Numeric_Reference_Backward,
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
	// Useful for later finding symbols etc.
	U32 token_index;
	// Whether evaluation lead to unresolved symbols.
	B32 unresolved;

	Relocation_Operator relocation_operator;

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
Binding_Power_from_Token_Kind(Token_Kind kind);

internal Expression_Kind
Expression_Kind_from_binary_Token_Kind(Token_Kind kind);

internal Expression_Kind
Expression_Kind_from_unary_Token_Kind(Token_Kind kind);

// Assumption: the underlying arena is used only for storing expressions.
typedef struct Expressions Expressions;
struct Expressions
{
	Arena *arena;
	Expression_Node *data;
	U32 count;
};

void
Expressions_initialize(Expressions *expressions, Arena *arena);

Expression_Node *
Expressions_push_empty(Expressions *expressions);

typedef enum Expression_Flags
{
	Expression_Flags__Deferred  = 1 << 0,
	Expression_Flags__Immediate = 1 << 1,
}
Expression_Flags;


#endif // EXPRESSION_H
