void
Resolver_error_set(Resolver *resolver, Resolver_Error_Kind kind)
{
	// TODO: set other fields as well, for example the token where it happened.
	if (!resolver->error.kind)
	{
		resolver->error.kind      = kind;
		resolver->error.statement = resolver->statement_current;

		// U32 row_index = resolver->statement_current.
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
	case Expression_Kind__Label_Numeric_Reference_Forward:
	{
		U16 section_current_index = resolver->section_current_index;
		U32 label_numeric_value   = node->integer_value; // e.g. 1b or 2b etc.

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
			B32 match = statement->kind == Statement_Kind__Label_Numeric && statement->label_numeric_value == label_numeric_value;
			if (match)
			{
				Resolver_expect(resolver, statement->section_index == section_current_index, Resolver_Error_Kind__Label_Numeric_Section_Cross);
				offset = statement->section_offset;
			}
		}

		if (!match)
		{
			Resolver_error_set(resolver, Resolver_Error_Kind__Label_Numeric_Forward_Not_Found);
		}
	} break;
	case Expression_Kind__Label_Numeric_Reference_Backward:
	{
		U16 section_current_index      = resolver->section_current_index;
		U32 label_numeric_value        = node->integer_value; // e.g. 1b or 2b etc.
		Vec2_U32 statement_index_maybe = resolver->labels_numeric_statement_index[label_numeric_value];

		Resolver_expect(resolver, statement_index_maybe.y, Resolver_Error_Kind__Label_Numeric_Backward_Not_Found);
		Statement *statement = &resolver->statements->data[statement_index_maybe.x];
		Resolver_expect(resolver, statement->section_index == section_current_index, Resolver_Error_Kind__Label_Numeric_Section_Cross);

		node->integer_value = statement_index_maybe.x;

	} break;
	case Expression_Kind__Current_Address:
	{
		U32 section_current_offset = resolver->sections_offset[resolver->section_current_index];
		node->integer_value = section_current_offset;
	} break;
	case Expression_Kind__Relocation:      {  todo_m(); } break;

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
	U32 section_offsets[ELF_Section__COUNT] = {0};

	for (;;)
	{
		B32 break_should = resolver->statements_end_reached;
		if (break_should)
		{
			break;
		}
		Statement *statement     = resolver->statement_current;
		U32 *section_offset      = &section_offsets[statement->section_index];
		statement->section_offset = *section_offset;

		if (statement->kind == Statement_Kind__Label)
		{
			Symbols_Table_Entry *entry = &resolver->symbols_table->entries[statement->label_symbol_slot];
			entry->value.value = statement->section_offset;
		}

		*section_offset += statement->size;
		Resolver_advance(resolver);
	}
}

