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
		assert_always_m(node->symbols_table_entry && "parser didn't set expression entry");
		if (node->symbols_table_entry->value.section_index)
		{
			node->integer_value = node->symbols_table_entry->value.value;
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
			match = statement->kind == Statement_Kind__Label_Numeric && statement->label_numeric_value == label_numeric_value;
			if (match)
			{
				Resolver_expect(resolver, statement->section_index == section_current_index, Resolver_Error_Kind__Label_Numeric_Section_Cross);
				offset = statement->section_offset;
			}
		}

		node->integer_value = offset;

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
	case Expression_Kind__Relocation:
	{
		// TODO: this could be done during parsing instead.
		Resolver_expect(resolver, resolver->statement_current->kind == Statement_Kind__Instruction, Resolver_Error_Kind__Relocation_Misplaced);

		if (resolver->flags & (Resolver_Flags__Relocations))
		{
			// NOTE: a case like addend + symbol + addend is NOT supported.
			// Only symbol + addend or viceversa is supported.


			// NOTE: this can probably be recycled.

			// We have to find the symbol and the addend.
			Symbols_Table_Entry *entry = &symbols_table_entry_none;
			U64 addend = 0;

			// Case 1: just a symbol
			Expression_Node *node_left = &resolver->expressions->data[node->index_left];

			B32 addend_required = node_left->symbols_table_entry == 0;

			// Case 2: symbol with addend
			if (addend_required)
			{
				Expression_Kind left_kind = node_left->kind;
				B32 left_kind_add_is = left_kind == Expression_Kind__Add;
				B32 left_kind_subtract_is = left_kind == Expression_Kind__Subtract;
				Resolver_expect(resolver, left_kind_add_is || left_kind_subtract_is, Resolver_Error_Kind__Relocation_Operand_Invalid);

				Expression_Node *node_left_left  = &resolver->expressions->data[node_left->index_left];
				Expression_Node *node_left_right = &resolver->expressions->data[node_left->index_right];

				Resolver_expect(resolver, node_left_left || node_left_right, Resolver_Error_Kind__Relocation_Operand_Symbol_Missing);

				if (node_left_left->symbols_table_entry && node_left_right->symbols_table_entry)
				{
					Resolver_expect(resolver, left_kind_subtract_is, Resolver_Error_Kind__Relocation_Operand_Invalid);
					// Check that both are local etc.
				}
				else if (node_left_right->symbols_table_entry)
				{
					B32 addend_resolved = !node_left_left->unresolved;
					Resolver_expect(resolver, addend_resolved, Resolver_Error_Kind__Relocation_Form_Invalid);
					addend = node_left_left->integer_value;
					entry = node_left_right->symbols_table_entry;
				}
				else if (node_left_left->symbols_table_entry)
				{
					B32 addend_resolved = !node_left_right->unresolved;
					Resolver_expect(resolver, addend_resolved, Resolver_Error_Kind__Relocation_Form_Invalid);
					addend = node_left_right->integer_value;
					entry = node_left_left->symbols_table_entry;
				}

			}
			else
			{
				Resolver_expect(resolver, node_left->symbols_table_entry, Resolver_Error_Kind__Relocation_Symbol_Missing);
				entry = node_left->symbols_table_entry;
			}

			Resolver_relocation_emit(resolver, entry->index, addend, node->relocation_operator);
		}
		else
		{
			// For simplicity, we evaluate relocation operators once relaxation is completed, and offset are known.
			node->unresolved = 1;
		}
	} break;

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
void
Resolver_relocation_emit(Resolver *resolver, U32 symbol_index, S64 addend, Relocation_Operator operator)
{
	Statement *statement = resolver->statement_current;
	ELF_Section section_index = ELF_Section_relocations[statement->section_index];
	Object_File_Section *section = &resolver->sections[section_index];
	Relocation_RISC_V relocation_kind = Relocation_RISC_V_from_Relocation_Operator(operator, statement->instruction_format);

	ELF64_Relocation_Addend relocation =
	{
		.offset = statement->section_offset,
		.info   = ELF64_Relocation_info_m(symbol_index, relocation_kind),
		.addend = addend,
	};

	Object_File_Section_write(section, (U8 *)&relocation, sizeof(ELF64_Relocation_Addend));
	return;
}

// Encode instructions and directives to object files. Assumes Resolver_relax has been called, as such all expressions
// have been evaluated.
void
Resolver_encode(Resolver *resolver)
{
	resolver->flags |= Resolver_Flags__Relocations;
	Resolver_cursor_reset(resolver);
	Object_File_Section *section_current = &resolver->sections[ELF_Section__Text];

	for (;;)
	{
		B32 break_should = resolver->statements_end_reached;
		if (break_should)
		{
			break;
		}

		Statement *statement = resolver->statement_current;

		switch (statement->kind)
		{
		case Statement_Kind__Directive:
		{

		} break;
		case Statement_Kind__Instruction:
		{
			if (statement->expressions_count)
			{
				Expression_Node *expression = &resolver->expressions->data[statement->expressions_indexes[0]];
				Resolver_expression_evaluate(resolver, expression);
			}

		} break;
		}

		Resolver_advance(resolver);
	}

	return;
}

// I can start writing a encoding function that also emits relocation as part of the process.
// While encoding instruction themselves isn't nothing particularly challenging, its more complex to handle relocations.
// Relocations essentially happen when some specific relocator operators are found or when some symbols are undefined
// i.e. they belong to section index undefined. In such case, due to the relocation with addend entry layout `Elf_Rela`
// or `ELF_Relocation_Addend`, if the expression node is `unresolved`, then we need to check the form of such
// expressions. The evaluation algorithm tries to evaluate as much as possible, so it must reduce to something like
// `symbol + addend` or `addend + symbol`, where addend is a signed integer.
