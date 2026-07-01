// NOTE: most of this is riscv-backend exclusive, so it should move somewhere accordingly.


// // Create a label string for the dot symbol, in the format `.D{index}`, where `index` is the current statement index.
// //
// // TODO(medium): this may not be needed, since every statements brings its own section offset.
// internal String8
// Parser_label_dot(Parser *parser)
// {
// 	U32 statement_index = parser->statements->count;
// 	U32 digits_count = number_to_digits_count(statement_index);
// 	U64 string_characters_count  = digits_count + 2;
// 	String8 key =
// 	{
// 		// .D{statement_index}
// 		.data = Arena__push_array_m(parser->arena, U8, string_characters_count),
// 		.count = string_characters_count,
// 	};
//
// 	key.data[0] = '.';
// 	key.data[1] = 'D';
// 	// TODO(low): I mean, snprintf is quite overkill for this.
// 	snprintf((char *)(key.data + 2), digits_count, "%d", statement_index);
//
// 	return key;
// }
//
// // Format: .L<number>^B<occurrences>, compliant to GNU as. ^B is 0x02.
// internal String8
// Parser_label_numeric_symbol_string(Parser *parser, U8 label_numeric)
// {
// 	assert_always_m(label_numeric < label_numeric_max);
// 	U16 occurrences = parser->symbols_table->label_numeric_count[label_numeric];
// 	U32 digits_count = number_to_digits_count(occurrences);
// 	U8 string_characters_count = 4 + digits_count;
// 	String8 key =
// 	{
// 		.data = Arena__push_array_m(parser->arena, U8, string_characters_count),
// 		.count = string_characters_count,
// 	};
//
// 	key.data[0] = '.';
// 	key.data[1] = '.';
// 	key.data[2] = '0' + label_numeric;
// 	key.data[3] = 0x02;
// 	// TODO(low): I mean, snprintf is quite overkill for this.
// 	snprintf((char *)(key.data + 4), digits_count, "%d", occurrences);
//
// 	return key;
// }

