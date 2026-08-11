#ifndef ELF_H
#define ELF_H

// Taken and adapted from https://sourceware.org/git/?p=glibc.git;a=blob_plain;f=elf/elf.h;hb=HEAD.

/* Standard ELF types. */

/* Type of addresses.  */
typedef U32 Address32;
typedef U64 Address64;

/* Type of file offsets.  */
typedef U32 Offset32;
typedef U64 Offset64;

/* Type for section indices, which are 16-bit quantities.  */
typedef U16 ELF32_Section_Index;
typedef U16 ELF_Section_Index;

/* Type for version symbol information.  */
typedef U16 ELF32_Version_Symbol;
typedef U16 ELF64_Version_Symbol;


/* The ELF file header.  This appears at the start of every ELF file.  */

#define ELF_id_size (16)

typedef struct ELF32_Header ELF32_Header;
struct ELF32_Header
{
        U8  identifier[ELF_id_size];
        U16 object_file_type;
        U16 architecture;
        U32 object_file_version;
        U32 entry_point_virtual_address;
        U32 program_header_table_file_offset;
        U32 section_header_table_file_offset;
        U32 processor_flags;
        U16 header_size;
        U16 program_header_table_entry_size;
        U16 program_header_table_entry_count;
        U16 section_header_table_entry_size;
        U16 section_header_table_entry_count;
        U16 section_header_string_table_index;
};

typedef struct ELF64_Header ELF64_Header;
struct ELF64_Header
{
        U8  identifier[ELF_id_size];
        U16 object_file_type;
        U16 architecture;
        U32 object_file_version;
        U64 entry_point_virtual_address;
        U64 program_header_table_file_offset;
        U64 section_header_table_file_offset;
        U32 processor_flags;
        U16 header_size;
        U16 program_header_table_entry_size;
        U16 program_header_table_entry_count;
        U16 section_header_table_entry_size;
        U16 section_header_table_entry_count;
        U16 section_header_string_table_index;
};

#define ELF_ID_Magic__Index 0
#define ELF_ID_Magic__0     0x7f
#define ELF_ID_Magic__1     'E'
#define ELF_ID_Magic__1     'E'
#define ELF_ID_Magic__2     'L'
#define ELF_ID_Magic__3     'F'

// 32-bit or 64-bit class.
#define ELF_ID_Class__Index  4
#define ELF_ID_Class__None   0
#define ELF_ID_Class__32     1
#define ELF_ID_Class__64     2

#define ELF_ID_Data__Index   5
#define ELF_ID_Data__None    0
#define ELF_ID_Data__2LSB    1
#define ELF_ID_Data__2MSB    2

#define ELF_ID_Version__Index   6
#define ELF_ID_Version__Current 1

#define ELF_ID_OS_ABI__Index      7
#define ELF_ID_OS_ABI__None       0 /* Unix System V ABI */
#define ELF_ID_OS_ABI__SYSV       1 /* Alias */
#define ELF_ID_OS_ABI__GNU        3 /* GNU ELF extensions. */
#define ELF_ID_OS_ABI__Linux      ELF_ID_OS_ABI__GNU
#define ELF_ID_OS_ABI__Standalone 255 /* Embedded application */

// TODO: which value?
#define ELF_ID_ABI_Version__Index 8

#define ELF_ID_Padding__Index     9

/* Legal values for e_type (object file type).  */

#define ELF_Type__NONE               0  /* No file type */
#define ELF_Type__Relocatable        1  /* Relocatable file */
#define ELF_Type__Executable         2  /* Executable file */
#define ELF_Type__Shared             3  /* Shared object file */
#define ELF_Type__Core               4  /* Core file */
#define ELF_Type__LOOS          0xfe00  /* OS-specific range start */
#define ELF_Type__HIOS          0xfeff  /* OS-specific range end */
#define ELF_Type__LOPROC        0xff00  /* Processor-specific range start */
#define ELF_Type__HIPROC        0xffff  /* Processor-specific range end */

#define ELF_Machine__RISCV 243

/* Section header.  */

typedef struct ELF32_Section_Header ELF32_Section_Header;
struct ELF32_Section_Header
{
        U32 string_table_offset;
        U32 type;
        U32 flags;
        U32 address_virtual;
        U32 offset;
        U32 size;
        U32 link;
        U32 info;
        U32 alignment;
        U32 entry_size;
};

