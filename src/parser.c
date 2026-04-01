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

// Returns the size of the literal string, as if were a byte slice, escaping characters.
// Assumes a string with valid escape sequences.
//
// Example: String8.data = [",\,n,h,e,l,l,o,\,n,"] -> 7
internal U32
String8_byte_size_escaped(String8 string)
{
	U32 size = 0;
	U32 index = 1;
	U32 count = string.count - 2; // No ".
	for (;;)
	{
		B32 break_should = index >= count;
		if (break_should)
		{
			break;
		}

		size += 1;

		U8 character = string.data[index];
		if (character == '\\')
		{
			index += 1;
			U8 character_escaped = string.data[index];
			B32 hex_prefix   = character_escaped == 'x';
			B32 octal_prefix = 0 <= character_escaped - '0' && character_escaped - '0' < 8;

				if (hex_prefix)
				{
					// E.g. \x1a
					//       ^--- cursor is here
					// We know from lexing the first one is guaranteed to be valid
					index += 2;
					// E.g. \x1a
					//         ^--- cursor is here
					U8 character = string.data[index];
					if (hex_table[character] != hex_table_sentinel_invalid)
					{
						index += 1;
					}
				}
				else if (octal_prefix)
				{
					// E.g. \377
					//       ^--- cursor is here
					index += 1;
					U8 character = string.data[index];
					if (character - '0' < 8)
					{
						index += 1;
					}

					character = string.data[index];
					if (character - '0' < 8)
					{
						index += 1;
					}
				}
				else
				{
					index += 2;
				}
		}
		else
		{
			index += 1;
		}
	}

	return size;
}

// Assumption: the provided arena is used ONLY for this.
internal void
Statements_initialize(Statements *statements, Arena *arena)
{
	*statements = (Statements)
	{
		.arena = arena,
		.data  = (Statement *)(Arena_push_zero_m(arena)),
		.count = 0,
	};
	return;
}

internal Statement *
Statements_push(Statements *statements, Statement statement)
{
	Statement *buffer = Arena_push_struct_m(statements->arena, Statement);
	*buffer = statement;

	statements->count += 1;
	return buffer;
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

internal String8
Parser_token_string_from_index(Parser *parser, U32 token_index)
{
	Token token = parser->tokens[token_index];
	String8 string =
	{
		.data  = parser->input->data + token.index,
		.count = (U64)token.size,
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
	node->token_index      = parser->token_index;
	Token token            = parser->token_current;

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

			Symbol symbol =
			{
				.section_index = parser->section_current_index,
			};
			Vec2_U32 slot_and_found = Symbols_Table_put(parser->symbols_table, key, symbol);
			assert_always_m(!slot_and_found.y);
		}

		Parser_advance(parser);
	} break;

	case Token_Kind__Label_Numeric_Reference_Forward:
	{
		node->kind          = Expression_Kind__Label_Numeric_Reference_Forward;
		node->integer_value = token.numerical_value;

		Parser_advance(parser);
	} break;
	case Token_Kind__Label_Numeric_Reference_Backward:
	{
		node->kind          = Expression_Kind__Label_Numeric_Reference_Backward;
		node->integer_value = token.numerical_value;

		Parser_advance(parser);
	} break;

	case Token_Kind__Minus:
	case Token_Kind__Tilde:
	case Token_Kind__Bang:
	{
		node->kind = Expression_Kind_from_unary_Token_Kind(token.kind);
		Parser_advance(parser);
		Expression_Node *operand = Parser__expression_parse(parser, Binding_Power__Unary, flags);
		node->index_left = operand->index;
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

Expression_Node *
Parser_expression_immediate_create(Parser *parser, U64 immediate)
{
	Expression_Node *node  = Expressions_push_empty(parser->expressions);
	node->integer_value = immediate;
	return node;
}

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
	case Expression_Kind__Identifier:
	{
		String8 key = Parser_token_string_from_index(parser, node->token_index);
		Symbols_Table_Entry *entry = Symbols_Table_get(parser->symbols_table, key);
		if (entry)
		{
			value = entry->value.value;
		}
		else
		{
			// emmo?
		}

	} break;
	// This below is also hard!
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

// Create a barebone, incomplete statement for an instruction.
internal Statement *
Parser_statement_instruction_create(Parser *parser)
{

	Statement statement =
	{
		.token_index_begin = parser->token_index_before,
		.token_index_end = parser->token_index,

		.size = 4, // TODO: make this a global?
		.section_index = parser->section_current_index,
		.kind = Statement_Kind__Instruction,
	};

	Statement *pointer = Statements_push(parser->statements, statement);
	return pointer;
}

internal void
Parser_instruction_I_parse(Parser *parser, Instruction_Kind instruction_kind)
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
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
}

internal void
Parser_instruction_R_parse(Parser *parser, Instruction_Kind instruction_kind)
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

	Statement *statement = Parser_statement_instruction_create(parser);

	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = register_source_2;
}