// It is a no-op if the end has been reached already.
// NOTE: this could be dropped, along with `token_current` etc. but we keep it for now for compatibility with existing
// code.
// internal void
// Parser_advance(Parser_2 *parser)
// {
// 	Token_2 token = Lexer_lex(parser->lexer);
// 	if (parser->lexer->error.kind)
// 	{
// 		Diagnostic *diagnostic = Parser__push_diagnostic_from_index(parser, parser->lexer->error.index);
// 		diagnostic->message = lexer_error_kind_messages[parser->lexer->error.kind];
// 	}
// 	parser->end_reached       = token.kind == 0;
// 	parser->token_current     = token;
//
// 	return;
// }
//
// internal Token *
// Parser_peek_next(Parser *parser)
// {
// 	Token *next = &parser->tokens[parser->token_index + 1];
// 	return next;
// }
//
// internal String8
// Parser_token_string(Parser *parser)
// {
// 	String8 string =
// 	{
// 		.data  = parser->input->data + parser->token_current.index,
// 		.count = (U64)parser->token_current.size,
// 	};
// 	return string;
// }
//
// internal void
// Parser_error_set(Parser *parser, Parser_Error_Kind kind)
// {
// 	Parser_Error error =
// 	{
// 		.kind               = kind,
// 		.row_index          = parser->token_current.row_index,
// 		.column_index_begin = parser->token_current.column_index,
// 		.column_index_end   = parser->token_current.column_index + parser->token_current.size - 1,
// 	};
//
// 	assert_always_m(error.column_index_begin <= error.column_index_end && "token_index bug");
// 	parser->error = error;
//
// #ifdef ASSEMBLER_EXPECT_PANIC
// 	assert_always_m(0 && "panic on expect");
// #endif
//
// 	return;
// }
//
// internal void
// Parser_expect(Parser_2 *parser, B32 condition, Parser_Error_Kind error_kind, U64 begin_index, U32 size)
// {
// 	// if (!condition && !(parser->error))
// 	// {
// 	// 	Parser_error_set(parser, error_kind);
// 	// }
// 	return;
// }
//
// internal void
// Parser_expect_token(Parser *parser, Token_Kind token_kind, Parser_Error_Kind error_kind)
// {
// 	B32 condition = parser->token_current.kind == token_kind;
// 	Parser_expect(parser, condition, error_kind);
// 	return;
// }
//
// internal U8
// Parser_register(Parser *parser)
// {
// 	String8 string = Parser_token_string(parser);
// 	U8 register_value = register_lookup(string);
// 	return register_value;
// }
//
// internal U8
// Parser_expect_register(Parser *parser)
// {
// 	String8 string = Parser_token_string(parser);
// 	U8 register_value = register_lookup(string);
// 	Parser_expect(parser, register_value != register_invalid, Parser_Error_Kind__Register_Invalid);
// 	return register_value;
// }
//
// internal Symbols_Table_Entry *
// Parser_symbol_declare(Parser *parser, String8 key)
// {
// 	Symbols_Table_Entry *entry = Symbols_Table_reserve(parser->symbols_table, key);
// 	entry->index_statement = parser->statements->count;
// 	entry->flags |= Symbol_Flags__Declared;
//
// 	return entry;
// }
//
// // A special note goes to symbols inside an expression. While absolute symbols, created with .set or .equ
// // directives are always accepted, others must be present only instruction which accept immediates.
// //
// // If met in other places, we should emit an error.
// internal void
// Parser_symbol_ensure_context_valid(Parser *parser)
// {
// 	Directive_Kind directive_kind = parser->statement_context->directive_kind;
// 	B32 directive_kind_valid = directive_kind == Directive_Kind__Byte
// 		                || directive_kind == Directive_Kind__Word_Half
// 		                || directive_kind == Directive_Kind__Word
// 		                || directive_kind == Directive_Kind__Word_Double
// 			        || directive_kind == Directive_Kind__Set
// 			        || directive_kind == Directive_Kind__Equality;
//
// 	Instruction_Format instruction_format = parser->statement_context->instruction_format;
// 	B32 instruction_format_valid = instruction_format == Instruction_Format__B
// 		                    || instruction_format == Instruction_Format__I
// 		                    || instruction_format == Instruction_Format__U
// 		                    || instruction_format == Instruction_Format__J
// 		                    || instruction_format == Instruction_Format__S
// 		                    || instruction_format == Instruction_Format__Expandable;
//
// 	B32 relocation_operator_present = parser->statement_context->relocation_operator != 0;
//
// 	B32 context_valid = directive_kind_valid
// 		         || instruction_format_valid
// 			 || relocation_operator_present;
//
// 	Parser_expect(parser, context_valid, Parser_Error_Kind__Symbol_Context_Invalid);
// 	return;
// }
//
// // A relocation operator can appear only once in an expression, and %hi can only work where there is a 20-bit immediate
// // like lui e auipc while %lo which takes lower 12 bits works elsewhere.
// // A relocation operator can be combined only with constant arithmetic, in the symbol + addend fashion.
//
// internal Expression_Node *
// Parser_parse_null_denotation(Parser *parser)
// {
//
// 	Expression_Node *node  = Expressions_push_empty(parser->expressions);
// 	node->token_index      = parser->token_index;
// 	Token token            = parser->token_current;
//
// 	switch (token.kind)
// 	{
// 	case Token_Kind__Number:
// 	{
// 		node->kind          = Expression_Kind__Number_Literal;
// 		node->integer_value = token.numerical_value;
//
// 		Parser_advance(parser);
// 	} break;
//
// 	case Token_Kind__Char_Literal:
// 	{
// 		node->kind          = Expression_Kind__Char_Literal;
// 		node->integer_value = token.numerical_value;
//
// 		Parser_advance(parser);
// 	} break;
//
// 	case Token_Kind__Dot:
// 	{
// 		node->kind = Expression_Kind__Identifier;
//
// 		String8 key = Parser_label_dot(parser);
// 		Symbols_Table_Entry *symbol = Symbols_Table_reserve(parser->symbols_table, key);
//
// 		symbol->elf.section_index = parser->statement_context->section_index;
// 		symbol->elf.type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Local, 0);
//
// 		node->symbols_table_entry = symbol;
// 		// Parser_symbol_ensure_context_valid(parser, symbol);
//
// 		Parser_advance(parser);
// 	} break;
//
// 	case Token_Kind__Identifier:
// 	{
// 		node->kind = Expression_Kind__Identifier;
//
// 		String8 key = Parser_token_string(parser);
// 		Symbols_Table_Entry *symbol = Symbols_Table_reserve(parser->symbols_table, key);
// 		// Parser_symbol_ensure_context_valid(parser, symbol);
//
// 		node->symbols_table_entry = symbol;
//
// 		Parser_advance(parser);
// 	} break;
//
// 	case Token_Kind__Label_Numeric_Reference_Forward:
// 	{
// 		Parser_expect(parser, parser->token_current.numerical_value < label_numeric_max, Parser_Error_Kind__Label_Numeric_Large);
//
// 		node->kind                = Expression_Kind__Label_Numeric_Reference_Forward;
// 		node->label_numeric_value = token.numerical_value;
//
// 		// NOTE: the symbol of this expression will be filled during evaluation.
//
// 		Parser_advance(parser);
// 	} break;
// 	case Token_Kind__Label_Numeric_Reference_Backward:
// 	{
// 		Parser_expect(parser, parser->token_current.numerical_value < label_numeric_max, Parser_Error_Kind__Label_Numeric_Large);
//
// 		node->kind                = Expression_Kind__Label_Numeric_Reference_Backward;
// 		node->label_numeric_value = token.numerical_value;
//
// 		// NOTE: the symbol of this expression will be filled during evaluation.
//
// 		Parser_advance(parser);
// 	} break;
//
// 	case Token_Kind__Minus:
// 	case Token_Kind__Tilde:
// 	case Token_Kind__Bang:
// 	{
// 		node->kind = Expression_Kind_from_unary_Token_Kind(token.kind);
// 		Parser_advance(parser);
// 		Expression_Node *operand = Parser_expression_parse_inner(parser, Binding_Power__Unary);
// 		node->index_left = operand->index;
// 	} break;
//
// 	case Token_Kind__Relocation_Prefix:
// 	{
// 		Statement *statement = parser->statement_context;
// 		Parser_expect(parser, statement->kind == Statement_Kind__Instruction, Parser_Error_Kind__Relocation_Instruction_Missing);
// 		B32 supported = Instruction_Encoding_table[statement->instruction_kind].flags & Instruction_Flags__Relocation_Operator;
// 		Parser_expect(parser, supported, Parser_Error_Kind__Relocation_Operator_Invalid);
//
// 		Parser_advance(parser);
// 		Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);
// 		String8 relocation_operator_string      = Parser_token_string(parser);
// 		Relocation_Operator relocation_operator = Relocation_Operator_lookup(relocation_operator_string);
// 		Parser_expect(parser, relocation_operator, Parser_Error_Kind__Relocation_Operator_Invalid);
// 		Parser_expect(parser, !parser->statement_context->relocation_operator, Parser_Error_Kind__Relocation_Operator_Multiple);
//
// 		B32 relocation_operator_valid = 0;
// 		switch (relocation_operator)
// 		{
// 		case Relocation_Operator__hi:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__LUI;
// 		} break;
// 		case Relocation_Operator__lo:
// 		{
// 			relocation_operator_valid = statement->instruction_format == Instruction_Format__I
// 				                 || statement->instruction_format == Instruction_Format__S;
// 		} break;
// 		case Relocation_Operator__pcrel_hi:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
// 		} break;
// 		case Relocation_Operator__pcrel_lo:
// 		{
// 			relocation_operator_valid = statement->instruction_format == Instruction_Format__I
// 						 || statement->instruction_format == Instruction_Format__S;
// 		} break;
// 		case Relocation_Operator__tprel_hi:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC
// 						 || statement->instruction_kind == Instruction_Kind__LUI;
// 		} break;
// 		case Relocation_Operator__tprel_lo:
// 		{
// 			relocation_operator_valid = statement->instruction_format == Instruction_Format__I
// 						 || statement->instruction_format == Instruction_Format__S;
// 		} break;
// 		case Relocation_Operator__tprel_add:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__ADD
// 				                && (statement->register_source_1 == register_tp
// 						||  statement->register_source_2 == register_tp);
// 		} break;
// 		case Relocation_Operator__got_pcrel_hi:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
// 		} break;
// 		case Relocation_Operator__tls_ie_pcrel_hi:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
// 		} break;
// 		case Relocation_Operator__tls_gd_pcrel_hi:
// 		{
// 			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
// 		} break;
// 		case Relocation_Operator__None:  {} break;
// 		case Relocation_Operator__COUNT: { unreachable_m(); } break;
// 		}
// 		Parser_expect(parser, !supported || relocation_operator_valid, Parser_Error_Kind__Relocation_Instruction_Invalid);
//
// 		node->relocation_operator = relocation_operator;
// 		parser->statement_context->relocation_operator = relocation_operator;
//
// 		Parser_advance(parser);
// 		Parser_expect_token(parser, Token_Kind__Parenthesis_Left, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);
//
// 		Parser_advance(parser);
// 		Expression_Node *inner = Parser_expression_parse_inner(parser, Binding_Power__None);
// 		Symbols_Table_Entry *symbol = inner->symbols_table_entry;
// 		Parser_expect(parser, symbol != 0, Parser_Error_Kind__Relocation_Symbol_Missing);
// 		// TODO: do context verification of symbol during evaluation, or at later stage, when all symbols are known.
// 		// More specifically:
// 		// %hi / %lo — symbol must have an absolute address in some section
// 		// %pcrel_hi / %pcrel_lo — symbol must be in a section (PC-relative offset is meaningless for an absolute value)
// 		// %got_pcrel_hi — symbol must have a GOT entry, so it must be a real symbol
// 		// %tprel_hi / %tprel_lo / %tprel_add — symbol must be in .tdata or .tbss (TLS sections)
// 		// %tls_ie_pcrel_hi — symbol must be a TLS symbol
// 		// %tls_gd_pcrel_hi — symbol must be a TLS symbol
// 		//
// 		// This may be correct/desiderable but in practice gas doesn't enforce almost anything.
// 		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);
// 		// TODO: %pcrel_lo addi, loads, storesI / Srs1 = rd of auipclabel of auipc. gas doesn't check it.
//
// 		node->kind       = Expression_Kind__Relocation;
// 		node->index_left = inner->index;
//
// 		Parser_advance(parser);
//
// 	} break;
//
// 	case Token_Kind__Parenthesis_Left:
// 	{
// 		Parser_advance(parser);
// 		Expression_Node *inner = Parser_expression_parse_inner(parser, Binding_Power__None);
//
// 		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Expression_Parenthesis_Right_Expected);
// 		Parser_advance(parser);
//
// 		node = inner;
// 	} break;
//
// 	default:
// 	{
// 		Parser_error_set(parser, Parser_Error_Kind__Expression_Unexpected_Token);
// 	} break;
// 	}
//
// 	return node;
// }
//
//
// // Consider evaluating already the constants.
//
// // Core Pratt parser loop. Parses an expression where all binary operators
// // must have binding power strictly greater than binding_power_minimum.
// // All operators are left-associative (the <= comparison ensures this).
// internal Expression_Node *
// Parser_expression_parse_inner(Parser *parser, Binding_Power binding_power_minimum)
// {
// 	local_persist U8 recursion_level = 0;
// 	recursion_level += 1;
//
// 	Expression_Node *left = 0;
// 	Parser_expect(parser, !parser->end_reached, Parser_Error_Kind__Expression_Unexpected_End);
// 	Parser_expect(parser, recursion_level <= 8, Parser_Error_Kind__Expression_Recursion_Max);
//
// 	left = Parser_parse_null_denotation(parser);
// 	for (;;)
// 	{
// 		Token_Kind operator_kind = parser->token_current.kind;
// 		Binding_Power next_power = Binding_Power_from_Token_Kind(operator_kind);
//
// 		B32 break_should = next_power <= binding_power_minimum || parser->end_reached || parser->error.kind;
// 		if (break_should)
// 		{
// 			break;
// 		}
//
// 		// Important to advance _after_ we have the right binding power otherwise we might tokens subsequent to
// 		// the expression, like commas.
// 		Parser_advance(parser);
//
// 		Expression_Node *right = Parser_expression_parse_inner(parser, next_power);
// 		// Expression_Node *node  = Arena__push_struct_m(parser->arena, Expression_Node);
// 		Expression_Node *node  = Expressions_push_empty(parser->expressions);
//
// 		node->kind        = Expression_Kind__binary_from_Token_Kind(operator_kind);
// 		node->index_left  = left->index;
// 		node->index_right = right->index;
//
// 		assert_always_m(node->kind);
//
// 		left = node;
// 	}
//
// 	recursion_level -= 1;
// 	return left;
// }

