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

	for (;;)
	{
		if (frame == 0 || !frame->node->kind) // or error
		{
			break;
		}
		Expression_Node *node = frame->node;

		if (node->evaluation == Expression_Kind__Constant)
		{
			result_end = node->integer_value;
			SLL_stack_pop_m(frame);
		}
		else if (node->index_right && !(frame->state & Evaluation_Frame_State__Right_Evaluated))
		{
			// We have to evaluate the inner expression
			frame->state |= Evaluation_Frame_State__Right_Evaluated;
			Evaluation_Frame *frame_new = Arena__push_struct_m(scratch.arena, Evaluation_Frame);
			frame_new->node = xar_get_m(expressions, node->index_right);
			SLL_stack_push_n_m(frame, frame_new, next);
		}
		else if (node->index_left && !(frame->state & Evaluation_Frame_State__Left_Evaluated))
		{
			// We have to evaluate the inner expression
			frame->state |= Evaluation_Frame_State__Left_Evaluated;
			Evaluation_Frame *frame_new = Arena__push_struct_m(scratch.arena, Evaluation_Frame);
			frame_new->node = xar_get_m(expressions, node->index_left);
			SLL_stack_push_n_m(frame, frame_new, next);
		}
		else if (node->index_right && node->index_left)
		{
			Expression_Node *left  = xar_get_m(expressions, node->index_left);
			Expression_Node *right = xar_get_m(expressions, node->index_right);

			// Rethink evaluation model which still keeps the parsed information?
			// Example: I can set a label difference to a constant if they belong to the same frag.

			// example: (symbol1 + 2) * (symbol2 + 4) =

			if (left->evaluation == Expression_Kind__Constant && right->evaluation == Expression_Kind__Constant)
			{
				S64 result = operation_evaluate(node->kind, left->integer_value, right->integer_value);
				node->integer_value = result;
				node->evaluation = Expression_Kind__Constant;
				result_end = result;
			}
			else if (left->kind == Expression_Kind__Symbol && right->kind == Expression_Kind__Symbol)
			{
				B32 same_fragment = left->symbol->fragment          == right->symbol->fragment;
				B32 same_section  = left->symbol->elf.section_index == right->symbol->elf.section_index;
				B32 subtract      = node->kind                      == Expression_Kind__Subtract;
				// same_fragment implies same_section
				assert_always_m(!same_section || same_fragment && "same fragment but different section");

				if (same_fragment && subtract)
				{
					// Fold to constant.
					node->evaluation    = Expression_Kind__Constant;
				}

				if (subtract && same_section)
				{
					node->integer_value = left->symbol->elf.value - right->symbol->elf.value;
				}
				else
				{
					node->symbol         = left->symbol;
					node->symbol_operand = right->symbol;
				}
			}
			SLL_stack_pop_m(frame);
		}
		else if (node->index_right)
		{
			Expression_Node *right = xar_get_m(expressions, node->index_right);
			if (right->kind == Expression_Kind__Constant)
			{
				S64 result = unary_evaluate(node->kind, right->integer_value);
				node->integer_value = result;
				node->evaluation = Expression_Kind__Constant;
				result_end = result;
			}
			else
			{
				// Absorb it.
				node->evaluation = node->kind;
				node->symbol     = right->symbol;
			}
			SLL_stack_pop_m(frame);
		}
		else
		{
			// Leaf reached.
			B32 constant = node->kind == Expression_Kind__Constant;
			assert_always_m(node->index_left == 0);
			assert_always_m(constant || node->kind == Expression_Kind__Symbol);

			if (constant)
			{
				node->evaluation = Expression_Kind__Constant;
			}

			Symbol_Ref *symbol = node->symbol;
			if (symbol && symbol->elf.section_index == ELF_Section_Index__Absolute)
			{
				node->evaluation = Expression_Kind__Constant;
				node->integer_value = symbol->elf.value;
			}

			result_end = node->integer_value;
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