typedef struct ELF64_Section_Header ELF64_Section_Header;
struct ELF64_Section_Header
{
        U32 string_table_offset; /* Section name (string tbl index) */
        U32 type;                /* Section type */
        U64 flags;               /* Section flags */
        U64 address_virtual;     /* Section virtual addr at execution */
        U64 offset;              /* Section file offset */
        U64 size;                /* Section size in bytes */
        U32 link;                /* Link to another section */
        U32 info;                /* Additional section information */
        U64 alignment;           /* Section alignment */
        U64 entry_size;          /* Entry size if section holds table */
};

/* Special section indices.  */

#define ELF_Section_Index__Undefined        0                /* Undefined section */
#define ELF_Section_Index__LORESERVE        0xff00                /* Start of reserved indices */
#define ELF_Section_Index__LOPROC        0xff00                /* Start of processor-specific */
#define ELF_Section_Index__BEFORE        0xff00                /* Order section before all others
                                           (Solaris).  */
#define ELF_Section_Index_AFTER        0xff01                /* Order section after all others
                                           (Solaris).  */
#define ELF_Section_Index__HIPROC        0xff1f                /* End of processor-specific */
#define ELF_Section_Index__LOOS        0xff20                /* Start of OS-specific */
#define ELF_Section_Index__HIOS        0xff3f                /* End of OS-specific */
#define ELF_Section_Index__Absolute  0xfff1                /* Associated symbol is absolute */
#define ELF_Section_Index__Common        0xfff2                /* Associated symbol is common */
#define ELF_Section_Index__XINDEX        0xffff                /* Index is in extra table.  */
#define ELF_Section_Index__HIRESERVE        0xffff                /* End of reserved indices */

/* Legal values for type (section type).  */

typedef U32 ELF_Section_Header_Type;

#define ELF_Section_Header_Type__None                             0 /* Section header table entry unused */
#define ELF_Section_Header_Type__Program_Data                     1 /* Program data */
#define ELF_Section_Header_Type__Symbols_Table                    2 /* Symbol table */
#define ELF_Section_Header_Type__Strings_Table                    3 /* String table */
#define ELF_Section_Header_Type__Relocations_Addends              4 /* Relocation entries with addends */
#define ELF_Section_Header_Type__Symbols_Hash_Table               5 /* Symbol hash table */
#define ELF_Section_Header_Type__Dynamic_Linking                  6 /* Dynamic linking information */
#define ELF_Section_Header_Type__Notes                            7 /* Notes */
#define ELF_Section_Header_Type__No_Data                          8 /* Program space with no data (bss) */
#define ELF_Section_Header_Type__Relocations                      9 /* Relocation entries, no addends */
#define ELF_Section_Header_Type__SHLIB                           10 /* Reserved */
#define ELF_Section_Header_Type__DYNSYM                          11 /* Dynamic linker symbol table */
#define ELF_Section_Header_Type__INIT_ARRAY                      14 /* Array of constructors */
#define ELF_Section_Header_Type__FINI_ARRAY                      15 /* Array of destructors */
#define ELF_Section_Header_Type__PREINIT_ARRAY                   16 /* Array of pre-constructors */
#define ELF_Section_Header_Type__GROUP                           17 /* Section group */
#define ELF_Section_Header_Type__SYMTAB_SHNDX                    18 /* Extended section indices */
#define ELF_Section_Header_Type__RelocationsR                    19 /* RelocationsR relative relocations */
#define ELF_Section_Header_Type__NUM                             20 /* Number of defined types.  */
#define ELF_Section_Header_Type__LOOS                    0x60000000 /* Start OS-specific.  */
#define ELF_Section_Header_Type__GNU_ATTRIBUTES          0x6ffffff5 /* Object attributes.  */
#define ELF_Section_Header_Type__GNU_Symbols_Hash_Table  0x6ffffff6 /* GNU-style hash table.  */
#define ELF_Section_Header_Type__GNU_LIBLIST             0x6ffffff7 /* Prelink library list */
#define ELF_Section_Header_Type__CHECKSUM                0x6ffffff8 /* Checksum for DSO content.  */
#define ELF_Section_Header_Type__LOSUNW                  0x6ffffffa /* Sun-specific low bound.  */
#define ELF_Section_Header_Type__SUNW_move               0x6ffffffa
#define ELF_Section_Header_Type__SUNW_COMDAT             0x6ffffffb
#define ELF_Section_Header_Type__SUNW_syminfo            0x6ffffffc
#define ELF_Section_Header_Type__GNU_verdef              0x6ffffffd /* Version definition section.  */
#define ELF_Section_Header_Type__GNU_verneed             0x6ffffffe /* Version needs section.  */
#define ELF_Section_Header_Type__GNU_versym              0x6fffffff /* Version symbol table.  */
#define ELF_Section_Header_Type__HISUNW                  0x6fffffff /* Sun-specific high bound.  */
#define ELF_Section_Header_Type__HIOS                    0x6fffffff /* End OS-specific type */
#define ELF_Section_Header_Type__LOPROC                  0x70000000 /* Start of processor-specific */
#define ELF_Section_Header_Type__RISCV_Attributes        0x70000003
#define ELF_Section_Header_Type__HIPROC                  0x7fffffff /* End of processor-specific */
#define ELF_Section_Header_Type__LOUSER                  0x80000000 /* Start of application-specific */
#define ELF_Section_Header_Type__HIUSER                  0x8fffffff /* End of application-specific */

