internal Binding_Power
Binding_Power_from_Token_Kind(Token_Kind kind)
{
        Binding_Power result = Binding_Power__None;

        switch (kind)
        {
        case Token_Kind__Pipe_2:         { result = Binding_Power__Logical_Or;     } break;
        case Token_Kind__Ampersand_2:    { result = Binding_Power__Logical_And;    } break;
        case Token_Kind__Pipe:           { result = Binding_Power__Bitwise_Or;     } break;
        case Token_Kind__Caret:          { result = Binding_Power__Bitwise_Xor;    } break;
        case Token_Kind__Ampersand:      { result = Binding_Power__Bitwise_And;    } break;
        case Token_Kind__Equal_2:
        case Token_Kind__Equal_Bang:     { result = Binding_Power__Equality;       } break;
        case Token_Kind__Less:
        case Token_Kind__Greater:
        case Token_Kind__Less_Equal:
        case Token_Kind__Greater_Equal:  { result = Binding_Power__Comparison;     } break;
        case Token_Kind__Less_2:
        case Token_Kind__Greater_2:      { result = Binding_Power__Shift;          } break;
        case Token_Kind__Plus:
        case Token_Kind__Minus:          { result = Binding_Power__Additive;       } break;
        case Token_Kind__Star:
        case Token_Kind__Slash:
        case Token_Kind__Percentage:     { result = Binding_Power__Multiplicative; } break;
        default:                         {} break;
        }

        return result;
}

internal Expression_Kind
Expression_Kind__binary_from_Token_Kind(Token_Kind kind)
{
        Expression_Kind result = Expression_Kind__None;

        switch (kind)
        {
        case Token_Kind__Plus:            { result = Expression_Kind__Add;           } break;
        case Token_Kind__Minus:           { result = Expression_Kind__Subtract;      } break;
        case Token_Kind__Star:            { result = Expression_Kind__Multiply;      } break;
        case Token_Kind__Slash:           { result = Expression_Kind__Divide;        } break;
        case Token_Kind__Percentage:      { result = Expression_Kind__Modulo;        } break;
        case Token_Kind__Pipe:            { result = Expression_Kind__Bitwise_Or;    } break;
        case Token_Kind__Caret:           { result = Expression_Kind__Bitwise_Xor;   } break;
        case Token_Kind__Ampersand:       { result = Expression_Kind__Bitwise_And;   } break;
        case Token_Kind__Less_2:          { result = Expression_Kind__Shift_Left;    } break;
        case Token_Kind__Greater_2:       { result = Expression_Kind__Shift_Right;   } break;
        case Token_Kind__Equal_2:         { result = Expression_Kind__Equal;         } break;
        case Token_Kind__Equal_Bang:      { result = Expression_Kind__Not_Equal;     } break;
        case Token_Kind__Less:            { result = Expression_Kind__Less_Than;     } break;
        case Token_Kind__Less_Equal:      { result = Expression_Kind__Less_Equal;    } break;
        case Token_Kind__Greater:         { result = Expression_Kind__Greater_Than;  } break;
        case Token_Kind__Greater_Equal:   { result = Expression_Kind__Greater_Equal; } break;
        case Token_Kind__Ampersand_2:     { result = Expression_Kind__Logical_And;   } break;
        case Token_Kind__Pipe_2:          { result = Expression_Kind__Logical_Or;    } break;
        default:                          {} break;
        }

        return result;
}

