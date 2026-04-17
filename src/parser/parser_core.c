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

// Create a label string for the dot symbol, in the format `.D{index}`, where `index` is the current statement index.
internal String8
Parser_label_dot(Parser *parser)
{
	U32 statement_index = parser->statements->count;
	U32 digits_count = snprintf(0, 0, "%d", statement_index);
	U64 string_characters_count  = digits_count + 2;
	String8 key =
	{
		// .D{statement_index}
		.data = Arena_push_array_m(parser->arena, U8, string_characters_count),
		.count = string_characters_count,
	};

	key.data[0] = '.';
	key.data[1] = 'D';
	snprintf((char *)(key.data + 2), digits_count, "%d", statement_index);

	return key;
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

internal void
Parser_error_set(Parser *parser, Parser_Error_Kind kind)
{
	Parser_Error error =
	{
		.kind               = kind,
		.row_index          = parser->token_current.row_index,
		.column_index_begin = parser->token_current.column_index,
		.column_index_end   = parser->token_current.column_index + parser->token_current.size - 1,
	};

	assert_always_m(error.column_index_begin <= error.column_index_end && "token_index bug");
	parser->error = error;

#ifdef ASSEMBLER_EXPECT_PANIC
	assert_always_m(0 && "panic on expect");
#endif

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
	B32 condition = parser->token_current.kind == token_kind;
	Parser_expect(parser, condition, error_kind);
	return;
}

internal U8
Parser_register(Parser *parser)
{
	String8 string = Parser_token_string(parser);
	U8 register_value = register_lookup(string);
	return register_value;
}

internal U8
Parser_expect_register(Parser *parser)
{
	String8 string = Parser_token_string(parser);
	U8 register_value = register_lookup(string);
	Parser_expect(parser, register_value != register_invalid, Parser_Error_Kind__Register_Invalid);
	return register_value;
}

internal Symbols_Table_Entry *
Parser_symbol_declare(Parser *parser, String8 key)
{
	Symbols_Table_Entry *entry = Symbols_Table_reserve(parser->symbols_table, key);
	entry->index_statement = parser->statements->count;
	entry->flags |= Symbol_Flags__Declared;

	return entry;
}

// A special note goes to symbols inside an expression. While absolute symbols, created with .set or .equ
// directives are always accepted, others must be present only instruction which accept immediates.
//
// If met in other places, we should emit an error.
internal void
Parser_symbol_ensure_context_valid(Parser *parser, Symbols_Table_Entry *symbol)
{
	Directive_Kind directive_kind = parser->statement_context->directive_kind;
	B32 directive_kind_valid = directive_kind == Directive_Kind__Byte
		                || directive_kind == Directive_Kind__Word_Half
		                || directive_kind == Directive_Kind__Word
		                || directive_kind == Directive_Kind__Word_Double
			        || directive_kind == Directive_Kind__Set
			        || directive_kind == Directive_Kind__Equality;

	Instruction_Format instruction_format = parser->statement_context->instruction_format;
	B32 instruction_format_valid = instruction_format == Instruction_Format__B
		                    || instruction_format == Instruction_Format__I
		                    || instruction_format == Instruction_Format__J
		                    || instruction_format == Instruction_Format__S
		                    || instruction_format == Instruction_Format__Expandable;

	B32 context_valid = directive_kind_valid
		         || instruction_format_valid;

	Parser_expect(parser, context_valid, Parser_Error_Kind__Symbol_Context_Invalid);
	return;
}

// A relocation operator can appear only once in an expression, and %hi can only work where there is a 20-bit immediate
// like lui e auipc while %lo which takes lower 12 bits works elsewhere.
// A relocation operator can be combined only with constant arithmetic, in the symbol + addend fashion.

internal Expression_Node *
Parser_parse_null_denotation(Parser *parser)
{

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
		node->kind = Expression_Kind__Identifier;

		String8 key = Parser_label_dot(parser);
		Symbols_Table_Entry *symbol = Symbols_Table_reserve(parser->symbols_table, key);

		symbol->elf.section_index = parser->statement_context->section_index;
		symbol->elf.type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Local, 0);

		node->symbols_table_entry = symbol;
		Parser_symbol_ensure_context_valid(parser, symbol);

		Parser_advance(parser);
	} break;

	case Token_Kind__Identifier:
	{
		node->kind = Expression_Kind__Identifier;

		String8 key = Parser_token_string(parser);
		Symbols_Table_Entry *symbol = Symbols_Table_reserve(parser->symbols_table, key);
		Parser_symbol_ensure_context_valid(parser, symbol);

		node->symbols_table_entry = symbol;

		Parser_advance(parser);
	} break;

	case Token_Kind__Label_Numeric_Reference_Forward:
	{
		Parser_expect(parser, parser->token_current.numerical_value <= label_numeric_max, Parser_Error_Kind__Label_Numeric_Large);

		node->kind                = Expression_Kind__Label_Numeric_Reference_Forward;
		node->label_numeric_value = token.numerical_value;

		Parser_advance(parser);
	} break;
	case Token_Kind__Label_Numeric_Reference_Backward:
	{
		Parser_expect(parser, parser->token_current.numerical_value <= label_numeric_max, Parser_Error_Kind__Label_Numeric_Large);

		node->kind                = Expression_Kind__Label_Numeric_Reference_Backward;
		node->label_numeric_value = token.numerical_value;

		Parser_advance(parser);
	} break;

	case Token_Kind__Minus:
	case Token_Kind__Tilde:
	case Token_Kind__Bang:
	{
		node->kind = Expression_Kind_from_unary_Token_Kind(token.kind);
		Parser_advance(parser);
		Expression_Node *operand = Parser_expression_parse_inner(parser, Binding_Power__Unary);
		node->index_left = operand->index;
	} break;

	case Token_Kind__Relocation_Prefix:
	{
		Statement *statement = parser->statement_context;
		Parser_expect(parser, statement->kind == Statement_Kind__Instruction, Parser_Error_Kind__Relocation_Instruction_Missing);
		B32 supported = Instruction_Encoding_table[statement->instruction_kind].flags & Instruction_Flags__Relocation_Operator;
		Parser_expect(parser, supported, Parser_Error_Kind__Relocation_Operator_Invalid);

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);
		String8 relocation_operator_string      = Parser_token_string(parser);
		Relocation_Operator relocation_operator = Relocation_Operator_lookup(relocation_operator_string);
		Parser_expect(parser, relocation_operator, Parser_Error_Kind__Relocation_Operator_Invalid);
		Parser_expect(parser, !parser->statement_context->relocation_operator, Parser_Error_Kind__Relocation_Operator_Multiple);

		B32 relocation_operator_valid = 0;
		switch (relocation_operator)
		{
		case Relocation_Operator__hi:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__LUI;
		} break;
		case Relocation_Operator__lo:
		{
			relocation_operator_valid = statement->instruction_format == Instruction_Format__I
				                 || statement->instruction_format == Instruction_Format__S;
		} break;
		case Relocation_Operator__pcrel_hi:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
		} break;
		case Relocation_Operator__pcrel_lo:
		{
			relocation_operator_valid = statement->instruction_format == Instruction_Format__I
						 || statement->instruction_format == Instruction_Format__S;
		} break;
		case Relocation_Operator__tprel_hi:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC
						 || statement->instruction_kind == Instruction_Kind__LUI;
		} break;
		case Relocation_Operator__tprel_lo:
		{
			relocation_operator_valid = statement->instruction_format == Instruction_Format__I
						 || statement->instruction_format == Instruction_Format__S;
		} break;
		case Relocation_Operator__tprel_add:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__ADD
				                && (statement->register_source_1 == register_tp
						||  statement->register_source_2 == register_tp);
		} break;
		case Relocation_Operator__got_pcrel_hi:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
		} break;
		case Relocation_Operator__tls_ie_pcrel_hi:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
		} break;
		case Relocation_Operator__tls_gd_pcrel_hi:
		{
			relocation_operator_valid = statement->instruction_kind == Instruction_Kind__AUIPC;
		} break;
		case Relocation_Operator__None:  {} break;
		case Relocation_Operator__COUNT: { unreachable_m(); } break;
		}
		Parser_expect(parser, !supported || relocation_operator_valid, Parser_Error_Kind__Relocation_Instruction_Invalid);

		node->relocation_operator = relocation_operator;
		parser->statement_context->relocation_operator = relocation_operator;

		Parser_advance(parser);
		Parser_expect_token(parser, Token_Kind__Parenthesis_Left, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);

		Parser_advance(parser);
		Expression_Node *inner = Parser_expression_parse_inner(parser, Binding_Power__None);
		Symbols_Table_Entry *symbol = inner->symbols_table_entry;
		Parser_expect(parser, symbol != 0, Parser_Error_Kind__Relocation_Symbol_Missing);
		// TODO: do context verification of symbol during evaluation, or at later stage, when all symbols are known.
		// More specifically:
		// %hi / %lo — symbol must have an absolute address in some section
		// %pcrel_hi / %pcrel_lo — symbol must be in a section (PC-relative offset is meaningless for an absolute value)
		// %got_pcrel_hi — symbol must have a GOT entry, so it must be a real symbol
		// %tprel_hi / %tprel_lo / %tprel_add — symbol must be in .tdata or .tbss (TLS sections)
		// %tls_ie_pcrel_hi — symbol must be a TLS symbol
		// %tls_gd_pcrel_hi — symbol must be a TLS symbol
		//
		// This may be correct/desiderable but in practice gas doesn't enforce almost anything.
		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Expression_Relocation_Syntax_Invalid);
		// TODO: %pcrel_lo addi, loads, storesI / Srs1 = rd of auipclabel of auipc. gas doesn't check it.

		node->kind       = Expression_Kind__Relocation;
		node->index_left = inner->index;

		Parser_advance(parser);

	} break;

	case Token_Kind__Parenthesis_Left:
	{
		Parser_advance(parser);
		Expression_Node *inner = Parser_expression_parse_inner(parser, Binding_Power__None);

		Parser_expect_token(parser, Token_Kind__Parenthesis_Right, Parser_Error_Kind__Expression_Parenthesis_Right_Expected);
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


// Consider evaluating already the constants.

// Core Pratt parser loop. Parses an expression where all binary operators
// must have binding power strictly greater than binding_power_minimum.
// All operators are left-associative (the <= comparison ensures this).
internal Expression_Node *
Parser_expression_parse_inner(Parser *parser, Binding_Power binding_power_minimum)
{
	local_persist U8 recursion_level = 0;
	recursion_level += 1;

	Expression_Node *left = 0;
	Parser_expect(parser, !parser->end_reached, Parser_Error_Kind__Expression_Unexpected_End);
	Parser_expect(parser, recursion_level <= 8, Parser_Error_Kind__Expression_Recursion_Max);

	left = Parser_parse_null_denotation(parser);
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

		Expression_Node *right = Parser_expression_parse_inner(parser, next_power);
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
Parser_expression_parse(Parser *parser)
{
	Expression_Node *node = Parser_expression_parse_inner(parser, Binding_Power__None);
	return node;
}

Expression_Node *
Parser_expression_immediate_create(Parser *parser, U64 immediate)
{
	Expression_Node *node  = Expressions_push_empty(parser->expressions);
	node->integer_value = immediate;
	return node;
}

U32 *
Parser_expression_sentinel_index(Parser *parser)
{
	Expression_Node *expression = &parser->expressions->data[0];
	return &expression->index;
}


internal void
Parser_statement_context_reset(Parser *parser)
{
	*parser->statement_context = (Statement)
	{
		.token_index_begin = parser->token_index,
		.section_index = parser->section_current_index,
		.flags = parser->flags,
	};

	return;
}

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
		Parser_statement_context_reset(parser);

		Token_Kind token_start_kind = parser->token_current.kind;

		switch (token_start_kind)
		{
		case Token_Kind__Newline: { Parser_advance(parser); } break;
		case Token_Kind__Label:
		{
			String8 key = Parser_token_string(parser);
			Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);
			B32 duplicate = entry->elf.section_index;
			Parser_expect(parser, !duplicate, Parser_Error_Kind__Label_Duplicate);

			entry->elf.section_index = parser->section_current_index,

			Parser_advance(parser);
			parser->statement_context->kind              = Statement_Kind__Label;
		} break;
		case Token_Kind__Label_Numeric:
		{
			Parser_expect(parser, parser->token_current.numerical_value <= label_numeric_max, Parser_Error_Kind__Label_Numeric_Large);
			U8 label_numeric_value = parser->token_current.numerical_value;

			Parser_advance(parser);
			parser->statement_context->label_numeric_value = label_numeric_value;
			parser->statement_context->kind                = Statement_Kind__Label_Numeric;
		} break;
		case Token_Kind__Directive:
		{
			String8 substring = Parser_token_string(parser);
			Directive_Kind directive_kind = Directive_Kind__from_String8(substring);


			parser->statement_context->kind = Statement_Kind__Directive;
			parser->statement_context->directive_kind = directive_kind;

			switch (directive_kind)
			{
			case Directive_Kind__None:
			{
				Parser_error_set(parser, Parser_Error_Kind__Directive_Unknown);
			} break;
			case Directive_Kind__Word_Double: { data_directive_size += 4; } // fallthrough
			case Directive_Kind__Word:        { data_directive_size += 2; } // fallthrough
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
					Expression_Node *expression = Parser_expression_parse(parser);
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

				parser->statement_context->expressions_indexes = expressions_indexes;
				parser->statement_context->expressions_count   = expressions_count;
				parser->statement_context->size                = data_directive_size * expressions_count;

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

				parser->statement_context->size = string_size;
				string_size = 0;

				Parser_advance(parser);

			} break;
			case Directive_Kind__Section:
			{
				Parser_advance(parser);

				String8 substring             = Parser_token_string(parser);
				Directive_Kind directive_kind = Directive_Kind__from_String8(substring);
				ELF_Section section_index   = ELF_Section_from_Directive_Kind(directive_kind);
				parser->section_current_index = section_index;

				Parser_expect(parser, section_index != 0, Parser_Error_Kind__Directive_Section_Argument_Invalid);
				Parser_advance(parser);
			} break;
			case Directive_Kind__Local:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 key = Parser_token_string(parser);
				Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);

				assert_always_m(ELF_Symbol_Binding__Local == 0 && "wrong assumption on local binding value");
				B32 demoted = ELF_Symbol_bind_m(entry->elf.type_and_binding) > ELF_Symbol_Binding__Local;
				Parser_expect(parser, !demoted, Parser_Error_Kind__Symbol_Demoted);

				Parser_advance(parser);
			} break;
			case Directive_Kind__Globl: {} // fallthrough
			case Directive_Kind__Global:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 key = Parser_token_string(parser);
				Symbols_Table_Entry *entry = Parser_symbol_declare(parser, key);

				U8 type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Global, ELF_Symbol_type_m(entry->elf.type_and_binding));
				entry->elf.type_and_binding = type_and_binding;

				Parser_advance(parser);
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
				Expression_Node *expression = Parser_expression_parse(parser);

				// TODO: what if set or equ creates an alias for a label? Special handling?

				Parser_symbol_declare(parser, key);

				parser->statement_context->expressions_indexes = &expression->index;
				parser->statement_context->expressions_count   = 1;
			} break;
			case Directive_Kind__Zero:
			{
				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser);

				parser->statement_context->expressions_indexes = &expression->index;
				parser->statement_context->expressions_count   = 1;
			} break;
			case Directive_Kind__Align:
			{
				parser->statement_context->flags |= Statement_Flags__Size_Variable;
			} // fallthrough, same parsing.
			case Directive_Kind__Skip:
			{
				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser);
				parser->statement_context->expressions_indexes[0] = expression->index;
				parser->statement_context->expressions_count = 1;

				if (parser->token_current.kind == Token_Kind__Comma)
				{
					Expression_Node *expression_second = Parser_expression_parse(parser);
					parser->statement_context->expressions_indexes[1] = expression_second->index;
					parser->statement_context->expressions_count = 2;
				}
			} break;
			case Directive_Kind__Common:
			{
				// .comm symbol, size, alignment
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 key = Parser_token_string(parser);
				Symbols_Table_Entry *entry = Symbols_Table_reserve(parser->symbols_table, key);
				B32 duplicate = entry->elf.section_index != 0;

				entry->elf.type_and_binding = ELF_Symbol_info_m(ELF_Symbol_Binding__Global, 0),
				entry->elf.section_index    = ELF_Section_Index__Common,

				Parser_expect(parser, !duplicate, Parser_Error_Kind__Symbol_Duplicate);

				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *size_expression = Parser_expression_parse(parser);

				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *alignment_expression = Parser_expression_parse(parser);

				U32 *expressions_indexes = Arena_push_array_m(parser->arena, U32, 2);
				expressions_indexes[0] = size_expression->index;
				expressions_indexes[1] = alignment_expression->index;

				parser->statement_context->expressions_indexes = expressions_indexes;
				parser->statement_context->expressions_count = 2;
			} break;
			case Directive_Kind__Option:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);

				String8 string = Parser_token_string(parser);
				if (os_memory_match(string.data, "norelax", min_m(string.count, 7)) == 0)
				{
					parser->flags |= Statement_Flags__Relax_Disabled;
				}
				else if (os_memory_match(string.data, "relax", min_m(string.count, 5)) == 0)
				{
					parser->flags &= ~Statement_Flags__Relax_Disabled;
				}
				else
				{
					Parser_error_set(parser, Parser_Error_Kind__Option_Invalid);
				}
				Parser_advance(parser);

			} break;
			default:
			{
				ELF_Section section_kind = ELF_Section_from_Directive_Kind(directive_kind);
				assert_always_m(section_kind && "unhandled directive");

				parser->section_current_index = section_kind;
				Parser_advance(parser);
			} break;
			}

			parser->statement_context->section_index      = parser->section_current_index;

		} break;
		case Token_Kind__Identifier:
		{
			// This must be an instruction.
			String8 instruction = Parser_token_string(parser);
			U32 instruction_hash = hash_FNV_1a(instruction);

			parser->statement_context->kind = Statement_Kind__Instruction;

			switch (instruction_hash)
			{
			// U-type
			case HASH_lui:       { Parser_instruction_U_parse(parser, Instruction_Kind__LUI);                   } break;
			case HASH_auipc:     { Parser_instruction_U_parse(parser, Instruction_Kind__AUIPC);                 } break;

			// J-type
			case HASH_jal:       { Parser_instruction_jal_parse(parser);                                        } break;

			// I-type (JALR)
			case HASH_jalr:      { Parser_instruction_jalr_parse(parser);                                       } break;

			// B-type
			case HASH_beq:       { Parser_instruction_B_parse(parser, Instruction_Kind__BEQ);                   } break;
			case HASH_bne:       { Parser_instruction_B_parse(parser, Instruction_Kind__BNE);                   } break;
			case HASH_blt:       { Parser_instruction_B_parse(parser, Instruction_Kind__BLT);                   } break;
			case HASH_bge:       { Parser_instruction_B_parse(parser, Instruction_Kind__BGE);                   } break;
			case HASH_bltu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BLTU);                  } break;
			case HASH_bgeu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BGEU);                  } break;

			// I-type (loads)
			case HASH_lb:        { Parser_instruction_I_load_parse(parser, Instruction_Kind__LB);               } break;
			case HASH_lh:        { Parser_instruction_I_load_parse(parser, Instruction_Kind__LH);               } break;
			case HASH_lw:        { Parser_instruction_I_load_parse(parser, Instruction_Kind__LW);               } break;
			case HASH_lbu:       { Parser_instruction_I_load_parse(parser, Instruction_Kind__LBU);              } break;
			case HASH_lhu:       { Parser_instruction_I_load_parse(parser, Instruction_Kind__LHU);              } break;

			// Store-type
			case HASH_sb:        { Parser_instruction_S_parse(parser, Instruction_Kind__SB);                    } break;
			case HASH_sh:        { Parser_instruction_S_parse(parser, Instruction_Kind__SH);                    } break;
			case HASH_sw:        { Parser_instruction_S_parse(parser, Instruction_Kind__SW);                    } break;

			// I-type (arithmetic)
			case HASH_addi:      { Parser_instruction_I_parse(parser, Instruction_Kind__ADDI);                  } break;
			case HASH_slti:      { Parser_instruction_I_parse(parser, Instruction_Kind__SLTI);                  } break;
			case HASH_sltiu:     { Parser_instruction_I_parse(parser, Instruction_Kind__SLTIU);                 } break;
			case HASH_xori:      { Parser_instruction_I_parse(parser, Instruction_Kind__XORI);                  } break;
			case HASH_ori:       { Parser_instruction_I_parse(parser, Instruction_Kind__ORI);                   } break;
			case HASH_andi:      { Parser_instruction_I_parse(parser, Instruction_Kind__ANDI);                  } break;
			case HASH_slli:      { Parser_instruction_I_parse(parser, Instruction_Kind__SLLI);                  } break;
			case HASH_srli:      { Parser_instruction_I_parse(parser, Instruction_Kind__SRLI);                  } break;
			case HASH_srai:      { Parser_instruction_I_parse(parser, Instruction_Kind__SRAI);                  } break;

			// R-type
			case HASH_add:       { Parser_instruction_R_parse(parser, Instruction_Kind__ADD);                   } break;
			case HASH_sub:       { Parser_instruction_R_parse(parser, Instruction_Kind__SUB);                   } break;
			case HASH_sll:       { Parser_instruction_R_parse(parser, Instruction_Kind__SLL);                   } break;
			case HASH_slt:       { Parser_instruction_R_parse(parser, Instruction_Kind__SLT);                   } break;
			case HASH_sltu:      { Parser_instruction_R_parse(parser, Instruction_Kind__SLTU);                  } break;
			case HASH_xor:       { Parser_instruction_R_parse(parser, Instruction_Kind__XOR);                   } break;
			case HASH_srl:       { Parser_instruction_R_parse(parser, Instruction_Kind__SRL);                   } break;
			case HASH_sra:       { Parser_instruction_R_parse(parser, Instruction_Kind__SRA);                   } break;
			case HASH_or:        { Parser_instruction_R_parse(parser, Instruction_Kind__OR);                    } break;
			case HASH_and:       { Parser_instruction_R_parse(parser, Instruction_Kind__AND);                   } break;

			// Pseudo-instructions

			// Pseudo-instructions (no operands)
			case HASH_nop:       { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__NOP);       } break;
			case HASH_ret:       { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__RET);       } break;
			// Pseudo-instructions (rd, rs)
			case HASH_mv:        { Parser_instruction_mv_parse(parser);                                         } break;
			case HASH_not:       { Parser_instruction_not_parse(parser);                                        } break;
			case HASH_sext_w:    { Parser_instruction_sext_w_parse(parser);                                     } break;
			case HASH_neg:       { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__NEG);            } break;
			case HASH_negw:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__NEGW);           } break;
			case HASH_seqz:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SEQZ);           } break;
			case HASH_snez:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SNEZ);           } break;
			case HASH_sltz:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SLTZ);           } break;
			case HASH_sgtz:      { Parser_instruction_R_pseudo_parse(parser, Instruction_Kind__SGTZ);           } break;
			// Pseudo-instructions (rs, offset)
			case HASH_beqz:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BEQZ);           } break;
			case HASH_bnez:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BNEZ);           } break;
			case HASH_blez:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BLEZ);           } break;
			case HASH_bgez:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BGEZ);           } break;
			case HASH_bltz:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BLTZ);           } break;
			case HASH_bgtz:      { Parser_instruction_B_pseudo_parse(parser, Instruction_Kind__BGTZ);           } break;
			// Pseudo-instructions (rs, rt, offset)
			case HASH_bgt:       { Parser_instruction_B_parse(parser, Instruction_Kind__BGT);                   } break;
			case HASH_ble:       { Parser_instruction_B_parse(parser, Instruction_Kind__BLE);                   } break;
			case HASH_bgtu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BGTU);                  } break;
			case HASH_bleu:      { Parser_instruction_B_parse(parser, Instruction_Kind__BLEU);                  } break;
			// Pseudo-instructions (offset only)
			case HASH_j:         { Parser_instruction_j_parse(parser);                                          } break;
			case HASH_call:      { Parser_instruction_call_parse(parser);                                       } break;
			case HASH_tail:      { Parser_instruction_tail_parse(parser);                                       } break;
			// Pseudo-instructions (rs only)
			case HASH_jr:        { Parser_instruction_jr_parse(parser);                                         } break;
			// Pseudo-instructions (rd, imm/symbol)
			case HASH_li:        { Parser_instruction_li_parse(parser);                                         } break;
			case HASH_la:        { Parser_instruction_la_parse(parser);                                         } break;

			// Others
			case HASH_ecall:     { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__ECALL);     } break;
			case HASH_ebreak:    { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__EBREAK);    } break;
			case HASH_pause:     { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__PAUSE);     } break;
			case HASH_fence_tso: { Parser_instruction_mnemonic_only_parse(parser, Instruction_Kind__FENCE_TSO); } break;
			case HASH_fence:     { Parser_instruction_fence_parse(parser);                                      } break;

			default:
			{
				Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
			} break;
			}

			Instruction_Kind instruction_kind = parser->statement_context->instruction_kind;
			B32 expandable = Instruction_Encoding_table[instruction_kind].flags & Instruction_Flags__Expandable;
			if (expandable)
			{
				parser->statement_context->flags |= Statement_Flags__Size_Variable;
			}

			// It is at most one.
			parser->statement_context->expressions_count = parser->statement_context->expressions_indexes ? 1 : 0;
		} break;
		default:
		{
			Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
		} break;
		}

		B32 loop_infinite_avoided = parser->token_index > parser->token_index_before || parser->error.kind || parser->end_reached;
		assert_always_m(loop_infinite_avoided && "infinite loop in parser");

		parser->statement_context->token_index_end = parser->token_index - 1;

		// The iteration has produced a new statement.
		if (token_start_kind != Token_Kind__Newline)
		{
			// We should have reached a newline, or the statement should be a label definition, otherwise there is junk.
			B32 correct_end_of_line = parser->token_current.kind == Token_Kind__Newline;
			Parser_expect(parser, correct_end_of_line, Parser_Error_Kind__Line_Extra_Content);
			Statements_push(parser->statements, *parser->statement_context);
		}
	}

	return;
}
