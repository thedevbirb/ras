// Relocation_Operator
// Relocation_Operator_lookup(String8 relocation)
// {
// 	Relocation_Operator operator = Relocation_Operator__None;
//
// 	U32 index = 0;
// 	for (;;)
// 	{
// 		B32 break_should = operator || index >= Relocation_Operator__COUNT;
// 		if (break_should)
// 		{
// 			break;
// 		}
//
// 		const char *target = Relocation_Operator_strings[index];
// 		U8 target_size     = Relocation_Operator_strings_sizes[index];
// 		U8 match_to_size   = min_m(relocation.count, target_size);
// 		S32 match          = memory_match((void *)relocation.data, (void *)target, match_to_size);
// 		operator           = ~((match == 0) - 1) & index;
//
// 		index += 1;
//
// 	}
//
// 	return operator;
// }

Relocation_RISC_V
Relocation_RISC_V__lookup(String8 string, Instruction_Format instruction_format)
{
	Relocation_RISC_V result = Relocation_RISC_V__None;

	if      (String8__match_exact(string, String8__literal("hi")))
	{
		result = Relocation_RISC_V__High_20;
	}
	else if (String8__match_exact(string, String8__literal("lo")))
	{
		if (instruction_format == Instruction_Format__S)
		{
			result = Relocation_RISC_V__Low_12_S_Type;
		}
		else if (instruction_format == Instruction_Format__I)
		{
			result = Relocation_RISC_V__Low_12_I_Type;
		}
	}
	else if (String8__match_exact(string, String8__literal("pcrel_hi")))
	{
		result = Relocation_RISC_V__PC_Relative_High_20;
	}
	else if (String8__match_exact(string, String8__literal("pcrel_lo")))
	{
		if (instruction_format == Instruction_Format__S)
		{
			result = Relocation_RISC_V__PC_Relative_Low_12_S_Type;
		}
		else if (instruction_format == Instruction_Format__I)
		{
			result = Relocation_RISC_V__PC_Relative_Low_12_I_Type;
		}
	}
	else if (String8__match_exact(string, String8__literal("tprel_hi")))
	{
		result = Relocation_RISC_V__Thread_Pointer_Relative_High_20;
	}
	else if (String8__match_exact(string, String8__literal("tprel_lo")))
	{
		if (instruction_format == Instruction_Format__S)
		{
			result = Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type;
		}
		else if (instruction_format == Instruction_Format__I)
		{
			result = Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type;
		}
	}
	else if (String8__match_exact(string, String8__literal("tprel_add")))
	{
		result = Relocation_RISC_V__Thread_Pointer_Relative_Add;
	}
	else if (String8__match_exact(string, String8__literal("got_pcrel_hi")))
	{
		result = Relocation_RISC_V__GOT_High_20;
	}
	else if (String8__match_exact(string, String8__literal("tls_ie_pcrel_hi")))
	{
		result = Relocation_RISC_V__TLS_GOT_High_20;
	}
	else if (String8__match_exact(string, String8__literal("tls_gd_pcrel_hi")))
	{
		result = Relocation_RISC_V__TLS_Global_Dynamic_High_20;
	}

	return result;
}

global const String8 Relocation_RISC_V_operator_strings[Relocation_RISC_V__COUNT] =
{
	[Relocation_RISC_V__High_20]                                    = String8__literal("hi"),
	[Relocation_RISC_V__Low_12_I_Type]                              = String8__literal("lo"),
	[Relocation_RISC_V__Low_12_S_Type]                              = String8__literal("lo"),
	[Relocation_RISC_V__PC_Relative_High_20]                        = String8__literal("pcrel_hi"),
	[Relocation_RISC_V__PC_Relative_Low_12_I_Type]                  = String8__literal("pcrel_lo"),
	[Relocation_RISC_V__PC_Relative_Low_12_S_Type]                  = String8__literal("pcrel_lo"),
	[Relocation_RISC_V__Thread_Pointer_Relative_High_20]            = String8__literal("tprel_hi"),
	[Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type]      = String8__literal("tprel_lo"),
	[Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type]      = String8__literal("tprel_lo"),
	[Relocation_RISC_V__Thread_Pointer_Relative_Add]                = String8__literal("tprel_add"),
	[Relocation_RISC_V__GOT_High_20]                                = String8__literal("got_pcrel_hi"),
	[Relocation_RISC_V__TLS_GOT_High_20]                            = String8__literal("tls_ie_pcrel_hi"),
	[Relocation_RISC_V__TLS_Global_Dynamic_High_20]                 = String8__literal("tls_gd_pcrel_hi"),
};
