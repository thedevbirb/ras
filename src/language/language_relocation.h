#ifndef LANGUAGE_RELOCATION_H
#define LANGUAGE_RELOCATION_H

typedef enum Relocation_Operator
{
	Relocation_Operator__None = 0,
	Relocation_Operator__hi,              // %hi(symbol)              -> R_RISCV_HI20
	Relocation_Operator__lo,              // %lo(symbol)              -> R_RISCV_LO12_I / _S
	Relocation_Operator__pcrel_hi,        // %pcrel_hi(symbol)        -> R_RISCV_PCREL_HI20
	Relocation_Operator__pcrel_lo,        // %pcrel_lo(label)         -> R_RISCV_PCREL_LO12_I / _S
	Relocation_Operator__tprel_hi,        // %tprel_hi(symbol)        -> R_RISCV_TPREL_HI20
	Relocation_Operator__tprel_lo,        // %tprel_lo(symbol)        -> R_RISCV_TPREL_LO12_I / _S
	Relocation_Operator__tprel_add,       // %tprel_add(symbol)       -> R_RISCV_TPREL_ADD
	Relocation_Operator__got_pcrel_hi,    // %got_pcrel_hi(symbol)    -> R_RISCV_GOT_HI20
	Relocation_Operator__tls_ie_pcrel_hi, // %tls_ie_pcrel_hi(symbol) -> R_RISCV_TLS_GOT_HI20
	Relocation_Operator__tls_gd_pcrel_hi, // %tls_gd_pcrel_hi(symbol) -> R_RISCV_TLS_GD_HI20
	Relocation_Operator__COUNT,
} Relocation_Operator;

// Without the % sign prepended.
global const char *Relocation_Operator_strings[Relocation_Operator__COUNT] =
{
	[Relocation_Operator__None]            = "",
	[Relocation_Operator__hi]              = "hi",
	[Relocation_Operator__lo]              = "lo",
	[Relocation_Operator__pcrel_hi]        = "pcrel_hi",
	[Relocation_Operator__pcrel_lo]        = "pcrel_lo",
	[Relocation_Operator__tprel_hi]        = "tprel_hi",
	[Relocation_Operator__tprel_lo]        = "tprel_lo",
	[Relocation_Operator__tprel_add]       = "tprel_add",
	[Relocation_Operator__got_pcrel_hi]    = "got_pcrel_hi",
	[Relocation_Operator__tls_ie_pcrel_hi] = "tls_ie_pcrel_hi",
	[Relocation_Operator__tls_gd_pcrel_hi] = "tls_gd_pcrel_hi",
};

global const U8 Relocation_Operator_strings_sizes[Relocation_Operator__COUNT] =
{
	[Relocation_Operator__None]            = 0,
	[Relocation_Operator__hi]              = 2,
	[Relocation_Operator__lo]              = 2,
	[Relocation_Operator__pcrel_hi]        = 8,
	[Relocation_Operator__pcrel_lo]        = 8,
	[Relocation_Operator__tprel_hi]        = 8,
	[Relocation_Operator__tprel_lo]        = 8,
	[Relocation_Operator__tprel_add]       = 9,
	[Relocation_Operator__got_pcrel_hi]    = 12,
	[Relocation_Operator__tls_ie_pcrel_hi] = 15,
	[Relocation_Operator__tls_gd_pcrel_hi] = 15,
};


Relocation_Operator
Relocation_Operator_lookup(String8 relocation);

Relocation_RISC_V
Relocation_RISC_V_from_Relocation_Operator(Relocation_Operator operator, Instruction_Format format)
{
	Relocation_RISC_V result = Relocation_RISC_V__None;

	switch (operator)
	{
		case Relocation_Operator__None:
		{
			result = Relocation_RISC_V__None;
		} break;
		case Relocation_Operator__hi:
		{
			result = Relocation_RISC_V__High_20;
		} break;
		case Relocation_Operator__lo:
		{
			result = format == Instruction_Format__S ? Relocation_RISC_V__Low_12_S_Type : Relocation_RISC_V__Low_12_I_Type;
		} break;
		case Relocation_Operator__pcrel_hi:
		{
			result = Relocation_RISC_V__PC_Relative_High_20;
		} break;
		case Relocation_Operator__pcrel_lo:
		{
			result = format == Instruction_Format__S ? Relocation_RISC_V__PC_Relative_Low_12_S_Type : Relocation_RISC_V__PC_Relative_Low_12_I_Type;
		} break;
		case Relocation_Operator__tprel_hi:
		{
			result = Relocation_RISC_V__Thread_Pointer_Relative_High_20;
		} break;
		case Relocation_Operator__tprel_lo:
		{
			result = format == Instruction_Format__S ? Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type : Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type;
		} break;
		case Relocation_Operator__tprel_add:
		{
			result = Relocation_RISC_V__Thread_Pointer_Relative_Add;
		} break;
		case Relocation_Operator__got_pcrel_hi:
		{
			result = Relocation_RISC_V__GOT_High_20;
		} break;
		case Relocation_Operator__tls_ie_pcrel_hi:
		{
			result = Relocation_RISC_V__TLS_GOT_High_20;
		} break;
		case Relocation_Operator__tls_gd_pcrel_hi:
		{
			result = Relocation_RISC_V__TLS_Global_Dynamic_High_20;
		} break;
		default: {} break;
	}

	return result;
}

#endif // LANGUAGE_RELOCATION_H