internal void
Parser_instruction_S_parse(Parser *parser, Instruction_Kind instruction_kind)
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

	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__S;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = register_source_2;
}

internal void
Parser_instruction_B_parse(Parser *parser, Instruction_Kind instruction_kind)
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

	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = register_source_2;
}

internal void
Parser_instruction_U_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__U;
	statement->register_destination = register_destination;
}

internal void
Parser_instruction_J_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__J;
	statement->register_destination = register_destination;
}

// nop -> addi x0, x0, 0
internal void
Parser_instruction_nop_parse(Parser *parser)
{
	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__ADDI;
	statement->instruction_format   = Instruction_Format__I;
}

// mv rd, rs -> addi rd, rs, 0
internal void
Parser_instruction_mv_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__ADDI;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
}

// not rd, rs -> xori rd, rs, -1
internal void
Parser_instruction_not_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Expression_Node *expression = Parser_expression_immediate_create(parser, -1);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__XORI;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
}

// neg rd, rs -> sub rd, x0, rs
internal void
Parser_instruction_neg_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__SUB;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = 0;
	statement->register_source_2    = register_source_1;
}

// negw rd, rs -> subw rd, x0, rs (RV64)
internal void
Parser_instruction_negw_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__SUBW;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = 0;
	statement->register_source_2    = register_source_1;
}

// sext.w rd, rs -> addiw rd, rs, 0 (RV64)
internal void
Parser_instruction_sext_w_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__ADDIW;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
}

// seqz rd, rs -> sltiu rd, rs, 1
internal void
Parser_instruction_seqz_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Expression_Node *expression = Parser_expression_immediate_create(parser, 1);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__SLTIU;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
}

// snez rd, rs -> sltu rd, x0, rs
internal void
Parser_instruction_snez_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__SLTU;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = 0;
	statement->register_source_2    = register_source_1;
}

// sltz rd, rs -> slt rd, rs, x0
internal void
Parser_instruction_sltz_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__SLT;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = 0;
}

// sgtz rd, rs -> slt rd, x0, rs
internal void
Parser_instruction_sgtz_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__SLT;
	statement->instruction_format   = Instruction_Format__R;
	statement->register_destination = register_destination;
	statement->register_source_1    = 0;
	statement->register_source_2    = register_source_1;
}

// beqz rs, offset -> beq rs, x0, offset
internal void
Parser_instruction_beqz_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BEQ;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = 0;
}

// bnez rs, offset -> bne rs, x0, offset
internal void
Parser_instruction_bnez_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BNE;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = 0;
}

// blez rs, offset -> bge x0, rs, offset
internal void
Parser_instruction_blez_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BGE;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = 0;
	statement->register_source_2    = register_source_1;
}

// bgez rs, offset -> bge rs, x0, offset
internal void
Parser_instruction_bgez_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BGE;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = 0;
}

// bltz rs, offset -> blt rs, x0, offset
internal void
Parser_instruction_bltz_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BLT;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_1;
	statement->register_source_2    = 0;
}

// bgtz rs, offset -> blt x0, rs, offset
internal void
Parser_instruction_bgtz_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BLT;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = 0;
	statement->register_source_2    = register_source_1;
}

