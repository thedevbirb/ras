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
Parser_instruction_I_load_parse(Parser *parser, Instruction_Kind instruction_kind)
{
	// Format: instruction rd, offset(rs1). But offset, is optional, and if that'string the case also the parenthesis
	// are.
	Parser_advance(parser);
	U8 register_destination = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_register(parser);

	Expression_Node *expression = 0;

	if (parser->token_current.kind == Token_Kind__Left_Parenthesis)
	{
		// Case: instruction rd, (rs1)
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);
		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Right_Parenthesis, Parser_Error_Kind__Parenthesis_Right_Expected);
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
		expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

		Parser_expect_token(parser, Token_Kind__Left_Parenthesis, Parser_Error_Kind__Parenthesis_Left_Expected);
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Right_Parenthesis, Parser_Error_Kind__Parenthesis_Right_Expected);

		Parser_advance(parser);
	}


	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = expression ? &expression->index : 0;
	statement->expressions_count    = expression != 0;
	statement->instruction_kind     = instruction_kind;
	statement->instruction_format   = Instruction_Format__I;
	statement->register_source_1    = register_source_1;
	statement->register_destination = register_destination;
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
	// Format: instruction rs2, offset(rs1). But offset, is optional, and if that'string the case also the parenthesis
	// are.
	Parser_advance(parser);
	U8 register_source_2 = Parser_expect_register(parser);

	Parser_advance(parser);
	Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

	Parser_advance(parser);
	U8 register_source_1 = Parser_register(parser);

	Expression_Node *expression = 0;

	if (parser->token_current.kind == Token_Kind__Left_Parenthesis)
	{
		// Case: instruction rs2, (rs1)
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);
		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Right_Parenthesis, Parser_Error_Kind__Parenthesis_Right_Expected);
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
		expression = Parser_expression_parse(parser, Expression_Flags__Deferred);

		Parser_expect_token(parser, Token_Kind__Left_Parenthesis, Parser_Error_Kind__Parenthesis_Left_Expected);
		Parser_advance(parser);
		register_source_1 = Parser_expect_register(parser);

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Right_Parenthesis, Parser_Error_Kind__Parenthesis_Right_Expected);

		Parser_advance(parser);
	}


	Statement *statement = Parser_statement_instruction_create(parser);

	statement->expressions_indexes  = expression ? &expression->index : 0;
	statement->expressions_count    = expression != 0;
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
	// Always auipc + addi (8 bytes) at assembly time. The linker may relax this further to a single gp-relative
	// addi, but the assembler cannot know that.
	statement->size                 = 8;
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

internal void
Parser_instruction_ecall_parse(Parser *parser)
{
	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind   = Instruction_Kind__ECALL;
	statement->instruction_format = Instruction_Format__I;
}

internal void
Parser_instruction_ebreak_parse(Parser *parser)
{
	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind   = Instruction_Kind__EBREAK;
	statement->instruction_format = Instruction_Format__I;
}

internal void
Parser_instruction_pause_parse(Parser *parser)
{
	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind   = Instruction_Kind__PAUSE;
	statement->instruction_format = Instruction_Format__I;
}

internal void
Parser_instruction_fence_tso_parse(Parser *parser)
{
	Parser_advance(parser);

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind   = Instruction_Kind__FENCE_TSO;
	statement->instruction_format = Instruction_Format__I;
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

	Statement *statement = Parser_statement_instruction_create(parser);
	statement->instruction_kind   = Instruction_Kind__FENCE;
	statement->instruction_format = Instruction_Format__I;
	// Pack predecessor and successor into the two source register fields,
	// since fence does not use registers. 4 bits each.
	statement->register_source_1  = predecessor;
	statement->register_source_2  = successor;
}
