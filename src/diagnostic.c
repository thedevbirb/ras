internal void
Diagnostic__print(Diagnostic *diagnostic)
{
	U32 line   = diagnostic->row_index += 1;
	U32 column = diagnostic->column_index += 1;

	fprintf(stderr, "\x1B[1m%s:%d:%d:\x1B[0m \x1B[1;31merror:\x1B[0m ", diagnostic->filename.data, line, column);
	fprintf(stderr, "%s\n",  diagnostic->message.data);
	fprintf(stderr, "%5d | ", line);

	U32 newline_index = 0;
	U32 index = 0;
	for (;;)
	{
		U8 character = diagnostic->line[index];
		character = character == '\t' ? ' ' : character;
		fputc(character, stderr);
		if (character == '\n')
		{
			newline_index = index;
			break;
		}
		index += 1;
	}

	fprintf(stderr, "      | ");
	index = 0;
	U8 character = ' ';
	for (;;)
	{
		if (index == newline_index)
		{
			break;
		}

		if (index == diagnostic->column_index)
		{
			character = '^';
		}

		for (U32 j = 0; character == ' ' && j < array_count_m(diagnostic->ranges); j++)
		{
			Vec2_U32 range = diagnostic->ranges[j];
			if (contains_U32(range, index))
			{
				character = '~';
			}
		}

		fprintf(stderr, "\x1B[1;31m%c\x1B[0m", character);

		index += 1;
		character = ' ';
	}
	fprintf(stderr, "\n");

	return;
}