// Custom added
#define ELF_Section_Header_Type__Invalid                 0xffffffff

typedef U32 ELF_Section_Header_Flags;

#define ELF_Section_Header_Flags__WRITE               (1 << 0)     /* Writable */
#define ELF_Section_Header_Flags__ALLOC               (1 << 1)     /* Occupies memory during execution */
#define ELF_Section_Header_Flags__EXECINSTR           (1 << 2)     /* Executable */
#define ELF_Section_Header_Flags__MERGE               (1 << 4)     /* Might be merged */
#define ELF_Section_Header_Flags__STRINGS             (1 << 5)     /* Contains nul-terminated strings */
#define ELF_Section_Header_Flags__INFO_LINK           (1 << 6)     /* `sh_info' contains SHT index */
#define ELF_Section_Header_Flags__LINK_ORDER          (1 << 7)     /* Preserve order after combining */
#define ELF_Section_Header_Flags__OS_NONCONFORMING    (1 << 8)     /* Non-standard OS specific handling required */
#define ELF_Section_Header_Flags__GROUP               (1 << 9)     /* Section is member of a group.  */
#define ELF_Section_Header_Flags__TLS                 (1 << 10)    /* Section hold thread-local data.  */
#define ELF_Section_Header_Flags__COMPRESSED          (1 << 11)    /* Section with compressed data. */
#define ELF_Section_Header_Flags__MASKOS              0x0ff00000   /* OS-specific.  */
#define ELF_Section_Header_Flags__MASKPROC            0xf0000000   /* Processor-specific */
#define ELF_Section_Header_Flags__GNU_RETAIN          (1 << 21)    /* Not to be GCed by linker.  */
#define ELF_Section_Header_Flags__ORDERED             (1 << 30)    /* Special ordering requirement (Solaris).  */
#define ELF_Section_Header_Flags__EXCLUDE             (1U << 31)   /* Section is excluded unless referenced or allocated (Solaris).*/

// Custom added
#define ELF_Section_Header_Flags__Invalid             0xffffffff

/* Section compression header.  Used when SHF_COMPRESSED is set.  */

typedef struct ELF32_Chdr ELF32_Chdr;
struct ELF32_Chdr
{
  U32        ch_type;        /* Compression format.  */
  U32        ch_size;        /* Uncompressed data size.  */
  U32        ch_addralign;        /* Uncompressed data alignment.  */
};

typedef struct
{
  U64        ch_type;        /* Compression format.  */
  U64        ch_reserved;
  U64        ch_size;        /* Uncompressed data size.  */
  U64        ch_addralign;        /* Uncompressed data alignment.  */
} ELF64_Chdr;

/* Legal values for ch_type (compression algorithm).  */
#define ELFCOMPRESS_ZLIB        1           /* ZLIB/DEFLATE algorithm.  */
#define ELFCOMPRESS_ZSTD        2           /* Zstandard algorithm.  */
#define ELFCOMPRESS_LOOS        0x60000000 /* Start of OS-specific.  */
#define ELFCOMPRESS_HIOS        0x6fffffff /* End of OS-specific.  */
#define ELFCOMPRESS_LOPROC        0x70000000 /* Start of processor-specific.  */
#define ELFCOMPRESS_HIPROC        0x7fffffff /* End of processor-specific.  */

/* Symbol table entry.  */

typedef struct ELF32_Symbol ELF32_Symbol;
struct ELF32_Symbol
{
        U32  string_table_offset;
        U32  value;
        U32  size;
        U8   type_and_binding;
        U8   visibility;
        U16  section_index;
};

typedef struct ELF64_Symbol ELF64_Symbol;
struct ELF64_Symbol
{
        U32 string_table_offset;
        // Packed field: upper 4 bits = Symbol_Binding, lower 4 bits = Symbol_Type.
        // Use ELF_Symbol_info_m / _bind_m / _type_m macros to pack/unpack.
        U8  type_and_binding;
        U8  visibility;
        U16 section_index;

        // NOTE: for labels, this is offset within the fragment.
        U64 value;
        U64 size;
};

