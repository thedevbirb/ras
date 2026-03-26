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
		.column_begin_index = parser->token_current.column_index,
		.column_end_index   = parser->token_current.column_index + parser->token_current.size - 1,
	};

	assert_always_m(error.column_begin_index <= error.column_end_index && "token_index bug");
	parser->error = error;

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
	B32 condition = parser->token_current.kind = token_kind;
	Parser_expect(parser, condition, error_kind);
	return;
}

// TODO: how am I transforming output after this stage?
internal void
Parser_parse(Parser *parser)
{
	U8 data_directive_size = 0;
	for (;;)
	{
		B32 break_should = parser->end_reached || parser->error.kind;
		if (break_should)
		{
			break;
		}

		U32 index_before = parser->token_index;

		switch (parser->token_current.kind)
		{
		case Token_Kind__Newline: { Parser_advance(parser); } break;
		case Token_Kind__Label:
		{
			String8 key = Parser_token_string(parser);
			U32 offset = Object_File_Section_write(parser->section_string_table, key.data, key.count);
			ELF64_Symbol symbol =
			{
				.string_table_offset = offset,
				.value = parser->section_current->offset,
				.section_index = parser->section_current->section_index,
			};
			B32 overridden = Symbols_Table_put(parser->symbols_table, key, symbol);
			Parser_expect(parser, !overridden, Parser_Error_Kind__Label_Duplicate);
			Parser_advance(parser);
		} break;
		case Token_Kind__Directive:
		{
			String8 substring = Parser_token_string(parser);
			Directive_Kind directive_kind = Directive_Kind__from_String8(substring);

			switch (directive_kind)
			{
			case Directive_Kind__None:
			{
				Parser_error_set(parser, Parser_Error_Kind__Directive_Unknown);
			} break;
			case Directive_Kind__Word_Double: { data_directive_size += 1; } // fallthrough
			case Directive_Kind__Word:        { data_directive_size += 1; } // fallthrough
			case Directive_Kind__Word_Half:   { data_directive_size += 1; } // fallthrough
			case Directive_Kind__Byte:
			{
				data_directive_size += 1;

				// Format: .byte <expr_1> , ..., <expr_n>
				for (;;)
				{
					Parser_advance(parser);
					Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Immediate);
					U64 value = Parser_expression_evaluate(parser, expression);

					U64 bit_size   = 8 << data_directive_size - 1;
					B32 size_valid = value >> bit_size == 0;
					Parser_expect(parser, size_valid, Parser_Error_Kind__Directive_Data_Value_Size_Invalid);

					U8 data[8] = {0};
					os_memory_copy(data, &value, 8);

					// Little-endian here plays well with data_directive_size.
					Object_File_Section_write(parser->section_current, data, data_directive_size);

					B32 token_newline = parser->token_current.kind == Token_Kind__Newline;
					B32 token_comma   = parser->token_current.kind == Token_Kind__Comma;

					Parser_expect(parser, token_comma || token_newline, Parser_Error_Kind__Directive_Data_Invalid);

					B32 break_should = parser->error.kind || parser->end_reached || token_newline;
					if (break_should)
					{
						break;
					}
				}

				data_directive_size = 0;
			} break;
			case Directive_Kind__Section:
			{
				Parser_advance(parser);

				String8 substring             = Parser_token_string(parser);
				Directive_Kind directive_kind = Directive_Kind__from_String8(substring);
				ELF64_Section section_index   = ELF64_Section_from_Directive_Kind[directive_kind];
				parser->section_current       = &parser->sections[section_index];

				Parser_expect(parser, section_index != 0, Parser_Error_Kind__Directive_Section_Argument_Invalid);
				Parser_advance(parser);
			} break;
			case Directive_Kind__Align:
			{
				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Immediate);
				U64 result = Parser_expression_evaluate(parser, expression);
				Object_File_Section_align(parser->section_current, result);
			} break;
			case Directive_Kind__Equality:
			{
				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Identifier, Parser_Error_Kind__Identifier_Expected);
				String8 key = Parser_token_string(parser);

				Parser_advance(parser);
				Parser_expect_token(parser, Token_Kind__Comma, Parser_Error_Kind__Comma_Expected);

				Parser_advance(parser);
				Expression_Node *expression = Parser_expression_parse(parser, Expression_Flags__Immediate);
				U64 result = Parser_expression_evaluate(parser, expression);

				U32 offset = Object_File_Section_write(parser->section_string_table, key.data, key.count);
				ELF64_Symbol symbol =
				{
					.string_table_offset = offset,
					.value = result,
					.section_index = parser->section_current->section_index,
				};
				Symbols_Table_put(parser->symbols_table, key, symbol);

				// TODO: Do I have to write in some section? Data?
			} break;
			default:
			{
				ELF64_Section section_kind = ELF64_Section_from_Directive_Kind[directive_kind];
				assert_always_m(section_kind && "unhandled directive");

				parser->section_current = &parser->sections[section_kind];
				Parser_advance(parser);
			} break;
			}
		} break;
		default:
		{
			Parser_error_set(parser, Parser_Error_Kind__Line_Invalid);
		} break;
		}

		B32 loop_infinite_avoided = parser->token_index > index_before || parser->error.kind;
		assert_always_m(loop_infinite_avoided && "infinite loop in parser");
	}

	return;
}
