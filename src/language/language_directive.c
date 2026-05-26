Directive_Kind
Directive_Kind__from_String8(String8 source)
{
	Directive_Kind result = Directive_Kind__None;
	U32 index = Directive_Kind__None;
	B32 found = 0;

	for (;;)
	{
		B32 break_should = found || index >= Directive_Kind__COUNT;
		if (break_should)
		{
			break;
		}

		const String8 target = Directive_Kind__String8_table[index];
		found = source.count == target.count && memory_match(source.data, target.data, source.count) == 0;
		if (found)
		{
			result = index;
		}

		index += 1;
	}
	return result;
}
