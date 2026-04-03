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

#endif // LANGUAGE_RELOCATION_H