// NOTE: parsing an expression right now mixes machine-dependent and independent code. It would be nice to provide a
// common ground for it if it makes sense.
internal Expression_Node *
expression_parse
(
	Arena              *arena,
	Token_Cursor       *cursor,
	Expressions        *expressions,
	Symbols_Table      *symbols_table,
	Diagnostic_List    *diagnostics
)
{
	// A stack of expression frames. Each sub-expression creates a frame associated to it.
	// When the sub-expression is parsed, the frame should be popped.
	//
	// A frame should have a node attached. An exception to this is when a new frame is created due to a left
	// parenthesis, because it defers creating such node.
	typedef struct Frame Frame;
	struct Frame
	{
		Frame            *next;
		Expression_Node  *node;
		Binding_Power     binding_power_minimum;
		B32               is_right_side_of_next;
		B32               null_denotation_parsed;

	};

	typedef struct Parenthesis_Frame Parenthesis_Frame;
	struct Parenthesis_Frame
	{
		Parenthesis_Frame *next;
		U32 location;
	};


	Arena_Temporary scratch = Arena__scratch_begin_m(&arena, 1);

	Diagnostic *error = 0;

	Frame *frame = Arena__push_struct_m(scratch.arena, Frame);
	frame->node  = Expressions_push_empty(expressions, arena);
	frame->node->location = cursor->current.location;

	// ZII node as initial result;
	Expression_Node *result = frame->node;

	Parenthesis_Frame *parenthesis_frame = 0;

	for (;;)
	{
		// Iterate until we pop the last frame.

		// Start by reading a null-denotation. We mark we've done this process be setting
		// `frame->null_denotation_parsed = 1`.
		//
		// Every branch advances the current token since it has its own custom logic.
		switch (cursor->current.kind)
		{
		case Token_Kind__Number:
		{
			frame->node->kind          = Expression_Kind__Constant;
			frame->node->evaluation    = Expression_Kind__Constant;
			frame->node->integer_value = cursor->current.numerical_value;
			frame->null_denotation_parsed = 1;

			token_next(cursor, diagnostics, arena);
		} break;

		case Token_Kind__Identifier:
		{
			String8 name        = Token_Cursor__text(cursor);
			Symbol_Ref *symbol  = Symbols_Table__get_or_default(symbols_table, name);

			frame->node->kind             = Expression_Kind__Symbol;
			frame->node->evaluation       = Expression_Kind__Symbol;
			frame->node->symbol           = symbol;
			frame->null_denotation_parsed = 1;

			token_next(cursor, diagnostics, arena);
		} break;

		case Token_Kind__Percentage:
		case Token_Kind__Minus:
		case Token_Kind__Tilde:
		case Token_Kind__Bang:
		{
			// unary_operator <expression>
			frame->node->kind = Expression_Kind_from_unary_Token_Kind(cursor->current.kind);
			frame->null_denotation_parsed = 1;

			Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
			frame_new->node = Expressions_push_empty(expressions, arena);
			frame_new->node->location = cursor->current.location;
			frame_new->binding_power_minimum = Binding_Power__Unary;
			frame_new->is_right_side_of_next = 1;

			token_next(cursor, diagnostics, arena);

			SLL_stack_push_m(frame, frame_new);
			continue;
		} break;

		case Token_Kind__Parenthesis_Left:
		{
			// ( <expression> )
			Parenthesis_Frame *parenthesis_frame_new = Arena__push_struct_m(scratch.arena, Parenthesis_Frame);
			parenthesis_frame_new->location = cursor->current.location;
			SLL_stack_push_m(parenthesis_frame, parenthesis_frame_new);

			token_next(cursor, diagnostics, arena);
			continue;
		} break;

		default:
		{
			if (!frame->null_denotation_parsed)
			{
				// Don't pollute codepaths to exit: mark an empty node, which is safe, and mark the error with
				// its diagnostic.
				frame->node = Expressions_push_empty(expressions, arena);
				frame->null_denotation_parsed = 1;
				frame->node->location = cursor->current.location;

				error = Arena__push_struct_m(arena, Diagnostic);
				error->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Expression_Null_Denotation_Expected];
				error->location = cursor->current.location;
				error->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
				SLL_queue_push_m(diagnostics->first, diagnostics->last, error);

				token_next(cursor, diagnostics, arena);
			}
		} break;
		}

		// Don't check binding power of a ')', handle it and then check what's next.
		if (cursor->current.kind == Token_Kind__Parenthesis_Right)
		{
			if (parenthesis_frame)
			{
				SLL_stack_pop_m(parenthesis_frame);
			}
			else
			{
				error = Arena__push_struct_m(arena, Diagnostic);
				error->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Expression_Parenthesis_Right_Unmatching];
				error->location = cursor->current.location;
				SLL_queue_push_m(diagnostics->first, diagnostics->last, error);
			}
			token_next(cursor, diagnostics, arena);
		}


		// The `binding_power_minimum` is used to describe precedence. Operator tokens have an associated power
		// that is transferred to the next null denotation to preserve context.
		//
		// Consider the example `4 + 3 * 5`. When 3 is parsed, we want to remember that it is currently the
		// right side of an additive operation. When we peek the next token, that will be a star, that will have
		// an associated multiplicative power, which is higher. This means 3 should be "absorbed" i.e.,
		// considered the left node of start. As such, the tree should look as follows.
		//
		//               +
		//             4   *
		//                3 5
		//
		// If we consider `4 + 3 - 5` instead, now the minus sign has the same additive power, which means that
		// `4 + 3` concludes an expression, and the current expression frame can be popped.
		//
		// Unary operators have the highest binding power so that they mark the end of an expression
		// immediately. Consider `-4 + 3`, when the plus sign is read the minimum binding power would be unary,
		// so we know that the expression is completed.
		//
		// From this, we can understand that parenthesis are simply tokens with zero binding power, used to
		// conclude expressions. If we had `(4 + 3) * 5`, reading the right parenthesis after 3, where the
		// former has zero binding power, would conclude reading the expression `4 + 3`.

		Binding_Power next_power = Binding_Power_from_Token_Kind(cursor->current.kind);
		B32 pop = next_power <= frame->binding_power_minimum || cursor->source_index >= cursor->source->count;
		if (pop)
		{
			if (frame->is_right_side_of_next)
			{
				assert_always_m(frame->next);
				frame->next->node->index_right = frame->node->index;
			}

			if (!frame->next || error)
			{
				// Save the result before popping the last frame.
				result = frame->node;
			}
			SLL_stack_pop_n_m(frame, next);
		}
		else
		{
			// <expression> binary_operator <expression>
			Expression_Node *left = frame->node;

			// Set central node.
			frame->node = Expressions_push_empty(expressions, arena);
			frame->node->kind = Expression_Kind__binary_from_Token_Kind(cursor->current.kind);
			frame->node->index_left = left->index;

			token_next(cursor, diagnostics, arena);

			// Prepare new frame
			Frame *frame_new = Arena__push_struct_m(scratch.arena, Frame);
			frame_new->node = Expressions_push_empty(expressions, arena);
			frame_new->node->location = cursor->current.location;
			frame_new->binding_power_minimum = next_power;
			frame_new->is_right_side_of_next = 1;
			SLL_stack_push_m(frame, frame_new);
		}

		B32 break_should = frame == 0;
		if (break_should)
		{
			break;
		}

	}

	if (parenthesis_frame)
	{
		error = Arena__push_struct_m(arena, Diagnostic);
		error->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Expression_Parenthesis_Left_Unclosed];
		error->location = parenthesis_frame->location;
		SLL_queue_push_m(diagnostics->first, diagnostics->last, error);
	}

	Arena_Temporary__end(scratch);
	return result;
}

// NOTE: both LLVM and GNU as have a precise way of handle relocation operators. They must appear at the beginning of
// the expression, and everything else is absorbed by it. Examples:
//
// - `addi x1, x0, %lo(foo) + 1` is equivalent to `addi x1, x0, %lo(foo + 1)`.
// - `addi x1, x0, 1 + %lo(foo)` is invalid.
internal Expression_Node *
expression_parse_with_relocation
(
	Arena               *arena,
	Token_Cursor        *cursor,
	Expressions         *expressions,
	Symbols_Table       *symbols_table,
	Diagnostic_List     *diagnostics,
	// Machine-dependent
	U16                 *relocation_out,
	// Zero-terminated.
	const Relocation_Operator *relocation_match
)
{

	if (cursor->current.kind == Token_Kind__Percentage)
	{
		assert_always_m(relocation_out && "relocation_out should be set");

		// Parse relocation
		token_next(cursor, diagnostics, arena);
		String8 text = Token_Cursor__text(cursor);

		B32 found = 0;
		for (;;)
		{
			B32 break_should = found || !relocation_match->relocation;
			if (break_should)
			{
				break;
			}
			found = String8__match_exact(relocation_match->text, text);
			relocation_match += 1;
		}

		if (!found)
		{
			Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
			diagnostic->location   = cursor->current.location;
			diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
			diagnostic->message    = String8__literal("invalid relocation operator for instruction");
			SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
		}

		*relocation_out = relocation_match->relocation;
		token_next(cursor, diagnostics, arena);
	}
	Expression_Node *result = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
	return result;
}