internal B32
Resolver_relax_pass(Resolver *resolver)
{
	os_memory_zero(resolver->labels_numeric_statement_index, label_numeric_max);

	B32 changed = 0;
	for (;;)
	{
		B32 break_should = resolver->statements_end_reached;
		if (break_should)
		{
			break;
		}

		Statement *statement = resolver->statement_current;

		if (statement->kind == Statement_Kind__Label_Numeric)
		{
			resolver->labels_numeric_statement_index[statement->label_numeric_value]
				= (Vec2_U32){ .x = resolver->statement_index, .y = 1 };
		}

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

// For each statement that contains an expression referencing a symbol:
//
// 1. If the expression's symbol is UNDEFINED (not defined anywhere in this translation unit):
//    -> Emit a relocation. The linker must resolve this.
//    The specific relocation depends on the context:
//      - %hi(symbol)              -> R_RISCV_HI20
//      - %lo(symbol) in I-type    -> R_RISCV_LO12_I
//      - %lo(symbol) in S-type    -> R_RISCV_LO12_S
//      - %pcrel_hi(symbol)        -> R_RISCV_PCREL_HI20
//      - %pcrel_lo(label) in I-type -> R_RISCV_PCREL_LO12_I
//      - %pcrel_lo(label) in S-type -> R_RISCV_PCREL_LO12_S
//      - %got_pcrel_hi(symbol)    -> R_RISCV_GOT_HI20
//      - %tprel_hi(symbol)        -> R_RISCV_TPREL_HI20
//      - %tprel_lo(symbol) in I-type -> R_RISCV_TPREL_LO12_I
//      - %tprel_lo(symbol) in S-type -> R_RISCV_TPREL_LO12_S
//      - %tprel_add(symbol)       -> R_RISCV_TPREL_ADD
//      - %tls_ie_pcrel_hi(symbol) -> R_RISCV_TLS_GOT_HI20
//      - %tls_gd_pcrel_hi(symbol) -> R_RISCV_TLS_GD_HI20
//      - B-type branch target     -> R_RISCV_BRANCH
//      - J-type jump target (jal) -> R_RISCV_JAL
//      - call/tail pseudo-inst    -> R_RISCV_CALL or R_RISCV_CALL_PLT
//      - .byte symbol             -> R_RISCV_32 (truncated) — rare, usually an error
//      - .half symbol             -> R_RISCV_32 (truncated) — rare, usually an error
//      - .word symbol             -> R_RISCV_32
//      - .dword symbol            -> R_RISCV_64
//
// 2. If the expression's symbol is defined and marked SHN_COMMON:
//    -> Emit a relocation. Common symbols are resolved by the linker.
//    Same relocation selection as case 1.
//
// 3. If the expression's symbol is defined and marked STB_GLOBAL or STB_WEAK:
//    -> Emit a relocation. Even if the symbol is defined locally, the linker
//      may override it with a definition from another translation unit (for
//      weak symbols) or needs the relocation for shared library interposition
//      (for global symbols). A strictly static, non-PIC assembler could
//      optimize some of these away, but the safe default is to emit.
//    Same relocation selection as case 1.
//
// 4. If the expression's symbol is defined, STB_LOCAL, and in a DIFFERENT section
//    from the statement:
//    -> Emit a relocation. The assembler does not know the final distance
//      between sections.
//    Same relocation selection as case 1.
//
// 5. If the expression's symbol is defined, STB_LOCAL, and in the SAME section
//    as the statement:
//
//    5a. If the reference type is ABSOLUTE (%hi, %lo, .word, .dword, etc.):
//        -> Emit a relocation. The absolute address depends on section placement,
//          which is determined by the linker.
//        Relocation selection:
//          - %hi(symbol)              -> R_RISCV_HI20
//          - %lo(symbol) in I-type    -> R_RISCV_LO12_I
//          - %lo(symbol) in S-type    -> R_RISCV_LO12_S
//          - .word symbol             -> R_RISCV_32
//          - .dword symbol            -> R_RISCV_64
//
//    5b. If the reference type is PC-RELATIVE and linker relaxation is ENABLED:
//        -> Emit the PC-relative relocation PLUS a companion R_RISCV_RELAX.
//          Intervening instructions may shrink during relaxation, changing
//          the offset.
//        Relocation selection (each paired with R_RISCV_RELAX):
//          - %pcrel_hi(symbol)          -> R_RISCV_PCREL_HI20 + R_RISCV_RELAX
//          - %pcrel_lo(label) in I-type -> R_RISCV_PCREL_LO12_I + R_RISCV_RELAX
//          - %pcrel_lo(label) in S-type -> R_RISCV_PCREL_LO12_S + R_RISCV_RELAX
//          - B-type branch target       -> R_RISCV_BRANCH + R_RISCV_RELAX
//          - J-type jump target (jal)   -> R_RISCV_JAL + R_RISCV_RELAX
//          - call/tail pseudo-inst      -> R_RISCV_CALL(_PLT) + R_RISCV_RELAX
//
//    5c. If the reference type is PC-RELATIVE and linker relaxation is DISABLED:
//        -> Do NOT emit a relocation. The assembler can compute the final offset
//          directly, since both the reference and the target are in the same
//          section and no relaxation will alter the distance.
//
// 6. If the expression is a CONSTANT (no symbol reference, purely numeric):
//    -> Do NOT emit a relocation. Encode the value directly.
//
// 7. If the expression is a SYMBOL DIFFERENCE (sym_a - sym_b):
//
//    7a. If both symbols are defined, STB_LOCAL, and in the SAME section,
//        and linker relaxation is DISABLED:
//        -> Do NOT emit a relocation. The difference is a fixed constant
//          that the assembler can compute directly.
//
//    7b. If both symbols are defined, STB_LOCAL, and in the SAME section,
//        and linker relaxation is ENABLED:
//        -> Emit a relocation pair. Relaxation may change the distance.
//        Relocation selection based on data width:
//          - 1-byte context (.byte) -> R_RISCV_ADD8  + R_RISCV_SUB8
//          - 2-byte context (.half) -> R_RISCV_ADD16 + R_RISCV_SUB16
//          - 4-byte context (.word) -> R_RISCV_ADD32 + R_RISCV_SUB32
//          - 8-byte context (.dword)-> R_RISCV_ADD64 + R_RISCV_SUB64
//          - 6-bit context (rare)   -> R_RISCV_SET6  + R_RISCV_SUB6
//
//    7c. If the symbols are in DIFFERENT sections, or either is STB_GLOBAL,
//        STB_WEAK, SHN_COMMON, or UNDEFINED:
//        -> Emit a relocation pair. The linker must resolve the difference.
//        Same relocation selection as 7b based on data width.
void
Resolver_relocation_emit(Resolver *resolver)
{
}


// I can start writing a encoding function that also emits relocation as part of the process.
// While encoding instruction themselves isn't nothing particularly challenging, its more complex to handle relocations.
// Relocations essentially happen when some specific relocator operators are found or when some symbols are undefined
// i.e. they belong to section index undefined. In such case, due to the relocation with addend entry layout `Elf_Rela`
// or `ELF_Relocation_Addend`, if the expression node is `unresolved`, then we need to check the form of such
// expressions. The evaluation algorithm tries to evaluate as much as possible, so it must reduce to something like
// `symbol + addend` or `addend + symbol`, where addend is a signed integer.
