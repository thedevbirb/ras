// Implementation notes.
//
// # Absolute Symbols and Expressions
//
// The definition of an absolute symbol is the following:
//
// A symbol is said to be absolute when its numeric value is invariant under relocation, that is, its value does not
// change regardless of where any section is loaded in memory. An absolute symbol is placed into the
// `ELF_Section_Index__Absolute` section inside the object file.
//
// More concretely, it means after relaxation it doesn't contain no residual dependency on any section base address, so
// it must reduce to a pure numeric constant.
//
// While this theoretical definition is helpful, transforming it into an algorithm is not as easy as it sounds. First,
// absolute symbols require a definition using a directive like `.set` or `.equ`. Then, the expression that defines the
// symbol must be recursively inspected and resolved, until we reach the base case consisting of numerical evaluation or
// subtraction of local labels defined within the same section.
//
// Since labels may change their section offset during the relaxation process, due to instruction expansion, we have to
// recompute the value of these symbols in every iteration. One thing that can be done is tracking whether labels occur.
//
// Similarly, it is helpful to define whether an expression is absolute. Again, it must only contain operations between
// absolute symbols, numeric literals, or subtraction between locally defined labels within the same section.
//
// ## Optimization
//
// Consider the following example:
//
// ```
// .set A, B
// .set B, ~C
// .set C, 1
// ```
//
// The algorithm described above will evaluate the expression which defines `A`, sees a definition of `B` and tries to
// evaluate it, and so on. After the process is finished, we will discover that `A` is indeed absolute, however it must
// be that every symbol definition encountered along the way is also absolute, meaning we can already set `B` and `C` as
// absolute as well.

void
Resolver_error_set(Resolver *resolver, Resolver_Error_Kind kind)
{
	// TODO: set other fields as well, for example the token where it happened.
	if (!resolver->error.kind)
	{
		resolver->error.kind      = kind;
		resolver->error.statement = resolver->statement_current;

		Token token_begin      = resolver->tokens[resolver->statement_current->token_index_begin];
		Token token_end        = resolver->tokens[resolver->statement_current->token_index_end];
		U32 row_index          = token_begin.row_index;
		U32 column_index_begin = token_begin.column_index;
		U32 column_index_end   = token_end.column_index + token_end.size;

		resolver->error.row_index          = row_index;
		resolver->error.column_index_begin = column_index_begin;
		resolver->error.column_index_end   = column_index_end;
	}

#ifdef ASSEMBLY_EXPECT_PANIC
	assert_always_m(0 && "panic on expect");
#endif

	return;
}

void
Resolver_expect(Resolver *resolver, B32 condition, Resolver_Error_Kind kind)
{
	if (!condition && !resolver->error.kind)
	{
		Resolver_error_set(resolver, kind);
	}

	return;
}

internal String8
Resolver_token_string_from_index(Resolver *resolver, U32 token_index)
{
	Token token = resolver->tokens[token_index];
	String8 string =
	{
		.data  = resolver->input->data + token.index,
		.count = (U64)token.size,
	};
	return string;
}

internal void
Resolver_advance(Resolver *resolver)
{
	resolver->statements_end_reached = resolver->statement_index + 1 == resolver->statements->count;
	resolver->statement_index       += !resolver->statements_end_reached;
	resolver->statement_current      = &resolver->statements->data[resolver->statement_index];

	return;
}

internal void
Resolver_cursor_reset(Resolver *resolver)
{
	resolver->statements_end_reached = 0 >= resolver->statements->count;
	resolver->statement_index        = 0;
	resolver->statement_current      = &resolver->statements->data[0];

	return;
}

internal S64
Resolver_operation_evaluate(Resolver *resolver, Expression_Kind kind, S64 a, S64 b)
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

	default: { Resolver_error_set(resolver, Resolver_Error_Kind__Expression_Kind_Unknown); } break;
	}

	return result;
}

// Notes on relocation when difference between labels, e.g. label_1 - label_2 is involved.
//
// Unless both symbols are of the same section, local, and it is explicitly set .option norelax for that
// statement, you have to emit a pair of relocation:
//
// 1. R_RISCV_ADD*, for label_1
// 2. R_RISCV_SUB*, for label_2
//
// Moreover, on R_RISCV_RELAX, the condition is: .option relax is enabled and the relocation is on an
// instruction that participates in a recognized relaxation pattern.
//
// As such, a statement can have these relocation patterns:
//
// 1. A R_RISCV_RELAX, in case the instruction is suitable and .option relax is enabled.
// 2. A specific relocation, or a R_RISCV_ADD/SUB pair.