//
// Expression_Node *
// Parser_expression_immediate_create(Parser *parser, U64 immediate)
// {
// 	Expression_Node *node  = Expressions_push_empty(parser->expressions);
// 	node->integer_value = immediate;
// 	return node;
// }
//
// U32 *
// Parser_expression_sentinel_index(Parser *parser)
// {
// 	Expression_Node *expression = &parser->expressions->data[0];
// 	return &expression->index;
// }
//
//
// internal void
// Parser_statement_context_reset(Parser *parser)
// {
// 	*parser->statement_context = (Statement)
// 	{
// 		.token_index_begin = parser->token_index,
// 		.section_index = parser->section_current_index,
// 		.flags = parser->flags,
// 	};
//
// 	return;
// }
//
// void
// Parser_parse(Parser *parser)
// {
// 	U8  data_directive_size = 0;
// 	U32 string_size = 0;
// 	for (;;)
// 	{
// 		B32 break_should = parser->end_reached || parser->error.kind;
// 		if (break_should)
// 		{
// 			break;
// 		}
//
// 		parser->token_index_before = parser->token_index;
// 		Parser_statement_context_reset(parser);
//
// 		Token_Kind token_start_kind = parser->token_current.kind;
//
// 		switch (token_start_kind)
// 		{
// 		case Token_Kind__Newline: { Parser_advance(parser); } break;
// 		case Token_Kind__Label:
// 		{
// 			String8 key = Parser_token_string(parser);
// 			Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);
// 			B32 duplicate = entry->elf.section_index;
// 			Parser_expect(parser, !duplicate, Parser_Error_Kind__Label_Duplicate);
//
// 			entry->elf.section_index = parser->section_current_index,
//
// 			Parser_advance(parser);
// 			parser->statement_context->s_symbol = entry;
// 			parser->statement_context->kind     = Statement_Kind__Label;
// 		} break;
// 		case Token_Kind__Label_Numeric:
// 		{
// 			Parser_expect(parser, parser->token_current.numerical_value < label_numeric_max, Parser_Error_Kind__Label_Numeric_Large);
// 			U8 label_numeric_value = parser->token_current.numerical_value;
//
// 			String8 key = Parser_label_numeric_symbol_string(parser, label_numeric_value);
// 			Symbols_Table_Entry *entry = Symbols_Table_reserve(parser->symbols_table, key);
//
// 			entry->elf.section_index    = parser->section_current_index;
// 			entry->elf.type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Local, 0);
//
// 			Parser_advance(parser);
// 			parser->statement_context->s_symbol = entry;
// 			parser->statement_context->label_numeric_value = label_numeric_value;
// 			parser->statement_context->kind                = Statement_Kind__Label_Numeric;
// 		} break;
// 		case Token_Kind__Directive:
// 		{
// 			String8 string_directive = Parser_token_string(parser);
// 			Directive_Kind directive_kind = Directive_Kind__from_String8(string_directive);
//
//
// 			parser->statement_context->kind = Statement_Kind__Directive;
// 			parser->statement_context->directive_kind = directive_kind;
//
// 			switch (directive_kind)
// 			{
// 			case Directive_Kind__None:
// 			{
// 				Parser_error_set(parser, Parser_Error_Kind__Directive_Unknown);
// 			} break;
// 			case Directive_Kind__Word_Double: { data_directive_size += 4; } // fallthrough
// 			case Directive_Kind__Word:        { data_directive_size += 2; } // fallthrough
// 			case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
// 			case Directive_Kind__Byte:
// 			{
// 				data_directive_size += 1;
//
// 				// TODO: expand max number of expressions;
// 				U32 *expressions_indexes = Arena__push_array_m(parser->arena, U32, 16);
// 				U32 expressions_count = 0;
//
// 				// Format: .byte <expr_1> , ..., <expr_n>
// 				for (;;)
// 				{
// 					Parser_advance(parser);
// 					Expression_Node *expression = Parser_expression_parse(parser);
// 					expressions_indexes[expressions_count] = expression->index;
// 					expressions_count += 1;
// 					assert_always_m(expressions_count < 16);
//
// 					B32 token_newline = parser->token_current.kind == Token_Kind__Newline;
// 					B32 token_comma   = parser->token_current.kind == Token_Kind__Comma;
//
// 					Parser_expect(parser, token_comma || token_newline, Parser_Error_Kind__Directive_Data_Invalid);
//
// 					B32 break_should_directive = parser->error.kind || parser->end_reached || token_newline;
// 					if (break_should_directive)
// 					{
// 						break;
// 					}
// 				}
//
// 				parser->statement_context->expressions_indexes = expressions_indexes;
// 				parser->statement_context->expressions_count   = expressions_count;
// 				parser->statement_context->size                = data_directive_size * expressions_count;
//
// 				data_directive_size = 0;
// 			} break;
// 			case Directive_Kind__String: {} // fallthrough
// 			case Directive_Kind__Asciz:  { string_size += 1; /* Add null-termination */ } // fallthrough
// 			case Directive_Kind__Ascii:
// 			{
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__String_Literal, Parser_Error_Kind__String_Literal_Expected);
// 				String8 ascii_text = Parser_token_string(parser);
//
// 				// We cannot emit bytes in the section yet, but we have to count the size of it.
// 				// NOTE: from lexing stage, we already know the string is well-formed.
// 				U32 size = String8_byte_size_escaped(ascii_text);
// 				string_size += size;
//
// 				parser->statement_context->size = string_size;
// 				string_size = 0;
//
// 				Parser_advance(parser);
//
// 			} break;
// 			case Directive_Kind__Section:
// 			{
// 				Parser_advance(parser);
//
// 				// FIX: `.section` can be used to create new sections so limiting to the ones that can
// 				// be a standalone directive is not desiderable. Moreover, the syntax is more complex
// 				// because it is like:
// 				// ```asm
// 				// .section .<section>, "<flags>", @<type>
// 				// ```
// 				String8 string_section        = Parser_token_string(parser);
// 				Directive_Kind section_kind   = Directive_Kind__from_String8(string_section);
// 				ELF_Section section_index     = ELF_Section_from_Directive_Kind(section_kind);
// 				parser->section_current_index = section_index;
//
// 				Parser_expect(parser, section_index != 0, Parser_Error_Kind__Directive_Section_Argument_Invalid);
// 				Parser_advance(parser);
// 			} break;
// 			case Directive_Kind__Local:
// 			{
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
//
// 				String8 key = Parser_token_string(parser);
// 				Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);
//
// 				assert_always_m(ELF_Symbol_Binding__Local == 0 && "wrong assumption on local binding value");
// 				B32 demoted = ELF_Symbol_bind_m(entry->elf.type_and_binding) > ELF_Symbol_Binding__Local;
// 				Parser_expect(parser, !demoted, Parser_Error_Kind__Symbol_Demoted);
//
// 				Parser_advance(parser);
// 			} break;
// 			case Directive_Kind__Globl: {} // fallthrough
// 			case Directive_Kind__Global:
// 			{
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
//
// 				String8 key = Parser_token_string(parser);
// 				Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);
//
// 				U8 type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Global, ELF_Symbol_type_m(entry->elf.type_and_binding));
// 				entry->elf.type_and_binding = type_and_binding;
//
// 				Parser_advance(parser);
// 			} break;
// 			case Directive_Kind__Set: {} // fallthrough
// 			case Directive_Kind__Equality:
// 			{
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
// 				String8 key = Parser_token_string(parser);
//
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);
//
// 				Parser_advance(parser);
// 				Expression_Node *expression = Parser_expression_parse(parser);
//
// 				// TODO: what if set or equ creates an alias for a label? Special handling?
//
// 				Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);
//
// 				parser->statement_context->s_symbol = entry;
// 				parser->statement_context->expressions_indexes = &expression->index;
// 				parser->statement_context->expressions_count   = 1;
// 			} break;
// 			case Directive_Kind__Zero:
// 			{
// 				Parser_advance(parser);
// 				Expression_Node *expression = Parser_expression_parse(parser);
//
// 				parser->statement_context->expressions_indexes = &expression->index;
// 				parser->statement_context->expressions_count   = 1;
// 			} break;
// 			case Directive_Kind__Align:
// 			{
// 				parser->statement_context->flags |= Statement_Flags__Size_Variable;
// 			} // fallthrough, same parsing.
// 			case Directive_Kind__Skip:
// 			{
// 				Parser_advance(parser);
// 				Expression_Node *expression = Parser_expression_parse(parser);
// 				parser->statement_context->expressions_indexes = &expression->index;
// 				parser->statement_context->expressions_count = 1;
//
// 				if (parser->token_current.kind == Token_Kind__Comma)
// 				{
// 					Expression_Node *expression_second = Parser_expression_parse(parser);
// 					parser->statement_context->expressions_indexes[1] = expression_second->index;
// 					parser->statement_context->expressions_count = 2;
// 				}
// 			} break;
// 			case Directive_Kind__Common:
// 			{
// 				// .comm symbol, size, alignment
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
//
// 				String8 key = Parser_token_string(parser);
// 				Symbols_Table_Entry *entry = Symbols_Table_reserve(parser->symbols_table, key);
// 				B32 duplicate = entry->elf.section_index != 0;
//
// 				entry->elf.type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Global, 0),
// 				entry->elf.section_index    = ELF_Section_Index__Common,
//
// 				Parser_expect(parser, !duplicate, Parser_Error_Kind__Symbol_Duplicate);
//
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);
//
// 				Parser_advance(parser);
// 				Expression_Node *size_expression = Parser_expression_parse(parser);
//
// 				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);
//
// 				Parser_advance(parser);
// 				Expression_Node *alignment_expression = Parser_expression_parse(parser);
//
// 				U32 *expressions_indexes = Arena__push_array_m(parser->arena, U32, 2);
// 				expressions_indexes[0] = size_expression->index;
// 				expressions_indexes[1] = alignment_expression->index;
//
// 				parser->statement_context->s_symbol = entry;
// 				parser->statement_context->expressions_indexes = expressions_indexes;
// 				parser->statement_context->expressions_count = 2;
// 			} break;
// 			case Directive_Kind__Option:
// 			{
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
//
// 				String8 string = Parser_token_string(parser);
// 				if (memory_match(string.data, "norelax", min_m(string.count, 7)) == 0)
// 				{
// 					parser->flags |= Statement_Flags__Relax_Disabled;
// 				}
// 				else if (memory_match(string.data, "relax", min_m(string.count, 5)) == 0)
// 				{
// 					parser->flags &= ~Statement_Flags__Relax_Disabled;
// 				}
// 				else
// 				{
// 					Parser_error_set(parser, Parser_Error_Kind__Option_Invalid);
// 				}
// 				Parser_advance(parser);
//
// 			} break;
// 			default:
// 			{
// 				ELF_Section section_kind = ELF_Section_from_Directive_Kind(directive_kind);
// 				assert_always_m(section_kind && "unhandled directive");
//
// 				parser->section_current_index = section_kind;
// 				Parser_advance(parser);
// 			} break;
// 			}
//
// 			parser->statement_context->section_index      = parser->section_current_index;
//
// 		} break;
// 		case Token_Kind__Identifier:
// 		{
// 			// This must be an instruction.
// 			String8 instruction = Parser_token_string(parser);
// 			U32 instruction_hash = hash_FNV_1a(instruction);
//
// 			parser->statement_context->kind = Statement_Kind__Instruction;
//
// 			switch (instruction_hash)
// 			{
// 			// U-type
// 			case HASH_lui:       { Parser_instruction_U_parse(parser, Instruction_Kind__LUI);                   } break;
// 			case HASH_auipc:     { Parser_instruction_U_parse(parser, Instruction_Kind__AUIPC);                 } break;
//
// 			// J-type
// 			case HASH_jal:       { Parser_instruction_jal_parse(parser);                                        } break;
//
// 			// I-type (JALR)
// 			case HASH_jalr:      { Parser_instruction_jalr_parse(parser);                                       } break;
//
// 			// B-type
// 			case HASH_beq:       { Parser_instruction_B_parse(parser, Instruction_Kind__BEQ);                   } break;
// 			case HASH_bne:       { Parser_instruction_B_parse(parser, Instruction_Kind__BNE);                   } break;
// 			case HASH_blt:       { Parser_instruction_B_parse(parser, Instruction_Kind__BLT);                   } break;
// 			case HASH_bge:       { Parser_instruction_B_parse(parser, Instruction_Kind__BGE);                   } break;
// 			case HASH_bltu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BLTU);                  } break;
// 			case HASH_bgeu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BGEU);                  } break;
//
// 			// I-type (loads)
// 			case HASH_lb:        { Parser_instruction_I_load_parse(parser, Instruction_Kind__LB);               } break;
// 			case HASH_lh:        { Parser_instruction_I_load_parse(parser, Instruction_Kind__LH);               } break;
// 			case HASH_lw:        { Parser_instruction_I_load_parse(parser, Instruction_Kind__LW);               } break;
// 			case HASH_lbu:       { Parser_instruction_I_load_parse(parser, Instruction_Kind__LBU);              } break;
// 			case HASH_lhu:       { Parser_instruction_I_load_parse(parser, Instruction_Kind__LHU);              } break;
//
// 			// Store-type
// 			case HASH_sb:        { Parser_instruction_S_parse(parser, Instruction_Kind__SB);                    } break;
// 			case HASH_sh:        { Parser_instruction_S_parse(parser, Instruction_Kind__SH);                    } break;
// 			case HASH_sw:        { Parser_instruction_S_parse(parser, Instruction_Kind__SW);                    } break;
//
// 			// I-type (arithmetic)
// 			case HASH_addi:      { Parser_instruction_I_parse(parser, Instruction_Kind__ADDI);                  } break;
// 			case HASH_slti:      { Parser_instruction_I_parse(parser, Instruction_Kind__SLTI);                  } break;
// 			case HASH_sltiu:     { Parser_instruction_I_parse(parser, Instruction_Kind__SLTIU);                 } break;
// 			case HASH_xori:      { Parser_instruction_I_parse(parser, Instruction_Kind__XORI);                  } break;
// 			case HASH_ori:       { Parser_instruction_I_parse(parser, Instruction_Kind__ORI);                   } break;
// 			case HASH_andi:      { Parser_instruction_I_parse(parser, Instruction_Kind__ANDI);                  } break;
// 			case HASH_slli:      { Parser_instruction_I_parse(parser, Instruction_Kind__SLLI);                  } break;
// 			case HASH_srli:      { Parser_instruction_I_parse(parser, Instruction_Kind__SRLI);                  } break;
// 			case HASH_srai:      { Parser_instruction_I_parse(parser, Instruction_Kind__SRAI);                  } break;
//
// 			// R-type
// 			case HASH_add:       { Parser_instruction_R_parse(parser, Instruction_Kind__ADD);                   } break;
// 			case HASH_sub:       { Parser_instruction_R_parse(parser, Instruction_Kind__SUB);                   } break;
// 			case HASH_sll:       { Parser_instruction_R_parse(parser, Instruction_Kind__SLL);                   } break;
// 			case HASH_slt:       { Parser_instruction_R_parse(parser, Instruction_Kind__SLT);                   } break;
// 			case HASH_sltu:      { Parser_instruction_R_parse(parser, Instruction_Kind__SLTU);                  } break;
// 			case HASH_xor:       { Parser_instruction_R_parse(parser, Instruction_Kind__XOR);                   } break;
// 			case HASH_srl:       { Parser_instruction_R_parse(parser, Instruction_Kind__SRL);                   } break;
// 			case HASH_sra:       { Parser_instruction_R_parse(parser, Instruction_Kind__SRA);                   } break;
// 			case HASH_or:        { Parser_instruction_R_parse(parser, Instruction_Kind__OR);                    } break;
// 			case HASH_and:       { Parser_instruction_R_parse(parser, Instruction_Kind__AND);                   } break;
//
// 			// Pseudo-instructions
//
// 			// Pseudo-instructions (no operands)
// 			case HASH_nop:       { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__NOP);       } break;
// 			case HASH_ret:       { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__RET);       } break;
// 			// Pseudo-instructions (rd, rs)
// 			case HASH_mv:        { Parser_instruction_mv_parse(parser);                                         } break;
// 			case HASH_not:       { Parser_instruction_not_parse(parser);                                        } break;
// 			case HASH_sext_w:    { Parser_instruction_sext_w_parse(parser);                                     } break;
// 			case HASH_neg:       { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__NEG);            } break;
// 			case HASH_negw:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__NEGW);           } break;
// 			case HASH_seqz:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SEQZ);           } break;
// 			case HASH_snez:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SNEZ);           } break;
// 			case HASH_sltz:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SLTZ);           } break;
// 			case HASH_sgtz:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SGTZ);           } break;
// 			// Pseudo-instructions (rs, offset)
// 			case HASH_beqz:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BEQZ);           } break;
// 			case HASH_bnez:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BNEZ);           } break;
// 			case HASH_blez:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BLEZ);           } break;
// 			case HASH_bgez:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BGEZ);           } break;
// 			case HASH_bltz:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BLTZ);           } break;
// 			case HASH_bgtz:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BGTZ);           } break;
// 			// Pseudo-instructions (rs, rt, offset)
// 			case HASH_bgt:       { Parser_instruction_B_parse(parser, Instruction_Kind__BGT);                   } break;
// 			case HASH_ble:       { Parser_instruction_B_parse(parser, Instruction_Kind__BLE);                   } break;
// 			case HASH_bgtu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BGTU);                  } break;
// 			case HASH_bleu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BLEU);                  } break;
// 			// Pseudo-instructions (offset only)
// 			case HASH_j:         { Parser_instruction_j_parse(parser);                                          } break;
// 			case HASH_call:      { Parser_instruction_call_parse(parser);                                       } break;
// 			case HASH_tail:      { Parser_instruction_tail_parse(parser);                                       } break;
// 			// Pseudo-instructions (rs only)
// 			case HASH_jr:        { Parser_instruction_jr_parse(parser);                                         } break;
// 			// Pseudo-instructions (rd, imm/symbol)
// 			case HASH_li:        { Parser_instruction_li_parse(parser);                                         } break;
// 			case HASH_la:        { Parser_instruction_la_parse(parser);                                         } break;
//
// 			// Others
// 			case HASH_ecall:     { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__ECALL);     } break;
// 			case HASH_ebreak:    { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__EBREAK);    } break;
// 			case HASH_pause:     { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__PAUSE);     } break;
// 			case HASH_fence_tso: { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__FENCE_TSO); } break;
// 			case HASH_fence:     { Parser_instruction_fence_parse(parser);                                      } break;
//
// 			default:
// 			{
// 				Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
// 			} break;
// 			}
//
// 			Instruction_Kind instruction_kind = parser->statement_context->instruction_kind;
// 			B32 expandable = Instruction_Encoding_table[instruction_kind].flags & Instruction_Flags__Expandable;
// 			if (expandable)
// 			{
// 				parser->statement_context->flags |= Statement_Flags__Size_Variable;
// 			}
//
// 			// It is at most one.
// 			parser->statement_context->expressions_count = parser->statement_context->expressions_indexes ? 1 : 0;
// 		} break;
// 		default:
// 		{
// 			Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
// 		} break;
// 		}
//
// 		B32 loop_infinite_avoided = parser->token_index > parser->token_index_before || parser->error.kind || parser->end_reached;
// 		assert_always_m(loop_infinite_avoided && "infinite loop in parser");
//
// 		parser->statement_context->token_index_end = parser->token_index - 1;
//
// 		// The iteration has produced a new statement.
// 		if (token_start_kind != Token_Kind__Newline)
// 		{
// 			// We should have reached a newline, or the statement should be a label definition, otherwise there is junk.
// 			B32 correct_end_of_line = parser->token_current.kind == Token_Kind__Newline;
// 			Parser_expect(parser, correct_end_of_line, Parser_Error_Kind__Line_Extra_Content);
// 			Statements_push(parser->statements, *parser->statement_context);
// 		}
// 	}
//
// 	return;
// }


