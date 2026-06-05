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

	default: { assert_always_m(0); } break;
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

	default: { assert_always_m(0); } break;
	}

	return result;
}

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
	Expression_Node  *node;
	Evaluation_Frame *next;
	Evaluation_Frame_State state;
};


internal S64
expression_evaluate(Expressions *expressions, U32 index)
{
	Arena_Temporary scratch = Arena__scratch_begin_m(0, 0);
	Evaluation_Frame *frame = Arena__push_struct_m(scratch.arena, Evaluation_Frame);
	frame->node = xar_get_m(expressions, index);
	S64 result_end = 0;

	// TODO: optimize to not re-evaluate constant expressions every time.
	// TODO: complete evaluation with symbols.

	for (;;)
	{
		if (frame == 0) // or error
		{
			break;
		}

		if (frame->node->index_right && !(frame->state & Evaluation_Frame_State__Right_Evaluated))
		{
			// We have to evaluate the inner expression
			frame->state |= Evaluation_Frame_State__Right_Evaluated;
			Evaluation_Frame *frame_new = Arena__push_struct_m(scratch.arena, Evaluation_Frame);
			frame_new->node = xar_get_m(expressions, frame->node->index_right);
			SLL_stack_push_n_m(frame, frame_new, next);
			continue;
		}

		if (frame->node->index_left && !(frame->state & Evaluation_Frame_State__Left_Evaluated))
		{
			// We have to evaluate the inner expression
			frame->state |= Evaluation_Frame_State__Left_Evaluated;
			Evaluation_Frame *frame_new = Arena__push_struct_m(scratch.arena, Evaluation_Frame);
			frame_new->node = xar_get_m(expressions, frame->node->index_left);
			SLL_stack_push_n_m(frame, frame_new, next);
			continue;
		}

		if (frame->node->index_right && frame->node->index_left)
		{
			Expression_Node *left  = xar_get_m(expressions, frame->node->index_left);
			Expression_Node *right = xar_get_m(expressions, frame->node->index_right);
			if (left->kind == Expression_Kind__Constant && right->kind == Expression_Kind__Constant)
			{
				S64 result = operation_evaluate(frame->node->kind, left->integer_value, right->integer_value);
				frame->node->integer_value = result;
				frame->node->evaluation = Evaluation__Constant;
				result_end = result;
			}
			else
			{
				// TODO: actually check symbols.
				frame->node->evaluation = Evaluation__Unresolved;
			}
			SLL_stack_pop_m(frame);
		}
		else if (frame->node->index_right)
		{
			Expression_Node *right = xar_get_m(expressions, frame->node->index_right);
			if (right->kind == Expression_Kind__Constant)
			{
				S64 result = unary_evaluate(frame->node->kind, right->integer_value);
				frame->node->integer_value = result;
				frame->node->evaluation = Evaluation__Constant;
				result_end = result;
			}
			else
			{
				// TODO: actually check symbols.
				frame->node->evaluation = Evaluation__Unresolved;
			}
			SLL_stack_pop_m(frame);
		}
		else
		{
			// Leaf reached.
			assert_always_m(frame->node->index_left == 0);
			result_end = frame->node->integer_value;
			// TODO: actually check symbols.

			SLL_stack_pop_m(frame);
		}
	}

	Arena__scratch_end_m(scratch);

	return result_end;
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

internal void
Expressions__initialize(Expressions *expressions, Arena *arena, U8 shift_amount)
{
	xar_initialize_m(expressions, shift_amount);
	Expressions_push_empty(expressions, arena);
	return;
}

Expression_Node *
Expressions_push_empty(Expressions *expressions, Arena *arena)
{
	U32 index             = expressions->header.count;
	Expression_Node *node = xar_push_m(expressions, arena);
	node->index           = index;

	return node;
}
