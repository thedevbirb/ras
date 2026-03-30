internal U32
hash_FNV_1a(String8 string)
{
	U32 hash = 2166136261u;

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= string.count;
		if (break_should)
		{
			break;
		}

		hash ^= (U8)string.data[index];
		hash *= 16777619u;

		index += 1;
	}

	return hash;
}


// Assumption: the provided arena is used ONLY for this.
internal void
Statements_initialize(Statements *statements, Arena *arena)
{
	statements->arena = arena;
	statements->count = 0;
	return;
}

internal void
Statements_push(Statements *statements, Statement statement)
{
	Statement *buffer = Arena_push_struct_m(statements->arena, Statement);
	*buffer = statement;

	statements->count += 1;
	return;
}

// It is a no-op if the end has been reached already.
internal void
Parser_advance(Parser *parser)
{
	parser->end_reached = parser->token_index + 1 == parser->token_count;
	parser->token_index      += !parser->end_reached;
	parser->token_current     = parser->tokens[parser->token_index];

	return;
}

internal Token *
Parser_peek_next(Parser *parser)
{
	Token *next = &parser->tokens[parser->token_index + 1];
	return next;
}

internal String8
Parser_token_string(Parser *parser)
{
	String8 string =
	{
		.data  = parser->input->data + parser->token_current.index,
		.count = (U64)parser->token_current.size,
	};
	return string;
}

internal void
Parser_error_set(Parser *parser, Parser_Error_Kind kind)
{
	Parser_Error error =
	{
		.kind               = kind,
		.row_index          = parser->token_current.row_index,
		.column_begin_index = parser->token_current.column_index,
		.column_end_index   = parser->token_current.column_index + parser->token_current.size - 1,
	};

	assert_always_m(error.column_begin_index <= error.column_end_index && "token_index bug");
	parser->error = error;

	return;
}

internal void
Parser_expect(Parser *parser, B32 condition, Parser_Error_Kind error_kind)
{
	if (!condition && !(parser->error.kind))
	{
		Parser_error_set(parser, error_kind);
	}
	return;
}

internal void
Parser_expect_token(Parser *parser, Token_Kind token_kind, Parser_Error_Kind error_kind)
{
	B32 condition = parser->token_current.kind = token_kind;
	Parser_expect(parser, condition, error_kind);
	return;
}

internal U8
Parser_expect_register(Parser *parser)
{
	String8 string = Parser_token_string(parser);
	U8 register_value = register_lookup(string);
	Parser_expect(parser, register_value != register_invalid, Parser_Error_Kind__Register_Invalid);
	return register_value;
}

internal Expression_Node *
Parser_parse_null_denotation(Parser *parser, Expression_Flags flags)
{
	// Expression_Node *node = Arena_push_struct_m(parser->arena, Expression_Node);
	Expression_Node *node  = Expressions_push_empty(parser->expressions);
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
		node->kind = Expression_Kind__Identifier;

		String8 key = Parser_token_string(parser);
		Symbols_Table_Entry *symbol = Symbols_Table_get(parser->symbols_table, key);

		Parser_expect(parser, flags & Expression_Flags__Deferred, Parser_Error_Kind__Expression_Identifier_Undefined);

		if (!symbol)
		{

			ELF64_Symbol symbol =
			{
				.section_index = parser->section_current->section_index,
			};
			Vec2_U32 slot_and_found = Symbols_Table_put(parser->symbols_table, key, symbol);
			assert_always_m(!slot_and_found.y);
		}

		Parser_advance(parser);
	} break;

	case Token_Kind__Minus:
	case Token_Kind__Tilde:
	case Token_Kind__Bang:
	{
		Expression_Node *operand = Parser__expression_parse(parser, Binding_Power__Unary, flags);
		node->kind = Expression_Kind_from_unary_Token_Kind(token.kind);
		node->index_left = operand->index;

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

		// node                     = Arena_push_struct_m(parser->arena, Expression_Node);
		node             = Expressions_push_empty(parser->expressions);
		node->kind       = Expression_Kind__Relocation;
		node->index_left = inner->index;

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
		// Expression_Node *node  = Arena_push_struct_m(parser->arena, Expression_Node);
		Expression_Node *node  = Expressions_push_empty(parser->expressions);

		node->kind        = Expression_Kind_from_binary_Token_Kind(operator_kind);
		node->index_left  = left->index;
		node->index_right = right->index;

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
		Expression_Node *node_left = &parser->expressions->data[node->index_left];
		U64 left = Parser_expression_evaluate(parser, node_left);
		value = ~(left - 1);
	} break;
	case Expression_Kind__Bitwise_Not:
	{
		Expression_Node *node_left = &parser->expressions->data[node->index_left];
		U64 left = Parser_expression_evaluate(parser, node_left);
		value = ~left;
	} break;
	case Expression_Kind__Logical_Not:
	{
		Expression_Node *node_left = &parser->expressions->data[node->index_left];
		U64 left = Parser_expression_evaluate(parser, node_left);
		value = !left;
	} break;
	default:
	{
		Expression_Node *node_right = &parser->expressions->data[node->index_right];
		U64 right = Parser_expression_evaluate(parser, node_right);
		Expression_Node *node_left = &parser->expressions->data[node->index_left];
		U64 left = Parser_expression_evaluate(parser, node_left);
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

internal void
Parser_instruction_I_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
	Expression_Unevaluated expression_unevaluated =
	{
		.expression = expression,
		.section_index = parser->section_current->section_index,
		.section_offset = parser->section_current->offset,
	};
	Expression_Unevaluated_List_push(parser->expression_unevaluated_list, expression_unevaluated);

	Parser_advance(parser);
}

