internal void
Parser_instruction_I_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

internal void
Parser_instruction_I_load_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	// Format: instruction rd, offset(rs1). But offset, is optional, and if that'string the case also the parenthesis
	// are.
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_register(parser);

	Expression_Node *expression = 0;

	if (parser->token_current.kind == Token_Kind__Parenthesis_Left)
	{
		// Case: instruction rd, (rs1)
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);
		parser->statement_context->register_source_1 = register_source_1;
		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Parenthesis_Right_Expected);
		Parser_advance(parser);
	}
	else if (register_source_1 != register_invalid)
	{
		// Case: instruction, rd, rs1
		Parser_advance(parser);
	}
	else
	{
		// Case: instruction rd, offset(rs1)
		expression = Parser_expression_parse(parser);

		Parser_expect_token(parser, Token_Kind__Parenthesis_Left, Parser_Error_Kind__Parenthesis_Left_Expected);
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);
		parser->statement_context->register_source_1 = register_source_1;

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Parenthesis_Right_Expected);

		Parser_advance(parser);
	}

	parser->statement_context->expressions_indexes  = expression ? &expression->index : 0;
}

internal void
Parser_instruction_R_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__R;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);
	parser->statement_context->register_source_2 = register_source_2;

	Parser_advance(parser);

	// Extra case: ADD supports an optional tprel_add relocation operator.
	B32 extra_expression = parser->token_current.kind == Token_Kind__Comma;
	if (instruction_kind == Instruction_Kind__ADD && extra_expression)
	{
		Parser_advance(parser);
		Expression_Node *expression = Parser_expression_parse(parser);
		parser->statement_context->expressions_indexes = &expression->index;
	}
}

// Generic parser for R-type pseudos with two operands: neg, negw, snez, sltz, sgtz
// Syntax: mnemonic rd, rs
internal void
Parser_instruction_R_pseudo_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__R;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source;

	Parser_advance(parser);
}

internal void
Parser_instruction_S_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__S;

	// Format: instruction rs2, offset(rs1). But offset, is optional, and if that's the case also the parenthesis
	// are.
	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);
	parser->statement_context->register_source_2 = register_source_2;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_register(parser);

	Expression_Node *expression = 0;

	if (parser->token_current.kind == Token_Kind__Parenthesis_Left)
	{
		// Case: instruction rs2, (rs1)
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);
		parser->statement_context->register_source_1 = register_source_1;
		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Parenthesis_Right_Expected);
		Parser_advance(parser);
	}
	else if (register_source_1 != register_invalid)
	{
		// Case: instruction, rs2, rs1
		Parser_advance(parser);
	}
	else
	{
		// Case: instruction rs2, offset(rs1)
		expression = Parser_expression_parse(parser);

		Parser_expect_token(parser, Token_Kind__Parenthesis_Left, Parser_Error_Kind__Parenthesis_Left_Expected);
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);
		parser->statement_context->register_source_1 = register_source_1;

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Parenthesis_Right_Expected);

		Parser_advance(parser);
	}


	parser->statement_context->expressions_indexes  = expression ? &expression->index : 0;
}

internal void
Parser_instruction_B_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__B;

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);
	parser->statement_context->register_source_2 = register_source_2;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

// Generic parser for branch pseudos with one register: beqz, bnez, bltz, bgez, blez, bgtz
// Syntax: mnemonic rs, offset
internal void
Parser_instruction_B_pseudo_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__B;

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}


internal void
Parser_instruction_U_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	parser->statement_context->instruction_kind     = instruction_kind;
	parser->statement_context->instruction_format   = Instruction_Format__U;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

internal void
Parser_instruction_jal_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__JAL;
	parser->statement_context->instruction_format   = Instruction_Format__J;

	// Two possible forms: `jal rd, offset` or `jal offset`, where `rd` is implicitly `ra`.

	Parser_advance(parser);
	Token *next = Parser_peek_next(parser);
	if (next->kind == Token_Kind__Comma)
	{
		// `jal rd, offset`
		Parser_expect_register(parser);
		Parser_advance(parser);
		// Comma already checked
		Parser_advance(parser);
	}
	else
	{
		parser->statement_context->flags |= Statement_Flags__JAL_Register_Destination_Unset;
	}

	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

// call symbol -> auipc ra, %pcrel_hi(symbol), + jalr ra, ra, %pcrel_lo(symbol)
internal void
Parser_instruction_call_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__CALL;
	parser->statement_context->instruction_format   = Instruction_Format__J;

	// Two possible forms: `call rd, offset` or `call offset`, where `rd` is implicitly `ra`.

	Parser_advance(parser);
	Token *next = Parser_peek_next(parser);
	if (next->kind == Token_Kind__Comma)
	{
		// `call rd, offset`
		Parser_expect_register(parser);
		Parser_advance(parser);
		// Comma already checked
		Parser_advance(parser);
	}
	else
	{
		parser->statement_context->flags |= Statement_Flags__CALL_Register_Destination_Unset;
	}

	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

// Pseudo-instructions section.
//
// These instruction are parsed as is, meaning that Instruction_Kind is not converted and no fake is injected. Later
// stages, close to encoding, will effectively treat them differently.

// mv rd, rs -> addi rd, rs, 0
internal void
Parser_instruction_mv_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__MV;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
}

