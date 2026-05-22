void
Diagnostic__print(Diagnostic *diagnostic, String8 *input)
{
	assert_always_m(diagnostic->column_index_begin <= diagnostic->column_index_end);

	U32 line = diagnostic->row_index += 1;

	fprintf(stderr, "\x1B[1m%s:%d:%d:\x1B[0m \x1B[1;31merror:\x1B[0m ", diagnostic->filename, line, diagnostic->column_index_begin);
	fprintf(stderr, "%s\n",   diagnostic->message_kind);
	fprintf(stderr, "%5d | ", line);

	U64 index = diagnostic->input_index_start;

	for (;;)
	{
		if (input->data[index] == '\n')
		{
			fputc('\n', stderr);
			break;
		}
		else if (input->data[index] == '\t')
		{
			fputc(' ', stderr);
		}
		else
		{
			fputc(input->data[index], stderr);
		}
		index += 1;
	}

	fprintf(stderr, "      | ");
	index = 0;

	for (;;)
	{
		if (index == diagnostic->column_index_begin)
		{
			fprintf(stderr, "\x1B[1;31m^");
			U32 tilde_index = 0;
			// There is already the caret, otherwise we output an extra tilde.
			U32 tilde_count = diagnostic->column_index_end - diagnostic->column_index_begin;
			for (;;)
			{
				if (tilde_index < tilde_count)
				{
					fputc('~', stderr);
					tilde_index += 1;
				}
				else
				{
					break;
				}
			}
			fprintf(stderr, "\x1B[0m\n");
			break;
		}

		fputc(' ', stderr);
		index += 1;
	}

	return;
}