/* The syminfo section if available contains additional information about
   every dynamic symbol.  */

typedef struct ELF32_Syminfo ELF32_Syminfo;
struct ELF32_Syminfo
{
  U16 si_boundto;                /* Direct bindings, symbol bound to */
  U16 si_flags;                        /* Per symbol flags */
};

typedef struct
{
  U16 si_boundto;                /* Direct bindings, symbol bound to */
  U16 si_flags;                        /* Per symbol flags */
} ELF64_Syminfo;

/* Possible values for si_boundto.  */
#define SYMINFO_BT_SELF                0xffff        /* Symbol bound to self */
#define SYMINFO_BT_PARENT        0xfffe        /* Symbol bound to parent */
#define SYMINFO_BT_LOWRESERVE        0xff00        /* Beginning of reserved entries */

/* Possible bitmasks for si_flags.  */
#define SYMINFO_FLG_DIRECT        0x0001        /* Direct bound symbol */
#define SYMINFO_FLG_PASSTHRU        0x0002        /* Pass-through symbol for translator */
#define SYMINFO_FLG_COPY        0x0004        /* Symbol is a copy-reloc */
#define SYMINFO_FLG_LAZYLOAD        0x0008        /* Symbol bound to object to be lazy
                                           loaded */
/* Syminfo version values.  */
#define SYMINFO_NONE                0
#define SYMINFO_CURRENT                1
#define SYMINFO_NUM                2

#define ELF_Symbol_info_m(bind, type)  (((bind) << 4) | ((type) & 0xf))
#define ELF_Symbol_bind_m(info)        ((info) >> 4)
#define ELF_Symbol_type_m(info)        ((info) & 0xf)

/* Legal values for ST_BIND subfield of type_and_binding (symbol binding).  */

typedef U8 ELF_Symbol_Binding;

#define ELF_Symbol_Binding__Local         0    /* Local  symbol  */
#define ELF_Symbol_Binding__Global        1    /* Global symbol  */
#define ELF_Symbol_Binding__Weak          2    /* Weak   symbol  */
#define ELF_Symbol_Binding__NUM           3    /* Number of defined types.  */
#define ELF_Symbol_Binding__LOOS         10    /* Start  of      OS-specific        */
#define ELF_Symbol_Binding__GNU_UNIQUE   10    /* Unique symbol.                    */
#define ELF_Symbol_Binding__HIOS         12    /* End    of      OS-specific        */
#define ELF_Symbol_Binding__LOPROC       13    /* Start  of      processor-specific */
#define ELF_Symbol_Binding__HIPROC       15    /* End    of      processor-specific */

/* Legal values for ST_TYPE subfield of type_and_binding (symbol type).  */

#define STT_NOTYPE           0                  /* Symbol   type       is                 unspecified */
#define STT_OBJECT           1                  /* Symbol   is         a                  data        object  */
#define STT_FUNC             2                  /* Symbol   is         a                  code        object  */
#define STT_SECTION          3                  /* Symbol   associated with               a           section */
#define STT_FILE             4                  /* Symbol's name       is                 file        name    */
#define STT_COMMON           5                  /* Symbol   is         a                  common      data    object */
#define STT_TLS              6                  /* Symbol is thread-local data   object*/
#define STT_NUM              7                  /* Number of defined      types.  */
#define STT_LOOS             10                 /* Start    of         OS-specific        */
#define STT_GNU_IFUNC        10                 /* Symbol   is         indirect           code        object  */
#define STT_HIOS             12                 /* End      of         OS-specific        */
#define STT_LOPROC           13                 /* Start    of         processor-specific */
#define STT_HIPROC           15                 /* End      of         processor-specific */

#define ELF_Symbol_Type__None                   0
#define ELF_Symbol_Type__Object                 1
#define ELF_Symbol_Type__Function               2
#define ELF_Symbol_Type__Section                3
#define ELF_Symbol_Type__File                   4
#define ELF_Symbol_Type__Common                 5
#define ELF_Symbol_Type__TLS                    6
#define ELF_Symbol_Type__NUM                    7
#define ELF_Symbol_Type__LOOS                  10
#define ELF_Symbol_Type__GNU_Indirect_Function 10
#define ELF_Symbol_Type__HIOS                  12
#define ELF_Symbol_Type__LOPROC                13
#define ELF_Symbol_Type__HIPROC                15


/* Symbol table indices are found in the hash buckets and chain table
   of a symbol hash table section.  This special index value indicates
   the end of a chain, meaning no further symbols are found in that bucket.  */

#define STN_UNDEF        0                /* End of a chain.  */


/* How to extract and insert information held in the visibility field.  */

