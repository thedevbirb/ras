void
Resolver_error_set(Resolver *resolver, Resolver_Error_Kind kind)
{
	// TODO: set other fields as well, for example the token where it happened.
	if (!resolver->error.kind)
	{
		resolver->error.kind = kind;
	}
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

Expression_Evaluation
Resolver_expression_evaluate(Resolver *resolver, Expression_Node *node)
{

	assert_always_m(node && "cannot evaluate null expression");
	assert_always_m(node->kind && "cannot evaluate unknown expression kind");

	U64 value      = 0;
	B32 unresolved = 0;
	Expression_Kind kind = node->kind & ~((resolver->error.kind == 0) - 1);

	switch (kind)
	{
	case Expression_Kind__None:            {} break;

	case Expression_Kind__Number_Literal:  {  value =  node->integer_value; } break;
	case Expression_Kind__Char_Literal:    {  value =  node->integer_value; } break;
	case Expression_Kind__Identifier:
	{
		String8 key = Resolver_token_string_from_index(resolver, node->token_index);
		Symbols_Table_Entry *entry = Symbols_Table_get(resolver->symbols_table, key);
		if (entry && entry->value.section_index)
		{
			value = entry->value.value;
		}
		else
		{
			unresolved = 1;
		}

	} break;
	case Expression_Kind__Current_Address:
	{
		U32 section_current_offset = resolver->sections_offset[resolver->section_current_index];
		node->integer_value = section_current_offset;
	} break;
	case Expression_Kind__Relocation:      {  assert_always_m(0 && "todo"); } break;

	case Expression_Kind__Negate:
	{
		Expression_Node *node_left       = &resolver->expressions->data[node->index_left];
		Expression_Evaluation evaluation = Resolver_expression_evaluate(resolver, node_left);
		unresolved = evaluation.unresolved;
		value = (unresolved - 1) & ~(evaluation.value - 1);
	} break;
	case Expression_Kind__Bitwise_Not:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Expression_Evaluation evaluation = Resolver_expression_evaluate(resolver, node_left);
		unresolved = evaluation.unresolved;
		value = (unresolved - 1) & ~evaluation.value;
	} break;
	case Expression_Kind__Logical_Not:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Expression_Evaluation evaluation = Resolver_expression_evaluate(resolver, node_left);
		unresolved = evaluation.unresolved;
		value = (unresolved - 1) & !evaluation.value;
	} break;
	default:
	{
		Expression_Node *node_right = &resolver->expressions->data[node->index_right];
		Expression_Evaluation right = Resolver_expression_evaluate(resolver, node_right);
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Expression_Evaluation left = Resolver_expression_evaluate(resolver, node_left);

		unresolved = right.unresolved || left.unresolved;
		if (!unresolved)
		{
			switch (node->kind)
			{
			case Expression_Kind__Add:           { value = left.value +  right.value; } break;
			case Expression_Kind__Subtract:      { value = left.value -  right.value; } break;
			case Expression_Kind__Multiply:      { value = left.value *  right.value; } break;
			case Expression_Kind__Divide:        { value = left.value /  right.value; } break;
			case Expression_Kind__Modulo:        { value = left.value %  right.value; } break;

			case Expression_Kind__Bitwise_Or:    { value = left.value |  right.value; } break;
			case Expression_Kind__Bitwise_Xor:   { value = left.value ^  right.value; } break;
			case Expression_Kind__Bitwise_And:   { value = left.value &  right.value; } break;
			case Expression_Kind__Shift_Left:    { value = left.value << right.value; } break;
			case Expression_Kind__Shift_Right:   { value = left.value >> right.value; } break;

			case Expression_Kind__Equal:         { value = left.value == right.value; } break;
			case Expression_Kind__Not_Equal:     { value = left.value != right.value; } break;
			case Expression_Kind__Less_Than:     { value = left.value <  right.value; } break;
			case Expression_Kind__Less_Equal:    { value = left.value <= right.value; } break;
			case Expression_Kind__Greater_Than:  { value = left.value >  right.value; } break;
			case Expression_Kind__Greater_Equal: { value = left.value >= right.value; } break;

			case Expression_Kind__Logical_And:   { value = left.value && right.value; } break;
			case Expression_Kind__Logical_Or:    { value = left.value || right.value; } break;

			default: { Resolver_error_set(resolver, Resolver_Error_Kind__Expression_Kind_Unknown); } break;
			}
		}
	} break;
	}

	Expression_Evaluation evaluation = { .value = value, .unresolved = unresolved };

	return evaluation;
}