void
Resolver_expression_evaluate(Resolver *resolver, Expression_Node *node)
{
	local_persist U8 recursion_level = 0;
	recursion_level += 1;

	assert_always_m(node && "cannot evaluate null expression");
	assert_always_m(node->kind && "cannot evaluate unknown expression kind");
	// TODO: print error immediately, and then exit.
	assert_always_m(recursion_level < expression_recursion_max && "max recursion");

	Expression_Kind kind = node->kind;

	switch (kind)
	{
	case Expression_Kind__None:            {} break;
	case Expression_Kind__Number_Literal:  {} break;
	case Expression_Kind__Char_Literal:    {} break;
	case Expression_Kind__Identifier:
	{
		Symbols_Table_Entry *symbol = node->symbols_table_entry;
		assert_always_m(symbol && "parser didn't set expression entry");

		B32 declared = symbol->flags & Symbol_Flags__Declared;
		B32 cyclic   = declared && symbol->flags & Symbol_Flags__Resolving;
		Resolver_expect(resolver, !cyclic, Resolver_Error_Kind__Symbol_Cyclic);

		Statement *statement = &resolver->statements->data[symbol->index_statement];
		B32 definition_is = statement->directive_kind == Directive_Kind__Equality
			        ||  statement->directive_kind == Directive_Kind__Set;

		if (definition_is && !cyclic)
		{
			// Search its definition, and evaluate it.
			Expression_Node *inner = &resolver->expressions->data[statement->expressions_indexes[0]];
			Resolver_expression_evaluate(resolver, inner);

			B32 symbol_absolute_is = definition_is && !(inner->flags & Expression_Flags__Unresolved);

			if (symbol_absolute_is)
			{
				symbol->elf.section_index = ELF_Section_Index__Absolute;
				node->integer_value = inner->integer_value;
			}

			node->flags |= inner->flags;
		}
		else
		{
			node->flags |= Expression_Flags__Unresolved;
		}
	} break;
	case Expression_Kind__Label_Numeric_Reference_Forward:
	{
		U16 section_current_index = resolver->section_current_index;
		U32 label_numeric_value   = node->label_numeric_value; // e.g. 1b or 2b etc.
		assert_always_m(label_numeric_value <= label_numeric_max);

		B32 match  = 0;
		U32 offset = 0;
		U32 index  = resolver->statement_index;
		for (;;)
		{
			index += 1;
			B32 break_should = match || index >= resolver->statements->count;
			if (break_should)
			{
				break;
			}
			Statement *statement = &resolver->statements->data[index];
			match = statement->kind == Statement_Kind__Label_Numeric && statement->label_numeric_value == label_numeric_value;
			B32 crossed = match && statement->section_index != section_current_index;
			Resolver_expect(resolver, !crossed, Resolver_Error_Kind__Label_Numeric_Section_Cross);
			offset = statement->section_offset;
		}

		Resolver_expect(resolver, match, Resolver_Error_Kind__Label_Numeric_Forward_Not_Found);

		node->integer_value = offset;
		node->flags |= Expression_Flags__Unresolved;

	} break;
	case Expression_Kind__Label_Numeric_Reference_Backward:
	{
		U16 section_current_index      = resolver->section_current_index;
		U32 label_numeric_value        = node->label_numeric_value; // e.g. 1b or 2b etc.
		assert_always_m(label_numeric_value <= label_numeric_max);

		Vec2_U32 statement_index_maybe = resolver->labels_numeric_statement_index[label_numeric_value];

		Resolver_expect(resolver, statement_index_maybe.y, Resolver_Error_Kind__Label_Numeric_Backward_Not_Found);
		Statement *statement = &resolver->statements->data[statement_index_maybe.x];
		Resolver_expect(resolver, statement->section_index == section_current_index, Resolver_Error_Kind__Label_Numeric_Section_Cross);

		node->integer_value = statement_index_maybe.x;
		node->flags |= Expression_Flags__Unresolved;

	} break;
	case Expression_Kind__Current_Address:
	{
		U32 section_current_offset = resolver->sections_offset[resolver->section_current_index];
		node->integer_value = section_current_offset;
		node->flags |= Expression_Flags__Unresolved;
	} break;
	case Expression_Kind__Relocation:
	{
		// TODO: redo all of this, probably just check valid expression inside and that's it.
		// Creating the relocation will be done at a later pass.

		Expression_Node *inner = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, inner);
		B32 unresolved = (inner->flags & Expression_Flags__Unresolved);
		// TODO: gas supports some relocation prefixes with absolute values. For example,
		// `%hi/lo(1234)` is accepted, and a relocation is not emitted. I think this is NOT desired.
		Resolver_expect(resolver, unresolved, Resolver_Error_Kind__Relocation_Operator_Absolute_Invalid);
		B32 symbol_single = inner->symbol_operand == 0;
		Resolver_expect(resolver, symbol_single, Resolver_Error_Kind__Relocation_Operator_Expression_Invalid);

		assert_always_m(inner->symbols_table_entry && "unresolved expression with no symbol");
		// This node absorbs the symbol and the value of the inner node.
		node->symbols_table_entry = inner->symbols_table_entry;
		node->integer_value       = inner->integer_value;
		node->flags |= Expression_Flags__Unresolved;

		// TODO: gas doesn't check that a `pcrel_lo` label points to an appropriate `pcrel_hi`, or it allows an
		// addend within it, leaving the heavy duty on the linker with basically no validation.

	} break;

	case Expression_Kind__Negate:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, node_left);

		B32 invalid = node_left->flags & Expression_Flags__Unresolved;
		Resolver_expect(resolver, !invalid, Resolver_Error_Kind__Operator_Expression_Unresolved);

		node->flags |= node_left->flags;
		node->integer_value = ~(node_left->integer_value - 1);
	} break;
	case Expression_Kind__Bitwise_Not:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, node_left);

		B32 invalid = node_left->flags & Expression_Flags__Unresolved;
		Resolver_expect(resolver, !invalid, Resolver_Error_Kind__Operator_Expression_Unresolved);

		node->flags |= node_left->flags;
		node->integer_value = ~node_left->integer_value;
	} break;
	case Expression_Kind__Logical_Not:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, node_left);

		B32 invalid = node_left->flags & Expression_Flags__Unresolved;
		Resolver_expect(resolver, !invalid, Resolver_Error_Kind__Operator_Expression_Unresolved);

		node->flags |= node_left->flags;
		node->integer_value = !node_left->integer_value;
	} break;
	default:
	{

		Expression_Node *node_right = &resolver->expressions->data[node->index_right];
		Expression_Node *node_left  = &resolver->expressions->data[node->index_left];

		Resolver_expression_evaluate(resolver, node_right);
		Resolver_expression_evaluate(resolver, node_left);

		Relocation_Operator relocation_operator_right = node_right->relocation_operator;
		Relocation_Operator relocation_operator_left = node_left->relocation_operator;

		// Propagating relocation operators is important for relocation emission later without further
		// expression investigation.
		assert_always_m(node_right->relocation_operator == 0 || node_left->relocation_operator == 0 && "parser didn't catch multiple relocation operators");
		node->relocation_operator = relocation_operator_right;
		node->relocation_operator = relocation_operator_left;

		B32 unresolved_right = node_right->flags & Expression_Flags__Unresolved;
		B32 unresolved_left  = node_left->flags  & Expression_Flags__Unresolved;

		Symbols_Table_Entry *symbol_right = node_right->symbols_table_entry;
		Symbols_Table_Entry *symbol_left  = node_left->symbols_table_entry;

		Symbols_Table_Entry *symbol_operand_right = node_right->symbol_operand;
		Symbols_Table_Entry *symbol_operand_left  = node_left->symbol_operand;

		// After here, either we have two leafs that we can evaluate straight away or if we have a subexpression
		// it has been evaluated.

		// This is always okay to perform, no harm.
		S64 integer_result  = Resolver_operation_evaluate(resolver, node->kind, node_right->integer_value, node_left->integer_value);
		node->integer_value = integer_result;

		if (unresolved_left && unresolved_right)
		{
			Resolver_expect(resolver, node->kind == Expression_Kind__Subtract, Resolver_Error_Kind__Operator_Between_Symbols_Invalid);
			Resolver_expect(resolver, symbol_operand_right == 0 && symbol_operand_left == 0, Resolver_Error_Kind__Operator_Expression_Unresolved);
			Resolver_expect(resolver, !node->relocation_operator, Resolver_Error_Kind__Relocation_Expression_Invalid);

			B32 local_left     = ELF_Symbol_bind_m(symbol_left->elf.type_and_binding)  == ELF_Symbol_Binding__Local;
			B32 local_right    = ELF_Symbol_bind_m(symbol_right->elf.type_and_binding) == ELF_Symbol_Binding__Local;
			B32 relax_disabled = resolver->statement_current->flags & Statement_Flags__Relax_Disabled;
			B32 section_same   = symbol_left->elf.section_index == symbol_right->elf.section_index;
			B32 evaluate       = local_left && local_right && relax_disabled && section_same;
			if (evaluate)
			{
				node->integer_value = node_left->integer_value - node_right->integer_value;
				// Symbols can be safely cancelled.
				node->symbols_table_entry = 0;
				node->symbol_operand      = 0;

				node->flags &= ~Expression_Flags__Unresolved;
			}
			else
			{
				node->flags |= Expression_Flags__Unresolved;
				node->symbols_table_entry = symbol_left;
				node->symbol_operand = node_right->symbols_table_entry;
			}
		}
		else if (unresolved_left)
		{
			node->flags |= Expression_Flags__Unresolved;
			node->symbols_table_entry = symbol_left;
			node->symbol_operand = symbol_operand_left;
		}
		else if (unresolved_right)
		{
			node->flags |= Expression_Flags__Unresolved;
			node->symbols_table_entry = symbol_right;
			node->symbol_operand = symbol_operand_right;
		}
	} break;
	}

	recursion_level -= 1;

	return;
}