#define ELF32_ST_VISIBILITY(o)        ((o) & 0x03)

/* For ELF64 the definitions are the same.  */
#define ELF64_ST_VISIBILITY(o)        ELF32_ST_VISIBILITY (o)

/* Symbol visibility specification encoded in the visibility field.  */
#define STV_DEFAULT        0                /* Default symbol visibility rules */
#define STV_INTERNAL        1                /* Processor specific hidden class */
#define STV_HIDDEN        2                /* Sym unavailable in other modules */
#define STV_PROTECTED        3                /* Not preemptible, not exported */


typedef struct ELF32_Relocation ELF32_Relocation;
struct ELF32_Relocation
{
        U32 offset;
        // Relocation type and symbol index
        U32 info;
};

typedef struct ELF64_Relocation ELF64_Relocation;
struct ELF64_Relocation
{
        U64 offset;
        // Relocation type and symbol index
        U64 info;
};

typedef struct ELF32_Relocation_Addend ELF32_Relocation_Addend;
struct ELF32_Relocation_Addend
{
        U32 offset;
        // Relocation type and symbol index
        U32 info;
        S32 addend;
};

typedef struct ELF64_Relocation_Addend ELF64_Relocation_Addend;
struct ELF64_Relocation_Addend
{
        U64 offset;
        // Relocation type and symbol index
        U64 info;
        S64 addend;
};

/* RelocationsR relocation table entry */

typedef U32        ELF32_Relr;
typedef U64        ELF64_Relr;

/* How to extract and insert information held in the r_info field.  */

#define ELF32_R_SYM(val)                ((val) >> 8)
#define ELF32_R_TYPE(val)                ((val) & 0xff)
#define ELF32_R_INFO(sym, type)                (((sym) << 8) + ((type) & 0xff))

#define ELF64_Relocation_symbol_m(i)          ((i) >> 32)
#define ELF64_Relocation_type_m(i)            ((i) & 0xffffffff)
#define ELF64_Relocation_info_m(symbol,type)  ((((U64)(symbol)) << 32) + (type))

// /* RISC-V relocations.  */

/* RISC-V ELF Relocations
 *
 * Acronyms used:
 *   TLS    = Thread-Local Storage
 *   GOT    = Global Offset Table
 *   PLT    = Procedure Linkage Table (used for lazy dynamic linking)
 *   PC     = Program Counter
 *   LEB128 = Little-Endian Base 128 (variable-length integer encoding)
 *   I_Type = I-Type instruction encoding (12-bit immediate, contiguous)
 *   S_Type = S-Type instruction encoding (12-bit immediate, split field)
 */

// U16 is large enough for all relocation types across all targets.
typedef U16 Relocation_RISC_V;

#define Relocation_RISC_V__None                                            0
#define Relocation_RISC_V__32_Bit                                          1
#define Relocation_RISC_V__64_Bit                                          2
#define Relocation_RISC_V__Relative                                        3
#define Relocation_RISC_V__Copy                                            4
#define Relocation_RISC_V__Jump_Slot                                       5
#define Relocation_RISC_V__TLS_Dynamic_Thread_Private_Module_32            6
#define Relocation_RISC_V__TLS_Dynamic_Thread_Private_Module_64            7
#define Relocation_RISC_V__TLS_Dynamic_Thread_Private_Relative_32          8
#define Relocation_RISC_V__TLS_Dynamic_Thread_Private_Relative_64          9
#define Relocation_RISC_V__TLS_Thread_Pointer_Relative_32                 10
#define Relocation_RISC_V__TLS_Thread_Pointer_Relative_64                 11
#define Relocation_RISC_V__TLS_Descriptor                                 12
#define Relocation_RISC_V__Branch                                         16
#define Relocation_RISC_V__JAL                                            17
#define Relocation_RISC_V__Call                                           18
#define Relocation_RISC_V__Call_PLT                                       19
#define Relocation_RISC_V__GOT_High_20                                    20
#define Relocation_RISC_V__TLS_GOT_High_20                                21
#define Relocation_RISC_V__TLS_Global_Dynamic_High_20                     22
#define Relocation_RISC_V__PC_Relative_High_20                            23
#define Relocation_RISC_V__PC_Relative_Low_12_I_Type                      24
#define Relocation_RISC_V__PC_Relative_Low_12_S_Type                      25
#define Relocation_RISC_V__High_20                                        26
#define Relocation_RISC_V__Low_12_I_Type                                  27
#define Relocation_RISC_V__Low_12_S_Type                                  28
#define Relocation_RISC_V__Thread_Pointer_Relative_High_20                29
#define Relocation_RISC_V__Thread_Pointer_Relative_Low_12_I_Type          30
#define Relocation_RISC_V__Thread_Pointer_Relative_Low_12_S_Type          31
#define Relocation_RISC_V__Thread_Pointer_Relative_Add                    32
#define Relocation_RISC_V__Add_8                                          33
#define Relocation_RISC_V__Add_16                                         34
#define Relocation_RISC_V__Add_32                                         35
#define Relocation_RISC_V__Add_64                                         36
#define Relocation_RISC_V__Sub_8                                          37
#define Relocation_RISC_V__Sub_16                                         38
#define Relocation_RISC_V__Sub_32                                         39
#define Relocation_RISC_V__Sub_64                                         40
#define Relocation_RISC_V__GOT_32_PC_Relative                             41
#define Relocation_RISC_V__Align                                          43
#define Relocation_RISC_V__Branch_Compressed                              44
#define Relocation_RISC_V__Jump_Compressed                                45
#define Relocation_RISC_V__Relax                                          51
#define Relocation_RISC_V__Sub_6                                          52
#define Relocation_RISC_V__Set_6                                          53
#define Relocation_RISC_V__Set_8                                          54
#define Relocation_RISC_V__Set_16                                         55
#define Relocation_RISC_V__Set_32                                         56
#define Relocation_RISC_V__32_Bit_PC_Relative                             57
#define Relocation_RISC_V__Indirect_Relative                              58
#define Relocation_RISC_V__PLT_32                                         59
#define Relocation_RISC_V__Set_Unsigned_LEB128                            60
#define Relocation_RISC_V__Sub_Unsigned_LEB128                            61
#define Relocation_RISC_V__TLS_Descriptor_High_20                         62
#define Relocation_RISC_V__TLS_Descriptor_Load_Low_12                     63
#define Relocation_RISC_V__TLS_Descriptor_Add_Low_12                      64
#define Relocation_RISC_V__TLS_Descriptor_Call                            65
#define Relocation_RISC_V__COUNT                                          66

