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

B32
Expression_Kind_leaf_is(Expression_Kind kind)
{
	B32 result = kind == Expression_Kind__Number_Literal
	          || kind == Expression_Kind__Char_Literal
	          || kind == Expression_Kind__Identifier
	          || kind == Expression_Kind__Label_Numeric_Reference_Forward
	          || kind == Expression_Kind__Label_Numeric_Reference_Backward
	          || kind == Expression_Kind__Current_Address;
	return result;
}

B32
Expression_Kind_constant_is(Expression_Kind kind)
{
	B32 result = kind == Expression_Kind__Number_Literal
		  || kind == Expression_Kind__Char_Literal;
	return result;
}

// The evaluation status of an `Expression_Node`. The higher, the stricter, with zero being not evaluated at all.
typedef enum Evaluation
{
	Evaluation__None       = 0,
	// Contains unresolved symbols that will be patched at link time.
	Evaluation__Unresolved = 1,
	// Involves symbols that can be resolved at assembly time.
	Evaluation__Absolute   = 2,
	// Only involves constant-time arithmetic.
	Evaluation__Constant   = 3,
}
Evaluation;

// A parsed expression, which can be evaluated.
//
// The expression contains, among other fields, enough information to emit proper relocations after an unresolved
// evaluation. This includes a "main" symbol fields and additional "operand" symbol field, along with an addend and the
// operation kind.
//
// Example evaluation:
//
// Consider the expression `label_1 + 2`, with the node being `+`. After evaluation, the addend will be set to `2`,
// while the symbol field will be set to `label_1`.
//
// Consider the expression `(global_1 + 2) - (global_2 + 1)`, with the examined being `-`. After evaluation, the addend
// will be to `1`, the two symbol fields will be set.
//
// The expression contains indexes referring to the list of `Expressions` it is contained, and where its children, if
// any, are placed.
typedef struct Expression_Node Expression_Node;
struct Expression_Node
{
	S64 integer_value;
	Symbols_Table_Entry *symbols_table_entry;
	Symbols_Table_Entry *symbol_operand;

	U32 index;
	U32 index_left;
	U32 index_right;

	// Useful for later finding symbols etc.
	U32 token_index;

	Relocation_Operator relocation_operator;

	Expression_Kind  kind;
	Evaluation evaluation;

	U8 label_numeric_value;
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

#endif // EXPRESSION_H
