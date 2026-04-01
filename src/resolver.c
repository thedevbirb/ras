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

void
Resolver_expression_evaluate(Resolver *resolver, Expression_Node *node)
{

	assert_always_m(node && "cannot evaluate null expression");
	assert_always_m(node->kind && "cannot evaluate unknown expression kind");

	Expression_Kind kind = node->kind;

	switch (kind)
	{
	case Expression_Kind__None:            {} break;
	case Expression_Kind__Number_Literal:  {} break;
	case Expression_Kind__Char_Literal:    {} break;
	case Expression_Kind__Identifier:
	{
		String8 key = Resolver_token_string_from_index(resolver, node->token_index);
		Symbols_Table_Entry *entry = Symbols_Table_get(resolver->symbols_table, key);
		if (entry && entry->value.section_index)
		{
			node->integer_value = entry->value.value;
		}
		else
		{
			node->unresolved = 1;
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
		Resolver_expression_evaluate(resolver, node_left);

		node->unresolved = node_left->unresolved;
		node->integer_value = (node_left->unresolved - 1) & ~(node_left->integer_value - 1);
	} break;
	case Expression_Kind__Bitwise_Not:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, node_left);
		node->unresolved = node_left->unresolved;
		node->integer_value = (node_left->unresolved - 1) & ~node_left->integer_value;
	} break;
	case Expression_Kind__Logical_Not:
	{
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, node_left);
		node->unresolved = node_left->unresolved;
		node->integer_value = (node_left->unresolved - 1) & !node_left->integer_value;
	} break;
	default:
	{
		Expression_Node *node_right = &resolver->expressions->data[node->index_right];
		Resolver_expression_evaluate(resolver, node_right);
		Expression_Node *node_left = &resolver->expressions->data[node->index_left];
		Resolver_expression_evaluate(resolver, node_left);

		node->unresolved = node_right->unresolved || node_left->unresolved;
		if (!node->unresolved)
		{
			U64 left_value  = node_left->integer_value;
			U64 right_value = node_right->integer_value;
			switch (node->kind)
			{
			case Expression_Kind__Add:           { node->integer_value = left_value +  right_value; } break;
			case Expression_Kind__Subtract:      { node->integer_value = left_value -  right_value; } break;
			case Expression_Kind__Multiply:      { node->integer_value = left_value *  right_value; } break;
			case Expression_Kind__Divide:        { node->integer_value = left_value /  right_value; } break;
			case Expression_Kind__Modulo:        { node->integer_value = left_value %  right_value; } break;

			case Expression_Kind__Bitwise_Or:    { node->integer_value = left_value |  right_value; } break;
			case Expression_Kind__Bitwise_Xor:   { node->integer_value = left_value ^  right_value; } break;
			case Expression_Kind__Bitwise_And:   { node->integer_value = left_value &  right_value; } break;
			case Expression_Kind__Shift_Left:    { node->integer_value = left_value << right_value; } break;
			case Expression_Kind__Shift_Right:   { node->integer_value = left_value >> right_value; } break;

			case Expression_Kind__Equal:         { node->integer_value = left_value == right_value; } break;
			case Expression_Kind__Not_Equal:     { node->integer_value = left_value != right_value; } break;
			case Expression_Kind__Less_Than:     { node->integer_value = left_value <  right_value; } break;
			case Expression_Kind__Less_Equal:    { node->integer_value = left_value <= right_value; } break;
			case Expression_Kind__Greater_Than:  { node->integer_value = left_value >  right_value; } break;
			case Expression_Kind__Greater_Equal: { node->integer_value = left_value >= right_value; } break;

			case Expression_Kind__Logical_And:   { node->integer_value = left_value && right_value; } break;
			case Expression_Kind__Logical_Or:    { node->integer_value = left_value || right_value; } break;

			default: { Resolver_error_set(resolver, Resolver_Error_Kind__Expression_Kind_Unknown); } break;
			}
		}
	} break;
	}

	return;
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
			Resolver_expression_evaluate(resolver, expression);

			S64 immediate = (S64)expression->integer_value;

			size_new = 24; // Worst-case for rv32 is 8.
			if (!expression->unresolved)
			{
				if (-(1 << 11) <= immediate && immediate <= (1 << 11) - 1)
				{
					size_new = 4;
				}
				else if (-(1LL << 31) <= immediate && immediate <= (1LL << 31) -1)
				{
					size_new = 8;
				}
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

			Resolver_expression_evaluate(resolver, expression);
			S64 delta = (S64)expression->integer_value - statement->section_offset;

			B32 range_in = -(1 << 20) <= delta && delta <= (1 << 20) - 1;
			if (expression->unresolved || !range_in)
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
			if (!expression->unresolved)
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

		if (statement->directive_kind == Directive_Kind__Align)
		{
			// .align computes padding based on current offset.
			// Padding = bytes needed to reach next alignment boundary.
			U32 expression_index        = statement->expressions_indexes[0];
			Expression_Node *expression = &resolver->expressions->data[expression_index];
			Resolver_expression_evaluate(resolver, expression);

			if (expression->unresolved)
			{
				U32 alignment = 1u << (U32)expression->integer_value;
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