internal void
Parser_Instruction_R_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
	Expression_Unevaluated expression_unevaluated =
	{
		.expression = expression,
		.section_index = parser->section_current->section_index,
		.section_offset = parser->section_current->offset,
	};
	Expression_Unevaluated_List_push(parser->expression_unevaluated_list, expression_unevaluated);

	Parser_advance(parser);
}

internal void
Parser_instruction_S_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
	Expression_Unevaluated expression_unevaluated =
	{
		.expression = expression,
		.section_index = parser->section_current->section_index,
		.section_offset = parser->section_current->offset,
	};
	Expression_Unevaluated_List_push(parser->expression_unevaluated_list, expression_unevaluated);

	Parser_advance(parser);
}

internal void
Parser_instruction_B_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
	Expression_Unevaluated expression_unevaluated =
	{
		.expression = expression,
		.section_index = parser->section_current->section_index,
		.section_offset = parser->section_current->offset,
	};
	Expression_Unevaluated_List_push(parser->expression_unevaluated_list, expression_unevaluated);

	Parser_advance(parser);
}

internal void
Parser_instruction_U_parse(Parser *parser)
{
	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
	Expression_Unevaluated expression_unevaluated =
	{
		.expression = expression,
		.section_index = parser->section_current->section_index,
		.section_offset = parser->section_current->offset,
	};
	Expression_Unevaluated_List_push(parser->expression_unevaluated_list, expression_unevaluated);

	Parser_advance(parser);
}

internal void
Parser_instruction_J_parse(Parser *parser)
{
	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
	Expression_Unevaluated expression_unevaluated =
	{
		.expression = expression,
		.section_index = parser->section_current->section_index,
		.section_offset = parser->section_current->offset,
	};
	Expression_Unevaluated_List_push(parser->expression_unevaluated_list, expression_unevaluated);

	Parser_advance(parser);
}

