#ifndef CORE_EXPRESSION_H
#define CORE_EXPRESSION_H

// This can be used both for _both_ parsing information and evaluation information.
//
// Consider the expression `1 + 2`, which creates a tree rooted in `+`.
// Such root node will have `Expression_Kind__Add` regarding parsing information,
// since the token underlying the node contains a plus sign.
//
// However, when the expression is evaluated the root can be folded to a constant expression
// which value is `3`, and so we would track it as a `Expression_Kind__Constant` expression.
// The use of this enumeration for evaluation purposes is akin to GNU as `operatorT`.
//
// When evaluating an expression, using `expression_evaluate` or `Symbol_Ref__resolve`, the `Expression.evaluation`
// field should always be NOT zero, and in the worst unresolvable case equal to the `Expression.kind` field.
typedef enum Expression_Kind
{
        Expression_Kind__None,

        // Leaf nodes
        Expression_Kind__Constant,
        Expression_Kind__Symbol,

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

internal B32
Expression_Kind__unary_is(Expression_Kind kind)
{
        B32 result = kind == Expression_Kind__Negate
                  || kind == Expression_Kind__Bitwise_Not
                  || kind == Expression_Kind__Logical_Not;
        return result;
}

internal B32
Expression_Kind__equality_is(Expression_Kind kind)
{
        B32 result = kind == Expression_Kind__Equal
                  || kind == Expression_Kind__Not_Equal;
        return result;
}

internal B32
Expression_Kind__comparison_is(Expression_Kind kind)
{
        B32 result = kind == Expression_Kind__Not_Equal
                  || kind == Expression_Kind__Less_Than
                  || kind == Expression_Kind__Less_Equal
                  || kind == Expression_Kind__Greater_Than
                  || kind == Expression_Kind__Greater_Equal;
        return result;
}

// An `Expression` contains information about both a parsed expression and its evaluation, where the latter can
// mutate as more information is providing during multiple evaluation rounds, like during the relaxation process.
typedef struct Expression Expression;
struct Expression
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

        // Parsing-related fields. Pointers to child expression nodes.
        Expression *left;
        Expression *right;
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

// TODO(low): replace Xar with Expression chunks. See diary.md

#ifndef Expressions__xar_chunks
#define Expressions__xar_chunks 14
#endif

// Assumes first expression is a sentinel expression.
typedef struct Expressions Expressions;
struct Expressions
{
        Xar_Metadata     metadata;
        Xar_Header       header;
        Expression *chunks[Expressions__xar_chunks];
};

// MUST be called.
internal void
Expressions__initialize(Expressions *expressions, Arena *arena, U8 shift_amount);

Expression *
Expressions_push_empty(Expressions *expressions, Arena *arena);

// Create a constant expression.
internal Expression *
Expressions__push_constant(Expressions *expressions, Arena *arena, S64 value);

// Create an expression based on a single symbol
internal Expression *
Expression__push_symbol(Expressions *expressions, Arena *arena, Symbol_Ref *symbol);

// An internal, generated expression is one with no source location range
internal B32
Expression__internal_is(Expression *expression);

internal S64
unary_evaluate(Expression_Kind kind, S64 a);

internal S64
operation_evaluation(Expression_Kind kind, S64 a, S64 b);

// Evaluate all expressions while finalizing symbols. See `Symbol_Ref__resolve`/`Symbols_Table__finalize`.
internal void
Expressions__finalize(Expressions *expressions, Arena *arena, Diagnostics *diagnostics);

#endif // CORE_EXPRESSION_H
