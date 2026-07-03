#ifndef CORE_EXPRESSION_H
#define CORE_EXPRESSION_H

// After the parser processes an item, it ALWAYS advances.

// This can be used both for parsing information and evaluation information.
//
// Consider the expression `1 + 2`, which creates a tree rooted in `+`.
// Such root node will have `Expression_Kind__Add` regarding parsing information,
// since the token underlying the node contains a plus sign.
// However, when the expression is evaluated the root can be folded to a constant expression
// which value is `3`, and so we would track it as a `Expression_Kind__Constant` expression.
// The use of this enumeration for evaluation purposes is akin to GNU as `operatorT`.
typedef enum Expression_Kind
{
	Expression_Kind__None,

	// Leaf nodes
	Expression_Kind__Constant,
	Expression_Kind__Symbol,
	// Expression_Kind__Number,
	// Expression_Kind__Char_Literal,
	// Expression_Kind__Identifier,
	// Expression_Kind__Label_Numeric_Reference_Forward,
	// Expression_Kind__Label_Numeric_Reference_Backward,
	// Expression_Kind__Current_Address,   // .
	// Expression_Kind__Relocation,        // %hi(expr), %lo(expr), etc.

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

// An `Expression_Node` contains information about both a parsed expression and its evaluation, where the latter can
// mutate as more information is providing during multiple evaluation rounds, like during the relaxation process.
typedef struct Expression_Node Expression_Node;
struct Expression_Node
{
	// Location tracking. Consider `1 + 2` as an example.

	// Points to the location of the "root" token of the expression. For example, if the node is `+`, it would point
	// to its location.
	U32        location;
	// The location range of this expression. For example, if the node is `+` it would cover the whole subexpression
	// `1 + 2`.
	Range1_U32 location_range;


	// Evaluation-related fields, in a relocation friendly format.
	S64 integer_value;
	Symbol_Ref *symbol;
	Symbol_Ref *symbol_operand;
	Expression_Kind  kind;

	// Parsing-related fields. These indexes are the positions inside the `Expressions` Xar.
	U32 index;
	U32 index_left;
	U32 index_right;
	Expression_Kind  evaluation;

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
Expression_Kind__binary_from_Token_Kind(Token_Kind kind);

internal Expression_Kind
Expression_Kind_from_unary_Token_Kind(Token_Kind kind);

#ifndef Expressions__xar_chunks
#define Expressions__xar_chunks 14
#endif

// Assumes first expression is a sentinel expression.
typedef struct Expressions Expressions;
struct Expressions
{
	Xar_Metadata     metadata;
	Xar_Header       header;
	Expression_Node *chunks[Expressions__xar_chunks];
};

// MUST be called.
internal void
Expressions__initialize(Expressions *expressions, Arena *arena, U8 shift_amount);

Expression_Node *
Expressions_push_empty(Expressions *expressions, Arena *arena);

#endif // CORE_EXPRESSION_H