internal void
Resolver_offsets_recompute(Resolver *resolver)
{
	U32 section_offsets[ELF64_Section__COUNT] = {0};

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= resolver->statements->count;
		if (break_should)
		{
			break;
		}
		Statement statement      = resolver->statements->data[index];
		U32 *section_offset      = &section_offsets[statement.section_index];
		statement.section_offset = *section_offset;

		if (statement.kind == Statement_Kind__Label)
		{
			Symbols_Table_Entry *entry = &resolver->symbols_table->entries[statement.label_symbol_slot];
			entry->value.value = statement.section_offset;
		}

		*section_offset += statement.size;
		index += 1;
	}
}

internal B32
Resolver_relax_pass(Resolver *resolver)
{
	B32 changed = 0;
	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= resolver->statements->count;
		if (break_should)
		{
			break;
		}

		Statement *statement = &resolver->statements->data[index];

		U32 size_old = statement->size;
		U32 size_new = size_old;

		switch (statement->instruction_kind)
		{
		case Instruction_Kind__LI:
		{
			// If the immediate fits in 12 bits, sign-extended, a single addi suffices.
			// Otherwise, we need a lui + addi, for 8 bytes total.

			U32 expression_index             = statement->expressions_indexes[0];
			Expression_Node *expression      = &resolver->expressions->data[expression_index];
			Expression_Evaluation evaluation = Resolver_expression_evaluate(resolver, expression);

			S64 immediate = (S64)evaluation.value;

			size_new = 24; // Worst-case for rv32 is 8.
			if (!evaluation.unresolved && -2048 <= immediate && immediate <= 2047)
			{
				size_new = 4;
			}
			else if (!evaluation.unresolved && -2147483648LL <= immediate && immediate <= 2147483647LL)
			{
				size_new = 8;
			}
		} break;
		case Instruction_Kind__J:
		{
			// Same encoding as TAIL: jal zero, offset.
			// Same range and expansion logic.
		} // fallthrough
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
			Expression_Evaluation offset = Resolver_expression_evaluate(resolver, expression);
			S64 delta                    = (S64)offset.value - statement->section_offset;

			B32 range_in = -(1 << 20) <= delta && delta <= (1 << 20) - 1;
			if (offset.unresolved || !range_in)
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
			U32 expression_index                  = statement->expressions_indexes[0];
			Expression_Node *expression           = &resolver->expressions->data[expression_index];
			Expression_Evaluation evaluation      = Resolver_expression_evaluate(resolver, expression);

			size_new = 12;
			S64 delta = (S64)evaluation.value - (S64)statement->section_offset;
			B32 range_in_small  = !evaluation.unresolved && -(1 << 12) <= delta && delta <= (1 << 12) - 1;
			B32 range_in_medium = !evaluation.unresolved && -(1 << 20) <= delta && delta <= (1 << 20) - 1;
			if (range_in_small)
			{
				size_new = 4;
			}
			else if (range_in_medium)
			{
				size_new = 8;
			}
		} break;
		default: {} break;
		}

		if (statement->directive_kind == Directive_Kind__Align)
		{
			// .align computes padding based on current offset.
			// Padding = bytes needed to reach next alignment boundary.
			U32 expression_index             = statement->expressions_indexes[0];
			Expression_Node *expression      = &resolver->expressions->data[expression_index];
			Expression_Evaluation evaluation = Resolver_expression_evaluate(resolver, expression);

			if (!evaluation.unresolved)
			{
				U32 alignment = 1u << (U32)evaluation.value;
				U32 offset    = statement->section_offset;
				U32 remainder = offset & (alignment - 1);
				size_new = remainder == 0 ? 0 : alignment - remainder;
			}
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

U32
Resolver_relax(Resolver *resolver)
{
	U32 pass_count = 0;
	for (;;)
	{
		Resolver_offsets_recompute(resolver);
		pass_count += 1;
		B32 changed = Resolver_relax_pass(resolver);

		if (!changed)
		{
			break;
		}
	}

	Resolver_offsets_recompute(resolver);
	return pass_count;
}
