internal Expression_Node *
Parser_parse_null_denotation(Parser *parser, Expression_Flags flags)
{
	Expression_Node *node = Arena_push_struct_m(parser->arena, Expression_Node);
	node->token_index     = parser->token_index;
	Token token           = parser->token_current;

	switch (token.kind)
	{
	case Token_Kind__Number_Literal:
	{
		node->kind          = Expression_Kind__Number_Literal;
		node->integer_value = token.numerical_value;

		Parser_advance(parser);
	} break;

	case Token_Kind__Char_Literal:
	{
		node->kind          = Expression_Kind__Char_Literal;
		node->integer_value = token.numerical_value;

		Parser_advance(parser);
	} break;

	case Token_Kind__Dot:
	{
		node->kind          = Expression_Kind__Current_Address;
		node->integer_value = parser->section_current->offset;

		Parser_advance(parser);
	} break;

	case Token_Kind__Identifier:
	{
		String8 key = Parser_token_string(parser);
		// FIX: there is always the problem of the colon after label.
		Symbol_Entry *symbol = Symbols_Table_get(parser->symbols_table, key);

		if (symbol)
		{
			node->kind = Expression_Kind__Number_Literal;
			node->integer_value = symbol->value.value;
		}
		else
		{
			Parser_expect(parser, flags & Expression_Flags__Deferred, Parser_Error_Kind__Expression_Identifier_Undefined);

			node->kind = Expression_Kind__Identifier;

			U32 offset = Object_File_Section_write(parser->section_string_table, key.data, key.count);
			ELF64_Symbol symbol =
			{
				.string_table_offset = offset,
				.value = 0,
				.section_index = parser->section_current->section_index,
			};
			B32 overridden = Symbols_Table_put(parser->symbols_table, key, symbol);
			assert_always_m(!overridden);
		}

		Parser_advance(parser);
	} break;

	case Token_Kind__Minus:
	case Token_Kind__Tilde:
	case Token_Kind__Bang:
	{
		Expression_Node *operand = Parser__expression_parse(parser, Binding_Power__Unary, flags);
		node->kind = Expression_Kind_from_unary_Token_Kind(token.kind);
		node->left = operand;

		Parser_advance(parser);
	} break;

	case Token_Kind__Relocation_Prefix:
	{	// %reloc(expression)
		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Left_Parenthesis, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);

		Parser_advance(parser);
		Expression_Node *inner = Parser__expression_parse(parser, Binding_Power__None, flags);

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Right_Parenthesis, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);

		node                     = Arena_push_struct_m(parser->arena, Expression_Node);
		node->kind               = Expression_Kind__Relocation;
		node->left               = inner;

		Parser_advance(parser);

	} break;

	case Token_Kind__Left_Parenthesis:
	{
		Parser_advance(parser);
		Expression_Node *inner = Parser__expression_parse(parser, Binding_Power__None, flags);

		Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Expression_Parenthesis_Right_Expected);
		Parser_advance(parser);

		node = inner;
	} break;

	default:
	{
		Parser_error_set(parser, Parser_Error_Kind__Expression_Unexpected_Token);
	} break;
	}

	return node;
}

// Core Pratt parser loop. Parses an expression where all binary operators
// must have binding power strictly greater than binding_power_minimum.
// All operators are left-associative (the <= comparison ensures this).
internal Expression_Node *
Parser__expression_parse(Parser *parser, Binding_Power binding_power_minimum, Expression_Flags flags)
{
	local_persist U8 recursion_level = 0;
	recursion_level += 1;

	Expression_Node *left = 0;
	Parser_expect(parser, !parser->end_reached, Parser_Error_Kind__Expression_Unexpected_End);
	Parser_expect(parser, recursion_level <= 8, Parser_Error_Kind__Expression_Recursion_Max);

	left = Parser_parse_null_denotation(parser, flags);
	for (;;)
	{
		Token_Kind operator_kind = parser->token_current.kind;
		Binding_Power next_power = Binding_Power_from_Token_Kind(operator_kind);

		B32 break_should = next_power <= binding_power_minimum || parser->end_reached || parser->error.kind;
		if (break_should)
		{
			break;
		}

		// Important to advance _after_ we have the right binding power otherwise we might tokens subsequent to
		// the expression, like commas.
		Parser_advance(parser);

		Expression_Node *right = Parser__expression_parse(parser, next_power, flags);
		Expression_Node *node  = Arena_push_struct_m(parser->arena, Expression_Node);

		node->kind     = Expression_Kind_from_binary_Token_Kind(operator_kind);
		node->left     = left;
		node->right    = right;

		left = node;
	}

	recursion_level -= 1;
	return left;
}