/// ParseStatement:
///   ::= EndOfStatement
///   ::= Label* Directive ...Operands... EndOfStatement
///   ::= Label* Identifier OperandList* EndOfStatement
// internal void
// Parser_2__parse(Parser_2 *parser, String8 *input, Arena *arena, Statements_Xar *statements)
// {
// 	for (;;)
// 	{
// 		Token_2 token = Lexer_lex(parser->lexer);
// 		B32 break_should = token.kind == 0;
// 		const char *token_str = Token_Kind_strings[token.kind];
// 		printf("%s ", token_str);
// 		if (break_should)
// 		{
// 			break;
// 		}
//
// 		switch (token.kind)
// 		{
// 		case Token_Kind__Identifier:
// 		{
// 			// B32 directive_is = token.content.data[0] == '.';
// 		} break;
// 		default: {} break;
// 		}
//
// 	}
// }
//
// internal void
// Parser_expect(Parser_2 *parser, B32 condition, Parser_Error_Kind error_kind)
// {
// 	// if (!condition)
// 	// {
// 	// 	Diagnostic error =
// 	// 	{
// 	// 		.filename     = parser->filename,
// 	// 		.message_kind = Parser_Error_Kind_messages[error_kind],
// 	//
// 	// 		.variant      = (U32)error_kind,
// 	// 	};
// 	// 	// Parser_error_set(parser, error_kind);
// 	// }
// 	return;
// }
//
// internal Diagnostic *
// Parser__push_diagnostic_from_index(Parser_2 *parser, U64 index)
// {
// 	Source *source = parser->source_current;
//
// 	U64 row_index        = Source_Lines__search(&source->lines, index);
// 	U64 line_start_index = *(U64 *)xar_get_m(&source->lines, row_index);
// 	U64 column_index = index - *(U64 *)xar_get_m(&source->lines, row_index) - 1;
// 	U32 location     = Source__location(source, index);
//
//
// 	Diagnostic *diagnostic = Diagnostics__push(parser->diagnostics);
//
// 	diagnostic->source_manager   = parser->source_manager;
// 	diagnostic->filename         = parser->source_current->name;
// 	diagnostic->line             = source->input.data + line_start_index;
// 	diagnostic->location         = location;
// 	diagnostic->row_index        = row_index;
// 	diagnostic->column_index     = column_index;
//
// 	return diagnostic;
// }