// TODO(low): maybe this should be done on U64 so we don't have UB. And division by zero is zero.
internal S64
operation_evaluate(Expression_Kind kind, S64 a, S64 b)
{
        S64 result = 0;

        switch (kind)
        {
        case Expression_Kind__Add:           { result = a +  b; } break;
        case Expression_Kind__Subtract:      { result = a -  b; } break;
        case Expression_Kind__Multiply:      { result = a *  b; } break;
        case Expression_Kind__Divide:        { result = a /  b; } break;
        case Expression_Kind__Modulo:        { result = a %  b; } break;

        case Expression_Kind__Bitwise_Or:    { result = a |  b; } break;
        case Expression_Kind__Bitwise_Xor:   { result = a ^  b; } break;
        case Expression_Kind__Bitwise_And:   { result = a &  b; } break;
        case Expression_Kind__Shift_Left:    { result = a << b; } break;
        case Expression_Kind__Shift_Right:   { result = a >> b; } break;

        case Expression_Kind__Equal:         { result = a == b; } break;
        case Expression_Kind__Not_Equal:     { result = a != b; } break;
        case Expression_Kind__Less_Than:     { result = a <  b; } break;
        case Expression_Kind__Less_Equal:    { result = a <= b; } break;
        case Expression_Kind__Greater_Than:  { result = a >  b; } break;
        case Expression_Kind__Greater_Equal: { result = a >= b; } break;

        case Expression_Kind__Logical_And:   { result = a && b; } break;
        case Expression_Kind__Logical_Or:    { result = a || b; } break;

        default: { unreachable_m(); } break;
        }

        return result;
}

internal S64
unary_evaluate(Expression_Kind kind, S64 a)
{
        S64 result = 0;

        switch (kind)
        {
        case Expression_Kind__Negate:        { result = -a; } break;
        case Expression_Kind__Logical_Not:   { result = !a; } break;
        case Expression_Kind__Bitwise_Not:   { result = ~a; } break;

        default: { unreachable_m(); } break;
        }

        return result;
}

// I think this can be dropped and we can use Expression_Kind in the evaluation for that.
typedef enum Evaluation_Frame_State
{
        Evaluation_Frame_State__None             = 0 << 0,
        Evaluation_Frame_State__Right_Evaluated  = 1 << 0,
        Evaluation_Frame_State__Left_Evaluated   = 1 << 1,
        Evaluation_Frame_State__COUNT,
}
Evaluation_Frame_State;


typedef struct Evaluation_Frame Evaluation_Frame;
struct Evaluation_Frame
{
        Expression  *node;
        Evaluation_Frame *next;
        Evaluation_Frame_State state;
};

internal S64
expression_evaluate(Expression *node_root)
{
        Symbol_Ref symbol = { .expression = node_root };
        S64 result = Symbol_Ref__resolve(&symbol, 0, Resolve_Level__None);
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

// // TODO(low): review utlity of this
// internal void
// Expressions__initialize(Expressions *expressions, Arena *arena)
// {
//         Expressions_push_empty(expressions, arena);
//         return;
// }

Expression *
Expressions_push_empty(Expressions *expressions, Arena *arena)
{
        Expression *node = Arena__push_struct_m(arena, Expression);
        SLL_queue_push_m(expressions->first, expressions->last, node);
        expressions->count += 1;

        return node;
}

Expression *
Expressions__push_constant(Expressions *expressions, Arena *arena, S64 constant)
{
        Expression *node = Expressions_push_empty(expressions, arena);

        node->integer_value = constant;
        node->kind          = Expression_Kind__Constant;
        node->evaluation    = Expression_Kind__Constant;

        return node;
}

internal Expression *
Expression__push_symbol(Expressions *expressions, Arena *arena, Symbol_Ref *symbol)
{
        Expression *result = Expressions_push_empty(expressions, arena);

        result->symbol     = symbol;
        result->kind       = Expression_Kind__Symbol;
        result->evaluation = Expression_Kind__Symbol;
        return result;
}

internal B32
Expression__internal_is(Expression *expression)
{
        B32 result = expression->location == 0
                  && expression->location_range.v[0] == 0
                  && expression->location_range.v[1] == 0;
        return result;
}

// Evaluate all expressions while finalizing symbols. See `Symbol_Ref__finalize`/`Symbols_Table__finalize`.
//
// After this is called expressions cannot be reduced further, since now symbols are frozen.
internal void
Expressions__finalize(Expressions *expressions, Diagnostics *diagnostics)
{
        // TODO(low): I don't like that the sentinel expression should be skipped. I would prefer a no-op here.
        for each_node_m(expressions->first->next, expression)
        {
                Symbol_Ref symbol_expression = { .expression = expression };
                Symbol_Ref__resolve(&symbol_expression, diagnostics, Resolve_Level__Finalize);
        }
        return;
}