// bgt rs, rt, offset -> blt rt, rs, offset
internal void
Parser_instruction_bgt_parse(Parser *parser)
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

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BLT;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_2;
	statement->register_source_2    = register_source_1;
}

// ble rs, rt, offset -> bge rt, rs, offset
internal void
Parser_instruction_ble_parse(Parser *parser)
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

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BGE;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_2;
	statement->register_source_2    = register_source_1;
}

// bgtu rs, rt, offset -> bltu rt, rs, offset
internal void
Parser_instruction_bgtu_parse(Parser *parser)
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

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BLTU;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_2;
	statement->register_source_2    = register_source_1;
}

// bleu rs, rt, offset -> bgeu rt, rs, offset
internal void
Parser_instruction_bleu_parse(Parser *parser)
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

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__BGEU;
	statement->instruction_format   = Instruction_Format__B;
	statement->register_source_1    = register_source_2;
	statement->register_source_2    = register_source_1;
}

// j offset -> jal x0, offset
internal void
Parser_instruction_j_parse(Parser *parser)
{
	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__JAL;
	statement->instruction_format   = Instruction_Format__J;
	statement->register_destination = 0;
}

// jr rs -> jalr x0, rs, 0
internal void
Parser_instruction_jr_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__JALR;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = 0;
	statement->register_source_1    = register_source_1;
}

// jalr rs -> jalr ra, rs, 0 (single-operand form)
internal void
Parser_instruction_jalr_pseudo_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);

	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__JALR;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = 1; // ra
	statement->register_source_1    = register_source_1;
}

// ret -> jalr x0, ra, 0
internal void
Parser_instruction_ret_parse(Parser *parser)
{
	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind     = Instruction_Kind__JALR;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_destination = 0;
	statement->register_source_1    = 1; // ra
}

// li rd, imm -> lui rd, %hi(imm) + addi rd, rd, %lo(imm)
// NOTE: For small immediates that fit in 12 bits, a single addi suffices.
//       The expansion decision may be deferred to a later pass.
internal void
Parser_instruction_li_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__LI;
	statement->instruction_format   = Instruction_Format__Expandable;
	statement->register_destination = register_destination;
}

// la rd, symbol -> auipc rd, %pcrel_hi(symbol) + addi rd, rd, %pcrel_lo(symbol)
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_la_parse(Parser *parser)
{
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__LA;
	statement->instruction_format   = Instruction_Format__Expandable;
	statement->register_destination = register_destination;
}

// call offset -> auipc ra, offsetHi + jalr ra, ra, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_call_parse(Parser *parser)
{
	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__CALL;
	statement->instruction_format   = Instruction_Format__Expandable;
	statement->register_destination = 1; // ra
}

// tail offset -> auipc t1, offsetHi + jalr x0, t1, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_tail_parse(Parser *parser)
{
	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->expressions_indexes  = &expression->index;
	statement->expressions_count    = 1;
	statement->instruction_kind     = Instruction_Kind__TAIL;
	statement->instruction_format   = Instruction_Format__Expandable;
	statement->register_destination = 0;
}