internal void
Resolver_offsets_recompute(Resolver *resolver)
{
	U32 section_offsets[ELF_Section__COUNT] = {0};

	for (;;)
	{
		B32 break_should = resolver->statements_end_reached;
		if (break_should)
		{
			break;
		}
		Statement *statement      = resolver->statement_current;
		U32 *section_offset       = &section_offsets[statement->section_index];
		statement->section_offset = *section_offset;

		// if (statement->kind == Statement_Kind__Label)
		// {
		// 	String8 symbol_key = Token_string(resolver->tokens[statement->token_index_begin]);
		// 	Symbols_Table_Entry *entry = &resolver->symbols_table->entries[statement->label_symbol_slot];
		// 	entry->elf.value = statement->section_offset;
		// }

		*section_offset += statement->size;
		Resolver_advance(resolver);
	}
}

// TODO: check whether some relocations are possible within certain statements. For example, the relocation pairs
// ADD/SUB can be emitted only in data directives.
internal B32
Resolver_relax_pass(Resolver *resolver)
{
	os_memory_zero(resolver->labels_numeric_statement_index, label_numeric_max);

	B32 changed = 0;
	for (;;)
	{
		B32 break_should = resolver->statements_end_reached || resolver->error.kind;
		if (break_should)
		{
			break;
		}

		Statement *statement          = resolver->statement_current;
		Directive_Kind directive_kind = statement->directive_kind;

		if (statement->kind == Statement_Kind__Label_Numeric)
		{
			resolver->labels_numeric_statement_index[statement->label_numeric_value]
				= (Vec2_U32){ .x = resolver->statement_index, .y = 1 };
		}

		U32 size_old = statement->size;
		U32 size_new = size_old;

		switch (directive_kind)
		{
		case Directive_Kind__Set: {} // fallthrough
		case Directive_Kind__Equality:
		{
			U32 symbol_token_index = statement->token_index_begin + 1;
			Token symbol_token = resolver->tokens[symbol_token_index];
			String8 symbol_string =
			{
				.data  = &resolver->input->data[symbol_token.index],
				.count = symbol_token.size,
			};
			Symbols_Table_Entry *symbol = Symbols_Table_get(resolver->symbols_table, symbol_string);

			if (symbol->elf.section_index != ELF_Section_Index__Absolute)
			{
				Expression_Node *expression = &resolver->expressions->data[statement->expressions_indexes[0]];
				symbol->flags |= Symbol_Flags__Resolving;
				Resolver_expression_evaluate(resolver, expression);
				symbol->flags &= ~Symbol_Flags__Resolving;

				B32 absolute_expression = !(expression->flags & Expression_Flags__Unresolved);
				if (absolute_expression)
				{
					symbol->elf.section_index = ELF_Section_Index__Absolute;
				}
				Resolver_expect(resolver, expression->symbol_operand == 0, Resolver_Error_Kind__Expression_Symbol_Operand);
			}
		} break;
		case Directive_Kind__Word_Double: {} // fallthrough
		case Directive_Kind__Word:        {} // fallthrough
		case Directive_Kind__Word_Half:   {} // fallthrough
		case Directive_Kind__Byte:
		{
			B32 byte_or_word_half = directive_kind == Directive_Kind__Byte
				             || directive_kind == Directive_Kind__Word_Half;
			U32 index = 0;
			for (;;)
			{
				B32 break_should = index >= statement->expressions_count;
				if (break_should)
				{
					break;
				}
				U32 expression_index        = statement->expressions_indexes[index];
				Expression_Node *expression = &resolver->expressions->data[expression_index];
				Resolver_expression_evaluate(resolver, expression);
				if (byte_or_word_half)
				{
					// No relocations types exist for these two, hence the error.
					B32 absolute = !(expression->flags & Expression_Flags__Unresolved);
					Resolver_expect(resolver, absolute, Resolver_Error_Kind__Expression_Unresolved);
				}
			}
		} break;
		// .align computes padding based on current offset.
		// Padding = bytes needed to reach next alignment boundary.
		case Directive_Kind__Align: {} // fallthrough, since most logic is shared with .skip.
		case Directive_Kind__Skip:
		{
			U32 expression_index        = statement->expressions_indexes[0];
			Expression_Node *expression = &resolver->expressions->data[expression_index];
			Resolver_expression_evaluate(resolver, expression);

			B32 absolute = !(expression->flags & Expression_Flags__Unresolved);
			Resolver_expect(resolver, absolute, Resolver_Error_Kind__Expression_Unresolved);

			if (statement->expressions_count > 1)
			{
				U32 expression_index        = statement->expressions_indexes[1];
				Expression_Node *expression = &resolver->expressions->data[expression_index];
				Resolver_expression_evaluate(resolver, expression);
			}

			size_new = expression->integer_value;
			if (directive_kind == Directive_Kind__Align)
			{
				U32 alignment = 1u << (U32)expression->integer_value;
				U32 offset    = statement->section_offset;
				U32 remainder = offset & (alignment - 1);
				size_new = remainder == 0 ? 0 : alignment - remainder;
			}
		} break;
		case Directive_Kind__Zero:
		{
			U32 expression_index        = statement->expressions_indexes[0];
			Expression_Node *expression = &resolver->expressions->data[expression_index];
			Resolver_expression_evaluate(resolver, expression);

			B32 absolute = !(expression->flags & Expression_Flags__Unresolved);
			Resolver_expect(resolver, absolute, Resolver_Error_Kind__Expression_Unresolved);

			size_new = expression->integer_value;
		} break;

		// Ignored directives, that we keep for exhaustive match warnings.
		case Directive_Kind__None:           {} break;
		case Directive_Kind__Section:        {} break;
		case Directive_Kind__Text:           {} break;
		case Directive_Kind__Data:           {} break;
		case Directive_Kind__Read_Only_Data: {} break;
		case Directive_Kind__BSS:            {} break;
		case Directive_Kind__Local:          {} break;
		case Directive_Kind__Globl:          {} break;
		case Directive_Kind__Global:         {} break;
		case Directive_Kind__Ascii:          {} break;
		case Directive_Kind__Asciz:          {} break;
		case Directive_Kind__String:         {} break;
		case Directive_Kind__Common:         {} break;
		case Directive_Kind__Option:         {} break;
		case Directive_Kind__COUNT:          {} break;
		}

		switch (statement->instruction_kind)
		{
		case Instruction_Kind__LI:
		{
			// If the immediate fits in 12 bits, sign-extended, a single addi suffices.
			// Otherwise, we need a lui + addi, for 8 bytes total.

			U32 expression_index             = statement->expressions_indexes[0];
			Expression_Node *expression      = &resolver->expressions->data[expression_index];

			Resolver_expression_evaluate(resolver, expression);
			B32 absolute = !(expression->flags & Expression_Flags__Unresolved);
			Resolver_expect(resolver, absolute, Resolver_Error_Kind__Expression_Unresolved);

			S64 immediate = (S64)expression->integer_value;

			size_new = 24; // Worst-case for rv32 is 8.
			if (-(1 << 11) <= immediate && immediate <= (1 << 11) - 1)
			{
				size_new = 4;
			}
			else if (-(1LL << 31) <= immediate && immediate <= (1LL << 31) -1)
			{
				size_new = 8;
			}
		} break;
		case Instruction_Kind__J:
		{
			// Expands to `jal, x0, immediate`. In code, an unresolved expression can be in place, and e JAL
			// relocation is emitted.

			U32 expression_index         = statement->expressions_indexes[0];
			Expression_Node *expression  = &resolver->expressions->data[expression_index];
			Resolver_expression_evaluate(resolver, expression);

			// B32 unresolved = expression->flags & Expression_Flags__Unresolved;
			// S64 delta = (S64)expression->integer_value - statement->section_offset;
			// B32 range_in = -(1 << 20) <= delta && delta <= (1 << 20) - 1;
			// Resolver_expect(resolver, unresolved || range_in, Resolver_Error_Kind__Immediate_Large);
			//
			// if (unresolved)
			// {
			//
			// }
		} break;
		case Instruction_Kind__TAIL:
		{
			// Same logic as CALL but uses zero instead of ra.
			// jal zero, offset            -> 4 bytes (±1 MiB range)
			// auipc t1, upper + jalr zero -> 8 bytes
		} // fallthrough
		case Instruction_Kind__CALL:
		{
			// jal has a 21-bit signed offset range. If the target is not within that range, than we need an
			// auipc + jalr, for 8 bytes total.

			U32 expression_index         = statement->expressions_indexes[0];
			Expression_Node *expression  = &resolver->expressions->data[expression_index];

			Resolver_expression_evaluate(resolver, expression);
			S64 delta = (S64)expression->integer_value - statement->section_offset;

			B32 range_in = -(1 << 20) <= delta && delta <= (1 << 20) - 1;
			if (expression->flags & Expression_Flags__Unresolved || !range_in)
			{
				size_new = 8;
			}
		} break;
		case Instruction_Kind__BEQ:  {} // fallthrough
		case Instruction_Kind__BNE:  {} // fallthrough
		case Instruction_Kind__BLT:  {} // fallthrough
		case Instruction_Kind__BGE:  {} // fallthrough
		case Instruction_Kind__BLTU: {} // fallthrough
		case Instruction_Kind__BGEU: {} // fallthrough
		{
			// Conditional branches have a 4 KiB range (13-bit signed offset).
			// If the target is within range:
			//   bxx rs1, rs2, offset -> 4 bytes
			// If the target is out of range, invert and jump:
			//   bxx_inv rs1, rs2, +8; jal zero, offset -> 8 bytes  (if jal range suffices)
			//   bxx_inv rs1, rs2, +12; auipc + jalr    -> 12 bytes (if beyond jal range too)
			U32 expression_index        = statement->expressions_indexes[0];
			Expression_Node *expression = &resolver->expressions->data[expression_index];
			Resolver_expression_evaluate(resolver, expression);

			size_new = 12;
			if (!(expression->flags & Expression_Flags__Unresolved))
			{
				S64 delta = (S64)expression->integer_value - (S64)statement->section_offset;
				if (-(1 << 12) <= delta && delta <= (1 << 12) - 1)
				{
					size_new = 4;
				}
				else if (-(1 << 20) <= delta && delta <= (1 << 20) - 1)
				{
					size_new = 8;
				}
			}
		} break;
		default: {} break;
		}

		if (size_new > size_old)
		{
			statement->size = size_new;
			changed = 1;
		}

		Resolver_advance(resolver);
	}

	return changed;
}

