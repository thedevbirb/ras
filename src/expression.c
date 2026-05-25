internal Binding_Power
Binding_Power_from_Token_Kind(Token_Kind kind)
{
	Binding_Power result = Binding_Power__None;

	switch (kind)
	{
	case Token_Kind__Pipe_2:    { result = Binding_Power__Logical_Or;     } break;
	case Token_Kind__Ampersand_2:   { result = Binding_Power__Logical_And;    } break;
	case Token_Kind__Pipe:          { result = Binding_Power__Bitwise_Or;     } break;
	case Token_Kind__Caret:         { result = Binding_Power__Bitwise_Xor;    } break;
	case Token_Kind__Ampersand:     { result = Binding_Power__Bitwise_And;    } break;
	case Token_Kind__Equal_2:
	case Token_Kind__Equal_Bang:     { result = Binding_Power__Equality;       } break;
	case Token_Kind__Less:
	case Token_Kind__Greater:
	case Token_Kind__Less_Equal:
	case Token_Kind__Greater_Equal: { result = Binding_Power__Comparison;     } break;
	case Token_Kind__Less_2:
	case Token_Kind__Greater_2:   { result = Binding_Power__Shift;          } break;
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
Expression_Kind__binary_from_Token_Kind(Token_Kind kind)
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
	case Token_Kind__Less_2:    { result = Expression_Kind__Shift_Left;    } break;
	case Token_Kind__Greater_2:   { result = Expression_Kind__Shift_Right;   } break;
	case Token_Kind__Equal_2:         { result = Expression_Kind__Equal;         } break;
	case Token_Kind__Equal_Bang:     { result = Expression_Kind__Not_Equal;     } break;
	case Token_Kind__Less:     { result = Expression_Kind__Less_Than;     } break;
	case Token_Kind__Less_Equal:    { result = Expression_Kind__Less_Equal;    } break;
	case Token_Kind__Greater:  { result = Expression_Kind__Greater_Than;  } break;
	case Token_Kind__Greater_Equal: { result = Expression_Kind__Greater_Equal; } break;
	case Token_Kind__Ampersand_2:   { result = Expression_Kind__Logical_And;   } break;
	case Token_Kind__Pipe_2:    { result = Expression_Kind__Logical_Or;    } break;
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

void
Expressions_initialize(Expressions *expressions, Arena *arena)
{
	*expressions = (Expressions)
	{
		.arena = arena,
		.data  = (Expression_Node *)(Arena__push_zero_m(arena)),
		.count = 0,
	};
	return;
}

Expression_Node *
Expressions_push_empty(Expressions *expressions)
{
	Expression_Node *node = Arena__push_struct_m(expressions->arena, Expression_Node);
	node->index           = expressions->count;
	expressions->count   += 1;

	return node;
}
