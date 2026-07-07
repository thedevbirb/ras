#ifndef RISCV_RELOCATION_H
#define RISCV_RELOCATION_H

typedef struct Relocation_Operator Relocation_Operator;
struct Relocation_Operator
{
        String8 text;
        Relocation_RISC_V relocation;
};

typedef struct Relocation_Operator_List Relocation_Operator_List;
struct Relocation_Operator_List
{
        const Relocation_Operator *data;
        U64 count;
};


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

global const Relocation_Operator Relocation_Operator__utype[] =
{
        {String8__literal("tprel_hi"),        Relocation_RISC_V__Thread_Pointer_Relative_High_20},
        {String8__literal("pcrel_hi"),        Relocation_RISC_V__PC_Relative_High_20            },
        {String8__literal("got_pcrel_hi"),    Relocation_RISC_V__GOT_32_PC_Relative             },
        {String8__literal("tlsdesc_hi"),      Relocation_RISC_V__TLS_Descriptor_High_20         },
        {String8__literal("tls_ie_pcrel_hi"), Relocation_RISC_V__GOT_High_20                    },
        {String8__literal("tls_gd_pcrel_hi"), Relocation_RISC_V__TLS_Global_Dynamic_High_20     },
        {String8__literal("hi"),              Relocation_RISC_V__High_20                        },
};

global const Relocation_Operator Relocation_Operator__itype[] =
{
        {String8__literal("lo"),              Relocation_RISC_V__Low_12_I_Type                        },
        {String8__literal("tprel_lo"),        Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type},
        {String8__literal("pcrel_lo"),        Relocation_RISC_V__PC_Relative_Low_12_I_Type            },
        {String8__literal("tlsdesc_load_lo"), Relocation_RISC_V__TLS_Descriptor_Load_Low_12           },
        {String8__literal("tlsdesc_add_lo"),  Relocation_RISC_V__TLS_Descriptor_Add_Low_12            },
};

global const Relocation_Operator Relocation_Operator__stype[] =
{
        {String8__literal("lo"),       Relocation_RISC_V__Low_12_S_Type                        },
        {String8__literal("tprel_lo"), Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type},
        {String8__literal("pcrel_lo"), Relocation_RISC_V__PC_Relative_Low_12_S_Type            },
};

global const Relocation_Operator Relocation_Operator__relax_only[] =
{
        {String8__literal("tlsdesc_call"), Relocation_RISC_V__TLS_Descriptor_Call        },
        {String8__literal("tprel_add"),    Relocation_RISC_V__Thread_Pointer_Relative_Add},
};

global const Relocation_Operator_List Relocation_Operator_List__utype      = { .data = Relocation_Operator__utype,      .count = array_count_m(Relocation_Operator__utype)      };
global const Relocation_Operator_List Relocation_Operator_List__itype      = { .data = Relocation_Operator__itype,      .count = array_count_m(Relocation_Operator__itype)      };
global const Relocation_Operator_List Relocation_Operator_List__stype      = { .data = Relocation_Operator__stype,      .count = array_count_m(Relocation_Operator__stype)      };
global const Relocation_Operator_List Relocation_Operator_List__relax_only = { .data = Relocation_Operator__relax_only, .count = array_count_m(Relocation_Operator__relax_only) };

#endif // RISCV_RELOCATION_H