// Maybe find statement boundary?
// Giving good diagnostics now it's harder because I can't reference past tokens.
//

// Just returns a statement

/// ParseStatement:
///   ::= EndOfStatement
///   ::= Label* Directive ...Operands... EndOfStatement
///   ::= Label* Identifier OperandList* EndOfStatement
internal void
statement_read
(
	Arena                   *arena,
	Token_Cursor            *cursor,
	Section                 *section,
	Diagnostic_List         *diagnostics,
	Expressions             *expressions,
	Fixups                  *fixups,
	Sections_Table          *sections_table,
	Symbols_Table           *symbols_table
)
{

	U32 source_index_start = cursor->source_index;
	token_next(cursor, diagnostics, arena);

	B32 progress = 1;
	B32 error =  0;

 	for (;;)
	{
		Directive_Kind directive_kind         = 0;
		U32            instruction_hash       = 0;
		B32            null_terminated_string = 0;

		// TODO: when to exit?
		progress = source_index_start < cursor->source_index;
		B32 break_should_outer = cursor->current.kind == Token_Kind__None
				      || cursor->current.kind == Token_Kind__Error
				      || error;
		assert_always_m((progress || break_should_outer) && "infinite loop detected");
		if (break_should_outer)
		{
			break;
		}

		switch (cursor->current.kind)
		{
		// no-op, continue;
		case Token_Kind__Newline:
		{
			token_next(cursor, diagnostics, arena);
		} break;
		// Instructions, directives and label start with an identifier. We have to discriminate further.
		case Token_Kind__Identifier:
		{

			String8 identifier = Token_Cursor__text(cursor);
			B32 dot_start = identifier.data[0] == '.';

			if (dot_start)
			{
				directive_kind = Directive_Kind__from_String8(identifier);
				assert_always_m(directive_kind && "machine-dependent directives not yet implemented");
			}

			Token_2 next = token_peek(cursor->source, cursor->source_index, diagnostics, arena);
			B32 label_found = next.kind == Token_Kind__Colon;
			if (label_found)
			{
				Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, identifier);
				ELF64_Symbol elf_empty = {0};
				B32 empty = memory_match_struct(&symbol->elf, &elf_empty);
				if (!empty)
				{
					{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location   = cursor->current.location;
					diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Label_Duplicate];
					diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
					{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind       = Diagnostic_Kind__Note;
					diagnostic->location   = symbol->location;
					diagnostic->message    = Diagnostic__previous_declaration_String8;
					diagnostic->ranges[0]  = (Range1_U32){{ symbol->location, symbol->location + identifier.count }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
				}
				symbol->location = cursor->current.location;
				token_next(cursor, diagnostics, arena);
				token_next(cursor, diagnostics, arena);
			}

			B32 instruction_expected = !label_found && !directive_kind;
			if (instruction_expected)
			{
				instruction_hash = hash_FNV_1a(identifier);
			}
		} break;
		default:
		{
			// Sort of catch-all
			Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
			diagnostic->location   = cursor->current.location;
			diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Line_Invalid];
			diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
			SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			error = 1;
		} break;
		}

		U16 relocation = 0;

		if (instruction_hash)
		{
			const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
			// TODO: change behaviour, emit error.
			assert_always_m(opcode);
			assert_always_m(opcode->match_function);

			RISCV_Instruction instruction = RISCV_Instruction__create(opcode);
			OP_Argument *arguments = opcode->arguments;

			U32 location_begin = cursor->current.location;
			token_next(cursor, diagnostics, arena);

			for (;;)
			{
				OP_Argument argument = *arguments;
				if (!argument)
				{
					B32 match = opcode->match_function(opcode, instruction.encoding);
					if (!match)
					{
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->location   = location_begin;
						diagnostic->message    = String8__literal("unrecognized opcode");
						diagnostic->ranges[0]  = (Range1_U32){{ location_begin, cursor->current.location + cursor->current.size }};
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
					break;
				}

				switch (argument)
				{
				case OP_Argument__Comma:
				{
					// NOTE: This whole thing could extracted into a `expect_comma_and_advance`.
					Token_2 token_before_comma = cursor->previous;
					if (cursor->current.kind == Token_Kind__Comma)
					{
						token_next(cursor, diagnostics, arena);
					}
					else
					{
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->location   = token_before_comma.location + token_before_comma.size;
						diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
				} break;
				case OP_Argument__Parenthesis_Left:
				{
					if (cursor->current.kind != Token_Kind__Parenthesis_Left)
					{
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->location   = cursor->current.location;
						diagnostic->message    = String8__literal("'(' expected");
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
				} break;
				case OP_Argument__Parenthesis_Right:
				{
					if (cursor->current.kind != Token_Kind__Parenthesis_Right)
					{
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->location   = cursor->current.location;
						diagnostic->message    = String8__literal("')' expected");
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}
				} break;
				case OP_Argument__RD:  {} // fallthrough
				case OP_Argument__RS3: {} // fallthrough
				case OP_Argument__RS2: {} // fallthrough
				case OP_Argument__RS1:
				{
					U8 reg = register_lookup(Token_Cursor__text(cursor));
					if (reg == register_invalid)
					{
						Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						diagnostic->location   = cursor->current.location;
						diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Register_Invalid];
						diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
						SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					}

					switch (argument)
					{
					case OP_Argument__RD:  { INSERT_OPERAND(RD,  instruction, reg); } break;
					case OP_Argument__RS3: { INSERT_OPERAND(RS3, instruction, reg); } break;
					case OP_Argument__RS2: { INSERT_OPERAND(RS2, instruction, reg); } break;
					case OP_Argument__RS1: { INSERT_OPERAND(RS1, instruction, reg); } break;
					}
					token_next(cursor, diagnostics, arena);
				} break;
				case OP_Argument__Offset_Load:
				{
					OP_Argument *next = arguments + 1;
					assert_always_m(next && "invalid operand list");

					Token_2 peek = token_peek(cursor->source, cursor->source_index, diagnostics, arena);
					if (*next == OP_Argument__Parenthesis_Left && peek.kind == Token_Kind__Parenthesis_Left)
					{
						// Omitted immediate, e.g. lw t1, (t0)
						arguments += 2;
					}
				} // fallthrough;
				case OP_Argument__Immediate_I:
				{
					U32 location_expression_begin = cursor->current.location;
					Expression_Node *expression   = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, diagnostics, &relocation, Relocation_Operator__itype);
					U32 location_expression_end   = cursor->current.location;
					S64 result = expression_evaluate(expressions, expression->index);

					// if (relocation)
					// {
					// 	U32 fixup_encoding_base_offset = section->fragment_list.last->size_fixed;
					// 	Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
					//
					// 	fixup->expression_index = expression->index;
					// 	fixup->fragment         = section->fragment_list.last;
					// 	fixup->encoding_offset  = fixup_encoding_base_offset;
					// 	// NOTE: the relocation type encodes information on where to set the resolved
					// 	// value.
					// 	fixup->relocation_type  = Relocation_RISC_V__Low_12_I_Type;
					// 	fixup->size             = 4;
					//
					// 	SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
					// 	result = 0;
					// }

					if (!relocation)
					{
						if (expression->kind == Expression_Kind__Constant)
						{
							// TODO: normalize constant expression?
							B32 fits = S64_bits_range_in(result, 12);
							if (!fits)
							{
								Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
								diagnostic->location   = location_expression_begin;
								diagnostic->message    = String8__literal("constant expression value must fits in 12 bits");
								diagnostic->ranges[0]  = (Range1_U32){{ location_expression_begin, location_expression_end }};
								SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
								result = 0;
							}

							// TODO: GNU as does this at a later step, and by default emits a
							// relocation. Consider doing the same.
							U32 encoding_immediate = encode_immediate_i_m(result);
							instruction.encoding |= encoding_immediate;
						}
						else
						{

							Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
							diagnostic->location   = location_expression_begin;
							diagnostic->message    = String8__literal("Non-constant expression without relocation operator provided");
							diagnostic->ranges[0]  = (Range1_U32){{ location_expression_begin, location_expression_end }};
							SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
						}
					}
				} break;
				default: { unreachable_m(); }
				}

				arguments += 1;
			}

			// TODO: this should be done later by also checking the relocation_out
			U8 *data = Fragment_List__fixed(&section->fragment_list, arena, location_begin, 4);
			memory_copy(data, &instruction.encoding, 4);
		}

		U8   data_directive_size = 0;
		S64  fill_size           = 0;
		B32  fill_size_set       = 0;
		S64  fill_pattern        = 0;
		B32  fill_pattern_set    = 0;

		switch (directive_kind)
		{
		case Directive_Kind__None: {} break;

		case Directive_Kind__Word_Double: { data_directive_size += 4; } // fallthrough
		case Directive_Kind__Word:        { data_directive_size += 2; } // fallthrough
		case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
		case Directive_Kind__Byte:
		{
			// TODO: finish this, it is a non-trivial directive to handle.
			data_directive_size += 1;

			// U32 expressions_index_start = expressions->header.count;
			String8 subsource = String8__skip(String8__new(cursor->source->data, cursor->source->count), cursor->source_index);
			U32 expressions_count = 1 + commas_until_newline(subsource.data, subsource.count);
			U32 fixup_encoding_base_offset = section->fragment_list.last->size_fixed;
			U8 *data = Fragment_List__fixed(&section->fragment_list, section->arena, cursor->current.location, expressions_count * data_directive_size);

			// Format: .byte <expr_1> , ..., <expr_n>.
			//
			// Advance to reach the first expression token.
			U32 index = 0;
			for (;;)
			{
				Expression_Node *expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);

				S64 result = expression_evaluate(expressions, expression->index);
				if (expression->evaluation != Expression_Kind__Constant)
				{
					Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
					fixup->expression_index = expression->index;
					fixup->fragment         = section->fragment_list.last;
					fixup->encoding_offset  = fixup_encoding_base_offset + index * data_directive_size;
					fixup->size             = data_directive_size;

					SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
				}
				else
				{
					// TODO: warn here if it doesn't fit.
					memory_copy(data + index * data_directive_size, (U8 *)result, data_directive_size);
				}

				B32 break_should_directive =  index >= expressions_count - 1
					                  || cursor->source_index >= cursor->source->count
							  || cursor->current.kind == Token_Kind__Newline;
				if (break_should_directive)
				{
					break;
				}

				if (cursor->current.kind != Token_Kind__Comma)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = cursor->current.location;
					diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}

				index += 1;
			}

			// statement.expressions_index = expressions_index_start;
			// statement.expressions_count = expressions_count;
			// statement.size              = data_directive_size * expressions_count;
		} break;
		case Directive_Kind__String: {} // fallthrough
		case Directive_Kind__Asciz:  { null_terminated_string = 1; } // fallthrough
		case Directive_Kind__Ascii:
		{
			token_next(cursor, diagnostics, arena);
			if (cursor->current.kind != Token_Kind__String)
			{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location = cursor->current.location;
				diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}

			// Can be of the form `"\nhello\n", so with quotes and optional escaped characters.
			String8 text = Token_Cursor__text(cursor);
			text = String8__skip(text, 1);
			text = String8__chop(text, 1);
			U32 size_escaped = String8__escaped_size(text) + !!null_terminated_string;

			U8 *data = Fragment_List__fixed(&section->fragment_list, section->arena, cursor->current.location, size_escaped);
			bytes_escaped_fill(text, data, size_escaped);

			token_next(cursor, diagnostics, arena);
		} break;
		case Directive_Kind__Section:
		{
			// Syntax: `.section name [, "flags"[, @type[, argument...]]]`
			token_next(cursor, diagnostics, arena);
			String8 name = String8__new(cursor->source->data + cursor->current.index, cursor->current.size);
			Section *section_new = Sections_Table__get_or_default(sections_table, name);

			token_next(cursor, diagnostics, arena);
			if (cursor->current.kind == Token_Kind__Comma)
			{
				// Read flags.
				token_next(cursor, diagnostics, arena);
				if (cursor->current.kind != Token_Kind__String)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = cursor->current.location;
					diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__String_Literal_Expected];
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
				String8 text    = Token_Cursor__text(cursor);
				String8 content = token_string_content(text);
				ELF_Section_Header_Flags flags = ELF_Section_Header_Flags__parse(content);

				if (flags == ELF_Section_Header_Flags__Invalid)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind       = Diagnostic_Kind__Error;
					diagnostic->location = cursor->current.location;
					diagnostic->message  = String8__literal("invalid section flags, expected: " ELF_Section_Header_Flags__cstring);
					diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}

				// TODO: are they ORed? Or overwritten?
				section_new->flags = flags;
				token_next(cursor, diagnostics, arena);
			}

			if (cursor->current.kind == Token_Kind__Comma)
			{
				// Parse type.
				token_next(cursor, diagnostics, arena);
				if (cursor->current.kind != Token_Kind__At)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location   = cursor->current.location;
					diagnostic->message    = String8__literal("invalid section type syntax, expected @type");
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
				token_next(cursor, diagnostics, arena);
				String8 text    = Token_Cursor__text(cursor);
				String8 content = token_string_content(text);
				ELF_Section_Header_Type type = ELF_Section_Header_Type__from_String8(content);
				if (type == ELF_Section_Header_Type__Invalid)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind       = Diagnostic_Kind__Error;
					diagnostic->location   = cursor->current.location;
					diagnostic->message    = String8__literal("invalid section type");
					diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
				section_new->type = type;
				token_next(cursor, diagnostics, arena);
			}


			section = section_new;
		} break;
		case Directive_Kind__Local:
		{
			token_next(cursor, diagnostics, arena);
			if (cursor->current.kind != Token_Kind__Identifier)
			{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location = cursor->current.location;
				diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}

			String8 name = Token_Cursor__text(cursor);
			Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

			U8 type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Local, ELF_Symbol_type_m(symbol->elf.type_and_binding));
			symbol->elf.section_index    = section->index;
			symbol->elf.type_and_binding = type_and_binding;
			symbol->location             = cursor->current.location;

			B32 demoted = ELF_Symbol_bind_m(symbol->elf.type_and_binding) > ELF_Symbol_Binding__Local;
			if (demoted)
			{
				{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location   = cursor->current.location;
				diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Symbol_Demoted];
				diagnostic->ranges[0] = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
				{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->kind       = Diagnostic_Kind__Note;
				diagnostic->location   = symbol->location;
				diagnostic->message    = Diagnostic__previous_declaration_String8;
				diagnostic->ranges[0] = (Range1_U32){{ symbol->location, symbol->location + name.count }};
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
			}

			token_next(cursor, diagnostics, arena);
		} break;
		case Directive_Kind__Globl: {} // fallthrough
		case Directive_Kind__Global:
		{
			token_next(cursor, diagnostics, arena);
			if (cursor->current.kind != Token_Kind__Identifier)
			{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location = cursor->current.location;
				diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}

			String8 name = Token_Cursor__text(cursor);
			Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);

			U8 type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Global, ELF_Symbol_type_m(symbol->elf.type_and_binding));
			symbol->elf.section_index    = section->index;
			symbol->elf.type_and_binding = type_and_binding;
			symbol->location             = cursor->current.location;

			token_next(cursor, diagnostics, arena);
		} break;
		// TODO: implement .eqv or .equiv which are more picky about re-definitions and forward references.
		// Lastly, support for `<identifier> = <expr>` could be added by jumping here.
		case Directive_Kind__Set: {} // fallthrough
		case Directive_Kind__Equality:
		{
			token_next(cursor, diagnostics, arena);
			// TODO: gas accepts also a string, in such case it is unquoted.
			if (cursor->current.kind != Token_Kind__Identifier && cursor->current.kind != Token_Kind__String)
			{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location = cursor->current.location;
				diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Identifier_Expected];
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}

			String8 name = Token_Cursor__text(cursor);
			if (cursor->current.kind == Token_Kind__String)
			{
				name = token_string_content(name);
			}
			Symbol_Ref *symbol = Symbols_Table__create(symbols_table, name);
			symbol->location = cursor->current.location;
			// TODO: not very clear whether all symbols come with volatile by default.
			// TODO: it might be that type and binding information is inherited.
			// Example:
			// ```asm
			// .global asdf
			// nop
			// .global asdf2
			// j asdf2-asdf
			// .set asdf, 5
			// ```
			symbol->flags |= Symbol_Flags__Volatile;
			symbol->elf.section_index = ELF_Section_Index__Absolute;

			token_next(cursor, diagnostics, arena);
			if (cursor->current.kind != Token_Kind__Comma)
			{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location = cursor->current.location;
				diagnostic->message  = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}
			Expression_Node *expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
			symbol->expression_index = expression->index;

			S64 result = expression_evaluate(expressions, expression->index);
			if (expression->evaluation == Expression_Kind__Constant)
			{
				symbol->elf.section_index = ELF_Section_Index__Absolute;
				symbol->elf.value         = result;
			}

			// NOTE: a fixup needs to be created only when some data has to be written, here is not
			// necessary.
			token_next(cursor, diagnostics, arena);
		} break;
		case Directive_Kind__Zero:
		{
			// Equavalent to .fill repeat, 1, 0
			fill_pattern     = 0;
			fill_pattern_set = 1;
		} // fallthrough
		case Directive_Kind__Space:
		{
		        // Equavalent to .fill repeat, 1, value
		} // fallthrough
		case Directive_Kind__Skip:
		{
			// Equavalent to .fill repeat, 1, value
			fill_size     = 1;
			fill_size_set = 1;
		} // fallthrough
		case Directive_Kind__Fill:
		{
			// .fill repeat [, size [, value ]].
			token_next(cursor, diagnostics, arena);
			U64 location_begin = cursor->current.location;

			Expression_Node *repeat_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
			S64 repeat = expression_evaluate(expressions, repeat_expression->index);
			if (repeat_expression->evaluation != Expression_Kind__Constant)
			{
				Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
				fixup->expression_index = repeat_expression->index;
				fixup->fragment         = section->fragment_list.last;
				fixup->encoding_offset  = section->fragment_list.last->size_fixed;

				SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
			}

			if (cursor->current.kind == Token_Kind__Comma && !fill_size_set)
			{
				// Read size
				token_next(cursor, diagnostics, arena);
				U64 location_expression_begin = cursor->current.location;
				Expression_Node *size_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
				U64 location_expression_end   = cursor->current.location;
				fill_size = expression_evaluate(expressions, size_expression->index);
				if (size_expression->evaluation != Expression_Kind__Constant)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = cursor->current.location;
					diagnostic->message  = String8__literal("constant expression expected");
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
				if (fill_size <= 0)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind     = Diagnostic_Kind__Warning;
					diagnostic->location = location_expression_begin;
					// TODO: nicer diagnostic with vsnprintf support in String8
					diagnostic->message  = String8__literal("non-positive fill size, ensuring it is zero");
					diagnostic->ranges[0] = (Range1_U32){{ location_expression_begin, location_expression_end }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					fill_size = 0;
				}
				if (fill_size > 8)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind     = Diagnostic_Kind__Warning;
					diagnostic->location = location_expression_begin;
					diagnostic->message  = String8__literal("capping fill size to 8 bytes");
					diagnostic->ranges[0] = (Range1_U32){{ location_expression_begin, location_expression_end }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					fill_size = 8;
				}
			}

			if (cursor->current.kind == Token_Kind__Comma && !fill_pattern_set)
			{
				// Read value
				token_next(cursor, diagnostics, arena);
				Expression_Node *value_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
				fill_pattern = expression_evaluate(expressions, value_expression->index);
				if (value_expression->evaluation != Expression_Kind__Constant)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = cursor->current.location;
					diagnostic->message  = String8__literal("constant expression expected");
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
			}
			Fragment_List__fill(&section->fragment_list, arena, location_begin, repeat, fill_pattern, fill_size);
		} break;
		case Directive_Kind__Align:
		{
			// .align <size> [, <pattern> [, <max_bytes>]]
			//
			// TODO: support omitting some values, e.g. .align 2, , 8
			//
			// .align is implementation-defined, in this case we interpret the next expression as a power of
			// two. See also .p2align.
			// For this expression, note that a label difference is allowed but there should be no expansion
			// between them. Probably a good way to check is making sure both are defined within the same
			// fragment
			U32 location_begin = cursor->current.location;
			U8  pattern   = 0;
			U8  bytes_max = 0;
			U8  alignment = 0;

			token_next(cursor, diagnostics, arena);
			Expression_Node *alignment_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
			S64 alignment_evaluation = expression_evaluate(expressions, alignment_expression->index);

			alignment = (U8)alignment_evaluation;
			if (alignment_expression->evaluation != Expression_Kind__Constant)
			{
				Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);

				fixup->expression_index = alignment_expression->index;
				fixup->fragment         = section->fragment_list.last;
				fixup->encoding_offset  = section->fragment_list.last->size_fixed;

				SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
			}

			if (cursor->current.kind == Token_Kind__Comma)
			{
				// Read pattern
				token_next(cursor, diagnostics, arena);

				U64 location_expression_begin = cursor->current.location;
				Expression_Node *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
				U64 location_expression_end   = cursor->current.location;

				S64 pattern_evaluation = expression_evaluate(expressions, pattern_expression->index);
				if (pattern_expression->evaluation != Expression_Kind__Constant)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = location_expression_begin;
					diagnostic->message  = String8__literal("constant expression expected");
					diagnostic->ranges[0] = (Range1_U32){{ location_expression_begin, location_expression_end }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}

				// TODO: check if between 0 and 255 instead?
				pattern = (U8)pattern_evaluation;
				if ((S64)pattern != pattern_evaluation)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = location_expression_begin;
					diagnostic->message  = String8__literal("expression result isn't a unsigned 8 bit integer");
					diagnostic->ranges[0] = (Range1_U32){{ location_expression_begin, location_expression_end }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
			}

			if (cursor->current.kind == Token_Kind__Comma)
			{
				// Read bytes_max
				token_next(cursor, diagnostics, arena);
				U64 location_expression_begin = cursor->current.location;
				Expression_Node *bytes_max_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
				U64 location_expression_end   = cursor->current.location;
				S64 bytes_max_evaluation = expression_evaluate(expressions, bytes_max_expression->index);
				if (bytes_max_expression->evaluation != Expression_Kind__Constant)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->location = cursor->current.location;
					diagnostic->message  = String8__literal("constant expression expected");
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
				}
				bytes_max = (U8)bytes_max_evaluation;

				if (bytes_max_evaluation <= 0)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind     = Diagnostic_Kind__Warning;
					diagnostic->location = location_expression_begin;
					// TODO: nicer diagnostic with vsnprintf support in String8
					diagnostic->message  = String8__literal("non-positive max bytes size, ensuring it is zero");
					diagnostic->ranges[0] = (Range1_U32){{ location_expression_begin, location_expression_end }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					bytes_max = 0;
				}
				// NOTE: I don't know what should be an upper limit but there should be one probably.
				// GNU as allows you to pass zero to NOT provide one which I think can be risky.
				if (bytes_max_evaluation > 64)
				{
					Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
					diagnostic->kind     = Diagnostic_Kind__Warning;
					diagnostic->location = location_expression_begin;
					diagnostic->message  = String8__literal("capping fill size to 64 bytes");
					diagnostic->ranges[0] = (Range1_U32){{ location_expression_begin, location_expression_end }};
					SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					bytes_max = 8;
				}
			}

			Fragment_List__align(&section->fragment_list, arena, location_begin, alignment, pattern, bytes_max);
		} break;
