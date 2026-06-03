internal void
diagnostic_print
(
	Diagnostic *diagnostic,
	Source     *source,
	// For lazily computing the line start indexes.
	Arena      *arena
)
{
	if (!source->line_start_indexes)
	{
		// Fill it
		U32 index   = 0;
		U32 counter = 1;
		for (;;)
		{
			if (index >= source->count)
			{
				break;
			}
			counter += source->data[index] == '\n';
			index   += 1;
		}

		source->line_start_indexes = Arena__push_array_m(arena, U32, counter);
		source->line_start_count   = counter;

		index   = 0;
		counter = 1;
		for (;;)
		{
			if (index >= source->count)
			{
				break;
			}
			if (source->data[index] == '\n')
			{
				source->line_start_indexes[counter] = index + 1;
			}
			index += 1;
		}
	}

	U32 row_index    = floor_search(source->line_start_indexes, source->line_start_count, diagnostic->location);
	U32 column_index = diagnostic->location - source->start_offset_logical - source->line_start_indexes[row_index];
	U32 line_number   = row_index    + 1;
	U32 column_number = column_index + 1;

	U32 line_start_index      = source->line_start_indexes[row_index];

	U32 index = 0;
	// for (;;)
	// {
	// 	if (source->data[index] == '\n' || index >= source->count)
	// 	{
	// 		break;
	// 	}
	// 	index += 1;
	// }
	// line_characters_count = index + 1;
	U8 *line = &source->data[line_start_index];

	fprintf(stderr, "\x1B[1m%s:%d:%d:\x1B[0m \x1B[1;31merror:\x1B[0m ", source->name, line_number, column_number);
	fprintf(stderr, "%s\n",  diagnostic->message.data);
	fprintf(stderr, "%5d | ", line_number);

	U32 newline_index = 0;
	index = 0;
	for (;;)
	{
		U8 character = line[index];
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

		if (index == column_index)
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