U32
Resolver_relax(Resolver *resolver)
{
	U32 pass_count = 0;
	for (;;)
	{
		pass_count += 1;
		Resolver_offsets_recompute(resolver);
		Resolver_cursor_reset(resolver);

		B32 changed = Resolver_relax_pass(resolver);
		Resolver_cursor_reset(resolver);

		if (!changed)
		{
			break;
		}
	}

	Resolver_offsets_recompute(resolver);
	return pass_count;
}

// ============================================================================
// RISC-V Assembler Relocation Decision Logic
// ============================================================================
//
// ELF Structures Reference (from <elf.h>):
// -----------------------------------------
//
//   Elf64_Sym / Elf32_Sym — Symbol table entry
//     .st_info   -> ELF64_ST_BIND() extracts binding: STB_LOCAL, STB_GLOBAL, STB_WEAK
//                -> ELF64_ST_TYPE() extracts type: STT_NOTYPE, STT_OBJECT, STT_FUNC, ...
//     .st_shndx  -> Section index where the symbol is defined:
//                     SHN_UNDEF   (0)      — symbol is not defined in this translation unit
//                     SHN_COMMON  (0xFFF2) — tentative (common) definition
//                     SHN_ABS     (0xFFF1) — absolute, not relocated
//                     1..N                 — index into the section header table
//     .st_value  -> Offset within the section (for defined symbols)
//     .st_size   -> Size of the symbol (optional, informational)
//
//   Elf64_Rela / Elf32_Rela — Relocation entry (RISC-V uses SHT_RELA exclusively)
//     .r_offset  -> Offset within the section where the relocation applies
//     .r_info    -> ELF64_R_SYM()  — index into the symbol table
//                -> ELF64_R_TYPE() — relocation type (R_RISCV_*)
//     .r_addend  -> Signed addend used in the relocation computation
//
//   Elf64_Shdr / Elf32_Shdr — Section header (for determining "same section")
//     .sh_type   -> SHT_RELA for relocation sections
//     .sh_link   -> Index of the associated symbol table
//     .sh_info   -> Index of the section to which relocations apply
//
// Relocation sections are named .rela.<section>, e.g. .rela.text, .rela.data.
// Each Elf64_Rela entry targets the section identified by the parent
// section header's sh_info field.
//
// ============================================================================
// Decision Procedure
// ============================================================================
//
// For each statement that references a symbol in an expression, the assembler
// walks the following cases in order. The FIRST matching case applies.
//
// ----------------------------------------------------------------------------
// CASE 1: Symbol is UNDEFINED (st_shndx == SHN_UNDEF)
// ----------------------------------------------------------------------------
//   -> Always emit a relocation. The linker must resolve this symbol.
//
//   Relocation type depends on the reference context:
//
//     Assembler modifier / context            Relocation type
//     ─────────────────────────────────────    ──────────────────────────
//     %hi(symbol)                             R_RISCV_HI20
//     %lo(symbol)        in I-type instr      R_RISCV_LO12_I
//     %lo(symbol)        in S-type instr      R_RISCV_LO12_S
//     %pcrel_hi(symbol)                       R_RISCV_PCREL_HI20
//     %pcrel_lo(label)   in I-type instr      R_RISCV_PCREL_LO12_I
//     %pcrel_lo(label)   in S-type instr      R_RISCV_PCREL_LO12_S
//     %got_pcrel_hi(symbol)                   R_RISCV_GOT_HI20
//     %tprel_hi(symbol)                       R_RISCV_TPREL_HI20
//     %tprel_lo(symbol)  in I-type instr      R_RISCV_TPREL_LO12_I
//     %tprel_lo(symbol)  in S-type instr      R_RISCV_TPREL_LO12_S
//     %tprel_add(symbol)                      R_RISCV_TPREL_ADD
//     %tls_ie_pcrel_hi(symbol)                R_RISCV_TLS_GOT_HI20
//     %tls_gd_pcrel_hi(symbol)                R_RISCV_TLS_GD_HI20
//     B-type branch target                    R_RISCV_BRANCH
//     J-type jump target (jal)                R_RISCV_JAL
//     call/tail pseudo-instruction            R_RISCV_CALL or R_RISCV_CALL_PLT
//     .word symbol                            R_RISCV_32
//     .dword symbol                           R_RISCV_64
//
//   Note on .byte/.half: An absolute symbol reference in a .byte or .half
//   directive has no standard RISC-V relocation type for 8-bit or 16-bit
//   absolute addresses. Most assemblers reject this as an error. Do not
//   confuse this with the R_RISCV_ADD8/SUB8 or ADD16/SUB16 pairs, which
//   apply only to symbol DIFFERENCES (see Case 7).
//
// ----------------------------------------------------------------------------
// CASE 2: Symbol is COMMON (st_shndx == SHN_COMMON)
// ----------------------------------------------------------------------------
//   -> Always emit a relocation. Common symbols are tentative definitions;
//      the linker merges them and assigns their final address.
//   Relocation type selection: same table as Case 1.
//
// ----------------------------------------------------------------------------
// CASE 3: Symbol is GLOBAL or WEAK (ELF64_ST_BIND(st_info) == STB_GLOBAL
//         or STB_WEAK), defined in this translation unit
// ----------------------------------------------------------------------------
//   -> Always emit a relocation.
//      - STB_GLOBAL: needed for shared library interposition (the dynamic
//        linker may redirect the symbol to a different definition).
//      - STB_WEAK: may be overridden by a strong definition from another
//        translation unit.
//      A non-PIC static assembler targeting a known-final executable could
//      in principle resolve some of these, but the safe and standard behavior
//      is to emit the relocation.
//   Relocation type selection: same table as Case 1.
//
// ----------------------------------------------------------------------------
// CASE 4: Symbol is LOCAL (STB_LOCAL), defined in a DIFFERENT section
// ----------------------------------------------------------------------------
//   -> Always emit a relocation. The assembler does not know the final
//      distance or offset between sections; only the linker does.
//   Relocation type selection: same table as Case 1.
//
// ----------------------------------------------------------------------------
// CASE 5: Symbol is LOCAL (STB_LOCAL), defined in the SAME section
// ----------------------------------------------------------------------------
//   The assembler knows the offset between the reference and the target
//   within the section. Whether it can resolve the reference depends on
//   the reference type and whether linker relaxation is active.
//
//   5a. ABSOLUTE reference (%hi, %lo, .word, .dword, etc.):
//       -> Emit a relocation. Even though the intra-section offset is known,
//          the absolute virtual address depends on where the linker places
//          the section.
//       Relocation types:
//         %hi(symbol)                          R_RISCV_HI20
//         %lo(symbol)        in I-type         R_RISCV_LO12_I
//         %lo(symbol)        in S-type         R_RISCV_LO12_S
//         .word symbol                         R_RISCV_32
//         .dword symbol                        R_RISCV_64
//
//   5b. PC-RELATIVE reference, linker relaxation ENABLED:
//       -> Emit the PC-relative relocation AND a companion R_RISCV_RELAX.
//          Relaxation passes may shrink or expand instructions between the
//          reference and the target (e.g., relaxing auipc+jalr into jal),
//          invalidating the assembler-computed offset.
//       Relocation types (each paired with R_RISCV_RELAX):
//         %pcrel_hi(symbol)                    R_RISCV_PCREL_HI20  + R_RISCV_RELAX
//         %pcrel_lo(label)   in I-type         R_RISCV_PCREL_LO12_I + R_RISCV_RELAX
//         %pcrel_lo(label)   in S-type         R_RISCV_PCREL_LO12_S + R_RISCV_RELAX
//         B-type branch target                 R_RISCV_BRANCH       + R_RISCV_RELAX
//         J-type jump target (jal)             R_RISCV_JAL          + R_RISCV_RELAX
//         call/tail pseudo-instruction         R_RISCV_CALL(_PLT)   + R_RISCV_RELAX
//
//   5c. PC-RELATIVE reference, linker relaxation DISABLED:
//       -> Do NOT emit a relocation. The assembler can compute the final
//          PC-relative offset directly: both reference and target are in the
//          same section, and no relaxation pass will alter the distance.
//          Encode the offset into the instruction's immediate field.
//
// ----------------------------------------------------------------------------
// CASE 6: Expression is a CONSTANT (no symbol reference, purely numeric)
// ----------------------------------------------------------------------------
//   -> Do NOT emit a relocation. Encode the value directly into the
//      instruction or data directive.
//
// ----------------------------------------------------------------------------
// CASE 7: Expression is a SYMBOL DIFFERENCE (sym_a - sym_b)
// ----------------------------------------------------------------------------
//   Symbol differences produce a pair of relocations (R_RISCV_ADDn +
//   R_RISCV_SUBn) so the linker can evaluate the subtraction after
//   final layout. The width of the pair matches the data context:
//
//     Context       ADD relocation    SUB relocation
//     ───────       ──────────────    ──────────────
//     .byte         R_RISCV_ADD8      R_RISCV_SUB8
//     .half         R_RISCV_ADD16     R_RISCV_SUB16
//     .word         R_RISCV_ADD32     R_RISCV_SUB32
//     .dword        R_RISCV_ADD64     R_RISCV_SUB64
//     6-bit (rare)  R_RISCV_SET6      R_RISCV_SUB6
//
//   Which sub-case applies:
//
//   7a. Both symbols are LOCAL, SAME section, relaxation DISABLED:
//       -> Do NOT emit a relocation. The difference is a fixed constant
//          that the assembler can compute at assembly time.
//
//   7b. Both symbols are LOCAL, SAME section, relaxation ENABLED:
//       -> Emit the ADDn + SUBn relocation pair. Relaxation may change the
//          distance between the two symbols.
//
//   7c. Symbols are in DIFFERENT sections, or either symbol is STB_GLOBAL,
//       STB_WEAK, SHN_COMMON, or SHN_UNDEF:
//       -> Emit the ADDn + SUBn relocation pair.
//
//       On UNDEFINED symbols in difference expressions:
//       The assembler CAN emit the pair when one or both symbols are
//       undefined (st_shndx == SHN_UNDEF). It emits R_RISCV_ADDn against
//       sym_a and R_RISCV_SUBn against sym_b, deferring resolution entirely
//       to the linker. However, the linker will typically require that both
//       symbols, once resolved, reside in the same output section — otherwise
//       the difference is not a link-time constant and will produce a link
//       error. In practice, GNU as permits this; the correctness check is
//       deferred to link time. This is uncommon in handwritten assembly but
//       can appear in compiler-generated metadata (e.g., exception tables,
//       DWARF debug info).
//
// ============================================================================
// Notes
// ============================================================================
//
// - RISC-V uses SHT_RELA sections exclusively (not SHT_REL). Every relocation
//   entry carries an explicit r_addend, even when it is zero.
//
// - The R_RISCV_RELAX relocation is a marker, not a standalone fix-up. It
//   occupies its own Elf64_Rela entry at the same r_offset as the primary
//   relocation, with r_addend = 0 and r_sym typically 0. It signals to the
//   linker that the associated instruction sequence is eligible for
//   relaxation.
//
// - %pcrel_lo(label) does NOT reference the target symbol directly. It
//   references a label on the instruction that carries the corresponding
//   %pcrel_hi relocation. The linker uses the %pcrel_hi's target to compute
//   the low 12 bits. This means the r_sym in the %pcrel_lo relocation entry
//   points to the label, not to the ultimate target.
//
// - The call/tail pseudo-instructions expand to an auipc+jalr pair. The
//   R_RISCV_CALL / R_RISCV_CALL_PLT relocation covers the full 8-byte
//   sequence. With relaxation enabled, the linker may replace this with a
//   single jal instruction (4 bytes) plus a NOP or compressed NOP, or
//   just a single jal if alignment permits.
//
// ============================================================================
// internal void
// relocation_emit(Statement *statement, Expression_Node *expression, Relocation_RISC_V kind)
// {
// 	ELF_Section section_index = ELF_Section_relocations[statement->section_index];
// 	Object_File_Section *section = &resolver->sections[section_index];
//
// 	ELF64_Relocation_Addend relocation =
// 	{
// 		.offset = statement->section_offset,
// 		.info   = ELF64_Relocation_info_m(symbol_index, relocation_kind),
// 		.addend = addend,
// 	};
//
// 	Object_File_Section_write(section, (U8 *)&relocation, sizeof(ELF64_Relocation_Addend));
// 	return;
// }