// not rd, rs -> xori rd, rs, -1
internal void
Parser_instruction_not_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__NOT;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
}

// sext.w rd, rs -> addiw rd, rs, 0 (RV64)
internal void
Parser_instruction_sext_w_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__SEXT_W;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
}

// seqz rd, rs -> sltiu rd, rs, 1
internal void
Parser_instruction_seqz_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__SEQZ;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
}

// j offset -> jal x0, offset
internal void
Parser_instruction_j_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__J;
	parser->statement_context->instruction_format   = Instruction_Format__J;

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

// jr rs -> jalr x0, rs, 0
internal void
Parser_instruction_jr_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__JR;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 register_source_1 = Parser_expect_register(parser);
	parser->statement_context->register_source_1 = register_source_1;

	Parser_advance(parser);
}

// jalr rd, rs1, offset  (three-operand form)
// jalr rs               (single-operand pseudo: jalr ra, rs, 0)
internal void
Parser_instruction_jalr_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__JALR;
	parser->statement_context->instruction_format   = Instruction_Format__I;

	Parser_advance(parser);
	U8 first_register = Parser_expect_register(parser);
	Parser_advance(parser);

	if (parser->token_current.kind == Token_Kind__Comma)
	{
		// Three-operand form: jalr rd, rs1, offset
		parser->statement_context->register_destination  = first_register;

		Parser_advance(parser);
		U8 register_source_1 = Parser_expect_register(parser);
		parser->statement_context->register_source_1 = register_source_1;

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

		Parser_advance(parser);
		Expression_Node *expression = Parser_expression_parse(parser);
		parser->statement_context->expressions_indexes = &expression->index;
	}
	else
	{
		// Single-operand pseudo: jalr rs -> jalr ra, rs, 0
		parser->statement_context->instruction_kind     = Instruction_Kind__JALR;
		parser->statement_context->register_destination = 1; // ra
		parser->statement_context->register_source_1    = first_register;
	}
}

// li rd, imm -> lui rd, %hi(imm) + addi rd, rd, %lo(imm)
// NOTE: For small immediates that fit in 12 bits, a single addi suffices.
//       The expansion decision may be deferred to a later pass.
internal void
Parser_instruction_li_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__LI;
	parser->statement_context->instruction_format   = Instruction_Format__Expandable;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

// la rd, symbol -> auipc rd, %pcrel_hi(symbol) + addi rd, rd, %pcrel_lo(symbol)
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_la_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__LA;
	parser->statement_context->instruction_format   = Instruction_Format__Expandable;

	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);
	parser->statement_context->register_destination = register_destination;

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
	// Always auipc + addi (8 bytes) at assembly time. The linker may relax this further to a single gp-relative
	// addi, but the assembler cannot know that.
	parser->statement_context->size                 = 8;
}

// tail offset -> auipc t1, offsetHi + jalr x0, t1, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_tail_parse(Parser *parser)
{
	parser->statement_context->instruction_kind     = Instruction_Kind__TAIL;
	parser->statement_context->instruction_format   = Instruction_Format__Expandable;

	Parser_advance(parser);
	Expression_Node *expression = Parser_expression_parse(parser);
	parser->statement_context->expressions_indexes  = &expression->index;
}

internal void
Parser_instruction_mnemonic_only_parse(Parser *parser, Instruction_Kind instruction_kind)
{

	parser->statement_context->instruction_kind   = instruction_kind;
	parser->statement_context->instruction_format = Instruction_Format__I;

	Parser_advance(parser);
}

// Parses a fence ordering operand: a string composed of the characters i, o, r, w
// in that order. Returns a 4-bit mask: i=bit3, o=bit2, r=bit1, w=bit0.
internal U8
Parser_expect_fence_operand(Parser *parser)
{
	String8 string = Parser_token_string(parser);
	U8 mask = 0;

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= string.count || parser->error.kind;
		if (break_should)
		{
			break;
		}

		switch (string.data[index])
		{
		case 'i': { mask |= (1 << 3); } break;
		case 'o': { mask |= (1 << 2); } break;
		case 'r': { mask |= (1 << 1); } break;
		case 'w': { mask |= (1 << 0); } break;
		default: { Parser_error_set(parser, Parser_Error_Kind__Fence_Operand_Invalid); } break;
		}
	}
	return mask;
}

// fence pred, succ   -> e.g. fence iorw, iorw
// fence              -> shorthand for fence iorw, iorw
internal void
Parser_instruction_fence_parse(Parser *parser)
{
	parser->statement_context->instruction_kind   = Instruction_Kind__FENCE;
	parser->statement_context->instruction_format = Instruction_Format__I;

	Parser_advance(parser);

	U8 predecessor = 0xF; // default iorw
	U8 successor   = 0xF; // default iorw

	B32 end_of_statement = parser->end_reached || parser->token_current.kind == Token_Kind__Newline;
	if (end_of_statement)
	{
		predecessor = Parser_expect_fence_operand(parser);
		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);
		Parser_advance(parser);
		successor = Parser_expect_fence_operand(parser);
		Parser_advance(parser);
	}

	// Pack predecessor and successor into the two source register fields,
	// since fence does not use registers. 4 bits each.
	parser->statement_context->register_source_1  = predecessor;
	parser->statement_context->register_source_2  = successor;
}
