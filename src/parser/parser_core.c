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

// Macros for encoding relaxation state for RVC branches and far jumps.
#define RELAX_BRANCH_ENCODE(uncond, rvc, length)	\
  ((U32) 					        \
   (0xc0000000						\
    | ((uncond) ? 1 : 0)				\
    | ((rvc) ? 2 : 0)					\
    | ((length) << 2)))
#define RELAX_BRANCH_P(i)      (((i) & 0xf0000000) == 0xc0000000)
#define RELAX_BRANCH_LENGTH(i) (((i) >> 2) & 0xF)
#define RELAX_BRANCH_RVC(i)    (((i) & 2) != 0)
#define RELAX_BRANCH_UNCOND(i) (((i) & 1) != 0)

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

// Maybe find statement boundary?
// Giving good diagnostics now it's harder because I can't reference past tokens.
//

internal void
RISCV_Instruction__parse
(
	Arena                   *arena,
	Token_Cursor            *cursor,
	Diagnostic_List         *diagnostics,
	Expressions             *expressions,
	Symbols_Table           *symbols_table,
	U32                      instruction_hash,

	U16                     *relocation_out,
	RISCV_Instruction       *instruction_out,
	U32                     *expression_index_out
)
{
	const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
	const char *opcode_name = opcode->name;

	U32 location_begin = cursor->current.location;
	token_next(cursor, diagnostics, arena);

	Expression_Node *expression = 0;
	B32 match = 0;

	// Iterate over opcode entries with the same name.
	for (;;)
	{
		if (match || opcode->hash == 0 || opcode->name != opcode_name)
		{
			break;
		}

		*instruction_out = RISCV_Instruction__create(opcode, location_begin);
		OP_Argument *arguments = opcode->arguments;

		// Iterate over opcode arguments.
		for (;;)
		{
			OP_Argument argument = *arguments;
			if (!argument)
			{
				match = !opcode->match_function || opcode->match_function(opcode, instruction_out->encoding);
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
				       case OP_Argument__RD:  { INSERT_OPERAND(RD,  *instruction_out, reg); } break;
				       case OP_Argument__RS3: { INSERT_OPERAND(RS3, *instruction_out, reg); } break;
				       case OP_Argument__RS2: { INSERT_OPERAND(RS2, *instruction_out, reg); } break;
				       case OP_Argument__RS1: { INSERT_OPERAND(RS1, *instruction_out, reg); } break;
				}
				token_next(cursor, diagnostics, arena);
			} break;
			case OP_Argument__Offset_PC_Relative_20:
			{
				// NOTE: we use GNU as approach to add mark a branch relocation immediately.
				// This relocation is temporary, and could be changed, since it depends on the
				// value of the expression and the symbols required.
				//
				// At assembly time, we may not know how many instructions this will expand to. It is
				// deferred later when we know all instructions. It is a different situation compared to
				// a `li` or `call` instruction which, during instruction parsing, are already expanded
				// into a known number of instructions (`INSN_MACRO`)
				*relocation_out = Relocation_RISC_V__JAL;
				expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
			} break;
			case OP_Argument__Offset_PC_Relative_12:
			{
				// See notes for `OP_Argument__Offset_PC_Relative_20`.
				*relocation_out = Relocation_RISC_V__Branch;
				expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
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
				expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, diagnostics, relocation_out, Relocation_Operator__itype);
				U32 location_expression_end   = cursor->current.location;

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

				if (*relocation_out)
				{
				       if (expression->kind == Expression_Kind__Constant)
				       {
					       // TODO: normalize constant expression?
					       B32 fits = S64_bits_range_in(expression->integer_value, 12);
					       if (!fits)
					       {
						       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
						       diagnostic->location   = location_expression_begin;
						       diagnostic->message    = String8__literal("constant expression value must fits in 12 bits");
						       diagnostic->ranges[0]  = (Range1_U32){{ location_expression_begin, location_expression_end }};
						       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
					       }

					       // TODO: GNU as does this at a later step, and by default emits a
					       // relocation. Consider doing the same.
					       U32 encoding_immediate = encode_immediate_i_m(expression->integer_value);
					       instruction_out->encoding |= encoding_immediate;
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


		opcode += 1;
	}

	if (!match)
	{
		Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
		diagnostic->location   = location_begin;
		diagnostic->message    = String8__literal("unrecognized opcode");
		diagnostic->ranges[0]  = (Range1_U32){{ location_begin, cursor->current.location + cursor->current.size }};
		SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
	}

	*expression_index_out = expression ? expression->index : 0;

	return;
}

internal void
track_instruction_in_fragment
(
	RISCV_Instruction *instruction,
	Fragment          *fragment,
	U32                offset
)
{
	instruction->fragment  = fragment;
	instruction->offset    = offset;

	if (instruction->fixup)
	{
		instruction->fixup->fragment        = fragment;
		instruction->fixup->encoding_offset = offset;
	}
}

internal void
add_instruction_relaxed
(
	Arena                   *arena,
	Fragment_List           *fragments,
	RISCV_Instruction       *instruction,
	U8                       worst_case_size,
	U8                       best_case_size,
	U32                      expression_index,
	U32                      subtype
)
{
	U32 offset = fragments->last->size_fixed;
	track_instruction_in_fragment(instruction, fragments->last, offset);

	U8 *data = Fragment_List__variable
	(
		 fragments,
		 arena,
		 instruction->location,
		 worst_case_size,
		 best_case_size,
		 expression_index,
		 subtype,
		 Relax_State__Machine
	);

	memory_copy(data, (U8 *)&instruction->encoding, RISCV_instruction_size(instruction->encoding));
	return;
}

internal void
add_instruction_fixed
(
	Arena                   *arena,
	Fragment_List           *fragments,
	RISCV_Instruction       *instruction
)
{
	U32 offset = fragments->last->size_fixed;
	track_instruction_in_fragment(instruction, fragments->last, offset);

	U8 instruction_size = RISCV_instruction_size(instruction->encoding);
	U8 *data = Fragment_List__fixed
	(
		 fragments,
		 arena,
		 instruction->location,
		 instruction_size
	);

	memory_copy(data, (U8 *)&instruction->encoding, instruction_size);
	return;
}

internal void
RISCV_Instruction__append
(
	Arena                   *arena,
	Token_Cursor            *cursor,
	Diagnostic_List         *diagnostics,
	Expressions             *expressions,
	Symbols_Table           *symbols_table,
	Section                 *section,

	RISCV_Instruction       *instruction,
	U32                      expression_index,
	U16			 relocation
)
{
	if (relocation)
	{
		B32 jump_is = relocation == Relocation_RISC_V__JAL;
		if (relocation == Relocation_RISC_V__Branch || jump_is)
		{
			// Add a relaxable fragment and that's it. Don't create a fixup yet because this relocation type
			// could be changed and these instructions could expand unpredictably.
			U8 best_case_size  = RISCV_instruction_size(instruction->encoding);
			U8 worst_case_size = 8;

			U32 subtype = RELAX_BRANCH_ENCODE (jump_is, best_case_size == 2, worst_case_size);
			add_instruction_relaxed
			(
				arena,
				&section->fragment_list,
				instruction,
				worst_case_size,
				best_case_size,
				expression_index,
				subtype
			);

		}
		else
		{
			// TODO: Create fixup, with HOWTO information.
		}
	}
	else
	{
		add_instruction_fixed
		(
			arena,
			&section->fragment_list,
			instruction
		);
	}
}

/// ParseStatement:
///   ::= EndOfStatement
///   ::= Label* Directive ...Operands... EndOfStatement
///   ::= Label* Identifier OperandList* EndOfStatement
internal void
statement_read
(
	Arena                   *arena,
	Token_Cursor            *cursor,
	Diagnostic_List         *diagnostics,
	Expressions             *expressions,
	Symbols_Table           *symbols_table,
	Section                 *section,
	Sections_Table          *sections_table,
	Fixups                  *fixups
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

		if (instruction_hash)
		{
			U16               relocation       =  0;
			RISCV_Instruction instruction      = {0};
			U32               expression_index =  0;

			RISCV_Instruction__parse
			(
				arena,
			 	cursor,
			 	diagnostics,
			 	expressions,
				symbols_table,
				instruction_hash,
				&relocation,
				&instruction,
				&expression_index
			);

			RISCV_Instruction__append
			(
				arena,
				cursor,
				diagnostics,
				expressions,
				symbols_table,
				section,
				&instruction,
				expression_index,
				relocation
			);

			// TODO: this should be done later by also checking the relocation_out
			// U8 *data = Fragment_List__fixed(&section->fragment_list, arena, location_begin, 4);
			// memory_copy(data, &instruction.encoding, 4);
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
			// .fill repeat [, size [, value ]]. See GNU as `s_fill` in `read.c`.
			token_next(cursor, diagnostics, arena);
			U64 location_begin = cursor->current.location;

			Expression_Node *repeat_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);

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
			Fragment_List__fill(&section->fragment_list, arena, location_begin, repeat_expression->index, fill_pattern, fill_size);
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

			token_next(cursor, diagnostics, arena);
			U32 location_alignment_expression_begin = cursor->current.location;
			Expression_Node *alignment_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
			U32 location_alignment_expression_end   = cursor->current.location;
			expression_evaluate(expressions, alignment_expression->index);

			if (alignment_expression->evaluation != Expression_Kind__Constant)
			{
				Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
				diagnostic->location = location_alignment_expression_begin;
				diagnostic->message  = String8__literal("constant expression expected");
				diagnostic->ranges[0] = (Range1_U32){{ location_alignment_expression_begin, location_alignment_expression_end }};
				SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
			}

			if (cursor->current.kind == Token_Kind__Comma)
			{
				// Read pattern
				token_next(cursor, diagnostics, arena);

				U32 location_expression_begin = cursor->current.location;
				Expression_Node *pattern_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
				U32 location_expression_end   = cursor->current.location;

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
				U32 location_expression_begin = cursor->current.location;
				Expression_Node *bytes_max_expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
				U32 location_expression_end   = cursor->current.location;
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

			Fragment_List__align(&section->fragment_list, arena, location_begin, alignment_expression->index, pattern, bytes_max);
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
