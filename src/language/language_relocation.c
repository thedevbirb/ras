Relocation_Operator
Relocation_Operator_lookup(String8 relocation)
{
	Relocation_Operator operator = Relocation_Operator__None;

	U32 index = 0;
	for (;;)
	{
		B32 break_should = operator || index >= Relocation_Operator__COUNT;
		if (break_should)
		{
			break;
		}

		const char *target = Relocation_Operator_strings[index];
		U8 target_size     = Relocation_Operator_strings_sizes[index];
		U8 match_to_size   = min_m(relocation.count, target_size);
		S32 match          = os_memory_match((void *)relocation.data, (void *)target, match_to_size);
		operator           = ~((match == 0) - 1) & index;

		index += 1;

	}

	return operator;
}