// TODO: how am I transforming output after this stage?
void
Parser_parse(Parser *parser)
{
	U8 data_directive_size = 0;
	for (;;)
	{
		B32 break_should = parser->end_reached || parser->error.kind;
		if (break_should)
		{
			break;
		}

		parser->token_index_before = parser->token_index;

		switch (parser->token_current.kind)
		{
		case Token_Kind__Newline: { Parser_advance(parser); } break;
		case Token_Kind__Label:
		{
			String8 key = Parser_token_string(parser);
			ELF64_Symbol symbol =
			{
				.value = parser->section_current->offset,
				.section_index = parser->section_current->section_index,
			};
			Vec2_U32 slot_and_found = Symbols_Table_put(parser->symbols_table, key, symbol);
			Parser_expect(parser, !slot_and_found.y, Parser_Error_Kind__Label_Duplicate);

			Statement statement =
			{
				.label_symbol_slot = slot_and_found.x,
				.token_index_begin = parser->token_index_before,
				.token_index_end   = parser->token_index,
				.section_index     = parser->section_current->section_index,
				.kind              = Statement_Kind__Label,
			};
			Statements_push(parser->statements, statement);

			Parser_advance(parser);
		} break;
		case Token_Kind__Directive:
		{
			String8 substring = Parser_token_string(parser);
			Directive_Kind directive_kind = Directive_Kind__from_String8(substring);

			Statement statement =
			{
				.kind = Statement_Kind__Directive,
				.directive_kind = directive_kind
			};

			switch (directive_kind)
			{
			case Directive_Kind__None:
			{
				Parser_error_set(parser, Parser_Error_Kind__Directive_Unknown);
			} break;
			case Directive_Kind__Word_Double: { data_directive_size += 1; } // fallthrough
			case Directive_Kind__Word:        { data_directive_size += 1; } // fallthrough
			case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
			case Directive_Kind__Byte:
			{
				data_directive_size += 1;

				// TODO: expand max number of expressions;
				U32 *expressions_indexes = Arena_push_array_m(parser->arena, U32, 16);
				U32 expressions_count = 0;

				// Format: .byte <expr_1> , ..., <expr_n>
				for (;;)
				{
					Parser_advance(parser);
					Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Immediate);
					expressions_indexes[expressions_count] = expression->index;
					expressions_count += 1;
					assert_always_m(expressions_count < 16);

					B32 token_newline = parser->token_current.kind == Token_Kind__Newline;
					B32 token_comma   = parser->token_current.kind == Token_Kind__Comma;

					Parser_expect(parser, token_comma || token_newline, Parser_Error_Kind__Directive_Data_Invalid);

					B32 break_should = parser->error.kind || parser->end_reached || token_newline;
					if (break_should)
					{
						break;
					}
				}
				statement.expressions_indexes = expressions_indexes;
				statement.expressions_count   = expressions_count;
				statement.size                = data_directive_size * expressions_count;

				data_directive_size = 0;
			} break;
			case Directive_Kind__Section:
			{
				Parser_advance(parser);

				String8 substring             = Parser_token_string(parser);
				Directive_Kind directive_kind = Directive_Kind__from_String8(substring);
				ELF64_Section section_index   = ELF64_Section_from_Directive_Kind[directive_kind];
				parser->section_current       = &parser->sections[section_index];

				Parser_expect(parser, section_index != 0, Parser_Error_Kind__Directive_Section_Argument_Invalid);
				Parser_advance(parser);
			} break;
			case Directive_Kind__Align:
			{
				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Immediate);
				U64 result = Parser_expression_evaluate(parser, expression);
				U64 power_two = 1 << result;

				statement.expressions_indexes = &expression->index;
				statement.expressions_count = 1;
				statement.size = power_two;
			} break;
			case Directive_Kind__Equality:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
				String8 key = Parser_token_string(parser);

				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

				ELF64_Symbol symbol =
				{
					.section_index = parser->section_current->section_index,
				};
				Symbols_Table_put(parser->symbols_table, key, symbol);

				statement.expressions_indexes = &expression->index;
				statement.expressions_count   = 1;
			} break;
			default:
			{
				ELF64_Section section_kind = ELF64_Section_from_Directive_Kind[directive_kind];
				assert_always_m(section_kind && "unhandled directive");

				parser->section_current = &parser->sections[section_kind];
				Parser_advance(parser);
			} break;
			}

			statement.token_index_begin  = parser->token_index_before;
			statement.token_index_end    = parser->token_index - 1;
			statement.section_index      = parser->section_current->section_index;

			Statements_push(parser->statements, statement);

		} break;
		case Token_Kind__Identifier:
		{
			// This must be an instruction.
			//
			// We assume worst size instruction expansion for simplicity.
			String8 instruction = Parser_token_string(parser);
			U32 instruction_hash = hash_FNV_1a(instruction);

			// TODO: actually, group by opcode as in the spec.

			switch (instruction_hash)
			{
			// case HASH_add:  {} // fallthrough
			// case HASH_sub:  {} // fallthrough
			// case HASH_sll:  {} // fallthrough
			// case HASH_slt:  {} // fallthrough
			// case HASH_sltu: {} // fallthrough
			// case HASH_xor:  {} // fallthrough
			// case HASH_srl:  {} // fallthrough
			// case HASH_sra:  {} // fallthrough
			// case HASH_or:   {} // fallthrough
			// case HASH_and:
			// {
			// 	Parser_Instruction_R_parse(parser);
			// } break;
			//
			// case HASH_jalr: {} // fallthrough
			// case HASH_addi: {} // fallthrough
			// case HASH_slti: {} // fallthrough
			// case HASH_sltu: {} // fallthrough
			// case HASH_xori: {} // fallthrough
			// case HASH_ori:  {} // fallthrough
			// case HASH_andi: {} // fallthrough
			// case HASH_slli: {} // fallthrough
			// case HASH_slri: {} // fallthrough
			// case HASH_srai:
			// {
			// 	Parser_instruction_I_parse(parser);
			// } break;
			//
			// case HASH_beq:  {} // fallthrough
			// case HASH_bne:  {} // fallthrough
			// case HASH_blt:  {} // fallthrough
			// case HASH_bge:  {} // fallthrough
			// case HASH_bltu: {} // fallthrough
			// case HASH_bgeu:
			// {
			// 	Parser_instruction_B_parse(parser);
			// } break;
			//
			// case HASH_lui:   {} // fallthrough
			// case HASH_auipc:
			// {
			// 	Parser_instruction_U_parse(parser);
			// } break;
			//
			// case HASH_jal:
			// {
			// 	Parser_instruction_J_parse(parser);
			// } break;
			//
			//
			// default:
			// {
			// 	Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
			// } break;
			}
		} break;
		default:
		{
			Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
		} break;
		}

		B32 loop_infinite_avoided = parser->token_index > parser->token_index_before || parser->error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop in parser");
	}

	return;
}