// TODO: how am I transforming output after this stage?
void
Parser_parse(Parser *parser)
{
	U8  data_directive_size = 0;
	U32 string_size = 0;
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
			Symbols_Table_Entry *entry = Symbols_Table_get(parser->symbols_table, key);
			B32 duplicate = entry && entry->value.label_defined;
			Parser_expect(parser, !duplicate, Parser_Error_Kind__Label_Duplicate);

			Symbol symbol =
			{
				.section_index = parser->section_current_index,
				.label_defined = 1,
			};
			Vec2_U32 slot_and_found = Symbols_Table_put(parser->symbols_table, key, symbol);

			Parser_advance(parser);
			Statement statement =
			{
				.label_symbol_slot = slot_and_found.x,
				.token_index_begin = parser->token_index_before,
				.token_index_end   = parser->token_index,
				.section_index     = parser->section_current_index,
				.kind              = Statement_Kind__Label,
			};
			Statements_push(parser->statements, statement);
		} break;
		case Token_Kind__Label_Numeric:
		{
			Parser_advance(parser);
			Statement statement =
			{
				.token_index_begin = parser->token_index_before,
				.token_index_end   = parser->token_index,
				.section_index     = parser->section_current_index,
				.kind              = Statement_Kind__Label_Numeric,
			};
			Statements_push(parser->statements, statement);
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
					Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);
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
			case Directive_Kind__String: {} // fallthrough
			case Directive_Kind__Asciz:  { string_size += 1; /* Add null-termination */ } // fallthrough
			case Directive_Kind__Ascii:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__String_Literal, Parser_Error_Kind__String_Literal_Expected);
				String8 ascii_text = Parser_token_string(parser);

				// We cannot emit bytes in the section yet, but we have to count the size of it.
				// NOTE: from lexing stage, we already know the string is well-formed.
				U32 size = String8_byte_size_escaped(ascii_text);
				string_size += size;

				statement.size = string_size;
				string_size = 0;

				Parser_advance(parser);

			} break;
			case Directive_Kind__Section:
			{
				Parser_advance(parser);

				String8 substring             = Parser_token_string(parser);
				Directive_Kind directive_kind = Directive_Kind__from_String8(substring);
				ELF64_Section section_index   = ELF64_Section_from_Directive_Kind[directive_kind];
				parser->section_current_index = section_index;

				Parser_expect(parser, section_index != 0, Parser_Error_Kind__Directive_Section_Argument_Invalid);
				Parser_advance(parser);
			} break;
			case Directive_Kind__Local:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 key = Parser_token_string(parser);
				Symbols_Table_Entry *entry = Symbols_Table_get(parser->symbols_table, key);
				if (entry)
				{
					B32 demoted = ELF64_Symbol_bind_m(entry->value.type_binding_info) > Symbol_Binding__Local;
					Parser_expect(parser, !demoted, Parser_Error_Kind__Symbol_Demoted);
					U8 type_binding_info =
						ELF64_Symbol_info_m(Symbol_Binding__Global, ELF64_Symbol_type_m(entry->value.type_binding_info));
					entry->value.type_binding_info = type_binding_info;
				}
				else
				{
					Symbol symbol =
					{
						.type_binding_info = ELF64_Symbol_info_m(Symbol_Binding__Global, 0),
						.section_index = parser->section_current_index,
					};
					Symbols_Table_put(parser->symbols_table, key, symbol);
				}

				Parser_advance(parser);
			} break;
			case Directive_Kind__Globl: {} // fallthrough
			case Directive_Kind__Global:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 key = Parser_token_string(parser);
				Symbols_Table_Entry *entry = Symbols_Table_get(parser->symbols_table, key);
				if (entry)
				{
					U8 type_binding_info =
						ELF64_Symbol_info_m(Symbol_Binding__Global, ELF64_Symbol_type_m(entry->value.type_binding_info));
					entry->value.type_binding_info = type_binding_info;
				}
				else
				{
					Symbol symbol =
					{
						.type_binding_info = ELF64_Symbol_info_m(Symbol_Binding__Global, 0),
						.section_index = parser->section_current_index,
					};
					Symbols_Table_put(parser->symbols_table, key, symbol);
				}

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
			case Directive_Kind__Set: {} // fallthrough
			case Directive_Kind__Equality:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
				String8 key = Parser_token_string(parser);

				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

				Symbol symbol =
				{
					.section_index = parser->section_current_index,
				};
				Symbols_Table_put(parser->symbols_table, key, symbol);

				statement.expressions_indexes = &expression->index;
				statement.expressions_count   = 1;
			} break;
			case Directive_Kind__Zero:
			{
				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

				statement.expressions_indexes = &expression->index;
				statement.expressions_count   = 1;
			} break;
			case Directive_Kind__Skip:
			{
				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

				if (parser->token_current.kind == Token_Kind__Comma)
				{
					Parser_advance(parser);
					// NOTE: Let's not allow too funky stuff for now
					Token token = parser->token_current;
					B32 condition = token.kind == Token_Kind__Number_Literal || token.kind == Token_Kind__Char_Literal;
					Parser_expect(parser, condition, Parser_Error_Kind__Directive_Argument_Invalid);

					Parser_advance(parser);
				}

				statement.expressions_indexes = &expression->index;
				statement.expressions_count   = 1;
			} break;
			case Directive_Kind__Common:
			{
				// .comm symbol, size, alignment
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 string = Parser_token_string(parser);
				Symbol symbol =
				{
					.type_binding_info = ELF64_Symbol_info_m(Symbol_Binding__Global, 0),
					.section_index = section_index_common,
				};

				Vec2_U32 slot_and_found = Symbols_Table_put(parser->symbols_table, string, symbol);
				Parser_expect(parser, !slot_and_found.y, Parser_Error_Kind__Symbol_Duplicate);

				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *size_expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *alignment_expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

				U32 *expressions_indexes = Arena_push_array_m(parser->arena, U32, 2);
				expressions_indexes[0] = size_expression->index;
				expressions_indexes[1] = alignment_expression->index;

				statement.expressions_indexes = expressions_indexes;
				statement.expressions_count = 2;
			} break;
			default:
			{
				ELF64_Section section_kind = ELF64_Section_from_Directive_Kind[directive_kind];
				assert_always_m(section_kind && "unhandled directive");

				parser->section_current_index = section_kind;
				Parser_advance(parser);
			} break;
			}

			statement.token_index_begin  = parser->token_index_before;
			statement.token_index_end    = parser->token_index - 1;
			statement.section_index      = parser->section_current_index;

			Statements_push(parser->statements, statement);

		} break;
		case Token_Kind__Identifier:
		{
			// This must be an instruction.
			String8 instruction = Parser_token_string(parser);
			U32 instruction_hash = hash_FNV_1a(instruction);

			switch (instruction_hash)
			{
			// U-type
			case HASH_lui:   { Parser_instruction_U_parse(parser, Instruction_Kind__LUI);   } break;
			case HASH_auipc: { Parser_instruction_U_parse(parser, Instruction_Kind__AUIPC); } break;

			// J-type
			case HASH_jal:   { Parser_instruction_J_parse(parser, Instruction_Kind__JAL);   } break;

			// I-type (JALR)
			case HASH_jalr:  { Parser_instruction_I_parse(parser, Instruction_Kind__JALR);  } break;

			// B-type
			case HASH_beq:   { Parser_instruction_B_parse(parser, Instruction_Kind__BEQ);   } break;
			case HASH_bne:   { Parser_instruction_B_parse(parser, Instruction_Kind__BNE);   } break;
			case HASH_blt:   { Parser_instruction_B_parse(parser, Instruction_Kind__BLT);   } break;
			case HASH_bge:   { Parser_instruction_B_parse(parser, Instruction_Kind__BGE);   } break;
			case HASH_bltu:  { Parser_instruction_B_parse(parser, Instruction_Kind__BLTU);  } break;
			case HASH_bgeu:  { Parser_instruction_B_parse(parser, Instruction_Kind__BGEU);  } break;

			// I-type (loads)
			case HASH_lb:    { Parser_instruction_I_parse(parser, Instruction_Kind__LB);    } break;
			case HASH_lh:    { Parser_instruction_I_parse(parser, Instruction_Kind__LH);    } break;
			case HASH_lw:    { Parser_instruction_I_parse(parser, Instruction_Kind__LW);    } break;
			case HASH_lbu:   { Parser_instruction_I_parse(parser, Instruction_Kind__LBU);   } break;
			case HASH_lhu:   { Parser_instruction_I_parse(parser, Instruction_Kind__LHU);   } break;

			// S-type
			case HASH_sb:    { Parser_instruction_S_parse(parser, Instruction_Kind__SB);    } break;
			case HASH_sh:    { Parser_instruction_S_parse(parser, Instruction_Kind__SH);    } break;
			case HASH_sw:    { Parser_instruction_S_parse(parser, Instruction_Kind__SW);    } break;

			// I-type (arithmetic)
			case HASH_addi:  { Parser_instruction_I_parse(parser, Instruction_Kind__ADDI);  } break;
			case HASH_slti:  { Parser_instruction_I_parse(parser, Instruction_Kind__SLTI);  } break;
			case HASH_sltiu: { Parser_instruction_I_parse(parser, Instruction_Kind__SLTIU); } break;
			case HASH_xori:  { Parser_instruction_I_parse(parser, Instruction_Kind__XORI);  } break;
			case HASH_ori:   { Parser_instruction_I_parse(parser, Instruction_Kind__ORI);   } break;
			case HASH_andi:  { Parser_instruction_I_parse(parser, Instruction_Kind__ANDI);  } break;
			case HASH_slli:  { Parser_instruction_I_parse(parser, Instruction_Kind__SLLI);  } break;
			case HASH_srli:  { Parser_instruction_I_parse(parser, Instruction_Kind__SRLI);  } break;
			case HASH_srai:  { Parser_instruction_I_parse(parser, Instruction_Kind__SRAI);  } break;

			// R-type
			case HASH_add:   { Parser_instruction_R_parse(parser, Instruction_Kind__ADD);   } break;
			case HASH_sub:   { Parser_instruction_R_parse(parser, Instruction_Kind__SUB);   } break;
			case HASH_sll:   { Parser_instruction_R_parse(parser, Instruction_Kind__SLL);   } break;
			case HASH_slt:   { Parser_instruction_R_parse(parser, Instruction_Kind__SLT);   } break;
			case HASH_sltu:  { Parser_instruction_R_parse(parser, Instruction_Kind__SLTU);  } break;
			case HASH_xor:   { Parser_instruction_R_parse(parser, Instruction_Kind__XOR);   } break;
			case HASH_srl:   { Parser_instruction_R_parse(parser, Instruction_Kind__SRL);   } break;
			case HASH_sra:   { Parser_instruction_R_parse(parser, Instruction_Kind__SRA);   } break;
			case HASH_or:    { Parser_instruction_R_parse(parser, Instruction_Kind__OR);    } break;
			case HASH_and:   { Parser_instruction_R_parse(parser, Instruction_Kind__AND);   } break;

			// Pseudo-instructions

			// Pseudo-instructions (no operands)
			case HASH_nop:    { Parser_instruction_nop_parse(parser);                       } break;
			case HASH_ret:    { Parser_instruction_ret_parse(parser);                       } break;
			// Pseudo-instructions (rd, rs)
			case HASH_mv:     { Parser_instruction_mv_parse(parser);                        } break;
			case HASH_not:    { Parser_instruction_not_parse(parser);                       } break;
			case HASH_neg:    { Parser_instruction_neg_parse(parser);                       } break;
			case HASH_negw:   { Parser_instruction_negw_parse(parser);                      } break;
			case HASH_sext_w: { Parser_instruction_sext_w_parse(parser);                    } break;
			case HASH_seqz:   { Parser_instruction_seqz_parse(parser);                      } break;
			case HASH_snez:   { Parser_instruction_snez_parse(parser);                      } break;
			case HASH_sltz:   { Parser_instruction_sltz_parse(parser);                      } break;
			case HASH_sgtz:   { Parser_instruction_sgtz_parse(parser);                      } break;
			// Pseudo-instructions (rs, offset)
			case HASH_beqz:   { Parser_instruction_beqz_parse(parser);                      } break;
			case HASH_bnez:   { Parser_instruction_bnez_parse(parser);                      } break;
			case HASH_blez:   { Parser_instruction_blez_parse(parser);                      } break;
			case HASH_bgez:   { Parser_instruction_bgez_parse(parser);                      } break;
			case HASH_bltz:   { Parser_instruction_bltz_parse(parser);                      } break;
			case HASH_bgtz:   { Parser_instruction_bgtz_parse(parser);                      } break;
			// Pseudo-instructions (rs, rt, offset)
			case HASH_bgt:    { Parser_instruction_bgt_parse(parser);                       } break;
			case HASH_ble:    { Parser_instruction_ble_parse(parser);                       } break;
			case HASH_bgtu:   { Parser_instruction_bgtu_parse(parser);                      } break;
			case HASH_bleu:   { Parser_instruction_bleu_parse(parser);                      } break;
			// Pseudo-instructions (offset only)
			case HASH_j:      { Parser_instruction_j_parse(parser);                         } break;
			case HASH_call:   { Parser_instruction_call_parse(parser);                      } break;
			case HASH_tail:   { Parser_instruction_tail_parse(parser);                      } break;
			// Pseudo-instructions (rs only)
			case HASH_jr:     { Parser_instruction_jr_parse(parser);                        } break;
			// Pseudo-instructions (rd, imm/symbol)
			case HASH_li:     { Parser_instruction_li_parse(parser);                        } break;
			case HASH_la:     { Parser_instruction_la_parse(parser);                        } break;
			default:
			{
				Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
			} break;
			}
		} break;
		default:
		{
			Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
		} break;
		}

		B32 loop_infinite_avoided = parser->token_index > parser->token_index_before || parser->error.kind || parser->end_reached;
		assert_always_m(loop_infinite_avoided && "infinite loop in parser");
	}

	return;
}

