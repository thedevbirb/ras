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

// TODO: how am I transforming output after this stage?
internal void
Parser_parse(Parser *parser)
{
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
			key.count -= key.count > 0; // Remove the colon.

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
			case Directive_Kind__Section:
			{
				Parser_advance(parser);

				String8 substring             = Parser_token_string(parser);
				Directive_Kind directive_kind = Directive_Kind__from_String8(substring);
				ELF64_Section section_index   = ELF64_Section_from_Directive_Kind[directive_kind];
				parser->section_current       = &parser->sections[section_index];

				Parser_expect(parser, section_index != 0, Parser_Error_Kind__Label_Duplicate);
				Parser_advance(parser);
			} break;
			case Directive_Kind__Align:
			{
				// Parser_advance(parser);
				// if (!token_next || token_next->kind == Token_Kind__Newline)
				// {
				// 	error = Parser_Error_new(Parser_Error_Kind__Directive_Align_Argument_Missing, parser);
				// }
				// // Actually, here I should grab all the tokens until newline, and try to parse the
				// // expression.
				// else if (token_next->kind == Token_Kind__Number_Literal)
				// {
				// 	// TODO: parse the number
				// 	Parser_advance(parser);
				// }
				// else
				// {
				// 	Parser_advance(parser);
				// 	error = Parser_Error_new(Parser_Error_Kind__Directive_Align_Argument_Invalid, parser);
				// }
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
