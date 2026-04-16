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

global const B32 Relocation_Operator_low_12[Relocation_Operator__COUNT] =
{
	[Relocation_Operator__lo]              = 1,
	[Relocation_Operator__pcrel_lo]        = 1,
	[Relocation_Operator__tprel_lo]        = 1,
};

global const B32 Relocation_Operator_high_20[Relocation_Operator__COUNT] =
{
	[Relocation_Operator__None]            = 1,
	[Relocation_Operator__pcrel_hi]        = 1,
	[Relocation_Operator__tprel_hi]        = 1,
	[Relocation_Operator__got_pcrel_hi]    = 1,
	[Relocation_Operator__tls_ie_pcrel_hi] = 1,
	[Relocation_Operator__tls_gd_pcrel_hi] = 1,
};

Relocation_Operator
Relocation_Operator_lookup(String8 relocation);

// Distinguishes between [not S-type] and [S-type].
//
// Smaller and faster compared to using a switch.
global const Relocation_RISC_V Relocation_RISCV_from_Relocation_Operator_table[Relocation_Operator__COUNT][2] =
{
    [Relocation_Operator__None]            =
    {
	    Relocation_RISC_V__None,
	    Relocation_RISC_V__None
    },
    [Relocation_Operator__hi]              =
    {
	    Relocation_RISC_V__High_20,
	    Relocation_RISC_V__High_20
    },
    [Relocation_Operator__lo]              =
    {
	    Relocation_RISC_V__Low_12_I_Type,
	    Relocation_RISC_V__Low_12_S_Type
    },
    [Relocation_Operator__pcrel_hi]        =
    {
	    Relocation_RISC_V__PC_Relative_High_20,
	    Relocation_RISC_V__PC_Relative_High_20
    },
    [Relocation_Operator__pcrel_lo]        =
    {
	    Relocation_RISC_V__PC_Relative_Low_12_I_Type,
	    Relocation_RISC_V__PC_Relative_Low_12_S_Type
    },
    [Relocation_Operator__tprel_hi]        =
    {
	    Relocation_RISC_V__Thread_Pointer_Relative_High_20,
	    Relocation_RISC_V__Thread_Pointer_Relative_High_20
    },
    [Relocation_Operator__tprel_lo]        =
    {
	    Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type,
	    Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type
    },
    [Relocation_Operator__tprel_add]       =
    {
	    Relocation_RISC_V__Thread_Pointer_Relative_Add,
	    Relocation_RISC_V__Thread_Pointer_Relative_Add
    },
    [Relocation_Operator__got_pcrel_hi]    =
    {
	    Relocation_RISC_V__GOT_High_20,
	    Relocation_RISC_V__GOT_High_20
    },
    [Relocation_Operator__tls_ie_pcrel_hi] =
    {
	    Relocation_RISC_V__TLS_GOT_High_20,
	    Relocation_RISC_V__TLS_GOT_High_20
    },
    [Relocation_Operator__tls_gd_pcrel_hi] =
    {
	    Relocation_RISC_V__TLS_Global_Dynamic_High_20,
	    Relocation_RISC_V__TLS_Global_Dynamic_High_20
    },
};

Relocation_RISC_V
Relocation_RISC_V_from_Relocation_Operator(Relocation_Operator operator, Instruction_Format format)
{
    B32 s_type_is = (format == Instruction_Format__S);
    return Relocation_RISCV_from_Relocation_Operator_table[operator][s_type_is];
}

global const B32 Relocation_RISC_V_low_12[Relocation_RISC_V__COUNT] =
{
	[Relocation_RISC_V__Low_12_I_Type]                              = 1,
	[Relocation_RISC_V__Low_12_S_Type]                              = 1,
	[Relocation_RISC_V__PC_Relative_Low_12_I_Type]                  = 1,
	[Relocation_RISC_V__PC_Relative_Low_12_S_Type]                  = 1,
	[Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type]      = 1,
	[Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type]      = 1,
};

global const B32 Relocation_RISC_V_high_20[Relocation_RISC_V__COUNT] =
{
	[Relocation_RISC_V__High_20]                                    = 1,
	[Relocation_RISC_V__PC_Relative_High_20]                        = 1,
	[Relocation_RISC_V__Thread_Pointer_Relative_High_20]            = 1,
	[Relocation_RISC_V__GOT_High_20]                                = 1,
	[Relocation_RISC_V__TLS_GOT_High_20]                            = 1,
	[Relocation_RISC_V__TLS_Global_Dynamic_High_20]                 = 1,
};

#endif // LANGUAGE_RELOCATION_H