// 			case Directive_Kind__Common:
// 			{
// 				// .comm symbol, size, alignment
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
//
// 				String8 key = Parser_token_string(parser);
// 				Symbols_Table_Entry *entry = Symbols_Table_reserve(parser->symbols_table, key);
// 				B32 duplicate = entry->elf.section_index != 0;
//
// 				entry->elf.type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Global, 0),
// 				entry->elf.section_index    = ELF_Section_Index__Common,
//
// 				Parser_expect(parser, !duplicate, Parser_Error_Kind__Symbol_Duplicate);
//
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);
//
// 				Parser_advance(parser);
// 				Expression_Node *size_expression = Parser_expression_parse(parser);
//
// 				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);
//
// 				Parser_advance(parser);
// 				Expression_Node *alignment_expression = Parser_expression_parse(parser);
//
// 				U32 *expressions_indexes = Arena__push_array_m(parser->arena, U32, 2);
// 				expressions_indexes[0] = size_expression->index;
// 				expressions_indexes[1] = alignment_expression->index;
//
// 				parser->statement_s_symbol = entry;
// 				parser->statement_expressions_indexes = expressions_indexes;
// 				parser->statement_expressions_count = 2;
// 			} break;
// 			case Directive_Kind__Option:
// 			{
// 				Parser_advance(parser);
// 				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
//
// 				String8 string = Parser_token_string(parser);
// 				if (memory_match(string.data, "norelax", min_m(string.count, 7)) == 0)
// 				{
// 					parser->flags |= Statement_Flags__Relax_Disabled;
// 				}
// 				else if (memory_match(string.data, "relax", min_m(string.count, 5)) == 0)
// 				{
// 					parser->flags &= ~Statement_Flags__Relax_Disabled;
// 				}
// 				else
// 				{
// 					Parser_error_set(parser, Parser_Error_Kind__Option_Invalid);
// 				}
// 				Parser_advance(parser);
//
// 			} break;
// 			default:
// 			{
// 				ELF_Section section_kind = ELF_Section_from_Directive_Kind(directive_kind);
// 				assert_always_m(section_kind && "unhandled directive");
//
// 				parser->section_current_index = section_kind;
// 				Parser_advance(parser);
// 			} break;
// 			}
//
// 			parser->statement_section_index      = parser->section_current_index;
		default: {} break;
		}

		// Find end of line junk
		U32 junk_location_begin = cursor->current.location;
		U32 junk_location_end   = 0;
		for (;;)
		{
 			Token_Kind kind = cursor->current.kind;
			B32 break_should = kind == Token_Kind__None
				|| kind == Token_Kind__Newline
				|| kind == Token_Kind__Semicolon;
			if (break_should)
			{
				token_next(cursor, diagnostics, arena);
				break;
			}
			junk_location_end = cursor->current.location + cursor->current.size;
			token_next(cursor, diagnostics, arena);
		}

		if (junk_location_end)
		{
			Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
			diagnostic->location   = junk_location_begin;
			diagnostic->message    = String8__literal("junk found at the end of line");
			diagnostic->ranges[0]  = (Range1_U32){{ junk_location_begin, junk_location_end }};
			SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
		}
	}


	return;
}
