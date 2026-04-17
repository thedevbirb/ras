// TODO: probably can change with just memcmp
Directive_Kind
Directive_Kind__from_String8(String8 string)
{
	Directive_Kind kind = Directive_Kind__None;

	U32 token_index = 0;
	B32 found = 0;
	for (;;)
	{
		B32 break_should_outer = found || token_index >= Directive_Kind__COUNT;
		if (break_should_outer)
		{
			break;
		}

		const char *target = Directive_Kind_strings[token_index];

		U32 index_match = 0;
		B32 mismatch = 0;
		for (;;)
		{
			B32 break_should_inner = mismatch || index_match >= string.count || target[index_match] == '\0';
			if (break_should_inner)
			{
				break;
			}

			mismatch = string.data[index_match] != target[index_match];
			index_match += 1;
		}

		found = !mismatch && index_match == string.count && target[index_match] == '\0';
		if (found)
		{
			kind = token_index;
		}
		else
		{
			token_index += 1;
		}
	}

	return kind;
}