// RISC-V ELF Flags
#define EF_RISCV_RVC 			0x0001
#define EF_RISCV_FLOAT_ABI 		0x0006
#define EF_RISCV_FLOAT_ABI_SOFT 	0x0000
#define EF_RISCV_FLOAT_ABI_SINGLE 	0x0002
#define EF_RISCV_FLOAT_ABI_DOUBLE 	0x0004
#define EF_RISCV_FLOAT_ABI_QUAD 	0x0006
#define EF_RISCV_RVE			0x0008
#define EF_RISCV_TSO			0x0010

// /* RISC-V specific values for the visibility field.  */
// #define STO_RISCV_VARIANT_CC        0x80        /* Function uses variant calling
//                                            convention */
//
// /* RISC-V specific values for the type field.  */
// #define ELF_Section_Header_Type__RISCV_ATTRIBUTES        (ELF_Section_Header_Type__LOPROC + 3)
//
// /* RISC-V specific values for the p_type field (deprecated).  */
// #define PT_RISCV_ATTRIBUTES        (PT_LOPROC + 3)
//
// /* RISC-V specific values for the d_tag field.  */
// #define DT_RISCV_VARIANT_CC        (DT_LOPROC + 1)
//
// /* BPF specific declarations.  */
//
// #define R_BPF_NONE                0        /* No reloc */
// #define R_BPF_64_64                1
// #define R_BPF_64_32                10
//
// /* Imagination Meta specific relocations. */
//
// #define R_METAG_HIADDR16        0
// #define R_METAG_LOADDR16        1
// #define R_METAG_ADDR32                2        /* 32bit absolute address */
// #define R_METAG_NONE                3        /* No reloc */
// #define R_METAG_RelocationsBRANCH        4
// #define R_METAG_GETSETOFF        5
//
// /* Backward compatibility */
// #define R_METAG_REG32OP1        6
// #define R_METAG_REG32OP2        7
// #define R_METAG_REG32OP3        8
// #define R_METAG_REG16OP1        9
// #define R_METAG_REG16OP2        10
// #define R_METAG_REG16OP3        11
// #define R_METAG_REG32OP4        12
//
// #define R_METAG_HIOG                13
// #define R_METAG_LOOG                14
//
// #define R_METAG_Relocations8                15
// #define R_METAG_Relocations16                16
//
// /* GNU */
// #define R_METAG_GNU_VTINHERIT        30
// #define R_METAG_GNU_VTENTRY        31
//
// /* PIC relocations */
// #define R_METAG_HI16_GOTOFF        32
// #define R_METAG_LO16_GOTOFF        33
// #define R_METAG_GETSET_GOTOFF        34
// #define R_METAG_GETSET_GOT        35
// #define R_METAG_HI16_GOTPC        36
// #define R_METAG_LO16_GOTPC        37
// #define R_METAG_HI16_PLT        38
// #define R_METAG_LO16_PLT        39
// #define R_METAG_RelocationsBRANCH_PLT        40
// #define R_METAG_GOTOFF                41
// #define R_METAG_PLT                42
// #define R_METAG_COPY                43
// #define R_METAG_JMP_SLOT        44
// #define R_METAG_Relocations_AddendsTIVE        45
// #define R_METAG_GLOB_DAT        46
//
// /* TLS relocations */
// #define R_METAG_TLS_GD                47
// #define R_METAG_TLS_LDM                48
// #define R_METAG_TLS_LDO_HI16        49
// #define R_METAG_TLS_LDO_LO16        50
// #define R_METAG_TLS_LDO                51
// #define R_METAG_TLS_IE                52
// #define R_METAG_TLS_IENONPIC        53
// #define R_METAG_TLS_IENONPIC_HI16 54
// #define R_METAG_TLS_IENONPIC_LO16 55
// #define R_METAG_TLS_TPOFF        56
// #define R_METAG_TLS_DTPMOD        57
// #define R_METAG_TLS_DTPOFF        58
// #define R_METAG_TLS_LE                59
// #define R_METAG_TLS_LE_HI16        60
// #define R_METAG_TLS_LE_LO16        61
//
// /* NDS32 relocations.  */
// #define R_NDS32_NONE                0
// #define R_NDS32_32_Relocations_Addends         20
// #define R_NDS32_COPY                39
// #define R_NDS32_GLOB_DAT        40
// #define R_NDS32_JMP_SLOT        41
// #define R_NDS32_Relocations_AddendsTIVE        42
// #define R_NDS32_TLS_TPOFF        102
// #define R_NDS32_TLS_DESC        119
//
// /* LoongArch ELF Flags */
// #define EF_LARCH_ABI_MODIFIER_MASK  0x07
// #define EF_LARCH_ABI_SOFT_FLOAT     0x01
// #define EF_LARCH_ABI_SINGLE_FLOAT   0x02
// #define EF_LARCH_ABI_DOUBLE_FLOAT   0x03
// #define EF_LARCH_OBJABI_V1          0x40
//
// /* LoongArch specific dynamic relocations */
// #define R_LARCH_NONE                0
// #define R_LARCH_32                1
// #define R_LARCH_64                2
// #define R_LARCH_Relocations_AddendsTIVE        3
// #define R_LARCH_COPY                4
// #define R_LARCH_JUMP_SLOT        5
// #define R_LARCH_TLS_DTPMOD32        6
// #define R_LARCH_TLS_DTPMOD64        7
// #define R_LARCH_TLS_DTPRelocations32        8
// #define R_LARCH_TLS_DTPRelocations64        9
// #define R_LARCH_TLS_TPRelocations32        10
// #define R_LARCH_TLS_TPRelocations64        11
// #define R_LARCH_IRelocations_AddendsTIVE        12
// #define R_LARCH_TLS_DESC32        13
// #define R_LARCH_TLS_DESC64        14
//
// /* Reserved for future relocs that the dynamic linker must understand.  */
//
// /* used by the static linker for relocating .text.  */
// #define R_LARCH_MARK_LA  20
// #define R_LARCH_MARK_PCRelocations  21
// #define R_LARCH_SOP_PUSH_PCRelocations  22
// #define R_LARCH_SOP_PUSH_ABSOLUTE  23
// #define R_LARCH_SOP_PUSH_DUP  24
// #define R_LARCH_SOP_PUSH_GPRelocations  25
// #define R_LARCH_SOP_PUSH_TLS_TPRelocations  26
// #define R_LARCH_SOP_PUSH_TLS_GOT  27
// #define R_LARCH_SOP_PUSH_TLS_GD  28
// #define R_LARCH_SOP_PUSH_PLT_PCRelocations  29
// #define R_LARCH_SOP_ASSERT  30
// #define R_LARCH_SOP_NOT  31
// #define R_LARCH_SOP_SUB  32
// #define R_LARCH_SOP_SL  33
// #define R_LARCH_SOP_SR  34
// #define R_LARCH_SOP_ADD  35
// #define R_LARCH_SOP_AND  36
// #define R_LARCH_SOP_IF_ELSE  37
// #define R_LARCH_SOP_POP_32_S_10_5  38
// #define R_LARCH_SOP_POP_32_U_10_12  39
// #define R_LARCH_SOP_POP_32_S_10_12  40
// #define R_LARCH_SOP_POP_32_S_10_16  41
// #define R_LARCH_SOP_POP_32_S_10_16_S2  42
// #define R_LARCH_SOP_POP_32_S_5_20  43
// #define R_LARCH_SOP_POP_32_S_0_5_10_16_S2  44
// #define R_LARCH_SOP_POP_32_S_0_10_10_16_S2  45
// #define R_LARCH_SOP_POP_32_U  46
//
// /* used by the static linker for relocating non .text.  */
// #define R_LARCH_ADD8  47
// #define R_LARCH_ADD16  48
// #define R_LARCH_ADD24  49
// #define R_LARCH_ADD32  50
// #define R_LARCH_ADD64  51
// #define R_LARCH_SUB8  52
// #define R_LARCH_SUB16  53
// #define R_LARCH_SUB24  54
// #define R_LARCH_SUB32  55
// #define R_LARCH_SUB64  56
// #define R_LARCH_GNU_VTINHERIT  57
// #define R_LARCH_GNU_VTENTRY  58
//
// /* reserved 59-63 */
//
// #define R_LARCH_B16 64
// #define R_LARCH_B21 65
// #define R_LARCH_B26 66
// #define R_LARCH_ABS_HI20 67
// #define R_LARCH_ABS_LO12 68
// #define R_LARCH_ABS64_LO20 69
// #define R_LARCH_ABS64_HI12 70
// #define R_LARCH_PCALA_HI20 71
// #define R_LARCH_PCALA_LO12 72
// #define R_LARCH_PCALA64_LO20 73
// #define R_LARCH_PCALA64_HI12 74
// #define R_LARCH_GOT_PC_HI20 75
// #define R_LARCH_GOT_PC_LO12 76
// #define R_LARCH_GOT64_PC_LO20 77
// #define R_LARCH_GOT64_PC_HI12 78
// #define R_LARCH_GOT_HI20 79
// #define R_LARCH_GOT_LO12 80
// #define R_LARCH_GOT64_LO20 81
// #define R_LARCH_GOT64_HI12 82
// #define R_LARCH_TLS_LE_HI20 83
// #define R_LARCH_TLS_LE_LO12 84
// #define R_LARCH_TLS_LE64_LO20 85
// #define R_LARCH_TLS_LE64_HI12 86
// #define R_LARCH_TLS_IE_PC_HI20 87
// #define R_LARCH_TLS_IE_PC_LO12 88
// #define R_LARCH_TLS_IE64_PC_LO20 89
// #define R_LARCH_TLS_IE64_PC_HI12 90
// #define R_LARCH_TLS_IE_HI20 91
// #define R_LARCH_TLS_IE_LO12 92
// #define R_LARCH_TLS_IE64_LO20 93
// #define R_LARCH_TLS_IE64_HI12 94
// #define R_LARCH_TLS_LD_PC_HI20 95
// #define R_LARCH_TLS_LD_HI20 96
// #define R_LARCH_TLS_GD_PC_HI20 97
// #define R_LARCH_TLS_GD_HI20 98
// #define R_LARCH_32_PCRelocations 99
// #define R_LARCH_Relocations_AddendsX 100
// #define R_LARCH_DELETE 101
// #define R_LARCH_ALIGN 102
// #define R_LARCH_PCRelocations20_S2 103
// #define R_LARCH_CFA 104
// #define R_LARCH_ADD6 105
// #define R_LARCH_SUB6 106
// #define R_LARCH_ADD_ULEB128 107
// #define R_LARCH_SUB_ULEB128 108
// #define R_LARCH_64_PCRelocations 109
// #define R_LARCH_CALL36 110
// #define R_LARCH_TLS_DESC_PC_HI20 111
// #define R_LARCH_TLS_DESC_PC_LO12 112
// #define R_LARCH_TLS_DESC64_PC_LO20 113
// #define R_LARCH_TLS_DESC64_PC_HI12 114
// #define R_LARCH_TLS_DESC_HI20 115
// #define R_LARCH_TLS_DESC_LO12 116
// #define R_LARCH_TLS_DESC64_LO20 117
// #define R_LARCH_TLS_DESC64_HI12 118
// #define R_LARCH_TLS_DESC_LD 119
// #define R_LARCH_TLS_DESC_CALL 120
// #define R_LARCH_TLS_LE_HI20_R 121
// #define R_LARCH_TLS_LE_ADD_R 122
// #define R_LARCH_TLS_LE_LO12_R 123
// #define R_LARCH_TLS_LD_PCRelocations20_S2 124
// #define R_LARCH_TLS_GD_PCRelocations20_S2 125
// #define R_LARCH_TLS_DESC_PCRelocations20_S2 126

#endif // ELF_H