internal void
Parser_offsets_recompute(Parser *parser)
{
	U32 section_offsets[ELF64_Section__COUNT] = {0};

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= parser->statements->count;
		if (break_should)
		{
			break;
		}
		Statement statement      = parser->statements->data[index];
		U32 *section_offset      = &section_offsets[statement.section_index];
		statement.section_offset = *section_offset;

		if (statement.kind == Statement_Kind__Label)
		{
			Symbols_Table_Entry *entry = &parser->symbols_table->entries[statement.label_symbol_slot];
			entry->value.value = statement.section_offset;
		}

		*section_offset += statement.size;
		index += 1;
	}
}

internal B32
Parser_relax_pass(Parser *parser)
{
	B32 changed = 0;
	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= parser->statements->count;
		if (break_should)
		{
			break;
		}

		Statement *statement = &parser->statements->data[index];

		U32 size_old = statement->size;
		U32 size_new = size_old;

		switch (statement->instruction_kind)
		{
		case Instruction_Kind__LI:
		{
			// If the immediate fits in 12 bits, sign-extended, a single addi suffices.
			// Otherwise, we need a lui + addi, for 8 bytes total.

			U32 expression_index        = statement->expressions_indexes[0];
			Expression_Node *expression = &parser->expressions->data[expression_index];
			S64 immediate               = (S64)Parser_expression_evaluate(parser, expression);

			B32 range_in = -2048 <= immediate && immediate <= 2047;
			if (!range_in)
			{
				size_new = 8;
			}
		} break;
		case Instruction_Kind__CALL:
		{
			// jal has a 21-bit signed offset range. If the target is not within that range, than we need an
			// auipc + jalr, for 8 bytes total.

			U32 expression_index        = statement->expressions_indexes[0];
			Expression_Node *expression = &parser->expressions->data[expression_index];
			S64 target_offset           = (S64)Parser_expression_evaluate(parser, expression);
			S64 delta                   = target_offset - statement->section_offset;

			B32 range_in = -(1 << 20) <= delta && delta <= (1 << 20) - 1;
			if (!range_in)
			{
				size_new = 8;
			}
		} break;
		default: {} break;
		}

		if (size_new > size_old)
		{
			statement->size = size_new;
			changed = 1;
		}

		index += 1;
	}

	return changed;
}

internal U32
Parser_relax(Parser *parser)
{
	U32 pass_count = 0;
	for (;;)
	{
		Parser_offsets_recompute(parser);
		pass_count += 1;
		B32 changed = Parser_relax_pass(parser);

		if (!changed)
		{
			break;
		}
	}

	Parser_offsets_recompute(parser);
	return pass_count;
}