// Encode instructions and directives to object files, emitting relocations if needed.
//
// Assumes Resolver_relax has been called.
//
// TODO: clarify a bit better which errors are caught here and what during evaluation.
void
Resolver_encode(Resolver *resolver)
{
	Resolver_cursor_reset(resolver);
	Object_File_Section *section_current = &resolver->sections[ELF_Section__Text];

	U8 data_directive_size = 0;

	for (;;)
	{
		// TODO: ensure that immediate values calculated can fit the instruction encoding.
		B32 break_should = resolver->statements_end_reached || resolver->error.kind;
		if (break_should)
		{
			break;
		}

		Statement *statement          = resolver->statement_current;
		Directive_Kind directive_kind = statement->directive_kind;

		Object_File_Section *section            = &resolver->sections[statement->section_index];
		Object_File_Section *section_relocation = &resolver->sections[statement->section_index];

		switch (statement->directive_kind)
		{
		case Directive_Kind__Word_Double: { data_directive_size += 1; } // fallthrough
		case Directive_Kind__Word:        { data_directive_size += 1; } // fallthrough
		case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
		case Directive_Kind__Byte:
		{
			data_directive_size += 1;
			B32 byte_or_word_half = directive_kind == Directive_Kind__Byte
				             || directive_kind == Directive_Kind__Word_Half;

			U32 index = 0;
			for (;;)
			{
				B32 break_should = index >= statement->expressions_count || resolver->error.kind;
				if (break_should)
				{
					break;
				}
				U32 expression_index        = statement->expressions_indexes[index];
				Expression_Node *expression = &resolver->expressions->data[expression_index];
				if (expression->flags & Expression_Flags__Unresolved)
				{
					Relocation_RISC_V relocation_kind =
						directive_kind == Directive_Kind__Word        ? Relocation_RISC_V__32_Bit
					      : directive_kind == Directive_Kind__Word_Double ? Relocation_RISC_V__64_Bit
					      : 0;
					assert_always_m(relocation_kind);

					ELF64_Relocation_Addend relocation =
					{
						.offset = statement->section_offset,
						.info   = ELF64_Relocation_info_m(expression->symbols_table_entry, relocation_kind),
						.addend = expression->integer_value,
					};
					Object_File_Section_relocation_write(section_relocation, &relocation);
				}
				else
				{
					// TODO: conversions etc might bite me off here.
					S64 value = expression->integer_value;
					U64 limit_low  =  1 << (8 * data_directive_size - 1);
					U64 limit_high = (1 << (8 * data_directive_size)) - 1;
					B32 range_in = -limit_low <= value && value <= limit_high;
					// TODO: this could be a warning, and truncation could be performed.
					Resolver_expect(resolver, range_in, Resolver_Error_Kind__Expression_Value_Bounds_Outside);
					Object_File_Section_write_bytes(section, (U8 *)value, data_directive_size);
				}
				index += 1;
			}

			data_directive_size = 0;
		} break;
		case Directive_Kind__Align: {} // fallthrough
		case Directive_Kind__Skip:
		{
			U8 value = 0;
			if (statement->expressions_count > 1)
			{
				U32 expression_index = statement->expressions_indexes[1];
				Expression_Node *expression = &resolver->expressions->data[expression_index];
				if (expression->flags && Expression_Flags__Unresolved)
				{
					// Emit relocations for each byte
					Resolver_expect(resolver, expression->symbol_operand != 0, Resolver_Error_Kind__Relocation_Byte);
					assert_always_m(expression->kind == Expression_Kind__Subtract);
					U32 index = 0;
					for (;;)
					{
						B32 break_should = index >= statement->size;
						if (break_should)
						{
							break;
						}
						// NOTE: only one relocation will have a non-zero addend if
						// expression->integer_value is non-zero.
						ELF64_Relocation_Addend relocation_add =
						{
							.offset = statement->section_offset + index,
							.info   = ELF64_Relocation_info_m(expression->symbols_table_entry, Relocation_RISC_V__Add_8),
							.addend = expression->integer_value,
						};
						ELF64_Relocation_Addend relocation_sub =
						{
							.offset = statement->section_offset + index,
							.info   = ELF64_Relocation_info_m(expression->symbols_table_entry, Relocation_RISC_V__Sub_8),
							.addend = 0,
						};
						Object_File_Section_relocation_write(section_relocation, &relocation_add);
						Object_File_Section_relocation_write(section_relocation, &relocation_sub);

						index += 1;
					}
				}
				else
				{
					value = expression->integer_value;
				}
			}
			Object_File_Section_write_byte(section, value, statement->size);
		}
		case Directive_Kind__Zero:
		{
			Object_File_Section_write_byte(section, 0, statement->size);
		}

		// Ignored directives, that we keep for exhaustive match warnings.
		case Directive_Kind__None:
		{
			assert_always_m(resolver->statement_index == 0 && "sentinel statement");
		} break;
		case Directive_Kind__Section:        {} break;
		case Directive_Kind__Text:           {} break;
		case Directive_Kind__Data:           {} break;
		case Directive_Kind__Read_Only_Data: {} break;
		case Directive_Kind__BSS:            {} break;
		case Directive_Kind__Local:          {} break;
		case Directive_Kind__Globl:          {} break;
		case Directive_Kind__Global:         {} break;
		case Directive_Kind__Ascii:          {} break;
		case Directive_Kind__Asciz:          {} break;
		case Directive_Kind__String:         {} break;
		// Both .set and .equ are zero-sized instructions!
		case Directive_Kind__Set:            {} break;
		case Directive_Kind__Equality:       {} break;
		case Directive_Kind__Common:         {} break;
		case Directive_Kind__Option:         {} break;
		case Directive_Kind__COUNT:          {} break;
		}

		switch (statement->instruction_kind)
		{
		default:
		{
			assert_always_m(resolver->statement_index == 0 && "sentinel statement");
		} break;
		}

		Resolver_advance(resolver);
	}

	return;
}