// Entry point. Parses an expression starting at the token_current parser position.
// Advances the parser past consumed tokens. On error, error->kind is nonzero.
Expression_Node *
Parser_expression_parse(Parser *parser, Expression_Flags flags)
{
	Expression_Node *node = Parser__expression_parse(parser, Binding_Power__None, flags);
	return node;
}

// It is a no-op if the parser has an error already.
U64
Parser_expression_evaluate(Parser *parser, Expression_Node *node)
{

	assert_always_m(node && "cannot evaluate null expression");
	assert_always_m(node->kind && "cannot evaluate unknown expression kind");

	U64 value = 0;
	Expression_Kind kind = node->kind & ~((parser->error.kind == 0) - 1);

	switch (kind)
	{
	case Expression_Kind__None:            {} break;

	case Expression_Kind__Number_Literal:  {  value             =  node->integer_value;     } break;
	case Expression_Kind__Char_Literal:    {  value             =  node->integer_value;     } break;
	case Expression_Kind__Identifier:      {  assert_always_m(0 && "todo");                 } break;
	case Expression_Kind__Current_Address: {  value             =  node->integer_value;     } break;
	case Expression_Kind__Relocation:      {  assert_always_m(0 && "todo");                 } break;

	case Expression_Kind__Negate:
	{
		U64 left = Parser_expression_evaluate(parser, node->left);
		value = ~(left - 1);
	} break;
	case Expression_Kind__Bitwise_Not:
	{
		U64 left = Parser_expression_evaluate(parser, node->left);
		value = ~left;
	} break;
	case Expression_Kind__Logical_Not:
	{
		U64 left = Parser_expression_evaluate(parser, node->left);
		value = !left;
	} break;
	default:
	{
		U64 right = Parser_expression_evaluate(parser, node->right);
		U64 left  = Parser_expression_evaluate(parser, node->left);
		switch (node->kind)
		{
		case Expression_Kind__Add:           { value = left +  right; } break;
		case Expression_Kind__Subtract:      { value = left -  right; } break;
		case Expression_Kind__Multiply:      { value = left *  right; } break;
		case Expression_Kind__Divide:        { value = left /  right; } break;
		case Expression_Kind__Modulo:        { value = left %  right; } break;

		case Expression_Kind__Bitwise_Or:    { value = left |  right; } break;
		case Expression_Kind__Bitwise_Xor:   { value = left ^  right; } break;
		case Expression_Kind__Bitwise_And:   { value = left &  right; } break;
		case Expression_Kind__Shift_Left:    { value = left << right; } break;
		case Expression_Kind__Shift_Right:   { value = left >> right; } break;

		case Expression_Kind__Equal:         { value = left == right; } break;
		case Expression_Kind__Not_Equal:     { value = left != right; } break;
		case Expression_Kind__Less_Than:     { value = left <  right; } break;
		case Expression_Kind__Less_Equal:    { value = left <= right; } break;
		case Expression_Kind__Greater_Than:  { value = left >  right; } break;
		case Expression_Kind__Greater_Equal: { value = left >= right; } break;

		case Expression_Kind__Logical_And:   { value = left && right; } break;
		case Expression_Kind__Logical_Or:    { value = left || right; } break;

		default: { Parser_error_set(parser, Parser_Error_Kind__Expression_Kind_Unknown); } break;
		}
	} break;
	}

	return value;
}
