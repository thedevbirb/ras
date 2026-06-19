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

// B32
// Expression_Kind_leaf_is(Expression_Kind kind)
// {
// 	B32 result = kind == Expression_Kind__Number_Literal
// 	          || kind == Expression_Kind__Char_Literal
// 	          || kind == Expression_Kind__Identifier
// 	          || kind == Expression_Kind__Label_Numeric_Reference_Forward
// 	          || kind == Expression_Kind__Label_Numeric_Reference_Backward
// 	          || kind == Expression_Kind__Current_Address;
// 	return result;
// }

// B32
// Expression_Kind_constant_is(Expression_Kind kind)
// {
// 	B32 result = kind == Expression_Kind__Number_Literal
// 		  || kind == Expression_Kind__Char_Literal;
// 	return result;
// }

// The evaluation status of an `Expression_Node`. The higher, the stricter, with zero being not evaluated at all.
// This ordering is important for comparisons, so changing it will break related code.
// typedef enum Evaluation
// {
// 	Evaluation__None       = 0,
// 	// Contains unresolved symbols that will be patched at link time.
// 	Evaluation__Unresolved = 1,
// 	// Involves symbols that can be resolved at assembly time.
// 	Evaluation__Absolute   = 2,
// 	// Only involves constant-time arithmetic.
// 	Evaluation__Constant   = 3,
// }
// Evaluation;
//
// // TODO(medium): use these instead of direct checks.
// internal B32
// Evaluation__absolute(Evaluation evaluation)
// {
// 	assert_always_m(evaluation <= Evaluation__Constant);
// 	B32 result = evaluation >= Evaluation__Absolute;
// 	return result;
// }
//
// // TODO(medium): use these instead of direct checks.
// internal B32
// Evaluation__unresolved(Evaluation evaluation)
// {
// 	assert_always_m(evaluation <= Evaluation__Constant);
// 	B32 result = evaluation != Evaluation__None && evaluation <= Evaluation__Unresolved;
// 	return result;
// }

// An `Expression_Node` contains information about both a parsed expression and its evaluation, where the latter can
// mutate as more information is providing during multiple evaluation rounds, like during the relaxation process.
typedef struct Expression_Node Expression_Node;
struct Expression_Node
{
	// The location across sources where this expression started.
	// With a binary operation like `1 + 2`, the location would point to `1`.
	//
	// TODO: this should be an U32. Then, would it make sense to save a range instead?
	// Perhaps it would be better to have a location for the root token, like '+' in this case,
	// and then a range containing the whole expresion.
	U64 location;

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

	// Useful for later finding symbols etc.
	// U32 token_index;
	//
	// Relocation_Operator relocation_operator;

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