// we want to have a more compact expression evaluation to find out whether the relocation syntax is valid. It must fit
// in a ELF64_Relocation_Addend, or add & sub pair, with both symbols being local and in the same section
//
// Examples:
// -  .word a + 2 - b
// -  la x1, symbol_1 * symbol_2
//
// We're reading an expression tree node:
//
// 1. If is resolved, gucci
// 2. If is not resolved, look at parent
//	- It can be addition, if the other end is evaluated e.g. case .word 2 + b
//	- It can always be subtraction e.g. (a + 2) - b

// TODO: check whether two symbols are in different sections, that needs relocation too

// I can start writing a encoding function that also emits relocation as part of the process.
// While encoding instruction themselves isn't nothing particularly challenging, its more complex to handle relocations.
// Relocations essentially happen when some specific relocator operators are found or when some symbols are undefined
// i.e. they belong to section index undefined. In such case, due to the relocation with addend entry layout `Elf_Rela`
// or `ELF_Relocation_Addend`, if the expression node is `unresolved`, then we need to check the form of such
// expressions. The evaluation algorithm tries to evaluate as much as possible, so it must reduce to something like
// `symbol + addend` or `addend + symbol`, where addend is a signed integer.

// Let's walk through both cases. I'll use R_RISCV_ADD32 and R_RISCV_SUB32 since these are .word directives.
// .word a - b
// The assembler emits 4 bytes initialized to zero at the current offset. It then creates two relocations, both pointing at that same offset:
//
// R_RISCV_ADD32 with symbol a, addend 0 — the linker adds the resolved value of a to the 4 bytes at the offset.
// R_RISCV_SUB32 with symbol b, addend 0 — the linker subtracts the resolved value of b from the 4 bytes at the offset.
//
// After the linker processes both, the 4 bytes contain a - b.
