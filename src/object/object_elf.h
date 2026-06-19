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
        U64 object_file_version;
        U64 entry_point_virtual_address;
        U64 program_header_table_file_offset;
        U64 section_header_table_file_offset;
        U64 processor_flags;
        U16 header_size;
        U16 program_header_table_entry_size;
        U16 program_header_table_entry_count;
        U16 section_header_table_entry_size;
        U16 section_header_table_entry_count;
        U16 section_header_string_table_index;
};

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

typedef struct ELF_Section_Header ELF_Section_Header;
struct ELF_Section_Header
{
  U64        string_table_offset;                /* Section name (string tbl index) */
  U64        type;                /* Section type */
  U64        flags;                /* Section flags */
  Address64        address_virtual;                /* Section virtual addr at execution */
  Offset64        offset;                /* Section file offset */
  U64        size;                /* Section size in bytes */
  U64        link;                /* Link to another section */
  U64        info;                /* Additional section information */
  U64        alignment;                /* Section alignment */
  U64        entry_size;                /* Entry size if section holds table */
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
#define ELF_Section_Header_Type__HIPROC                  0x7fffffff /* End of processor-specific */
#define ELF_Section_Header_Type__LOUSER                  0x80000000 /* Start of application-specific */
#define ELF_Section_Header_Type__HIUSER                  0x8fffffff /* End of application-specific */

// Custom added
#define ELF_Section_Header_Type__Invalid                 0xffffffff

/* Legal values for sh_flags (section flags).  */

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
  Offset32        string_table_offset;                /* Symbol name (string tbl index) */
  Address32        value;                /* Symbol value */
  U32        size;                /* Symbol size */
  U8        type_and_binding;                /* Symbol type and binding */
  U8        visibility;                /* Symbol visibility */
  ELF32_Section_Index        section_index;                /* Section index */
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
        S64 value;
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

#define ELF_Symbol_Binding__Local        0                /* Local symbol */
#define ELF_Symbol_Binding__Global        1                /* Global symbol */
#define ELF_Symbol_Binding__Weak        2                /* Weak symbol */
#define        ELF_Symbol_Binding__NUM                3                /* Number of defined types.  */
#define ELF_Symbol_Binding__LOOS        10                /* Start of OS-specific */
#define ELF_Symbol_Binding__GNU_UNIQUE        10                /* Unique symbol.  */
#define ELF_Symbol_Binding__HIOS        12                /* End of OS-specific */
#define ELF_Symbol_Binding__LOPROC        13                /* Start of processor-specific */
#define ELF_Symbol_Binding__HIPROC        15                /* End of processor-specific */

/* Legal values for ST_TYPE subfield of type_and_binding (symbol type).  */

#define STT_NOTYPE        0                /* Symbol type is unspecified */
#define STT_OBJECT        1                /* Symbol is a data object */
#define STT_FUNC        2                /* Symbol is a code object */
#define STT_SECTION        3                /* Symbol associated with a section */
#define STT_FILE        4                /* Symbol's name is file name */
#define STT_COMMON        5                /* Symbol is a common data object */
#define STT_TLS                6                /* Symbol is thread-local data object*/
#define        STT_NUM                7                /* Number of defined types.  */
#define STT_LOOS        10                /* Start of OS-specific */
#define STT_GNU_IFUNC        10                /* Symbol is indirect code object */
#define STT_HIOS        12                /* End of OS-specific */
#define STT_LOPROC        13                /* Start of processor-specific */
#define STT_HIPROC        15                /* End of processor-specific */


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


/* Relocation table entry without addend (in section of type ELF_Section_Header_Type__Relocations).  */

typedef struct ELF32_Rel ELF32_Rel;
struct ELF32_Rel
{
  Address32        r_offset;                /* Address */
  U32        r_info;                        /* Relocation type and symbol index */
};

/* I have seen two different definitions of the ELF64_Rel and
   ELF64_Rela structures, so we'll leave them out until Novell (or
   whoever) gets their act together.  */
/* The following, at least, is used on Sparc v9, MIPS, and Alpha.  */

typedef struct
{
  Address64        r_offset;                /* Address */
  U64        r_info;                        /* Relocation type and symbol index */
} ELF64_Rel;

/* Relocation table entry with addend (in section of type ELF_Section_Header_Type__Relocations_Addends).  */

typedef struct ELF32_Rela ELF32_Rela;
struct ELF32_Rela
{
  Address32        r_offset;                /* Address */
  U32        r_info;                        /* Relocation type and symbol index */
  S32        r_addend;                /* Addend */
};

typedef struct
{
	U64 offset;                /* Address */
        U64 info;                        /* Relocation type and symbol index */
        S64 addend;                /* Addend */
} ELF64_Relocation_Addend;

/* RelocationsR relocation table entry */

typedef U32        ELF32_Relr;
typedef U64        ELF64_Relr;

/* How to extract and insert information held in the r_info field.  */

#define ELF32_R_SYM(val)                ((val) >> 8)
#define ELF32_R_TYPE(val)                ((val) & 0xff)
#define ELF32_R_INFO(sym, type)                (((sym) << 8) + ((type) & 0xff))

#define ELF64_Relocation_symbol_m(i)          ((i) >> 32)
#define ELF64_Relocation_type_m(i)            ((i) & 0xffffffff)
#define ELF64_Relocation_info_m(symbol,type)  ((((U64) (symbol)) << 32) + (type))

/* Program segment header.  */

typedef struct ELF32_Phdr ELF32_Phdr;
struct ELF32_Phdr
{
  U32        p_type;                        /* Segment type */
  Offset32        p_offset;                /* Segment file offset */
  Address32        p_vaddr;                /* Segment virtual address */
  Address32        p_paddr;                /* Segment physical address */
  U32        p_filesz;                /* Segment size in file */
  U32        p_memsz;                /* Segment size in memory */
  U32        p_flags;                /* Segment flags */
  U32        p_align;                /* Segment alignment */
};

typedef struct
{
  U64        p_type;                        /* Segment type */
  U64        p_flags;                /* Segment flags */
  Offset64        p_offset;                /* Segment file offset */
  Address64        p_vaddr;                /* Segment virtual address */
  Address64        p_paddr;                /* Segment physical address */
  U64        p_filesz;                /* Segment size in file */
  U64        p_memsz;                /* Segment size in memory */
  U64        p_align;                /* Segment alignment */
} ELF64_Phdr;

// /* Special value for program_header_table_entry_count.  This indicates that the real number of
//    program headers is too large to fit into program_header_table_entry_count.  Instead the real
//    value is in the field info of section 0.  */
//
// #define PN_XNUM                0xffff
//
// /* Legal values for p_type (segment type).  */
//
// #define        PT_NULL                0                /* Program header table entry unused */
// #define PT_LOAD                1                /* Loadable program segment */
// #define PT_Dynamic_Linking        2                /* Dynamic linking information */
// #define PT_INTERP        3                /* Program interpreter */
// #define PT_Notes                4                /* Auxiliary information */
// #define PT_SHLIB        5                /* Reserved */
// #define PT_PHDR                6                /* Entry for header table itself */
// #define PT_TLS                7                /* Thread-local storage segment */
// #define        PT_NUM                8                /* Number of defined types */
// #define PT_LOOS                0x60000000        /* Start of OS-specific */
// #define PT_GNU_EH_FRAME        0x6474e550        /* GCC .eh_frame_hdr segment */
// #define PT_GNU_STACK        0x6474e551        /* Indicates stack executability */
// #define PT_GNU_RelocationsRO        0x6474e552        /* Read-only after relocation */
// #define PT_GNU_PROPERTY        0x6474e553        /* GNU property */
// #define PT_GNU_SFRAME        0x6474e554        /* SFrame segment.  */
// #define PT_LOSUNW        0x6ffffffa
// #define PT_SUNWBSS        0x6ffffffa        /* Sun Specific segment */
// #define PT_SUNWSTACK        0x6ffffffb        /* Stack segment */
// #define PT_HISUNW        0x6fffffff
// #define PT_HIOS                0x6fffffff        /* End of OS-specific */
// #define PT_LOPROC        0x70000000        /* Start of processor-specific */
// #define PT_HIPROC        0x7fffffff        /* End of processor-specific */
//
// /* Legal values for p_flags (segment flags).  */
//
// #define PF_X                (1 << 0)        /* Segment is executable */
// #define PF_W                (1 << 1)        /* Segment is writable */
// #define PF_R                (1 << 2)        /* Segment is readable */
// #define PF_MASKOS        0x0ff00000        /* OS-specific */
// #define PF_MASKPROC        0xf0000000        /* Processor-specific */
//
// /* Legal values for note segment descriptor types for core files. */
//
// #define NT_PRSTATUS        1                /* Contains copy of prstatus struct */
// #define NT_PRFPREG        2                /* Contains copy of fpregset
//                                            struct.  */
// #define NT_FPREGSET        2                /* Contains copy of fpregset struct */
// #define NT_PRPSINFO        3                /* Contains copy of prpsinfo struct */
// #define NT_PRXREG        4                /* Contains copy of prxregset struct */
// #define NT_TASKSTRUCT        4                /* Contains copy of task structure */
// #define NT_PLATFORM        5                /* String from sysinfo(SI_PLATFORM) */
// #define NT_AUXV                6                /* Contains copy of auxv array */
// #define NT_GWINDOWS        7                /* Contains copy of gwindows struct */
// #define NT_ASRS                8                /* Contains copy of asrset struct */
// #define NT_PSTATUS        10                /* Contains copy of pstatus struct */
// #define NT_PSINFO        13                /* Contains copy of psinfo struct */
// #define NT_PRCRED        14                /* Contains copy of prcred struct */
// #define NT_UTSNAME        15                /* Contains copy of utsname struct */
// #define NT_LWPSTATUS        16                /* Contains copy of lwpstatus struct */
// #define NT_LWPSINFO        17                /* Contains copy of lwpinfo struct */
// #define NT_PRFPXREG        20                /* Contains copy of fprxregset struct */
// #define NT_SIGINFO        0x53494749        /* Contains copy of siginfo_t,
//                                            size might increase */
// #define NT_FILE                0x46494c45        /* Contains information about mapped
//                                            files */
// #define NT_PRXFPREG        0x46e62b7f        /* Contains copy of user_fxsr_struct */
// #define NT_PPC_VMX        0x100                /* PowerPC Altivec/VMX registers */
// #define NT_PPC_SPE        0x101                /* PowerPC SPE/EVR registers */
// #define NT_PPC_VSX        0x102                /* PowerPC VSX registers */
// #define NT_PPC_TAR        0x103                /* Target Address Register */
// #define NT_PPC_PPR        0x104                /* Program Priority Register */
// #define NT_PPC_DSCR        0x105                /* Data Stream Control Register */
// #define NT_PPC_EBB        0x106                /* Event Based Branch Registers */
// #define NT_PPC_PMU        0x107                /* Performance Monitor Registers */
// #define NT_PPC_TM_CGPR        0x108                /* TM checkpointed GPR Registers */
// #define NT_PPC_TM_CFPR        0x109                /* TM checkpointed FPR Registers */
// #define NT_PPC_TM_CVMX        0x10a                /* TM checkpointed VMX Registers */
// #define NT_PPC_TM_CVSX        0x10b                /* TM checkpointed VSX Registers */
// #define NT_PPC_TM_SPR        0x10c                /* TM Special Purpose Registers */
// #define NT_PPC_TM_CTAR        0x10d                /* TM checkpointed Target Address
//                                            Register */
// #define NT_PPC_TM_CPPR        0x10e                /* TM checkpointed Program Priority
//                                            Register */
// #define NT_PPC_TM_CDSCR        0x10f                /* TM checkpointed Data Stream Control
//                                            Register */
// #define NT_PPC_PKEY        0x110                /* Memory Protection Keys
//                                            registers.  */
// #define NT_PPC_DEXCR        0x111                /* PowerPC DEXCR registers.  */
// #define NT_PPC_Symbols_Hash_TableKEYR        0x112                /* PowerPC Symbols_Hash_TableKEYR register.  */
// #define NT_386_TLS        0x200                /* i386 TLS slots (struct user_desc) */
// #define NT_386_IOPERM        0x201                /* x86 io permission bitmap (1=deny) */
// #define NT_X86_XSTATE        0x202                /* x86 extended state using xsave */
// #define NT_X86_SHSTK        0x204                /* x86 SHSTK state */
// #define NT_X86_XSAVE_LAYOUT        0x205                /* XSAVE layout description.  */
// #define NT_S390_HIGH_GPRS        0x300        /* s390 upper register halves */
// #define NT_S390_TIMER        0x301                /* s390 timer register */
// #define NT_S390_TODCMP        0x302                /* s390 TOD clock comparator register */
// #define NT_S390_TODPREG        0x303                /* s390 TOD programmable register */
// #define NT_S390_CTRS        0x304                /* s390 control registers */
// #define NT_S390_PREFIX        0x305                /* s390 prefix register */
// #define NT_S390_LAST_BREAK        0x306        /* s390 breaking event address */
// #define NT_S390_SYSTEM_CALL        0x307        /* s390 system call restart data */
// #define NT_S390_TDB        0x308                /* s390 transaction diagnostic block */
// #define NT_S390_VXRS_LOW        0x309        /* s390 vector registers 0-15
//                                            upper half.  */
// #define NT_S390_VXRS_HIGH        0x30a        /* s390 vector registers 16-31.  */
// #define NT_S390_GS_CB        0x30b                /* s390 guarded storage registers.  */
// #define NT_S390_GS_BC        0x30c                /* s390 guarded storage
//                                            broadcast control block.  */
// #define NT_S390_RI_CB        0x30d                /* s390 runtime instrumentation.  */
// #define NT_S390_PV_CPU_DATA        0x30e        /* s390 protvirt cpu dump data.  */
// #define NT_ARM_VFP        0x400                /* ARM VFP/NEON registers */
// #define NT_ARM_TLS        0x401                /* ARM TLS register */
// #define NT_ARM_HW_BREAK        0x402                /* ARM hardware breakpoint registers */
// #define NT_ARM_HW_WATCH        0x403                /* ARM hardware watchpoint registers */
// #define NT_ARM_SYSTEM_CALL        0x404        /* ARM system call number */
// #define NT_ARM_SVE        0x405                /* ARM Scalable Vector Extension
//                                            registers */
// #define NT_ARM_PAC_MASK        0x406                /* ARM pointer authentication
//                                            code masks.  */
// #define NT_ARM_PACA_KEYS        0x407        /* ARM pointer authentication
//                                            address keys.  */
// #define NT_ARM_PACG_KEYS        0x408        /* ARM pointer authentication
//                                            generic key.  */
// #define NT_ARM_TAGGED_ADDR_CTRL        0x409        /* AArch64 tagged address
//                                            control.  */
// #define NT_ARM_PAC_ENABLED_KEYS        0x40a        /* AArch64 pointer authentication
//                                            enabled keys.  */
// #define NT_ARM_SSVE        0x40b                /* ARM Streaming SVE registers.  */
// #define NT_ARM_ZA        0x40c                /* ARM SME ZA registers.  */
// #define NT_ARM_ZT        0x40d                /* ARM SME ZT registers.  */
// #define NT_ARM_FPMR        0x40e                /* ARM floating point mode register.  */
// #define NT_ARM_POE        0x40f                /* ARM POE registers.  */
// #define NT_ARM_GCS        0x410                /* ARM GCS state.  */
// #define NT_VMCOREDD        0x700                /* Vmcore Device Dump Note.  */
// #define NT_MIPS_DSP        0x800                /* MIPS DSP ASE registers.  */
// #define NT_MIPS_FP_MODE        0x801                /* MIPS floating-point mode.  */
// #define NT_MIPS_MSA        0x802                /* MIPS SIMD registers.  */
// #define NT_RISCV_CSR        0x900                /* RISC-V Control and Status Registers */
// #define NT_RISCV_VECTOR        0x901                /* RISC-V vector registers */
// #define NT_RISCV_TAGGED_ADDR_CTRL        0x902        /* RISC-V tagged
//                                                    address control */
// #define NT_LOONGARCH_CPUCFG        0xa00        /* LoongArch CPU config registers.  */
// #define NT_LOONGARCH_CSR        0xa01        /* LoongArch control and
//                                            status registers.  */
// #define NT_LOONGARCH_LSX        0xa02        /* LoongArch Loongson SIMD
//                                            Extension registers.  */
// #define NT_LOONGARCH_LASX        0xa03        /* LoongArch Loongson Advanced
//                                            SIMD Extension registers.  */
// #define NT_LOONGARCH_LBT        0xa04        /* LoongArch Loongson Binary
//                                            Translation registers.  */
// #define NT_LOONGARCH_HW_BREAK        0xa05   /* LoongArch hardware breakpoint registers */
// #define NT_LOONGARCH_HW_WATCH        0xa06   /* LoongArch hardware watchpoint registers */
//
// /* Legal values for the note segment descriptor types for object files.  */
//
// #define NT_VERSION        1                /* Contains a version string.  */
//
//
// /* Dynamic section entry.  */
//
// typedef struct ELF32_Dyn ELF32_Dyn;
// struct ELF32_Dyn
// {
//   S32        d_tag;                        /* Dynamic entry type */
//   union
//     {
//       U32 d_val;                        /* Integer value */
//       Address32 d_ptr;                        /* Address value */
//     } d_un;
// };
//
// typedef struct
// {
//   S64        d_tag;                        /* Dynamic entry type */
//   union
//     {
//       U64 d_val;                /* Integer value */
//       Address64 d_ptr;                        /* Address value */
//     } d_un;
// } ELF64_Dyn;
//
// /* Legal values for d_tag (dynamic entry type).  */
//
// #define DT_NULL                0                /* Marks end of dynamic section */
// #define DT_NEEDED        1                /* Name of needed library */
// #define DT_PLTRelocationsSZ        2                /* Size in bytes of PLT relocs */
// #define DT_PLTGOT        3                /* Processor defined value */
// #define DT_Symbols_Hash_Table                4                /* Address of symbol hash table */
// #define DT_Strings_Table        5                /* Address of string table */
// #define DT_Symbols_Table        6                /* Address of symbol table */
// #define DT_Relocations_Addends                7                /* Address of Rela relocs */
// #define DT_Relocations_AddendsSZ        8                /* Total size of Rela relocs */
// #define DT_Relocations_AddendsENT        9                /* Size of one Rela reloc */
// #define DT_STRSZ        10                /* Size of string table */
// #define DT_SYMENT        11                /* Size of one symbol table entry */
// #define DT_INIT                12                /* Address of init function */
// #define DT_FINI                13                /* Address of termination function */
// #define DT_SONAME        14                /* Name of shared object */
// #define DT_RPATH        15                /* Library search path (deprecated) */
// #define DT_SYMBOLIC        16                /* Start symbol search here */
// #define DT_Relocations                17                /* Address of Rel relocs */
// #define DT_RelocationsSZ        18                /* Total size of Rel relocs */
// #define DT_RelocationsENT        19                /* Size of one Rel reloc */
// #define DT_PLTRelocations        20                /* Type of reloc in PLT */
// #define DT_DEBUG        21                /* For debugging; unspecified */
// #define DT_TEXTRelocations        22                /* Reloc might modify .text */
// #define DT_JMPRelocations        23                /* Address of PLT relocs */
// #define        DT_BIND_NOW        24                /* Process relocations of object */
// #define        DT_INIT_ARRAY        25                /* Array with addresses of init fct */
// #define        DT_FINI_ARRAY        26                /* Array with addresses of fini fct */
// #define        DT_INIT_ARRAYSZ        27                /* Size in bytes of DT_INIT_ARRAY */
// #define        DT_FINI_ARRAYSZ        28                /* Size in bytes of DT_FINI_ARRAY */
// #define DT_RUNPATH        29                /* Library search path */
// #define DT_FLAGS        30                /* Flags for the object being loaded */
// #define DT_ENCODING        32                /* Start of encoded range */
// #define DT_PREINIT_ARRAY 32                /* Array with addresses of preinit fct*/
// #define DT_PREINIT_ARRAYSZ 33                /* size in bytes of DT_PREINIT_ARRAY */
// #define DT_Symbols_Table_ELF_Section_IndexDX        34                /* Address of Symbols_Table_ELF_Section_IndexDX section */
// #define DT_RelocationsRSZ        35                /* Total size of RelocationsR relative relocations */
// #define DT_RelocationsR                36                /* Address of RelocationsR relative relocations */
// #define DT_RelocationsRENT        37                /* Size of one RelocationsR relative relocation */
// #define        DT_NUM                38                /* Number used */
// #define DT_LOOS                0x6000000d        /* Start of OS-specific */
// #define DT_HIOS                0x6ffff000        /* End of OS-specific */
// #define DT_LOPROC        0x70000000        /* Start of processor-specific */
// #define DT_HIPROC        0x7fffffff        /* End of processor-specific */
// #define        DT_PROCNUM        DT_MIPS_NUM        /* Most used by any processor */
//
// /* DT_* entries which fall between DT_VALRNGHI & DT_VALRNGLO use the
//    Dyn.d_un.d_val field of the Elf*_Dyn structure.  This follows Sun's
//    approach.  */
// #define DT_VALRNGLO        0x6ffffd00
// #define DT_GNU_PRelocationsINKED 0x6ffffdf5        /* Prelinking timestamp */
// #define DT_GNU_CONFLICTSZ 0x6ffffdf6        /* Size of conflict section */
// #define DT_GNU_LIBLISTSZ 0x6ffffdf7        /* Size of library list */
// #define DT_CHECKSUM        0x6ffffdf8
// #define DT_PLTPADSZ        0x6ffffdf9
// #define DT_MOVEENT        0x6ffffdfa
// #define DT_MOVESZ        0x6ffffdfb
// #define DT_FEATURE_1        0x6ffffdfc        /* Feature selection (DTF_*).  */
// #define DT_POSFLAG_1        0x6ffffdfd        /* Flags for DT_* entries, effecting
//                                            the following DT_* entry.  */
// #define DT_SYMINSZ        0x6ffffdfe        /* Size of syminfo table (in bytes) */
// #define DT_SYMINENT        0x6ffffdff        /* Entry size of syminfo */
// #define DT_VALRNGHI        0x6ffffdff
// #define DT_VALTAGIDX(tag)        (DT_VALRNGHI - (tag))        /* Reverse order! */
// #define DT_VALNUM 12
//
// /* DT_* entries which fall between DT_ADDRRNGHI & DT_ADDRRNGLO use the
//    Dyn.d_un.d_ptr field of the Elf*_Dyn structure.
//
//    If any adjustment is made to the ELF object after it has been
//    built these entries will need to be adjusted.  */
// #define DT_ADDRRNGLO        0x6ffffe00
// #define DT_GNU_Symbols_Hash_Table        0x6ffffef5        /* GNU-style hash table.  */
// #define DT_TLSDESC_PLT        0x6ffffef6
// #define DT_TLSDESC_GOT        0x6ffffef7
// #define DT_GNU_CONFLICT        0x6ffffef8        /* Start of conflict section */
// #define DT_GNU_LIBLIST        0x6ffffef9        /* Library list */
// #define DT_CONFIG        0x6ffffefa        /* Configuration information.  */
// #define DT_DEPAUDIT        0x6ffffefb        /* Dependency auditing.  */
// #define DT_AUDIT        0x6ffffefc        /* Object auditing.  */
// #define        DT_PLTPAD        0x6ffffefd        /* PLT padding.  */
// #define        DT_MOVETAB        0x6ffffefe        /* Move table.  */
// #define DT_SYMINFO        0x6ffffeff        /* Syminfo table.  */
// #define DT_ADDRRNGHI        0x6ffffeff
// #define DT_ADDRTAGIDX(tag)        (DT_ADDRRNGHI - (tag))        /* Reverse order! */
// #define DT_ADDRNUM 11
//
// /* The versioning entry types.  The next are defined as part of the
//    GNU extension.  */
// #define DT_VERSYM        0x6ffffff0
//
// #define DT_Relocations_AddendsCOUNT        0x6ffffff9
// #define DT_RelocationsCOUNT        0x6ffffffa
//
// /* These were chosen by Sun.  */
// #define DT_FLAGS_1        0x6ffffffb        /* State flags, see DF_1_* below.  */
// #define        DT_VERDEF        0x6ffffffc        /* Address of version definition
//                                            table */
// #define        DT_VERDEFNUM        0x6ffffffd        /* Number of version definitions */
// #define        DT_VERNEED        0x6ffffffe        /* Address of table with needed
//                                            versions */
// #define        DT_VERNEEDNUM        0x6fffffff        /* Number of needed versions */
// #define DT_VERSIONTAGIDX(tag)        (DT_VERNEEDNUM - (tag))        /* Reverse order! */
// #define DT_VERSIONTAGNUM 16
//
// /* Sun added these machine-independent extensions in the "processor-specific"
//    range.  Be compatible.  */
// #define DT_AUXILIARY    0x7ffffffd      /* Shared object to load before self */
// #define DT_FILTER       0x7fffffff      /* Shared object to get values from */
// #define DT_EXTRATAGIDX(tag)        ((U32)-((S32) (tag) <<1>>1)-1)
// #define DT_EXTRANUM        3
//
// /* Values of `d_un.d_val' in the DT_FLAGS entry.  */
// #define DF_ORIGIN        0x00000001        /* Object may use DF_ORIGIN */
// #define DF_SYMBOLIC        0x00000002        /* Symbol resolutions starts here */
// #define DF_TEXTRelocations        0x00000004        /* Object contains text relocations */
// #define DF_BIND_NOW        0x00000008        /* No lazy binding for this object */
// #define DF_STATIC_TLS        0x00000010        /* Module uses the static TLS model */
//
// /* State flags selectable in the `d_un.d_val' element of the DT_FLAGS_1
//    entry in the dynamic section.  */
// #define DF_1_NOW        0x00000001        /* Set RTLD_NOW for this object.  */
// #define DF_1_GLOBAL        0x00000002        /* Set RTLD_GLOBAL for this object.  */
// #define DF_1_GROUP        0x00000004        /* Set RTLD_GROUP for this object.  */
// #define DF_1_NODELETE        0x00000008        /* Set RTLD_NODELETE for this object.*/
// #define DF_1_LOADFLTR        0x00000010        /* Trigger filtee loading at runtime.*/
// #define DF_1_INITFIRST        0x00000020        /* Set RTLD_INITFIRST for this object*/
// #define DF_1_NOOPEN        0x00000040        /* Set RTLD_NOOPEN for this object.  */
// #define DF_1_ORIGIN        0x00000080        /* $ORIGIN must be handled.  */
// #define DF_1_DIRECT        0x00000100        /* Direct binding enabled.  */
// #define DF_1_TRANS        0x00000200
// #define DF_1_INTERPOSE        0x00000400        /* Object is used to interpose.  */
// #define DF_1_NODEFLIB        0x00000800        /* Ignore default lib search path.  */
// #define DF_1_NODUMP        0x00001000        /* Object can't be dldump'ed.  */
// #define DF_1_CONFALT        0x00002000        /* Configuration alternative created.*/
// #define DF_1_ENDFILTEE        0x00004000        /* Filtee terminates filters search. */
// #define        DF_1_DISPRelocationsDNE        0x00008000        /* Disp reloc applied at build time. */
// #define        DF_1_DISPRelocationsPND        0x00010000        /* Disp reloc applied at run-time.  */
// #define        DF_1_NODIRECT        0x00020000        /* Object has no-direct binding. */
// #define        DF_1_IGNMULDEF        0x00040000
// #define        DF_1_NOKSYMS        0x00080000
// #define        DF_1_NOHDR        0x00100000
// #define        DF_1_EDITED        0x00200000        /* Object is modified after built.  */
// #define        DF_1_NORelocationsOC        0x00400000
// #define        DF_1_SYMINTPOSE        0x00800000        /* Object has individual interposers.  */
// #define        DF_1_GLOBAUDIT        0x01000000        /* Global auditing required.  */
// #define        DF_1_SINGLETON        0x02000000        /* Singleton symbols are used.  */
// #define        DF_1_STUB        0x04000000
// #define        DF_1_PIE        0x08000000
// #define        DF_1_KMOD       0x10000000
// #define        DF_1_WEAKFILTER 0x20000000
// #define        DF_1_NOCOMMON   0x40000000
//
// /* Flags for the feature selection in DT_FEATURE_1.  */
// #define DTF_1_PARINIT        0x00000001
// #define DTF_1_CONFEXP        0x00000002
//
// /* Flags in the DT_POSFLAG_1 entry effecting only the next DT_* entry.  */
// #define DF_P1_LAZYLOAD        0x00000001        /* Lazyload following object.  */
// #define DF_P1_GROUPPERM        0x00000002        /* Symbols from next object are not
//                                            generally available.  */
//
// /* Version definition sections.  */
//
// typedef struct ELF32_Verdef ELF32_Verdef;
// struct ELF32_Verdef
// {
//   U16        vd_version;                /* Version revision */
//   U16        vd_flags;                /* Version information */
//   U16        vd_ndx;                        /* Version Index */
//   U16        vd_cnt;                        /* Number of associated aux entries */
//   U32        vd_hash;                /* Version name hash value */
//   U32        vd_aux;                        /* Offset in bytes to verdaux array */
//   U32        vd_next;                /* Offset in bytes to next verdef
//                                            entry */
// };
//
// typedef struct
// {
//   U16        vd_version;                /* Version revision */
//   U16        vd_flags;                /* Version information */
//   U16        vd_ndx;                        /* Version Index */
//   U16        vd_cnt;                        /* Number of associated aux entries */
//   U64        vd_hash;                /* Version name hash value */
//   U64        vd_aux;                        /* Offset in bytes to verdaux array */
//   U64        vd_next;                /* Offset in bytes to next verdef
//                                            entry */
// } ELF64_Verdef;
//
//
// /* Legal values for vd_version (version revision).  */
// #define VER_DEF_NONE        0                /* No version */
// #define VER_DEF_CURRENT        1                /* Current version */
// #define VER_DEF_NUM        2                /* Given version number */
//
// /* Legal values for vd_flags (version information flags).  */
// #define VER_FLG_BASE        0x1                /* Version definition of file itself */
// #define VER_FLG_WEAK        0x2                /* Weak version identifier.  Also
//                                            used by vna_flags below.  */
//
// /* Versym symbol index values.  */
// #define        VER_NDX_LOCAL                0        /* Symbol is local.  */
// #define        VER_NDX_GLOBAL                1        /* Symbol is global.  */
// #define        VER_NDX_LORESERVE        0xff00        /* Beginning of reserved entries.  */
// #define        VER_NDX_ELIMINATE        0xff01        /* Symbol is to be eliminated.  */
//
// /* Auxiliary version information.  */
//
// typedef struct ELF32_Verdaux ELF32_Verdaux;
// struct ELF32_Verdaux
// {
//   U32        vda_name;                /* Version or dependency names */
//   U32        vda_next;                /* Offset in bytes to next verdaux
//                                            entry */
// };
//
// typedef struct
// {
//   U64        vda_name;                /* Version or dependency names */
//   U64        vda_next;                /* Offset in bytes to next verdaux
//                                            entry */
// } ELF64_Verdaux;
//
//
// /* Version dependency section.  */
//
// typedef struct ELF32_Verneed ELF32_Verneed;
// struct ELF32_Verneed
// {
//   U16        vn_version;                /* Version of structure */
//   U16        vn_cnt;                        /* Number of associated aux entries */
//   U32        vn_file;                /* Offset of filename for this
//                                            dependency */
//   U32        vn_aux;                        /* Offset in bytes to vernaux array */
//   U32        vn_next;                /* Offset in bytes to next verneed
//                                            entry */
// };
//
// typedef struct
// {
//   U16        vn_version;                /* Version of structure */
//   U16        vn_cnt;                        /* Number of associated aux entries */
//   U64        vn_file;                /* Offset of filename for this
//                                            dependency */
//   U64        vn_aux;                        /* Offset in bytes to vernaux array */
//   U64        vn_next;                /* Offset in bytes to next verneed
//                                            entry */
// } ELF64_Verneed;
//
//
// /* Legal values for vn_version (version revision).  */
// #define VER_NEED_NONE         0                /* No version */
// #define VER_NEED_CURRENT 1                /* Current version */
// #define VER_NEED_NUM         2                /* Given version number */
//
// /* Auxiliary needed version information.  */
//
// typedef struct ELF32_Vernaux ELF32_Vernaux;
// struct ELF32_Vernaux
// {
//   U32        vna_hash;                /* Hash value of dependency name */
//   U16        vna_flags;                /* Dependency specific information */
//   U16        vna_other;                /* Unused */
//   U32        vna_name;                /* Dependency name string offset */
//   U32        vna_next;                /* Offset in bytes to next vernaux
//                                            entry */
// };
//
// typedef struct
// {
//   U64        vna_hash;                /* Hash value of dependency name */
//   U16        vna_flags;                /* Dependency specific information */
//   U16        vna_other;                /* Unused */
//   U64        vna_name;                /* Dependency name string offset */
//   U64        vna_next;                /* Offset in bytes to next vernaux
//                                            entry */
// } ELF64_Vernaux;
//
//
// /* Auxiliary vector.  */
//
// /* This vector is normally only used by the program interpreter.  The
//    usual definition in an ABI supplement uses the name auxv_t.  The
//    vector is not usually defined in a standard <elf.h> file, but it
//    can't hurt.  We rename it to avoid conflicts.  The sizes of these
//    types are an arrangement between the exec server and the program
//    interpreter, so we don't fully specify them here.  */
//
// typedef struct ELF32_auxv_t ELF32_auxv_t;
// struct ELF32_auxv_t
// {
//   U32 a_type;                /* Entry type */
//   union
//     {
//       U32 a_val;                /* Integer value */
//       /* We use to have pointer elements added here.  We cannot do that,
//          though, since it does not work when using 32-bit definitions
//          on 64-bit platforms and vice versa.  */
//     } a_un;
// };
//
// typedef struct
// {
//   U64 a_type;                /* Entry type */
//   union
//     {
//       U64 a_val;                /* Integer value */
//       /* We use to have pointer elements added here.  We cannot do that,
//          though, since it does not work when using 32-bit definitions
//          on 64-bit platforms and vice versa.  */
//     } a_un;
// } ELF64_auxv_t;
//
// /* Legal values for a_type (entry type).  */
//
// #define AT_NULL                0                /* End of vector */
// #define AT_IGNORE        1                /* Entry should be ignored */
// #define AT_EXECFD        2                /* File descriptor of program */
// #define AT_PHDR                3                /* Program headers for program */
// #define AT_PHENT        4                /* Size of program header entry */
// #define AT_PHNUM        5                /* Number of program headers */
// #define AT_PAGESZ        6                /* System page size */
// #define AT_BASE                7                /* Base address of interpreter */
// #define AT_FLAGS        8                /* Flags */
// #define AT_ENTRY        9                /* Entry point of program */
// #define AT_NotesLF        10                /* Program is not ELF */
// #define AT_UID                11                /* Real uid */
// #define AT_EUID                12                /* Effective uid */
// #define AT_GID                13                /* Real gid */
// #define AT_EGID                14                /* Effective gid */
// #define AT_CLKTCK        17                /* Frequency of times() */
//
// /* Some more special a_type values describing the hardware.  */
// #define AT_PLATFORM        15                /* String identifying platform.  */
// #define AT_HWCAP        16                /* Machine-dependent hints about
//                                            processor capabilities.  */
//
// /* This entry gives some information about the FPU initialization
//    performed by the kernel.  */
// #define AT_FPUCW        18                /* Used FPU control word.  */
//
// /* Cache block sizes.  */
// #define AT_DCACHEBSIZE        19                /* Data cache block size.  */
// #define AT_ICACHEBSIZE        20                /* Instruction cache block size.  */
// #define AT_UCACHEBSIZE        21                /* Unified cache block size.  */
//
// /* A special ignored value for PPC, used by the kernel to control the
//    interpretation of the AUXV. Must be > 16.  */
// #define AT_IGNOREPPC        22                /* Entry should be ignored.  */
//
// #define        AT_SECURE        23                /* Boolean, was exec setuid-like?  */
//
// #define AT_BASE_PLATFORM 24                /* String identifying real platforms.*/
//
// #define AT_RANDOM        25                /* Address of 16 random bytes.  */
//
// #define AT_HWCAP2        26                /* More machine-dependent hints about
//                                            processor capabilities.  */
//
// #define AT_RSEQ_FEATURE_SIZE        27        /* rseq supported feature size.  */
// #define AT_RSEQ_ALIGN        28                /* rseq allocation alignment.  */
//
// /* More machine-dependent hints about processor capabilities.  */
// #define AT_HWCAP3        29                /* extension of AT_HWCAP.  */
// #define AT_HWCAP4        30                /* extension of AT_HWCAP.  */
//
// #define AT_EXECFN        31                /* Filename of executable.  */
//
// /* Pointer to the global system page used for system calls and other
//    nice things.  */
// #define AT_SYSINFO        32
// #define AT_SYSINFO_EHDR        33
//
// /* Shapes of the caches.  Bits 0-3 contains associativity; bits 4-7 contains
//    log2 of line size; mask those to get cache size.  */
// #define AT_L1I_CACHESHAPE        34
// #define AT_L1D_CACHESHAPE        35
// #define AT_L2_CACHESHAPE        36
// #define AT_L3_CACHESHAPE        37
//
// /* Shapes of the caches, with more room to describe them.
//    *GEOMETRY are comprised of cache line size in bytes in the bottom 16 bits
//    and the cache associativity in the next 16 bits.  */
// #define AT_L1I_CACHESIZE        40
// #define AT_L1I_CACHEGEOMETRY        41
// #define AT_L1D_CACHESIZE        42
// #define AT_L1D_CACHEGEOMETRY        43
// #define AT_L2_CACHESIZE                44
// #define AT_L2_CACHEGEOMETRY        45
// #define AT_L3_CACHESIZE                46
// #define AT_L3_CACHEGEOMETRY        47
//
// #define AT_MINSIGSTKSZ                51 /* Stack needed for signal delivery  */
//
// /* Note section contents.  Each entry in the note section begins with
//    a header of a fixed form.  */
//
// typedef struct ELF32_Nhdr ELF32_Nhdr;
// struct ELF32_Nhdr
// {
//   U32 n_namesz;                        /* Length of the note's name.  */
//   U32 n_descsz;                        /* Length of the note's descriptor.  */
//   U32 n_type;                        /* Type of the note.  */
// };
//
// typedef struct
// {
//   U64 n_namesz;                        /* Length of the note's name.  */
//   U64 n_descsz;                        /* Length of the note's descriptor.  */
//   U64 n_type;                        /* Type of the note.  */
// } ELF64_Nhdr;
//
// /* Known names of notes.  */
//
// /* Solaris entries in the note section have this name.  */
// #define ELF_Notes_SOLARIS        "SUNW Solaris"
//
// /* Note entries for GNU systems have this name.  */
// #define ELF_Notes_GNU                "GNU"
//
// /* Note entries for freedesktop.org have this name.  */
// #define ELF_Notes_FDO                "FDO"
//
// /* Defined types of notes for Solaris.  */
//
// /* Value of descriptor (one word) is desired pagesize for the binary.  */
// #define ELF_Notes_PAGESIZE_HINT        1
//
//
// /* Defined note types for GNU systems.  */
//
// /* ABI information.  The descriptor consists of words:
//    word 0: OS descriptor
//    word 1: major version of the ABI
//    word 2: minor version of the ABI
//    word 3: subminor version of the ABI
// */
// #define NT_GNU_ABI_TAG        1
// #define ELF_Notes_ABI        NT_GNU_ABI_TAG /* Old name.  */
//
// /* Known OSes.  These values can appear in word 0 of an
//    NT_GNU_ABI_TAG note section entry.  */
// #define ELF_Notes_OS_LINUX        0
// #define ELF_Notes_OS_GNU                1
// #define ELF_Notes_OS_SOLARIS2        2
// #define ELF_Notes_OS_FREEBSD        3
//
// /* Synthetic hwcap information.  The descriptor begins with two words:
//    word 0: number of entries
//    word 1: bitmask of enabled entries
//    Then follow variable-length entries, one byte followed by a
//    '\0'-terminated hwcap name string.  The byte gives the bit
//    number to test if enabled, (1U << bit) & bitmask.  */
// #define NT_GNU_HWCAP        2
//
// /* Build ID bits as generated by ld --build-id.
//    The descriptor consists of any nonzero number of bytes.  */
// #define NT_GNU_BUILD_ID        3
//
// /* Version note generated by GNU gold containing a version string.  */
// #define NT_GNU_GOLD_VERSION        4
//
// /* Program property.  */
// #define NT_GNU_PROPERTY_TYPE_0 5
//
// /* Packaging metadata as defined on
//    https://systemd.io/ELF_PACKAGE_METADATA/ */
// #define NT_FDO_PACKAGING_METADATA 0xcafe1a7e
//
// /* dlopen metadata as defined on
//    https://systemd.io/ELF_DLOPEN_METADATA/ */
// #define NT_FDO_DLOPEN_METADATA 0x407c0c0a
//
// /* Note section name of program property.   */
// #define Notes_GNU_PROPERTY_SECTION_NAME ".note.gnu.property"
//
// /* Values used in GNU .note.gnu.property notes (NT_GNU_PROPERTY_TYPE_0).  */
//
// /* Stack size.  */
// #define GNU_PROPERTY_STACK_SIZE                        1
// /* No copy relocation on protected data symbol.  */
// #define GNU_PROPERTY_NO_COPY_ON_PROTECTED        2
//
// /* A 4-byte unsigned integer property: A bit is set if it is set in all
//    relocatable inputs.  */
// #define GNU_PROPERTY_UINT32_AND_LO        0xb0000000
// #define GNU_PROPERTY_UINT32_AND_HI        0xb0007fff
//
// /* A 4-byte unsigned integer property: A bit is set if it is set in any
//    relocatable inputs.  */
// #define GNU_PROPERTY_UINT32_OR_LO        0xb0008000
// #define GNU_PROPERTY_UINT32_OR_HI        0xb000ffff
//
// /* The needed properties by the object file.  */
// #define GNU_PROPERTY_1_NEEDED                GNU_PROPERTY_UINT32_OR_LO
//
// /* Set if the object file requires canonical function pointers and
//    cannot be used with copy relocation.  */
// #define GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS (1U << 0)
//
// /* Processor-specific semantics, lo */
// #define GNU_PROPERTY_LOPROC                        0xc0000000
// /* Processor-specific semantics, hi */
// #define GNU_PROPERTY_HIPROC                        0xdfffffff
// /* Application-specific semantics, lo */
// #define GNU_PROPERTY_LOUSER                        0xe0000000
// /* Application-specific semantics, hi */
// #define GNU_PROPERTY_HIUSER                        0xffffffff
//
// /* AArch64 specific GNU properties.  */
// #define GNU_PROPERTY_AARCH64_FEATURE_1_AND        0xc0000000
//
// #define GNU_PROPERTY_AARCH64_FEATURE_1_BTI        (1U << 0)
// #define GNU_PROPERTY_AARCH64_FEATURE_1_PAC        (1U << 1)
// #define GNU_PROPERTY_AARCH64_FEATURE_1_GCS        (1U << 2)
//
// /* The x86 instruction sets indicated by the corresponding bits are
//    used in program.  Their support in the hardware is optional.  */
// #define GNU_PROPERTY_X86_ISA_1_USED                0xc0010002
// /* The x86 instruction sets indicated by the corresponding bits are
//    used in program and they must be supported by the hardware.   */
// #define GNU_PROPERTY_X86_ISA_1_NEEDED                0xc0008002
// /* X86 processor-specific features used in program.  */
// #define GNU_PROPERTY_X86_FEATURE_1_AND                0xc0000002
//
// /* GNU_PROPERTY_X86_ISA_1_BASELINE: CMOV, CX8 (cmpxchg8b), FPU (fld),
//    MMX, OSFXSR (fxsave), SCE (syscall), SSE and SSE2.  */
// #define GNU_PROPERTY_X86_ISA_1_BASELINE                (1U << 0)
// /* GNU_PROPERTY_X86_ISA_1_V2: GNU_PROPERTY_X86_ISA_1_BASELINE,
//    CMPXCHG16B (cmpxchg16b), LAHF-SAHF (lahf), POPCNT (popcnt), SSE3,
//    SSSE3, SSE4.1 and SSE4.2.  */
// #define GNU_PROPERTY_X86_ISA_1_V2                (1U << 1)
// /* GNU_PROPERTY_X86_ISA_1_V3: GNU_PROPERTY_X86_ISA_1_V2, AVX, AVX2, BMI1,
//    BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE.  */
// #define GNU_PROPERTY_X86_ISA_1_V3                (1U << 2)
// /* GNU_PROPERTY_X86_ISA_1_V4: GNU_PROPERTY_X86_ISA_1_V3, AVX512F,
//    AVX512BW, AVX512CD, AVX512DQ and AVX512VL.  */
// #define GNU_PROPERTY_X86_ISA_1_V4                (1U << 3)
//
// /* This indicates that all executable sections are compatible with
//    IBT.  */
// #define GNU_PROPERTY_X86_FEATURE_1_IBT                (1U << 0)
// /* This indicates that all executable sections are compatible with
//    SHSTK.  */
// #define GNU_PROPERTY_X86_FEATURE_1_SHSTK        (1U << 1)
//
// /* Move records.  */
// typedef struct ELF32_Move ELF32_Move;
// struct ELF32_Move
// {
//   U64 m_value;                /* Symbol value.  */
//   U32 m_info;                /* Size and index.  */
//   U32 m_poffset;                /* Symbol offset.  */
//   U16 m_repeat;                /* Repeat count.  */
//   U16 m_stride;                /* Stride info.  */
// };
//
// typedef struct
// {
//   U64 m_value;                /* Symbol value.  */
//   U64 m_info;                /* Size and index.  */
//   U64 m_poffset;        /* Symbol offset.  */
//   U16 m_repeat;                /* Repeat count.  */
//   U16 m_stride;                /* Stride info.  */
// } ELF64_Move;
//
// /* Macro to construct move records.  */
// #define ELF32_M_SYM(info)        ((info) >> 8)
// #define ELF32_M_SIZE(info)        ((U8) (info))
// #define ELF32_M_INFO(sym, size)        (((sym) << 8) + (U8) (size))
//
// #define ELF64_M_SYM(info)        ELF32_M_SYM (info)
// #define ELF64_M_SIZE(info)        ELF32_M_SIZE (info)
// #define ELF64_M_INFO(sym, size)        ELF32_M_INFO (sym, size)
//
//
// /* Motorola 68k specific definitions.  */
//
// /* Values for ELF32_Ehdr.processor_flags.  */
// #define EF_CPU32        0x00810000
//
// /* m68k relocs.  */
//
// #define R_68K_NONE        0                /* No reloc */
// #define R_68K_32        1                /* Direct 32 bit  */
// #define R_68K_16        2                /* Direct 16 bit  */
// #define R_68K_8                3                /* Direct 8 bit  */
// #define R_68K_PC32        4                /* PC relative 32 bit */
// #define R_68K_PC16        5                /* PC relative 16 bit */
// #define R_68K_PC8        6                /* PC relative 8 bit */
// #define R_68K_GOT32        7                /* 32 bit PC relative GOT entry */
// #define R_68K_GOT16        8                /* 16 bit PC relative GOT entry */
// #define R_68K_GOT8        9                /* 8 bit PC relative GOT entry */
// #define R_68K_GOT32O        10                /* 32 bit GOT offset */
// #define R_68K_GOT16O        11                /* 16 bit GOT offset */
// #define R_68K_GOT8O        12                /* 8 bit GOT offset */
// #define R_68K_PLT32        13                /* 32 bit PC relative PLT address */
// #define R_68K_PLT16        14                /* 16 bit PC relative PLT address */
// #define R_68K_PLT8        15                /* 8 bit PC relative PLT address */
// #define R_68K_PLT32O        16                /* 32 bit PLT offset */
// #define R_68K_PLT16O        17                /* 16 bit PLT offset */
// #define R_68K_PLT8O        18                /* 8 bit PLT offset */
// #define R_68K_COPY        19                /* Copy symbol at runtime */
// #define R_68K_GLOB_DAT        20                /* Create GOT entry */
// #define R_68K_JMP_SLOT        21                /* Create PLT entry */
// #define R_68K_Relocations_AddendsTIVE        22                /* Adjust by program base */
// #define R_68K_TLS_GD32      25          /* 32 bit GOT offset for GD */
// #define R_68K_TLS_GD16      26          /* 16 bit GOT offset for GD */
// #define R_68K_TLS_GD8       27          /* 8 bit GOT offset for GD */
// #define R_68K_TLS_LDM32     28          /* 32 bit GOT offset for LDM */
// #define R_68K_TLS_LDM16     29          /* 16 bit GOT offset for LDM */
// #define R_68K_TLS_LDM8      30          /* 8 bit GOT offset for LDM */
// #define R_68K_TLS_LDO32     31          /* 32 bit module-relative offset */
// #define R_68K_TLS_LDO16     32          /* 16 bit module-relative offset */
// #define R_68K_TLS_LDO8      33          /* 8 bit module-relative offset */
// #define R_68K_TLS_IE32      34          /* 32 bit GOT offset for IE */
// #define R_68K_TLS_IE16      35          /* 16 bit GOT offset for IE */
// #define R_68K_TLS_IE8       36          /* 8 bit GOT offset for IE */
// #define R_68K_TLS_LE32      37          /* 32 bit offset relative to
//                                            static TLS block */
// #define R_68K_TLS_LE16      38          /* 16 bit offset relative to
//                                            static TLS block */
// #define R_68K_TLS_LE8       39          /* 8 bit offset relative to
//                                            static TLS block */
// #define R_68K_TLS_DTPMOD32  40          /* 32 bit module number */
// #define R_68K_TLS_DTPRelocations32  41          /* 32 bit module-relative offset */
// #define R_68K_TLS_TPRelocations32   42          /* 32 bit TP-relative offset */
// /* Keep this the last entry.  */
// #define R_68K_NUM        43
//
// /* Intel 80386 specific definitions.  */
//
// /* i386 relocs.  */
//
// #define R_386_NONE           0                /* No reloc */
// #define R_386_32           1                /* Direct 32 bit  */
// #define R_386_PC32           2                /* PC relative 32 bit */
// #define R_386_GOT32           3                /* 32 bit GOT entry */
// #define R_386_PLT32           4                /* 32 bit PLT address */
// #define R_386_COPY           5                /* Copy symbol at runtime */
// #define R_386_GLOB_DAT           6                /* Create GOT entry */
// #define R_386_JMP_SLOT           7                /* Create PLT entry */
// #define R_386_Relocations_AddendsTIVE           8                /* Adjust by program base */
// #define R_386_GOTOFF           9                /* 32 bit offset to GOT */
// #define R_386_GOTPC           10                /* 32 bit PC relative offset to GOT */
// #define R_386_32PLT           11
// #define R_386_TLS_TPOFF           14                /* Offset in static TLS block */
// #define R_386_TLS_IE           15                /* Address of GOT entry for static TLS
//                                            block offset */
// #define R_386_TLS_GOTIE           16                /* GOT entry for static TLS block
//                                            offset */
// #define R_386_TLS_LE           17                /* Offset relative to static TLS
//                                            block */
// #define R_386_TLS_GD           18                /* Direct 32 bit for GNU version of
//                                            general dynamic thread local data */
// #define R_386_TLS_LDM           19                /* Direct 32 bit for GNU version of
//                                            local dynamic thread local data
//                                            in LE code */
// #define R_386_16           20
// #define R_386_PC16           21
// #define R_386_8                   22
// #define R_386_PC8           23
// #define R_386_TLS_GD_32           24                /* Direct 32 bit for general dynamic
//                                            thread local data */
// #define R_386_TLS_GD_PUSH  25                /* Tag for pushl in GD TLS code */
// #define R_386_TLS_GD_CALL  26                /* Relocation for call to
//                                            __tls_get_addr() */
// #define R_386_TLS_GD_POP   27                /* Tag for popl in GD TLS code */
// #define R_386_TLS_LDM_32   28                /* Direct 32 bit for local dynamic
//                                            thread local data in LE code */
// #define R_386_TLS_LDM_PUSH 29                /* Tag for pushl in LDM TLS code */
// #define R_386_TLS_LDM_CALL 30                /* Relocation for call to
//                                            __tls_get_addr() in LDM code */
// #define R_386_TLS_LDM_POP  31                /* Tag for popl in LDM TLS code */
// #define R_386_TLS_LDO_32   32                /* Offset relative to TLS block */
// #define R_386_TLS_IE_32           33                /* GOT entry for negated static TLS
//                                            block offset */
// #define R_386_TLS_LE_32           34                /* Negated offset relative to static
//                                            TLS block */
// #define R_386_TLS_DTPMOD32 35                /* ID of module containing symbol */
// #define R_386_TLS_DTPOFF32 36                /* Offset in TLS block */
// #define R_386_TLS_TPOFF32  37                /* Negated offset in static TLS block */
// #define R_386_SIZE32           38                 /* 32-bit symbol size */
// #define R_386_TLS_GOTDESC  39                /* GOT offset for TLS descriptor.  */
// #define R_386_TLS_DESC_CALL 40                /* Marker of call through TLS
//                                            descriptor for
//                                            relaxation.  */
// #define R_386_TLS_DESC     41                /* TLS descriptor containing
//                                            pointer to code and to
//                                            argument, returning the TLS
//                                            offset for the symbol.  */
// #define R_386_IRelocations_AddendsTIVE           42                /* Adjust indirectly by program base */
// #define R_386_GOT32X           43                /* Load from 32 bit GOT entry,
//                                            relaxable. */
// /* Keep this the last entry.  */
// #define R_386_NUM           44
//
// /* SUN SPARC specific definitions.  */
//
// /* Legal values for ST_TYPE subfield of type_and_binding (symbol type).  */
//
// #define STT_SPARC_REGISTER        13        /* Global register reserved to app. */
//
// /* Values for ELF64_Section_Header.processor_flags.  */
//
// #define EF_SPARCV9_MM                3
// #define EF_SPARCV9_TSO                0
// #define EF_SPARCV9_PSO                1
// #define EF_SPARCV9_RMO                2
// #define EF_SPARC_LEDATA                0x800000 /* little endian data */
// #define EF_SPARC_EXT_MASK        0xFFFF00
// #define EF_SPARC_32PLUS                0x000100 /* generic V8+ features */
// #define EF_SPARC_SUN_US1        0x000200 /* Sun UltraSPARC1 extensions */
// #define EF_SPARC_HAL_R1                0x000400 /* HAL R1 extensions */
// #define EF_SPARC_SUN_US3        0x000800 /* Sun UltraSPARCIII extensions */
//
// /* SPARC relocs.  */
//
// #define R_SPARC_NONE                0        /* No reloc */
// #define R_SPARC_8                1        /* Direct 8 bit */
// #define R_SPARC_16                2        /* Direct 16 bit */
// #define R_SPARC_32                3        /* Direct 32 bit */
// #define R_SPARC_DISP8                4        /* PC relative 8 bit */
// #define R_SPARC_DISP16                5        /* PC relative 16 bit */
// #define R_SPARC_DISP32                6        /* PC relative 32 bit */
// #define R_SPARC_WDISP30                7        /* PC relative 30 bit shifted */
// #define R_SPARC_WDISP22                8        /* PC relative 22 bit shifted */
// #define R_SPARC_HI22                9        /* High 22 bit */
// #define R_SPARC_22                10        /* Direct 22 bit */
// #define R_SPARC_13                11        /* Direct 13 bit */
// #define R_SPARC_LO10                12        /* Truncated 10 bit */
// #define R_SPARC_GOT10                13        /* Truncated 10 bit GOT entry */
// #define R_SPARC_GOT13                14        /* 13 bit GOT entry */
// #define R_SPARC_GOT22                15        /* 22 bit GOT entry shifted */
// #define R_SPARC_PC10                16        /* PC relative 10 bit truncated */
// #define R_SPARC_PC22                17        /* PC relative 22 bit shifted */
// #define R_SPARC_WPLT30                18        /* 30 bit PC relative PLT address */
// #define R_SPARC_COPY                19        /* Copy symbol at runtime */
// #define R_SPARC_GLOB_DAT        20        /* Create GOT entry */
// #define R_SPARC_JMP_SLOT        21        /* Create PLT entry */
// #define R_SPARC_Relocations_AddendsTIVE        22        /* Adjust by program base */
// #define R_SPARC_UA32                23        /* Direct 32 bit unaligned */
//
// /* Additional Sparc64 relocs.  */
//
// #define R_SPARC_PLT32                24        /* Direct 32 bit ref to PLT entry */
// #define R_SPARC_HIPLT22                25        /* High 22 bit PLT entry */
// #define R_SPARC_LOPLT10                26        /* Truncated 10 bit PLT entry */
// #define R_SPARC_PCPLT32                27        /* PC rel 32 bit ref to PLT entry */
// #define R_SPARC_PCPLT22                28        /* PC rel high 22 bit PLT entry */
// #define R_SPARC_PCPLT10                29        /* PC rel trunc 10 bit PLT entry */
// #define R_SPARC_10                30        /* Direct 10 bit */
// #define R_SPARC_11                31        /* Direct 11 bit */
// #define R_SPARC_64                32        /* Direct 64 bit */
// #define R_SPARC_OLO10                33        /* 10bit with secondary 13bit addend */
// #define R_SPARC_HH22                34        /* Top 22 bits of direct 64 bit */
// #define R_SPARC_HM10                35        /* High middle 10 bits of ... */
// #define R_SPARC_LM22                36        /* Low middle 22 bits of ... */
// #define R_SPARC_PC_HH22                37        /* Top 22 bits of pc rel 64 bit */
// #define R_SPARC_PC_HM10                38        /* High middle 10 bit of ... */
// #define R_SPARC_PC_LM22                39        /* Low miggle 22 bits of ... */
// #define R_SPARC_WDISP16                40        /* PC relative 16 bit shifted */
// #define R_SPARC_WDISP19                41        /* PC relative 19 bit shifted */
// #define R_SPARC_GLOB_JMP        42        /* was part of v9 ABI but was removed */
// #define R_SPARC_7                43        /* Direct 7 bit */
// #define R_SPARC_5                44        /* Direct 5 bit */
// #define R_SPARC_6                45        /* Direct 6 bit */
// #define R_SPARC_DISP64                46        /* PC relative 64 bit */
// #define R_SPARC_PLT64                47        /* Direct 64 bit ref to PLT entry */
// #define R_SPARC_HIX22                48        /* High 22 bit complemented */
// #define R_SPARC_LOX10                49        /* Truncated 11 bit complemented */
// #define R_SPARC_H44                50        /* Direct high 12 of 44 bit */
// #define R_SPARC_M44                51        /* Direct mid 22 of 44 bit */
// #define R_SPARC_L44                52        /* Direct low 10 of 44 bit */
// #define R_SPARC_REGISTER        53        /* Global register usage */
// #define R_SPARC_UA64                54        /* Direct 64 bit unaligned */
// #define R_SPARC_UA16                55        /* Direct 16 bit unaligned */
// #define R_SPARC_TLS_GD_HI22        56
// #define R_SPARC_TLS_GD_LO10        57
// #define R_SPARC_TLS_GD_ADD        58
// #define R_SPARC_TLS_GD_CALL        59
// #define R_SPARC_TLS_LDM_HI22        60
// #define R_SPARC_TLS_LDM_LO10        61
// #define R_SPARC_TLS_LDM_ADD        62
// #define R_SPARC_TLS_LDM_CALL        63
// #define R_SPARC_TLS_LDO_HIX22        64
// #define R_SPARC_TLS_LDO_LOX10        65
// #define R_SPARC_TLS_LDO_ADD        66
// #define R_SPARC_TLS_IE_HI22        67
// #define R_SPARC_TLS_IE_LO10        68
// #define R_SPARC_TLS_IE_LD        69
// #define R_SPARC_TLS_IE_LDX        70
// #define R_SPARC_TLS_IE_ADD        71
// #define R_SPARC_TLS_LE_HIX22        72
// #define R_SPARC_TLS_LE_LOX10        73
// #define R_SPARC_TLS_DTPMOD32        74
// #define R_SPARC_TLS_DTPMOD64        75
// #define R_SPARC_TLS_DTPOFF32        76
// #define R_SPARC_TLS_DTPOFF64        77
// #define R_SPARC_TLS_TPOFF32        78
// #define R_SPARC_TLS_TPOFF64        79
// #define R_SPARC_GOTDATA_HIX22        80
// #define R_SPARC_GOTDATA_LOX10        81
// #define R_SPARC_GOTDATA_OP_HIX22        82
// #define R_SPARC_GOTDATA_OP_LOX10        83
// #define R_SPARC_GOTDATA_OP        84
// #define R_SPARC_H34                85
// #define R_SPARC_SIZE32                86
// #define R_SPARC_SIZE64                87
// #define R_SPARC_WDISP10                88
// #define R_SPARC_JMP_IRelocations        248
// #define R_SPARC_IRelocations_AddendsTIVE        249
// #define R_SPARC_GNU_VTINHERIT        250
// #define R_SPARC_GNU_VTENTRY        251
// #define R_SPARC_REV32                252
// /* Keep this the last entry.  */
// #define R_SPARC_NUM                253
//
// /* For Sparc64, legal values for d_tag of ELF64_Dyn.  */
//
// #define DT_SPARC_REGISTER        0x70000001
// #define DT_SPARC_NUM                2
//
// /* MIPS R3000 specific definitions.  */
//
// /* Legal values for processor_flags field of ELF32_Ehdr.  */
//
// #define EF_MIPS_NOREORDER        1     /* A .noreorder directive was used.  */
// #define EF_MIPS_PIC                2     /* Contains PIC code.  */
// #define EF_MIPS_CPIC                4     /* Uses PIC calling sequence.  */
// #define EF_MIPS_XGOT                8
// #define EF_MIPS_UCODE                16
// #define EF_MIPS_ABI2                32
// #define EF_MIPS_ABI_ON32        64
// #define EF_MIPS_OPTIONS_FIRST        0x00000080 /* Process the .MIPS.options
//                                               section first by ld.  */
// #define EF_MIPS_32BITMODE        0x00000100 /* Indicates code compiled for
//                                               a 64-bit machine in 32-bit
//                                               mode (regs are 32-bits
//                                               wide).  */
// #define EF_MIPS_FP64                512  /* Uses FP64 (12 callee-saved).  */
// #define EF_MIPS_NAN2008        1024  /* Uses IEEE 754-2008 NaN encoding.  */
// #define EF_MIPS_ARCH_ASE        0x0f000000 /* Architectural Extensions
//                                               used by this file.  */
// #define EF_MIPS_ARCH_ASE_MDMX        0x08000000 /* Use MDMX multimedia
//                                               extensions.  */
// #define EF_MIPS_ARCH_ASE_M16        0x04000000 /* Use MIPS-16 ISA
//                                               extensions.  */
// #define EF_MIPS_ARCH_ASE_MICROMIPS        0x02000000 /* Use MICROMIPS ISA
//                                                       extensions.  */
// #define EF_MIPS_ARCH                0xf0000000 /* MIPS architecture level.  */
//
// /* Legal values for MIPS architecture level.  */
//
// #define EF_MIPS_ARCH_1                0x00000000 /* -mips1 code.  */
// #define EF_MIPS_ARCH_2                0x10000000 /* -mips2 code.  */
// #define EF_MIPS_ARCH_3                0x20000000 /* -mips3 code.  */
// #define EF_MIPS_ARCH_4                0x30000000 /* -mips4 code.  */
// #define EF_MIPS_ARCH_5                0x40000000 /* -mips5 code.  */
// #define EF_MIPS_ARCH_32                0x50000000 /* MIPS32 code.  */
// #define EF_MIPS_ARCH_64                0x60000000 /* MIPS64 code.  */
// #define EF_MIPS_ARCH_32R2        0x70000000 /* MIPS32r2 code.  */
// #define EF_MIPS_ARCH_64R2        0x80000000 /* MIPS64r2 code.  */
// #define EF_MIPS_ARCH_32R6        0x90000000 /* MIPS32r6 code.  */
// #define EF_MIPS_ARCH_64R6        0xa0000000 /* MIPS64r6 code.  */
// #define EF_MIPS_ABI                0x0000F000 /* The ABI of the file.  Also
//                                               see EF_MIPS_ABI2 above.  */
// #define EF_MIPS_ABI_O32                0x00001000 /* The original o32 abi.  */
// #define EF_MIPS_ABI_O64                0x00002000 /* O32 extended to work on
//                                               64 bit architectures.  */
// #define EF_MIPS_ABI_EABI32        0x00003000 /* EABI in 32 bit mode.  */
// #define EF_MIPS_ABI_EABI64        0x00004000 /* EABI in 64 bit mode.  */
// #define EF_MIPS_MACH                0x00FF0000
// #define EF_MIPS_MACH_3900        0x00810000
// #define EF_MIPS_MACH_4010        0x00820000
// #define EF_MIPS_MACH_4100        0x00830000
// #define EF_MIPS_MACH_ALLEGREX        0x00840000
// #define EF_MIPS_MACH_4650        0x00850000
// #define EF_MIPS_MACH_4120        0x00870000
// #define EF_MIPS_MACH_4111        0x00880000
// #define EF_MIPS_MACH_SB1        0x008a0000
// #define EF_MIPS_MACH_OCTEON        0x008b0000
// #define EF_MIPS_MACH_XLR        0x008c0000
// #define EF_MIPS_MACH_OCTEON2        0x008d0000
// #define EF_MIPS_MACH_OCTEON3        0x008e0000
// #define EF_MIPS_MACH_5400        0x00910000
// #define EF_MIPS_MACH_5900        0x00920000
// #define EF_MIPS_MACH_IAMR2        0x00930000
// #define EF_MIPS_MACH_5500        0x00980000
// #define EF_MIPS_MACH_9000        0x00990000
// #define EF_MIPS_MACH_LS2E        0x00A00000
// #define EF_MIPS_MACH_LS2F        0x00A10000
// #define EF_MIPS_MACH_GS464        0x00A20000
// #define EF_MIPS_MACH_GS464E        0x00A30000
// #define EF_MIPS_MACH_GS264E        0x00A40000
//
// /* The following are unofficial names and should not be used.  */
//
// #define E_MIPS_ARCH_1                EF_MIPS_ARCH_1
// #define E_MIPS_ARCH_2                EF_MIPS_ARCH_2
// #define E_MIPS_ARCH_3                EF_MIPS_ARCH_3
// #define E_MIPS_ARCH_4                EF_MIPS_ARCH_4
// #define E_MIPS_ARCH_5                EF_MIPS_ARCH_5
// #define E_MIPS_ARCH_32                EF_MIPS_ARCH_32
// #define E_MIPS_ARCH_64                EF_MIPS_ARCH_64
//
// /* Special section indices.  */
//
// #define ELF_Section_Index_MIPS_ACOMMON        0xff00        /* Allocated common symbols.  */
// #define ELF_Section_Index_MIPS_TEXT                0xff01        /* Allocated test symbols.  */
// #define ELF_Section_Index_MIPS_DATA                0xff02        /* Allocated data symbols.  */
// #define ELF_Section_Index_MIPS_SCOMMON         0xff03        /* Small common symbols.  */
// #define ELF_Section_Index_MIPS_SUNDEFINED        0xff04        /* Small undefined symbols.  */
//
// /* Legal values for type field of ELF32_Section_Header.  */
//
// #define ELF_Section_Header_Type__MIPS_LIBLIST        0x70000000 /* Shared objects used in link.  */
// #define ELF_Section_Header_Type__MIPS_MSYM                0x70000001
// #define ELF_Section_Header_Type__MIPS_CONFLICT        0x70000002 /* Conflicting symbols.  */
// #define ELF_Section_Header_Type__MIPS_GPTAB                0x70000003 /* Global data area sizes.  */
// #define ELF_Section_Header_Type__MIPS_UCODE                0x70000004 /* Reserved for SGI/MIPS compilers */
// #define ELF_Section_Header_Type__MIPS_DEBUG                0x70000005 /* MIPS ECOFF debugging info.  */
// #define ELF_Section_Header_Type__MIPS_REGINFO        0x70000006 /* Register usage information.  */
// #define ELF_Section_Header_Type__MIPS_PACKAGE        0x70000007
// #define ELF_Section_Header_Type__MIPS_PACKSYM        0x70000008
// #define ELF_Section_Header_Type__MIPS_RelocationsD                0x70000009
// #define ELF_Section_Header_Type__MIPS_IFACE                0x7000000b
// #define ELF_Section_Header_Type__MIPS_CONTENT        0x7000000c
// #define ELF_Section_Header_Type__MIPS_OPTIONS        0x7000000d /* Miscellaneous options.  */
// #define ELF_Section_Header_Type__MIPS_SHDR                0x70000010
// #define ELF_Section_Header_Type__MIPS_FDESC                0x70000011
// #define ELF_Section_Header_Type__MIPS_EXTSYM                0x70000012
// #define ELF_Section_Header_Type__MIPS_DENSE                0x70000013
// #define ELF_Section_Header_Type__MIPS_PDESC                0x70000014
// #define ELF_Section_Header_Type__MIPS_LOCSYM                0x70000015
// #define ELF_Section_Header_Type__MIPS_AUXSYM                0x70000016
// #define ELF_Section_Header_Type__MIPS_OPTSYM                0x70000017
// #define ELF_Section_Header_Type__MIPS_LOCSTR                0x70000018
// #define ELF_Section_Header_Type__MIPS_LINE                0x70000019
// #define ELF_Section_Header_Type__MIPS_RFDESC                0x7000001a
// #define ELF_Section_Header_Type__MIPS_DELTASYM        0x7000001b
// #define ELF_Section_Header_Type__MIPS_DELTAINST        0x7000001c
// #define ELF_Section_Header_Type__MIPS_DELTACLASS        0x7000001d
// #define ELF_Section_Header_Type__MIPS_DWARF                0x7000001e /* DWARF debugging information.  */
// #define ELF_Section_Header_Type__MIPS_DELTADECL        0x7000001f
// #define ELF_Section_Header_Type__MIPS_SYMBOL_LIB        0x70000020
// #define ELF_Section_Header_Type__MIPS_EVENTS                0x70000021 /* Event section.  */
// #define ELF_Section_Header_Type__MIPS_TRANSLATE        0x70000022
// #define ELF_Section_Header_Type__MIPS_PIXIE                0x70000023
// #define ELF_Section_Header_Type__MIPS_XLATE                0x70000024
// #define ELF_Section_Header_Type__MIPS_XLATE_DEBUG        0x70000025
// #define ELF_Section_Header_Type__MIPS_WHIRL                0x70000026
// #define ELF_Section_Header_Type__MIPS_EH_REGION        0x70000027
// #define ELF_Section_Header_Type__MIPS_XLATE_OLD        0x70000028
// #define ELF_Section_Header_Type__MIPS_PDR_EXCEPTION        0x70000029
// #define ELF_Section_Header_Type__MIPS_ABIFLAGS        0x7000002a
// #define ELF_Section_Header_Type__MIPS_XSymbols_Hash_Table                0x7000002b
//
// /* Legal values for flags field of ELF32_Section_Header.  */
//
// #define SHF_MIPS_GPRelocations                0x10000000 /* Must be in global data area.  */
// #define SHF_MIPS_MERGE                0x20000000
// #define SHF_MIPS_ADDR                0x40000000
// #define SHF_MIPS_STRINGS        0x80000000
// #define SHF_MIPS_NOSTRIP        0x08000000
// #define SHF_MIPS_LOCAL                0x04000000
// #define SHF_MIPS_NAMES                0x02000000
// #define SHF_MIPS_NODUPE                0x01000000
//
//
// /* Symbol tables.  */
//
// /* MIPS specific values for `visibility'.  */
// #define STO_MIPS_DEFAULT                0x0
// #define STO_MIPS_INTERNAL                0x1
// #define STO_MIPS_HIDDEN                        0x2
// #define STO_MIPS_PROTECTED                0x3
// #define STO_MIPS_PLT                        0x8
// #define STO_MIPS_SC_ALIGN_UNUSED        0xff
//
// /* MIPS specific values for `type_and_binding'.  */
// #define ELF_Symbol_Binding_MIPS_SPLIT_COMMON                13
//
// /* Entries found in sections of type ELF_Section_Header_Type__MIPS_GPTAB.  */
//
// typedef union
// {
//   struct
//     {
//       U32 gt_current_g_value;        /* -G value used for compilation.  */
//       U32 gt_unused;                /* Not used.  */
//     } gt_header;                        /* First entry in section.  */
//   struct
//     {
//       U32 gt_g_value;                /* If this value were used for -G.  */
//       U32 gt_bytes;                /* This many bytes would be used.  */
//     } gt_entry;                                /* Subsequent entries in section.  */
// } ELF32_gptab;
//
// /* Entry found in sections of type ELF_Section_Header_Type__MIPS_REGINFO.  */
//
// typedef struct ELF32_RegInfo ELF32_RegInfo;
// struct ELF32_RegInfo
// {
//   U32 ri_gprmask;                /* General registers used.  */
//   U32 ri_cprmask[4];                /* Coprocessor registers used.  */
//   S32 ri_gp_value;                /* $gp register value.  */
// };
//
// /* Entries found in sections of type ELF_Section_Header_Type__MIPS_OPTIONS.  */
//
// typedef struct
// {
//   U8 kind;                /* Determines interpretation of the
//                                    variable part of descriptor.  */
//   U8 size;                /* Size of descriptor, including header.  */
//   ELF32_Section_Index section;        /* Section header index of section affected,
//                                    0 for global options.  */
//   U32 info;                /* Kind-specific information.  */
// } Elf_Options;
//
// /* Values for `kind' field in Elf_Options.  */
//
// #define ODK_NULL        0        /* Undefined.  */
// #define ODK_REGINFO        1        /* Register usage information.  */
// #define ODK_EXCEPTIONS        2        /* Exception processing options.  */
// #define ODK_PAD                3        /* Section padding options.  */
// #define ODK_HWPATCH        4        /* Hardware workarounds performed */
// #define ODK_FILL        5        /* record the fill value used by the linker. */
// #define ODK_TAGS        6        /* reserve space for desktop tools to write. */
// #define ODK_HWAND        7        /* HW workarounds.  'AND' bits when merging. */
// #define ODK_HWOR        8        /* HW workarounds.  'OR' bits when merging.  */
//
// /* Values for `info' in Elf_Options for ODK_EXCEPTIONS entries.  */
//
// #define OEX_FPU_MIN        0x1f        /* FPE's which MUST be enabled.  */
// #define OEX_FPU_MAX        0x1f00        /* FPE's which MAY be enabled.  */
// #define OEX_PAGE0        0x10000        /* page zero must be mapped.  */
// #define OEX_SMM                0x20000        /* Force sequential memory mode?  */
// #define OEX_FPDBUG        0x40000        /* Force floating point debug mode?  */
// #define OEX_PRECISEFP        OEX_FPDBUG
// #define OEX_DISMISS        0x80000        /* Dismiss invalid address faults?  */
//
// #define OEX_FPU_INVAL        0x10
// #define OEX_FPU_DIV0        0x08
// #define OEX_FPU_OFLO        0x04
// #define OEX_FPU_UFLO        0x02
// #define OEX_FPU_INEX        0x01
//
// /* Masks for `info' in Elf_Options for an ODK_HWPATCH entry.  */
//
// #define OHW_R4KEOP        0x1        /* R4000 end-of-page patch.  */
// #define OHW_R8KPFETCH        0x2        /* may need R8000 prefetch patch.  */
// #define OHW_R5KEOP        0x4        /* R5000 end-of-page patch.  */
// #define OHW_R5KCVTL        0x8        /* R5000 cvt.[ds].l bug.  clean=1.  */
//
// #define OPAD_PREFIX        0x1
// #define OPAD_POSTFIX        0x2
// #define OPAD_SYMBOL        0x4
//
// /* Entry found in `.options' section.  */
//
// typedef struct Elf_Options_Hw Elf_Options_Hw;
// struct Elf_Options_Hw
// {
//   U32 hwp_flags1;        /* Extra flags.  */
//   U32 hwp_flags2;        /* Extra flags.  */
// };
//
//
// /* Entries found in sections of type ELF_Section_Header_Type__MIPS_CONFLICT.  */
//
// typedef Address32 ELF32_Conflict;
//
// typedef struct
// {
//   /* Version of flags structure.  */
//   U16 version;
//   /* The level of the ISA: 1-5, 32, 64.  */
//   U8 isa_level;
//   /* The revision of ISA: 0 for MIPS V and below, 1-n otherwise.  */
//   U8 isa_rev;
//   /* The size of general purpose registers.  */
//   U8 gpr_size;
//   /* The size of co-processor 1 registers.  */
//   U8 cpr1_size;
//   /* The size of co-processor 2 registers.  */
//   U8 cpr2_size;
//   /* The floating-point ABI.  */
//   U8 fp_abi;
//   /* Processor-specific extension.  */
//   U32 isa_ext;
//   /* Mask of ASEs used.  */
//   U32 ases;
//   /* Mask of general flags.  */
//   U32 flags1;
//   U32 flags2;
// } Elf_MIPS_ABIFlags_v0;
//
// /* Values for the register size bytes of an abi flags structure.  */
//
// #define MIPS_AFL_REG_NONE        0x00         /* No registers.  */
// #define MIPS_AFL_REG_32                0x01         /* 32-bit registers.  */
// #define MIPS_AFL_REG_64                0x02         /* 64-bit registers.  */
// #define MIPS_AFL_REG_128        0x03         /* 128-bit registers.  */
//
// /* Masks for the ases word of an ABI flags structure.  */
//
// #define MIPS_AFL_ASE_DSP        0x00000001 /* DSP ASE.  */
// #define MIPS_AFL_ASE_DSPR2        0x00000002 /* DSP R2 ASE.  */
// #define MIPS_AFL_ASE_EVA        0x00000004 /* Enhanced VA Scheme.  */
// #define MIPS_AFL_ASE_MCU        0x00000008 /* MCU (MicroController) ASE.  */
// #define MIPS_AFL_ASE_MDMX        0x00000010 /* MDMX ASE.  */
// #define MIPS_AFL_ASE_MIPS3D        0x00000020 /* MIPS-3D ASE.  */
// #define MIPS_AFL_ASE_MT                0x00000040 /* MT ASE.  */
// #define MIPS_AFL_ASE_SMARTMIPS        0x00000080 /* SmartMIPS ASE.  */
// #define MIPS_AFL_ASE_VIRT        0x00000100 /* VZ ASE.  */
// #define MIPS_AFL_ASE_MSA        0x00000200 /* MSA ASE.  */
// #define MIPS_AFL_ASE_MIPS16        0x00000400 /* MIPS16 ASE.  */
// #define MIPS_AFL_ASE_MICROMIPS        0x00000800 /* MICROMIPS ASE.  */
// #define MIPS_AFL_ASE_XPA        0x00001000 /* XPA ASE.  */
// #define MIPS_AFL_ASE_MASK        0x00001fff /* All ASEs.  */
//
// /* Values for the isa_ext word of an ABI flags structure.  */
//
// #define MIPS_AFL_EXT_XLR          1   /* RMI Xlr instruction.  */
// #define MIPS_AFL_EXT_OCTEON2          2   /* Cavium Networks Octeon2.  */
// #define MIPS_AFL_EXT_OCTEONP          3   /* Cavium Networks OcteonP.  */
// #define MIPS_AFL_EXT_LOONGSON_3A  4   /* Loongson 3A.  */
// #define MIPS_AFL_EXT_OCTEON          5   /* Cavium Networks Octeon.  */
// #define MIPS_AFL_EXT_5900          6   /* MIPS R5900 instruction.  */
// #define MIPS_AFL_EXT_4650          7   /* MIPS R4650 instruction.  */
// #define MIPS_AFL_EXT_4010          8   /* LSI R4010 instruction.  */
// #define MIPS_AFL_EXT_4100          9   /* NEC VR4100 instruction.  */
// #define MIPS_AFL_EXT_3900          10  /* Toshiba R3900 instruction.  */
// #define MIPS_AFL_EXT_10000          11  /* MIPS R10000 instruction.  */
// #define MIPS_AFL_EXT_SB1          12  /* Broadcom SB-1 instruction.  */
// #define MIPS_AFL_EXT_4111          13  /* NEC VR4111/VR4181 instruction.  */
// #define MIPS_AFL_EXT_4120          14  /* NEC VR4120 instruction.  */
// #define MIPS_AFL_EXT_5400          15  /* NEC VR5400 instruction.  */
// #define MIPS_AFL_EXT_5500          16  /* NEC VR5500 instruction.  */
// #define MIPS_AFL_EXT_LOONGSON_2E  17  /* ST Microelectronics Loongson 2E.  */
// #define MIPS_AFL_EXT_LOONGSON_2F  18  /* ST Microelectronics Loongson 2F.  */
//
// /* Masks for the flags1 word of an ABI flags structure.  */
// #define MIPS_AFL_FLAGS1_ODDSPREG  1  /* Uses odd single-precision registers.  */
//
// /* Object attribute values.  */
// enum
// {
//   /* Not tagged or not using any ABIs affected by the differences.  */
//   Val_GNU_MIPS_ABI_FP_ANY = 0,
//   /* Using hard-float -mdouble-float.  */
//   Val_GNU_MIPS_ABI_FP_DOUBLE = 1,
//   /* Using hard-float -msingle-float.  */
//   Val_GNU_MIPS_ABI_FP_SINGLE = 2,
//   /* Using soft-float.  */
//   Val_GNU_MIPS_ABI_FP_SOFT = 3,
//   /* Using -mips32r2 -mfp64.  */
//   Val_GNU_MIPS_ABI_FP_OLD_64 = 4,
//   /* Using -mfpxx.  */
//   Val_GNU_MIPS_ABI_FP_XX = 5,
//   /* Using -mips32r2 -mfp64.  */
//   Val_GNU_MIPS_ABI_FP_64 = 6,
//   /* Using -mips32r2 -mfp64 -mno-odd-spreg.  */
//   Val_GNU_MIPS_ABI_FP_64A = 7,
//   /* Maximum allocated FP ABI value.  */
//   Val_GNU_MIPS_ABI_FP_MAX = 7
// };
//
// /* HPPA specific definitions.  */
//
// /* Legal values for processor_flags field of ELF32_Ehdr.  */
//
// #define EF_PARISC_TRAPNIL        0x00010000 /* Trap nil pointer dereference.  */
// #define EF_PARISC_EXT                0x00020000 /* Program uses arch. extensions. */
// #define EF_PARISC_LSB                0x00040000 /* Program expects little endian. */
// #define EF_PARISC_WIDE                0x00080000 /* Program expects wide mode.  */
// #define EF_PARISC_NO_KABP        0x00100000 /* No kernel assisted branch
//                                               prediction.  */
// #define EF_PARISC_LAZYSWAP        0x00400000 /* Allow lazy swapping.  */
// #define EF_PARISC_ARCH                0x0000ffff /* Architecture version.  */
//
// /* Defined values for `processor_flags & EF_PARISC_ARCH' are:  */
//
// #define EFA_PARISC_1_0                    0x020b /* PA-RISC 1.0 big-endian.  */
// #define EFA_PARISC_1_1                    0x0210 /* PA-RISC 1.1 big-endian.  */
// #define EFA_PARISC_2_0                    0x0214 /* PA-RISC 2.0 big-endian.  */
//
// /* Additional section indices.  */
//
// #define ELF_Section_Index_PARISC_ANSI_COMMON        0xff00           /* Section for tentatively declared
//                                               symbols in ANSI C.  */
// #define ELF_Section_Index_PARISC_HUGE_COMMON        0xff01           /* Common blocks in huge model.  */
//
// /* Legal values for type field of ELF32_Section_Header.  */
//
// #define ELF_Section_Header_Type__PARISC_EXT                0x70000000 /* Contains product specific ext. */
// #define ELF_Section_Header_Type__PARISC_UNWIND        0x70000001 /* Unwind information.  */
// #define ELF_Section_Header_Type__PARISC_DOC                0x70000002 /* Debug info for optimized code. */
//
// /* Legal values for flags field of ELF32_Section_Header.  */
//
// #define SHF_PARISC_SHORT        0x20000000 /* Section with short addressing. */
// #define SHF_PARISC_HUGE                0x40000000 /* Section far from gp.  */
// #define SHF_PARISC_SBP                0x80000000 /* Static branch prediction code. */
//
// /* Legal values for ST_TYPE subfield of type_and_binding (symbol type).  */
//
// #define STT_PARISC_MILLICODE        13        /* Millicode function entry point.  */
//
// #define STT_HP_OPAQUE                (STT_LOOS + 0x1)
// #define STT_HP_STUB                (STT_LOOS + 0x2)
//
// /* HPPA relocs.  */
//
// #define R_PARISC_NONE                0        /* No reloc.  */
// #define R_PARISC_DIR32                1        /* Direct 32-bit reference.  */
// #define R_PARISC_DIR21L                2        /* Left 21 bits of eff. address.  */
// #define R_PARISC_DIR17R                3        /* Right 17 bits of eff. address.  */
// #define R_PARISC_DIR17F                4        /* 17 bits of eff. address.  */
// #define R_PARISC_DIR14R                6        /* Right 14 bits of eff. address.  */
// #define R_PARISC_PCRelocations32        9        /* 32-bit rel. address.  */
// #define R_PARISC_PCRelocations21L        10        /* Left 21 bits of rel. address.  */
// #define R_PARISC_PCRelocations17R        11        /* Right 17 bits of rel. address.  */
// #define R_PARISC_PCRelocations17F        12        /* 17 bits of rel. address.  */
// #define R_PARISC_PCRelocations14R        14        /* Right 14 bits of rel. address.  */
// #define R_PARISC_DPRelocations21L        18        /* Left 21 bits of rel. address.  */
// #define R_PARISC_DPRelocations14R        22        /* Right 14 bits of rel. address.  */
// #define R_PARISC_GPRelocations21L        26        /* GP-relative, left 21 bits.  */
// #define R_PARISC_GPRelocations14R        30        /* GP-relative, right 14 bits.  */
// #define R_PARISC_LTOFF21L        34        /* LT-relative, left 21 bits.  */
// #define R_PARISC_LTOFF14R        38        /* LT-relative, right 14 bits.  */
// #define R_PARISC_SECRelocations32        41        /* 32 bits section rel. address.  */
// #define R_PARISC_SEGBASE        48        /* No relocation, set segment base.  */
// #define R_PARISC_SEGRelocations32        49        /* 32 bits segment rel. address.  */
// #define R_PARISC_PLTOFF21L        50        /* PLT rel. address, left 21 bits.  */
// #define R_PARISC_PLTOFF14R        54        /* PLT rel. address, right 14 bits.  */
// #define R_PARISC_LTOFF_FPTR32        57        /* 32 bits LT-rel. function pointer. */
// #define R_PARISC_LTOFF_FPTR21L        58        /* LT-rel. fct ptr, left 21 bits. */
// #define R_PARISC_LTOFF_FPTR14R        62        /* LT-rel. fct ptr, right 14 bits. */
// #define R_PARISC_FPTR64                64        /* 64 bits function address.  */
// #define R_PARISC_PLABEL32        65        /* 32 bits function address.  */
// #define R_PARISC_PLABEL21L        66        /* Left 21 bits of fdesc address.  */
// #define R_PARISC_PLABEL14R        70        /* Right 14 bits of fdesc address.  */
// #define R_PARISC_PCRelocations64        72        /* 64 bits PC-rel. address.  */
// #define R_PARISC_PCRelocations22F        74        /* 22 bits PC-rel. address.  */
// #define R_PARISC_PCRelocations14WR        75        /* PC-rel. address, right 14 bits.  */
// #define R_PARISC_PCRelocations14DR        76        /* PC rel. address, right 14 bits.  */
// #define R_PARISC_PCRelocations16F        77        /* 16 bits PC-rel. address.  */
// #define R_PARISC_PCRelocations16WF        78        /* 16 bits PC-rel. address.  */
// #define R_PARISC_PCRelocations16DF        79        /* 16 bits PC-rel. address.  */
// #define R_PARISC_DIR64                80        /* 64 bits of eff. address.  */
// #define R_PARISC_DIR14WR        83        /* 14 bits of eff. address.  */
// #define R_PARISC_DIR14DR        84        /* 14 bits of eff. address.  */
// #define R_PARISC_DIR16F                85        /* 16 bits of eff. address.  */
// #define R_PARISC_DIR16WF        86        /* 16 bits of eff. address.  */
// #define R_PARISC_DIR16DF        87        /* 16 bits of eff. address.  */
// #define R_PARISC_GPRelocations64        88        /* 64 bits of GP-rel. address.  */
// #define R_PARISC_GPRelocations14WR        91        /* GP-rel. address, right 14 bits.  */
// #define R_PARISC_GPRelocations14DR        92        /* GP-rel. address, right 14 bits.  */
// #define R_PARISC_GPRelocations16F        93        /* 16 bits GP-rel. address.  */
// #define R_PARISC_GPRelocations16WF        94        /* 16 bits GP-rel. address.  */
// #define R_PARISC_GPRelocations16DF        95        /* 16 bits GP-rel. address.  */
// #define R_PARISC_LTOFF64        96        /* 64 bits LT-rel. address.  */
// #define R_PARISC_LTOFF14WR        99        /* LT-rel. address, right 14 bits.  */
// #define R_PARISC_LTOFF14DR        100        /* LT-rel. address, right 14 bits.  */
// #define R_PARISC_LTOFF16F        101        /* 16 bits LT-rel. address.  */
// #define R_PARISC_LTOFF16WF        102        /* 16 bits LT-rel. address.  */
// #define R_PARISC_LTOFF16DF        103        /* 16 bits LT-rel. address.  */
// #define R_PARISC_SECRelocations64        104        /* 64 bits section rel. address.  */
// #define R_PARISC_SEGRelocations64        112        /* 64 bits segment rel. address.  */
// #define R_PARISC_PLTOFF14WR        115        /* PLT-rel. address, right 14 bits.  */
// #define R_PARISC_PLTOFF14DR        116        /* PLT-rel. address, right 14 bits.  */
// #define R_PARISC_PLTOFF16F        117        /* 16 bits LT-rel. address.  */
// #define R_PARISC_PLTOFF16WF        118        /* 16 bits PLT-rel. address.  */
// #define R_PARISC_PLTOFF16DF        119        /* 16 bits PLT-rel. address.  */
// #define R_PARISC_LTOFF_FPTR64        120        /* 64 bits LT-rel. function ptr.  */
// #define R_PARISC_LTOFF_FPTR14WR        123        /* LT-rel. fct. ptr., right 14 bits. */
// #define R_PARISC_LTOFF_FPTR14DR        124        /* LT-rel. fct. ptr., right 14 bits. */
// #define R_PARISC_LTOFF_FPTR16F        125        /* 16 bits LT-rel. function ptr.  */
// #define R_PARISC_LTOFF_FPTR16WF        126        /* 16 bits LT-rel. function ptr.  */
// #define R_PARISC_LTOFF_FPTR16DF        127        /* 16 bits LT-rel. function ptr.  */
// #define R_PARISC_LORESERVE        128
// #define R_PARISC_COPY                128        /* Copy relocation.  */
// #define R_PARISC_IPLT                129        /* Dynamic reloc, imported PLT */
// #define R_PARISC_EPLT                130        /* Dynamic reloc, exported PLT */
// #define R_PARISC_TPRelocations32        153        /* 32 bits TP-rel. address.  */
// #define R_PARISC_TPRelocations21L        154        /* TP-rel. address, left 21 bits.  */
// #define R_PARISC_TPRelocations14R        158        /* TP-rel. address, right 14 bits.  */
// #define R_PARISC_LTOFF_TP21L        162        /* LT-TP-rel. address, left 21 bits. */
// #define R_PARISC_LTOFF_TP14R        166        /* LT-TP-rel. address, right 14 bits.*/
// #define R_PARISC_LTOFF_TP14F        167        /* 14 bits LT-TP-rel. address.  */
// #define R_PARISC_TPRelocations64        216        /* 64 bits TP-rel. address.  */
// #define R_PARISC_TPRelocations14WR        219        /* TP-rel. address, right 14 bits.  */
// #define R_PARISC_TPRelocations14DR        220        /* TP-rel. address, right 14 bits.  */
// #define R_PARISC_TPRelocations16F        221        /* 16 bits TP-rel. address.  */
// #define R_PARISC_TPRelocations16WF        222        /* 16 bits TP-rel. address.  */
// #define R_PARISC_TPRelocations16DF        223        /* 16 bits TP-rel. address.  */
// #define R_PARISC_LTOFF_TP64        224        /* 64 bits LT-TP-rel. address.  */
// #define R_PARISC_LTOFF_TP14WR        227        /* LT-TP-rel. address, right 14 bits.*/
// #define R_PARISC_LTOFF_TP14DR        228        /* LT-TP-rel. address, right 14 bits.*/
// #define R_PARISC_LTOFF_TP16F        229        /* 16 bits LT-TP-rel. address.  */
// #define R_PARISC_LTOFF_TP16WF        230        /* 16 bits LT-TP-rel. address.  */
// #define R_PARISC_LTOFF_TP16DF        231        /* 16 bits LT-TP-rel. address.  */
// #define R_PARISC_GNU_VTENTRY        232
// #define R_PARISC_GNU_VTINHERIT        233
// #define R_PARISC_TLS_GD21L        234        /* GD 21-bit left.  */
// #define R_PARISC_TLS_GD14R        235        /* GD 14-bit right.  */
// #define R_PARISC_TLS_GDCALL        236        /* GD call to __t_g_a.  */
// #define R_PARISC_TLS_LDM21L        237        /* LD module 21-bit left.  */
// #define R_PARISC_TLS_LDM14R        238        /* LD module 14-bit right.  */
// #define R_PARISC_TLS_LDMCALL        239        /* LD module call to __t_g_a.  */
// #define R_PARISC_TLS_LDO21L        240        /* LD offset 21-bit left.  */
// #define R_PARISC_TLS_LDO14R        241        /* LD offset 14-bit right.  */
// #define R_PARISC_TLS_DTPMOD32        242        /* DTP module 32-bit.  */
// #define R_PARISC_TLS_DTPMOD64        243        /* DTP module 64-bit.  */
// #define R_PARISC_TLS_DTPOFF32        244        /* DTP offset 32-bit.  */
// #define R_PARISC_TLS_DTPOFF64        245        /* DTP offset 32-bit.  */
// #define R_PARISC_TLS_LE21L        R_PARISC_TPRelocations21L
// #define R_PARISC_TLS_LE14R        R_PARISC_TPRelocations14R
// #define R_PARISC_TLS_IE21L        R_PARISC_LTOFF_TP21L
// #define R_PARISC_TLS_IE14R        R_PARISC_LTOFF_TP14R
// #define R_PARISC_TLS_TPRelocations32        R_PARISC_TPRelocations32
// #define R_PARISC_TLS_TPRelocations64        R_PARISC_TPRelocations64
// #define R_PARISC_HIRESERVE        255
//
// /* Legal values for p_type field of ELF32_Phdr/ELF64_Phdr.  */
//
// #define PT_HP_TLS                (PT_LOOS + 0x0)
// #define PT_HP_CORE_NONE                (PT_LOOS + 0x1)
// #define PT_HP_CORE_VERSION        (PT_LOOS + 0x2)
// #define PT_HP_CORE_KERNEL        (PT_LOOS + 0x3)
// #define PT_HP_CORE_COMM                (PT_LOOS + 0x4)
// #define PT_HP_CORE_PROC                (PT_LOOS + 0x5)
// #define PT_HP_CORE_LOADABLE        (PT_LOOS + 0x6)
// #define PT_HP_CORE_STACK        (PT_LOOS + 0x7)
// #define PT_HP_CORE_SHM                (PT_LOOS + 0x8)
// #define PT_HP_CORE_MMF                (PT_LOOS + 0x9)
// #define PT_HP_PARALLEL                (PT_LOOS + 0x10)
// #define PT_HP_FAELF_Symbol_BindingIND                (PT_LOOS + 0x11)
// #define PT_HP_OPT_ANNOT                (PT_LOOS + 0x12)
// #define PT_HP_HSL_ANNOT                (PT_LOOS + 0x13)
// #define PT_HP_STACK                (PT_LOOS + 0x14)
//
// #define PT_PARISC_ARCHEXT        0x70000000
// #define PT_PARISC_UNWIND        0x70000001
//
// /* Legal values for p_flags field of ELF32_Phdr/ELF64_Phdr.  */
//
// #define PF_PARISC_SBP                0x08000000
//
// #define PF_HP_PAGE_SIZE                0x00100000
// #define PF_HP_FAR_SHARED        0x00200000
// #define PF_HP_NEAR_SHARED        0x00400000
// #define PF_HP_CODE                0x01000000
// #define PF_HP_MODIFY                0x02000000
// #define PF_HP_LAZYSWAP                0x04000000
// #define PF_HP_SBP                0x08000000
//
//
// /* Alpha specific definitions.  */
//
// /* Legal values for processor_flags field of ELF64_Section_Header.  */
//
// #define EF_ALPHA_32BIT                1        /* All addresses must be < 2GB.  */
// #define EF_ALPHA_CANRelocations_AddendsX        2        /* Relocations for relaxing exist.  */
//
// /* Legal values for type field of ELF64_Shdr.  */
//
// /* These two are primerily concerned with ECOFF debugging info.  */
// #define ELF_Section_Header_Type__ALPHA_DEBUG                0x70000001
// #define ELF_Section_Header_Type__ALPHA_REGINFO        0x70000002
//
// /* Legal values for flags field of ELF64_Shdr.  */
//
// #define SHF_ALPHA_GPRelocations                0x10000000
//
// /* Legal values for visibility field of ELF64_Sym.  */
// #define STO_ALPHA_NOPV                0x80        /* No PV required.  */
// #define STO_ALPHA_STD_GPLOAD        0x88        /* PV only used for initial ldgp.  */
//
// /* Alpha relocs.  */
//
// #define R_ALPHA_NONE                0        /* No reloc */
// #define R_ALPHA_REFLONG                1        /* Direct 32 bit */
// #define R_ALPHA_REFQUAD                2        /* Direct 64 bit */
// #define R_ALPHA_GPRelocations32                3        /* GP relative 32 bit */
// #define R_ALPHA_LITERAL                4        /* GP relative 16 bit w/optimization */
// #define R_ALPHA_LITUSE                5        /* Optimization hint for LITERAL */
// #define R_ALPHA_GPDISP                6        /* Add displacement to GP */
// #define R_ALPHA_BRADDR                7        /* PC+4 relative 23 bit shifted */
// #define R_ALPHA_HINT                8        /* PC+4 relative 16 bit shifted */
// #define R_ALPHA_SRelocations16                9        /* PC relative 16 bit */
// #define R_ALPHA_SRelocations32                10        /* PC relative 32 bit */
// #define R_ALPHA_SRelocations64                11        /* PC relative 64 bit */
// #define R_ALPHA_GPRelocationsHIGH        17        /* GP relative 32 bit, high 16 bits */
// #define R_ALPHA_GPRelocationsLOW        18        /* GP relative 32 bit, low 16 bits */
// #define R_ALPHA_GPRelocations16                19        /* GP relative 16 bit */
// #define R_ALPHA_COPY                24        /* Copy symbol at runtime */
// #define R_ALPHA_GLOB_DAT        25        /* Create GOT entry */
// #define R_ALPHA_JMP_SLOT        26        /* Create PLT entry */
// #define R_ALPHA_Relocations_AddendsTIVE        27        /* Adjust by program base */
// #define R_ALPHA_TLS_GD_HI        28
// #define R_ALPHA_TLSGD                29
// #define R_ALPHA_TLS_LDM                30
// #define R_ALPHA_DTPMOD64        31
// #define R_ALPHA_GOTDTPRelocations        32
// #define R_ALPHA_DTPRelocations64        33
// #define R_ALPHA_DTPRelocationsHI        34
// #define R_ALPHA_DTPRelocationsLO        35
// #define R_ALPHA_DTPRelocations16        36
// #define R_ALPHA_GOTTPRelocations        37
// #define R_ALPHA_TPRelocations64                38
// #define R_ALPHA_TPRelocationsHI                39
// #define R_ALPHA_TPRelocationsLO                40
// #define R_ALPHA_TPRelocations16                41
// /* Keep this the last entry.  */
// #define R_ALPHA_NUM                46
//
// /* Magic values of the LITUSE relocation addend.  */
// #define LITUSE_ALPHA_ADDR        0
// #define LITUSE_ALPHA_BASE        1
// #define LITUSE_ALPHA_BYTOFF        2
// #define LITUSE_ALPHA_JSR        3
// #define LITUSE_ALPHA_TLS_GD        4
// #define LITUSE_ALPHA_TLS_LDM        5
//
// /* Legal values for d_tag of ELF64_Dyn.  */
// #define DT_ALPHA_PLTRO                (DT_LOPROC + 0)
// #define DT_ALPHA_NUM                1
//
// /* PowerPC specific declarations */
//
// /* Values for ELF32/64_Ehdr.processor_flags.  */
// #define EF_PPC_EMB                0x80000000        /* PowerPC embedded flag */
//
// /* Cygnus local bits below */
// #define EF_PPC_RelocationsOCATABLE        0x00010000        /* PowerPC -mrelocatable flag*/
// #define EF_PPC_RelocationsOCATABLE_LIB        0x00008000        /* PowerPC -mrelocatable-lib
//                                                    flag */
//
// /* PowerPC relocations defined by the ABIs */
// #define R_PPC_NONE                0
// #define R_PPC_ADDR32                1        /* 32bit absolute address */
// #define R_PPC_ADDR24                2        /* 26bit address, 2 bits ignored.  */
// #define R_PPC_ADDR16                3        /* 16bit absolute address */
// #define R_PPC_ADDR16_LO                4        /* lower 16bit of absolute address */
// #define R_PPC_ADDR16_HI                5        /* high 16bit of absolute address */
// #define R_PPC_ADDR16_HA                6        /* adjusted high 16bit */
// #define R_PPC_ADDR14                7        /* 16bit address, 2 bits ignored */
// #define R_PPC_ADDR14_BRTAKEN        8
// #define R_PPC_ADDR14_BRNTAKEN        9
// #define R_PPC_Relocations24                10        /* PC relative 26 bit */
// #define R_PPC_Relocations14                11        /* PC relative 16 bit */
// #define R_PPC_Relocations14_BRTAKEN        12
// #define R_PPC_Relocations14_BRNTAKEN        13
// #define R_PPC_GOT16                14
// #define R_PPC_GOT16_LO                15
// #define R_PPC_GOT16_HI                16
// #define R_PPC_GOT16_HA                17
// #define R_PPC_PLTRelocations24                18
// #define R_PPC_COPY                19
// #define R_PPC_GLOB_DAT                20
// #define R_PPC_JMP_SLOT                21
// #define R_PPC_Relocations_AddendsTIVE                22
// #define R_PPC_LOCAL24PC                23
// #define R_PPC_UADDR32                24
// #define R_PPC_UADDR16                25
// #define R_PPC_Relocations32                26
// #define R_PPC_PLT32                27
// #define R_PPC_PLTRelocations32                28
// #define R_PPC_PLT16_LO                29
// #define R_PPC_PLT16_HI                30
// #define R_PPC_PLT16_HA                31
// #define R_PPC_SDARelocations16                32
// #define R_PPC_SECTOFF                33
// #define R_PPC_SECTOFF_LO        34
// #define R_PPC_SECTOFF_HI        35
// #define R_PPC_SECTOFF_HA        36
//
// /* PowerPC relocations defined for the TLS access ABI.  */
// #define R_PPC_TLS                67 /* none        (sym+add)@tls */
// #define R_PPC_DTPMOD32                68 /* word32        (sym+add)@dtpmod */
// #define R_PPC_TPRelocations16                69 /* half16*        (sym+add)@tprel */
// #define R_PPC_TPRelocations16_LO        70 /* half16        (sym+add)@tprel@l */
// #define R_PPC_TPRelocations16_HI        71 /* half16        (sym+add)@tprel@h */
// #define R_PPC_TPRelocations16_HA        72 /* half16        (sym+add)@tprel@ha */
// #define R_PPC_TPRelocations32                73 /* word32        (sym+add)@tprel */
// #define R_PPC_DTPRelocations16                74 /* half16*        (sym+add)@dtprel */
// #define R_PPC_DTPRelocations16_LO        75 /* half16        (sym+add)@dtprel@l */
// #define R_PPC_DTPRelocations16_HI        76 /* half16        (sym+add)@dtprel@h */
// #define R_PPC_DTPRelocations16_HA        77 /* half16        (sym+add)@dtprel@ha */
// #define R_PPC_DTPRelocations32                78 /* word32        (sym+add)@dtprel */
// #define R_PPC_GOT_TLSGD16        79 /* half16*        (sym+add)@got@tlsgd */
// #define R_PPC_GOT_TLSGD16_LO        80 /* half16        (sym+add)@got@tlsgd@l */
// #define R_PPC_GOT_TLSGD16_HI        81 /* half16        (sym+add)@got@tlsgd@h */
// #define R_PPC_GOT_TLSGD16_HA        82 /* half16        (sym+add)@got@tlsgd@ha */
// #define R_PPC_GOT_TLSLD16        83 /* half16*        (sym+add)@got@tlsld */
// #define R_PPC_GOT_TLSLD16_LO        84 /* half16        (sym+add)@got@tlsld@l */
// #define R_PPC_GOT_TLSLD16_HI        85 /* half16        (sym+add)@got@tlsld@h */
// #define R_PPC_GOT_TLSLD16_HA        86 /* half16        (sym+add)@got@tlsld@ha */
// #define R_PPC_GOT_TPRelocations16        87 /* half16*        (sym+add)@got@tprel */
// #define R_PPC_GOT_TPRelocations16_LO        88 /* half16        (sym+add)@got@tprel@l */
// #define R_PPC_GOT_TPRelocations16_HI        89 /* half16        (sym+add)@got@tprel@h */
// #define R_PPC_GOT_TPRelocations16_HA        90 /* half16        (sym+add)@got@tprel@ha */
// #define R_PPC_GOT_DTPRelocations16        91 /* half16*        (sym+add)@got@dtprel */
// #define R_PPC_GOT_DTPRelocations16_LO        92 /* half16*        (sym+add)@got@dtprel@l */
// #define R_PPC_GOT_DTPRelocations16_HI        93 /* half16*        (sym+add)@got@dtprel@h */
// #define R_PPC_GOT_DTPRelocations16_HA        94 /* half16*        (sym+add)@got@dtprel@ha */
// #define R_PPC_TLSGD                95 /* none        (sym+add)@tlsgd */
// #define R_PPC_TLSLD                96 /* none        (sym+add)@tlsld */
//
// /* The remaining relocs are from the Embedded ELF ABI, and are not
//    in the SVR4 ELF ABI.  */
// #define R_PPC_EMB_NADDR32        101
// #define R_PPC_EMB_NADDR16        102
// #define R_PPC_EMB_NADDR16_LO        103
// #define R_PPC_EMB_NADDR16_HI        104
// #define R_PPC_EMB_NADDR16_HA        105
// #define R_PPC_EMB_SDAI16        106
// #define R_PPC_EMB_SDA2I16        107
// #define R_PPC_EMB_SDA2Relocations        108
// #define R_PPC_EMB_SDA21                109        /* 16 bit offset in SDA */
// #define R_PPC_EMB_MRKREF        110
// #define R_PPC_EMB_RelocationsSEC16        111
// #define R_PPC_EMB_RelocationsST_LO        112
// #define R_PPC_EMB_RelocationsST_HI        113
// #define R_PPC_EMB_RelocationsST_HA        114
// #define R_PPC_EMB_BIT_FLD        115
// #define R_PPC_EMB_RelocationsSDA        116        /* 16 bit relative offset in SDA */
//
// /* Diab tool relocations.  */
// #define R_PPC_DIAB_SDA21_LO        180        /* like EMB_SDA21, but lower 16 bit */
// #define R_PPC_DIAB_SDA21_HI        181        /* like EMB_SDA21, but high 16 bit */
// #define R_PPC_DIAB_SDA21_HA        182        /* like EMB_SDA21, adjusted high 16 */
// #define R_PPC_DIAB_RelocationsSDA_LO        183        /* like EMB_RelocationsSDA, but lower 16 bit */
// #define R_PPC_DIAB_RelocationsSDA_HI        184        /* like EMB_RelocationsSDA, but high 16 bit */
// #define R_PPC_DIAB_RelocationsSDA_HA        185        /* like EMB_RelocationsSDA, adjusted high 16 */
//
// /* GNU extension to support local ifunc.  */
// #define R_PPC_IRelocations_AddendsTIVE                248
//
// /* GNU relocs used in PIC code sequences.  */
// #define R_PPC_Relocations16                249        /* half16   (sym+add-.) */
// #define R_PPC_Relocations16_LO                250        /* half16   (sym+add-.)@l */
// #define R_PPC_Relocations16_HI                251        /* half16   (sym+add-.)@h */
// #define R_PPC_Relocations16_HA                252        /* half16   (sym+add-.)@ha */
//
// /* This is a phony reloc to handle any old fashioned TOC16 references
//    that may still be in object files.  */
// #define R_PPC_TOC16                255
//
// /* PowerPC specific values for the Dyn d_tag field.  */
// #define DT_PPC_GOT                (DT_LOPROC + 0)
// #define DT_PPC_OPT                (DT_LOPROC + 1)
// #define DT_PPC_NUM                2
//
// /* PowerPC specific values for the DT_PPC_OPT Dyn entry.  */
// #define PPC_OPT_TLS                1
//
// /* PowerPC64 relocations defined by the ABIs */
// #define R_PPC64_NONE                R_PPC_NONE
// #define R_PPC64_ADDR32                R_PPC_ADDR32 /* 32bit absolute address */
// #define R_PPC64_ADDR24                R_PPC_ADDR24 /* 26bit address, word aligned */
// #define R_PPC64_ADDR16                R_PPC_ADDR16 /* 16bit absolute address */
// #define R_PPC64_ADDR16_LO        R_PPC_ADDR16_LO        /* lower 16bits of address */
// #define R_PPC64_ADDR16_HI        R_PPC_ADDR16_HI        /* high 16bits of address. */
// #define R_PPC64_ADDR16_HA        R_PPC_ADDR16_HA /* adjusted high 16bits.  */
// #define R_PPC64_ADDR14                R_PPC_ADDR14 /* 16bit address, word aligned */
// #define R_PPC64_ADDR14_BRTAKEN        R_PPC_ADDR14_BRTAKEN
// #define R_PPC64_ADDR14_BRNTAKEN        R_PPC_ADDR14_BRNTAKEN
// #define R_PPC64_Relocations24                R_PPC_Relocations24 /* PC-rel. 26 bit, word aligned */
// #define R_PPC64_Relocations14                R_PPC_Relocations14 /* PC relative 16 bit */
// #define R_PPC64_Relocations14_BRTAKEN        R_PPC_Relocations14_BRTAKEN
// #define R_PPC64_Relocations14_BRNTAKEN        R_PPC_Relocations14_BRNTAKEN
// #define R_PPC64_GOT16                R_PPC_GOT16
// #define R_PPC64_GOT16_LO        R_PPC_GOT16_LO
// #define R_PPC64_GOT16_HI        R_PPC_GOT16_HI
// #define R_PPC64_GOT16_HA        R_PPC_GOT16_HA
//
// #define R_PPC64_COPY                R_PPC_COPY
// #define R_PPC64_GLOB_DAT        R_PPC_GLOB_DAT
// #define R_PPC64_JMP_SLOT        R_PPC_JMP_SLOT
// #define R_PPC64_Relocations_AddendsTIVE        R_PPC_Relocations_AddendsTIVE
//
// #define R_PPC64_UADDR32                R_PPC_UADDR32
// #define R_PPC64_UADDR16                R_PPC_UADDR16
// #define R_PPC64_Relocations32                R_PPC_Relocations32
// #define R_PPC64_PLT32                R_PPC_PLT32
// #define R_PPC64_PLTRelocations32        R_PPC_PLTRelocations32
// #define R_PPC64_PLT16_LO        R_PPC_PLT16_LO
// #define R_PPC64_PLT16_HI        R_PPC_PLT16_HI
// #define R_PPC64_PLT16_HA        R_PPC_PLT16_HA
//
// #define R_PPC64_SECTOFF                R_PPC_SECTOFF
// #define R_PPC64_SECTOFF_LO        R_PPC_SECTOFF_LO
// #define R_PPC64_SECTOFF_HI        R_PPC_SECTOFF_HI
// #define R_PPC64_SECTOFF_HA        R_PPC_SECTOFF_HA
// #define R_PPC64_ADDR30                37 /* word30 (S + A - P) >> 2 */
// #define R_PPC64_ADDR64                38 /* doubleword64 S + A */
// #define R_PPC64_ADDR16_HIGHER        39 /* half16 #higher(S + A) */
// #define R_PPC64_ADDR16_HIGHERA        40 /* half16 #highera(S + A) */
// #define R_PPC64_ADDR16_HIGHEST        41 /* half16 #highest(S + A) */
// #define R_PPC64_ADDR16_HIGHESTA        42 /* half16 #highesta(S + A) */
// #define R_PPC64_UADDR64                43 /* doubleword64 S + A */
// #define R_PPC64_Relocations64                44 /* doubleword64 S + A - P */
// #define R_PPC64_PLT64                45 /* doubleword64 L + A */
// #define R_PPC64_PLTRelocations64        46 /* doubleword64 L + A - P */
// #define R_PPC64_TOC16                47 /* half16* S + A - .TOC */
// #define R_PPC64_TOC16_LO        48 /* half16 #lo(S + A - .TOC.) */
// #define R_PPC64_TOC16_HI        49 /* half16 #hi(S + A - .TOC.) */
// #define R_PPC64_TOC16_HA        50 /* half16 #ha(S + A - .TOC.) */
// #define R_PPC64_TOC                51 /* doubleword64 .TOC */
// #define R_PPC64_PLTGOT16        52 /* half16* M + A */
// #define R_PPC64_PLTGOT16_LO        53 /* half16 #lo(M + A) */
// #define R_PPC64_PLTGOT16_HI        54 /* half16 #hi(M + A) */
// #define R_PPC64_PLTGOT16_HA        55 /* half16 #ha(M + A) */
//
// #define R_PPC64_ADDR16_DS        56 /* half16ds* (S + A) >> 2 */
// #define R_PPC64_ADDR16_LO_DS        57 /* half16ds  #lo(S + A) >> 2 */
// #define R_PPC64_GOT16_DS        58 /* half16ds* (G + A) >> 2 */
// #define R_PPC64_GOT16_LO_DS        59 /* half16ds  #lo(G + A) >> 2 */
// #define R_PPC64_PLT16_LO_DS        60 /* half16ds  #lo(L + A) >> 2 */
// #define R_PPC64_SECTOFF_DS        61 /* half16ds* (R + A) >> 2 */
// #define R_PPC64_SECTOFF_LO_DS        62 /* half16ds  #lo(R + A) >> 2 */
// #define R_PPC64_TOC16_DS        63 /* half16ds* (S + A - .TOC.) >> 2 */
// #define R_PPC64_TOC16_LO_DS        64 /* half16ds  #lo(S + A - .TOC.) >> 2 */
// #define R_PPC64_PLTGOT16_DS        65 /* half16ds* (M + A) >> 2 */
// #define R_PPC64_PLTGOT16_LO_DS        66 /* half16ds  #lo(M + A) >> 2 */
//
// /* PowerPC64 relocations defined for the TLS access ABI.  */
// #define R_PPC64_TLS                67 /* none        (sym+add)@tls */
// #define R_PPC64_DTPMOD64        68 /* doubleword64 (sym+add)@dtpmod */
// #define R_PPC64_TPRelocations16                69 /* half16*        (sym+add)@tprel */
// #define R_PPC64_TPRelocations16_LO        70 /* half16        (sym+add)@tprel@l */
// #define R_PPC64_TPRelocations16_HI        71 /* half16        (sym+add)@tprel@h */
// #define R_PPC64_TPRelocations16_HA        72 /* half16        (sym+add)@tprel@ha */
// #define R_PPC64_TPRelocations64                73 /* doubleword64 (sym+add)@tprel */
// #define R_PPC64_DTPRelocations16        74 /* half16*        (sym+add)@dtprel */
// #define R_PPC64_DTPRelocations16_LO        75 /* half16        (sym+add)@dtprel@l */
// #define R_PPC64_DTPRelocations16_HI        76 /* half16        (sym+add)@dtprel@h */
// #define R_PPC64_DTPRelocations16_HA        77 /* half16        (sym+add)@dtprel@ha */
// #define R_PPC64_DTPRelocations64        78 /* doubleword64 (sym+add)@dtprel */
// #define R_PPC64_GOT_TLSGD16        79 /* half16*        (sym+add)@got@tlsgd */
// #define R_PPC64_GOT_TLSGD16_LO        80 /* half16        (sym+add)@got@tlsgd@l */
// #define R_PPC64_GOT_TLSGD16_HI        81 /* half16        (sym+add)@got@tlsgd@h */
// #define R_PPC64_GOT_TLSGD16_HA        82 /* half16        (sym+add)@got@tlsgd@ha */
// #define R_PPC64_GOT_TLSLD16        83 /* half16*        (sym+add)@got@tlsld */
// #define R_PPC64_GOT_TLSLD16_LO        84 /* half16        (sym+add)@got@tlsld@l */
// #define R_PPC64_GOT_TLSLD16_HI        85 /* half16        (sym+add)@got@tlsld@h */
// #define R_PPC64_GOT_TLSLD16_HA        86 /* half16        (sym+add)@got@tlsld@ha */
// #define R_PPC64_GOT_TPRelocations16_DS        87 /* half16ds*        (sym+add)@got@tprel */
// #define R_PPC64_GOT_TPRelocations16_LO_DS 88 /* half16ds (sym+add)@got@tprel@l */
// #define R_PPC64_GOT_TPRelocations16_HI        89 /* half16        (sym+add)@got@tprel@h */
// #define R_PPC64_GOT_TPRelocations16_HA        90 /* half16        (sym+add)@got@tprel@ha */
// #define R_PPC64_GOT_DTPRelocations16_DS        91 /* half16ds*        (sym+add)@got@dtprel */
// #define R_PPC64_GOT_DTPRelocations16_LO_DS 92 /* half16ds (sym+add)@got@dtprel@l */
// #define R_PPC64_GOT_DTPRelocations16_HI        93 /* half16        (sym+add)@got@dtprel@h */
// #define R_PPC64_GOT_DTPRelocations16_HA        94 /* half16        (sym+add)@got@dtprel@ha */
// #define R_PPC64_TPRelocations16_DS        95 /* half16ds*        (sym+add)@tprel */
// #define R_PPC64_TPRelocations16_LO_DS        96 /* half16ds        (sym+add)@tprel@l */
// #define R_PPC64_TPRelocations16_HIGHER        97 /* half16        (sym+add)@tprel@higher */
// #define R_PPC64_TPRelocations16_HIGHERA        98 /* half16        (sym+add)@tprel@highera */
// #define R_PPC64_TPRelocations16_HIGHEST        99 /* half16        (sym+add)@tprel@highest */
// #define R_PPC64_TPRelocations16_HIGHESTA 100 /* half16        (sym+add)@tprel@highesta */
// #define R_PPC64_DTPRelocations16_DS        101 /* half16ds* (sym+add)@dtprel */
// #define R_PPC64_DTPRelocations16_LO_DS        102 /* half16ds        (sym+add)@dtprel@l */
// #define R_PPC64_DTPRelocations16_HIGHER        103 /* half16        (sym+add)@dtprel@higher */
// #define R_PPC64_DTPRelocations16_HIGHERA 104 /* half16        (sym+add)@dtprel@highera */
// #define R_PPC64_DTPRelocations16_HIGHEST 105 /* half16        (sym+add)@dtprel@highest */
// #define R_PPC64_DTPRelocations16_HIGHESTA 106 /* half16        (sym+add)@dtprel@highesta */
// #define R_PPC64_TLSGD                107 /* none        (sym+add)@tlsgd */
// #define R_PPC64_TLSLD                108 /* none        (sym+add)@tlsld */
// #define R_PPC64_TOCSAVE                109 /* none */
//
// /* Added when HA and HI relocs were changed to report overflows.  */
// #define R_PPC64_ADDR16_HIGH        110
// #define R_PPC64_ADDR16_HIGHA        111
// #define R_PPC64_TPRelocations16_HIGH        112
// #define R_PPC64_TPRelocations16_HIGHA        113
// #define R_PPC64_DTPRelocations16_HIGH        114
// #define R_PPC64_DTPRelocations16_HIGHA        115
//
// /* GNU extension to support local ifunc.  */
// #define R_PPC64_JMP_IRelocations        247
// #define R_PPC64_IRelocations_AddendsTIVE        248
// #define R_PPC64_Relocations16                249        /* half16   (sym+add-.) */
// #define R_PPC64_Relocations16_LO        250        /* half16   (sym+add-.)@l */
// #define R_PPC64_Relocations16_HI        251        /* half16   (sym+add-.)@h */
// #define R_PPC64_Relocations16_HA        252        /* half16   (sym+add-.)@ha */
//
// /* processor_flags bits specifying ABI.
//    1 for original function descriptor using ABI,
//    2 for revised ABI without function descriptors,
//    0 for unspecified or not using any features affected by the differences.  */
// #define EF_PPC64_ABI        3
//
// /* PowerPC64 specific values for the Dyn d_tag field.  */
// #define DT_PPC64_GLINK  (DT_LOPROC + 0)
// #define DT_PPC64_OPD        (DT_LOPROC + 1)
// #define DT_PPC64_OPDSZ        (DT_LOPROC + 2)
// #define DT_PPC64_OPT        (DT_LOPROC + 3)
// #define DT_PPC64_NUM    4
//
// /* PowerPC64 specific bits in the DT_PPC64_OPT Dyn entry.  */
// #define PPC64_OPT_TLS                1
// #define PPC64_OPT_MULTI_TOC        2
// #define PPC64_OPT_LOCALENTRY        4
//
// /* PowerPC64 specific values for the ELF64_Sym visibility field.  */
// #define STO_PPC64_LOCAL_BIT        5
// #define STO_PPC64_LOCAL_MASK        (7 << STO_PPC64_LOCAL_BIT)
// #define PPC64_LOCAL_ENTRY_OFFSET(other)                                \
//  (((1 << (((other) & STO_PPC64_LOCAL_MASK) >> STO_PPC64_LOCAL_BIT)) >> 2) << 2)
//
//
// /* ARM specific declarations */
//
// /* Processor specific flags for the ELF header processor_flags field.  */
// #define EF_ARM_RelocationsEXEC                0x01
// #define EF_ARM_HASENTRY                0x02
// #define EF_ARM_INTERWORK        0x04
// #define EF_ARM_APCS_26                0x08
// #define EF_ARM_APCS_FLOAT        0x10
// #define EF_ARM_PIC                0x20
// #define EF_ARM_ALIGN8                0x40 /* 8-bit structure alignment is in use */
// #define EF_ARM_NEW_ABI                0x80
// #define EF_ARM_OLD_ABI                0x100
// #define EF_ARM_SOFT_FLOAT        0x200
// #define EF_ARM_VFP_FLOAT        0x400
// #define EF_ARM_MAVERICK_FLOAT        0x800
//
// #define EF_ARM_ABI_FLOAT_SOFT        0x200   /* NB conflicts with EF_ARM_SOFT_FLOAT */
// #define EF_ARM_ABI_FLOAT_HARD        0x400   /* NB conflicts with EF_ARM_VFP_FLOAT */
//
//
// /* Other constants defined in the ARM ELF spec. version B-01.  */
// /* NB. These conflict with values defined above.  */
// #define EF_ARM_SYMSARESORTED        0x04
// #define EF_ARM_DYNSYMSUSESEGIDX        0x08
// #define EF_ARM_MAPSYMSFIRST        0x10
// #define EF_ARM_EABIMASK                0XFF000000
//
// /* Constants defined in AAELF.  */
// #define EF_ARM_BE8            0x00800000
// #define EF_ARM_LE8            0x00400000
//
// #define EF_ARM_EABI_VERSION(flags)        ((flags) & EF_ARM_EABIMASK)
// #define EF_ARM_EABI_UNKNOWN        0x00000000
// #define EF_ARM_EABI_VER1        0x01000000
// #define EF_ARM_EABI_VER2        0x02000000
// #define EF_ARM_EABI_VER3        0x03000000
// #define EF_ARM_EABI_VER4        0x04000000
// #define EF_ARM_EABI_VER5        0x05000000
//
// /* Additional symbol types for Thumb.  */
// #define STT_ARM_TFUNC                STT_LOPROC /* A Thumb function.  */
// #define STT_ARM_16BIT                STT_HIPROC /* A Thumb label.  */
//
// /* ARM-specific values for flags */
// #define SHF_ARM_ENTRYSECT        0x10000000 /* Section contains an entry point */
// #define SHF_ARM_COMDEF                0x80000000 /* Section may be multiply defined
//                                               in the input to a link step.  */
//
// /* ARM-specific program header flags */
// #define PF_ARM_SB                0x10000000 /* Segment contains the location
//                                               addressed by the static base. */
// #define PF_ARM_PI                0x20000000 /* Position-independent segment.  */
// #define PF_ARM_ABS                0x40000000 /* Absolute segment.  */
//
// /* Processor specific values for the Phdr p_type field.  */
// #define PT_ARM_EXIDX                (PT_LOPROC + 1)        /* ARM unwind segment.  */
//
// /* Processor specific values for the Shdr type field.  */
// #define ELF_Section_Header_Type__ARM_EXIDX                (ELF_Section_Header_Type__LOPROC + 1) /* ARM unwind section.  */
// #define ELF_Section_Header_Type__ARM_PREEMPTMAP        (ELF_Section_Header_Type__LOPROC + 2) /* Preemption details.  */
// #define ELF_Section_Header_Type__ARM_ATTRIBUTES        (ELF_Section_Header_Type__LOPROC + 3) /* ARM attributes section.  */
//
//
// /* AArch64 relocs.  */
//
// #define R_AARCH64_NONE            0        /* No relocation.  */
//
// #define R_AARCH64_ABS64         257        /* Direct 64 bit. */
// #define R_AARCH64_ABS32         258        /* Direct 32 bit.  */
// #define R_AARCH64_ABS16                259        /* Direct 16-bit.  */
// #define R_AARCH64_PRelocations64        260        /* PC-relative 64-bit.        */
// #define R_AARCH64_PRelocations32        261        /* PC-relative 32-bit.        */
// #define R_AARCH64_PRelocations16        262        /* PC-relative 16-bit.        */
// #define R_AARCH64_MOVW_UABS_G0        263        /* Dir. MOVZ imm. from bits 15:0.  */
// #define R_AARCH64_MOVW_UABS_G0_NC 264        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_UABS_G1        265        /* Dir. MOVZ imm. from bits 31:16.  */
// #define R_AARCH64_MOVW_UABS_G1_NC 266        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_UABS_G2        267        /* Dir. MOVZ imm. from bits 47:32.  */
// #define R_AARCH64_MOVW_UABS_G2_NC 268        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_UABS_G3        269        /* Dir. MOV{K,Z} imm. from 63:48.  */
// #define R_AARCH64_MOVW_SABS_G0        270        /* Dir. MOV{N,Z} imm. from 15:0.  */
// #define R_AARCH64_MOVW_SABS_G1        271        /* Dir. MOV{N,Z} imm. from 31:16.  */
// #define R_AARCH64_MOVW_SABS_G2        272        /* Dir. MOV{N,Z} imm. from 47:32.  */
// #define R_AARCH64_LD_PRelocations_LO19        273        /* PC-rel. LD imm. from bits 20:2.  */
// #define R_AARCH64_ADR_PRelocations_LO21        274        /* PC-rel. ADR imm. from bits 20:0.  */
// #define R_AARCH64_ADR_PRelocations_PG_HI21 275        /* Page-rel. ADRP imm. from 32:12.  */
// #define R_AARCH64_ADR_PRelocations_PG_HI21_NC 276 /* Likewise; no overflow check.  */
// #define R_AARCH64_ADD_ABS_LO12_NC 277        /* Dir. ADD imm. from bits 11:0.  */
// #define R_AARCH64_LDST8_ABS_LO12_NC 278        /* Likewise for LD/ST; no check. */
// #define R_AARCH64_TELF_Symbol_BindingR14        279        /* PC-rel. TBZ/TBNZ imm. from 15:2.  */
// #define R_AARCH64_CONDBR19        280        /* PC-rel. cond. br. imm. from 20:2. */
// #define R_AARCH64_JUMP26        282        /* PC-rel. B imm. from bits 27:2.  */
// #define R_AARCH64_CALL26        283        /* Likewise for CALL.  */
// #define R_AARCH64_LDST16_ABS_LO12_NC 284 /* Dir. ADD imm. from bits 11:1.  */
// #define R_AARCH64_LDST32_ABS_LO12_NC 285 /* Likewise for bits 11:2.  */
// #define R_AARCH64_LDST64_ABS_LO12_NC 286 /* Likewise for bits 11:3.  */
// #define R_AARCH64_MOVW_PRelocations_G0        287        /* PC-rel. MOV{N,Z} imm. from 15:0.  */
// #define R_AARCH64_MOVW_PRelocations_G0_NC 288        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_PRelocations_G1        289        /* PC-rel. MOV{N,Z} imm. from 31:16. */
// #define R_AARCH64_MOVW_PRelocations_G1_NC 290        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_PRelocations_G2        291        /* PC-rel. MOV{N,Z} imm. from 47:32. */
// #define R_AARCH64_MOVW_PRelocations_G2_NC 292        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_PRelocations_G3        293        /* PC-rel. MOV{N,Z} imm. from 63:48. */
// #define R_AARCH64_LDST128_ABS_LO12_NC 299 /* Dir. ADD imm. from bits 11:4.  */
// #define R_AARCH64_MOVW_GOTOFF_G0 300        /* GOT-rel. off. MOV{N,Z} imm. 15:0. */
// #define R_AARCH64_MOVW_GOTOFF_G0_NC 301        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_GOTOFF_G1 302        /* GOT-rel. o. MOV{N,Z} imm. 31:16.  */
// #define R_AARCH64_MOVW_GOTOFF_G1_NC 303        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_GOTOFF_G2 304        /* GOT-rel. o. MOV{N,Z} imm. 47:32.  */
// #define R_AARCH64_MOVW_GOTOFF_G2_NC 305        /* Likewise for MOVK; no check.  */
// #define R_AARCH64_MOVW_GOTOFF_G3 306        /* GOT-rel. o. MOV{N,Z} imm. 63:48.  */
// #define R_AARCH64_GOTRelocations64        307        /* GOT-relative 64-bit.  */
// #define R_AARCH64_GOTRelocations32        308        /* GOT-relative 32-bit.  */
// #define R_AARCH64_GOT_LD_PRelocations19        309        /* PC-rel. GOT off. load imm. 20:2.  */
// #define R_AARCH64_LD64_GOTOFF_LO15 310        /* GOT-rel. off. LD/ST imm. 14:3.  */
// #define R_AARCH64_ADR_GOT_PAGE        311        /* P-page-rel. GOT off. ADRP 32:12.  */
// #define R_AARCH64_LD64_GOT_LO12_NC 312        /* Dir. GOT off. LD/ST imm. 11:3.  */
// #define R_AARCH64_LD64_GOTPAGE_LO15 313        /* GOT-page-rel. GOT off. LD/ST 14:3 */
// #define R_AARCH64_TLSGD_ADR_PRelocations21 512        /* PC-relative ADR imm. 20:0.  */
// #define R_AARCH64_TLSGD_ADR_PAGE21 513        /* page-rel. ADRP imm. 32:12.  */
// #define R_AARCH64_TLSGD_ADD_LO12_NC 514        /* direct ADD imm. from 11:0.  */
// #define R_AARCH64_TLSGD_MOVW_G1        515        /* GOT-rel. MOV{N,Z} 31:16.  */
// #define R_AARCH64_TLSGD_MOVW_G0_NC 516        /* GOT-rel. MOVK imm. 15:0.  */
// #define R_AARCH64_TLSLD_ADR_PRelocations21 517        /* Like 512; local dynamic model.  */
// #define R_AARCH64_TLSLD_ADR_PAGE21 518        /* Like 513; local dynamic model.  */
// #define R_AARCH64_TLSLD_ADD_LO12_NC 519        /* Like 514; local dynamic model.  */
// #define R_AARCH64_TLSLD_MOVW_G1        520        /* Like 515; local dynamic model.  */
// #define R_AARCH64_TLSLD_MOVW_G0_NC 521        /* Like 516; local dynamic model.  */
// #define R_AARCH64_TLSLD_LD_PRelocations19 522        /* TLS PC-rel. load imm. 20:2.  */
// #define R_AARCH64_TLSLD_MOVW_DTPRelocations_G2 523 /* TLS DTP-rel. MOV{N,Z} 47:32.  */
// #define R_AARCH64_TLSLD_MOVW_DTPRelocations_G1 524 /* TLS DTP-rel. MOV{N,Z} 31:16.  */
// #define R_AARCH64_TLSLD_MOVW_DTPRelocations_G1_NC 525 /* Likewise; MOVK; no check.  */
// #define R_AARCH64_TLSLD_MOVW_DTPRelocations_G0 526 /* TLS DTP-rel. MOV{N,Z} 15:0.  */
// #define R_AARCH64_TLSLD_MOVW_DTPRelocations_G0_NC 527 /* Likewise; MOVK; no check.  */
// #define R_AARCH64_TLSLD_ADD_DTPRelocations_HI12 528 /* DTP-rel. ADD imm. from 23:12. */
// #define R_AARCH64_TLSLD_ADD_DTPRelocations_LO12 529 /* DTP-rel. ADD imm. from 11:0.  */
// #define R_AARCH64_TLSLD_ADD_DTPRelocations_LO12_NC 530 /* Likewise; no ovfl. check.  */
// #define R_AARCH64_TLSLD_LDST8_DTPRelocations_LO12 531 /* DTP-rel. LD/ST imm. 11:0.  */
// #define R_AARCH64_TLSLD_LDST8_DTPRelocations_LO12_NC 532 /* Likewise; no check.  */
// #define R_AARCH64_TLSLD_LDST16_DTPRelocations_LO12 533 /* DTP-rel. LD/ST imm. 11:1.  */
// #define R_AARCH64_TLSLD_LDST16_DTPRelocations_LO12_NC 534 /* Likewise; no check.  */
// #define R_AARCH64_TLSLD_LDST32_DTPRelocations_LO12 535 /* DTP-rel. LD/ST imm. 11:2.  */
// #define R_AARCH64_TLSLD_LDST32_DTPRelocations_LO12_NC 536 /* Likewise; no check.  */
// #define R_AARCH64_TLSLD_LDST64_DTPRelocations_LO12 537 /* DTP-rel. LD/ST imm. 11:3.  */
// #define R_AARCH64_TLSLD_LDST64_DTPRelocations_LO12_NC 538 /* Likewise; no check.  */
// #define R_AARCH64_TLSIE_MOVW_GOTTPRelocations_G1 539 /* GOT-rel. MOV{N,Z} 31:16.  */
// #define R_AARCH64_TLSIE_MOVW_GOTTPRelocations_G0_NC 540 /* GOT-rel. MOVK 15:0.  */
// #define R_AARCH64_TLSIE_ADR_GOTTPRelocations_PAGE21 541 /* Page-rel. ADRP 32:12.  */
// #define R_AARCH64_TLSIE_LD64_GOTTPRelocations_LO12_NC 542 /* Direct LD off. 11:3.  */
// #define R_AARCH64_TLSIE_LD_GOTTPRelocations_PRelocations19 543 /* PC-rel. load imm. 20:2.  */
// #define R_AARCH64_TLSLE_MOVW_TPRelocations_G2 544 /* TLS TP-rel. MOV{N,Z} 47:32.  */
// #define R_AARCH64_TLSLE_MOVW_TPRelocations_G1 545 /* TLS TP-rel. MOV{N,Z} 31:16.  */
// #define R_AARCH64_TLSLE_MOVW_TPRelocations_G1_NC 546 /* Likewise; MOVK; no check.  */
// #define R_AARCH64_TLSLE_MOVW_TPRelocations_G0 547 /* TLS TP-rel. MOV{N,Z} 15:0.  */
// #define R_AARCH64_TLSLE_MOVW_TPRelocations_G0_NC 548 /* Likewise; MOVK; no check.  */
// #define R_AARCH64_TLSLE_ADD_TPRelocations_HI12 549 /* TP-rel. ADD imm. 23:12.  */
// #define R_AARCH64_TLSLE_ADD_TPRelocations_LO12 550 /* TP-rel. ADD imm. 11:0.  */
// #define R_AARCH64_TLSLE_ADD_TPRelocations_LO12_NC 551 /* Likewise; no ovfl. check.  */
// #define R_AARCH64_TLSLE_LDST8_TPRelocations_LO12 552 /* TP-rel. LD/ST off. 11:0.  */
// #define R_AARCH64_TLSLE_LDST8_TPRelocations_LO12_NC 553 /* Likewise; no ovfl. check. */
// #define R_AARCH64_TLSLE_LDST16_TPRelocations_LO12 554 /* TP-rel. LD/ST off. 11:1.  */
// #define R_AARCH64_TLSLE_LDST16_TPRelocations_LO12_NC 555 /* Likewise; no check.  */
// #define R_AARCH64_TLSLE_LDST32_TPRelocations_LO12 556 /* TP-rel. LD/ST off. 11:2.  */
// #define R_AARCH64_TLSLE_LDST32_TPRelocations_LO12_NC 557 /* Likewise; no check.  */
// #define R_AARCH64_TLSLE_LDST64_TPRelocations_LO12 558 /* TP-rel. LD/ST off. 11:3.  */
// #define R_AARCH64_TLSLE_LDST64_TPRelocations_LO12_NC 559 /* Likewise; no check.  */
// #define R_AARCH64_TLSDESC_LD_PRelocations19 560        /* PC-rel. load immediate 20:2.  */
// #define R_AARCH64_TLSDESC_ADR_PRelocations21 561 /* PC-rel. ADR immediate 20:0.  */
// #define R_AARCH64_TLSDESC_ADR_PAGE21 562 /* Page-rel. ADRP imm. 32:12.  */
// #define R_AARCH64_TLSDESC_LD64_LO12 563        /* Direct LD off. from 11:3.  */
// #define R_AARCH64_TLSDESC_ADD_LO12 564        /* Direct ADD imm. from 11:0.  */
// #define R_AARCH64_TLSDESC_OFF_G1 565        /* GOT-rel. MOV{N,Z} imm. 31:16.  */
// #define R_AARCH64_TLSDESC_OFF_G0_NC 566        /* GOT-rel. MOVK imm. 15:0; no ck.  */
// #define R_AARCH64_TLSDESC_LDR        567        /* Relax LDR.  */
// #define R_AARCH64_TLSDESC_ADD        568        /* Relax ADD.  */
// #define R_AARCH64_TLSDESC_CALL        569        /* Relax BLR.  */
// #define R_AARCH64_TLSLE_LDST128_TPRelocations_LO12 570 /* TP-rel. LD/ST off. 11:4.  */
// #define R_AARCH64_TLSLE_LDST128_TPRelocations_LO12_NC 571 /* Likewise; no check.  */
// #define R_AARCH64_TLSLD_LDST128_DTPRelocations_LO12 572 /* DTP-rel. LD/ST imm. 11:4. */
// #define R_AARCH64_TLSLD_LDST128_DTPRelocations_LO12_NC 573 /* Likewise; no check.  */
// #define R_AARCH64_COPY         1024        /* Copy symbol at runtime.  */
// #define R_AARCH64_GLOB_DAT     1025        /* Create GOT entry.  */
// #define R_AARCH64_JUMP_SLOT    1026        /* Create PLT entry.  */
// #define R_AARCH64_Relocations_AddendsTIVE     1027        /* Adjust by program base.  */
// #define R_AARCH64_TLS_DTPMOD   1028        /* Module number, 64 bit.  */
// #define R_AARCH64_TLS_DTPRelocations   1029        /* Module-relative offset, 64 bit.  */
// #define R_AARCH64_TLS_TPRelocations    1030        /* TP-relative offset, 64 bit.  */
// #define R_AARCH64_TLSDESC      1031        /* TLS Descriptor.  */
// #define R_AARCH64_IRelocations_AddendsTIVE        1032        /* STT_GNU_IFUNC relocation.  */
//
// /* MTE memory tag segment type.  */
// #define PT_AARCH64_MEMTAG_MTE        (PT_LOPROC + 2)
//
// /* AArch64 specific values for the Dyn d_tag field.  */
// #define DT_AARCH64_BTI_PLT        (DT_LOPROC + 1)
// #define DT_AARCH64_PAC_PLT        (DT_LOPROC + 3)
// #define DT_AARCH64_VARIANT_PCS        (DT_LOPROC + 5)
// #define DT_AARCH64_NUM                6
//
// /* AArch64 specific values for the visibility field.  */
// #define STO_AARCH64_VARIANT_PCS 0x80
//
// /* ARM relocs.  */
//
// #define R_ARM_NONE                0        /* No reloc */
// #define R_ARM_PC24                1        /* Deprecated PC relative 26
//                                            bit branch.  */
// #define R_ARM_ABS32                2        /* Direct 32 bit  */
// #define R_ARM_Relocations32                3        /* PC relative 32 bit */
// #define R_ARM_PC13                4
// #define R_ARM_ABS16                5        /* Direct 16 bit */
// #define R_ARM_ABS12                6        /* Direct 12 bit */
// #define R_ARM_THM_ABS5                7        /* Direct & 0x7C (LDR, STR).  */
// #define R_ARM_ABS8                8        /* Direct 8 bit */
// #define R_ARM_SBRelocations32                9
// #define R_ARM_THM_PC22                10        /* PC relative 24 bit (Thumb32 BL).  */
// #define R_ARM_THM_PC8                11        /* PC relative & 0x3FC
//                                            (Thumb16 LDR, ADD, ADR).  */
// #define R_ARM_AMP_VCALL9        12
// #define R_ARM_SWI24                13        /* Obsolete static relocation.  */
// #define R_ARM_TLS_DESC                13      /* Dynamic relocation.  */
// #define R_ARM_THM_SWI8                14        /* Reserved.  */
// #define R_ARM_XPC25                15        /* Reserved.  */
// #define R_ARM_THM_XPC22                16        /* Reserved.  */
// #define R_ARM_TLS_DTPMOD32        17        /* ID of module containing symbol */
// #define R_ARM_TLS_DTPOFF32        18        /* Offset in TLS block */
// #define R_ARM_TLS_TPOFF32        19        /* Offset in static TLS block */
// #define R_ARM_COPY                20        /* Copy symbol at runtime */
// #define R_ARM_GLOB_DAT                21        /* Create GOT entry */
// #define R_ARM_JUMP_SLOT                22        /* Create PLT entry */
// #define R_ARM_Relocations_AddendsTIVE                23        /* Adjust by program base */
// #define R_ARM_GOTOFF                24        /* 32 bit offset to GOT */
// #define R_ARM_GOTPC                25        /* 32 bit PC relative offset to GOT */
// #define R_ARM_GOT32                26        /* 32 bit GOT entry */
// #define R_ARM_PLT32                27        /* Deprecated, 32 bit PLT address.  */
// #define R_ARM_CALL                28        /* PC relative 24 bit (BL, BLX).  */
// #define R_ARM_JUMP24                29        /* PC relative 24 bit
//                                            (B, BL<cond>).  */
// #define R_ARM_THM_JUMP24        30        /* PC relative 24 bit (Thumb32 B.W).  */
// #define R_ARM_BASE_ABS                31        /* Adjust by program base.  */
// #define R_ARM_ALU_PCRelocations_7_0        32        /* Obsolete.  */
// #define R_ARM_ALU_PCRelocations_15_8        33        /* Obsolete.  */
// #define R_ARM_ALU_PCRelocations_23_15        34        /* Obsolete.  */
// #define R_ARM_LDR_SBRelocations_11_0        35        /* Deprecated, prog. base relative.  */
// #define R_ARM_ALU_SBRelocations_19_12        36        /* Deprecated, prog. base relative.  */
// #define R_ARM_ALU_SBRelocations_27_20        37        /* Deprecated, prog. base relative.  */
// #define R_ARM_TARGET1                38
// #define R_ARM_SBRelocations31                39        /* Program base relative.  */
// #define R_ARM_V4BX                40
// #define R_ARM_TARGET2                41
// #define R_ARM_PRelocations31                42        /* 32 bit PC relative.  */
// #define R_ARM_MOVW_ABS_NC        43        /* Direct 16-bit (MOVW).  */
// #define R_ARM_MOVT_ABS                44        /* Direct high 16-bit (MOVT).  */
// #define R_ARM_MOVW_PRelocations_NC        45        /* PC relative 16-bit (MOVW).  */
// #define R_ARM_MOVT_PRelocations                46        /* PC relative (MOVT).  */
// #define R_ARM_THM_MOVW_ABS_NC        47        /* Direct 16 bit (Thumb32 MOVW).  */
// #define R_ARM_THM_MOVT_ABS        48        /* Direct high 16 bit
//                                            (Thumb32 MOVT).  */
// #define R_ARM_THM_MOVW_PRelocations_NC        49        /* PC relative 16 bit
//                                            (Thumb32 MOVW).  */
// #define R_ARM_THM_MOVT_PRelocations        50        /* PC relative high 16 bit
//                                            (Thumb32 MOVT).  */
// #define R_ARM_THM_JUMP19        51        /* PC relative 20 bit
//                                            (Thumb32 B<cond>.W).  */
// #define R_ARM_THM_JUMP6                52        /* PC relative X & 0x7E
//                                            (Thumb16 CBZ, CBNZ).  */
// #define R_ARM_THM_ALU_PRelocations_11_0        53        /* PC relative 12 bit
//                                            (Thumb32 ADR.W).  */
// #define R_ARM_THM_PC12                54        /* PC relative 12 bit
//                                            (Thumb32 LDR{D,SB,H,SH}).  */
// #define R_ARM_ABS32_NOI                55        /* Direct 32-bit.  */
// #define R_ARM_Relocations32_NOI                56        /* PC relative 32-bit.  */
// #define R_ARM_ALU_PC_G0_NC        57        /* PC relative (ADD, SUB).  */
// #define R_ARM_ALU_PC_G0                58        /* PC relative (ADD, SUB).  */
// #define R_ARM_ALU_PC_G1_NC        59        /* PC relative (ADD, SUB).  */
// #define R_ARM_ALU_PC_G1                60        /* PC relative (ADD, SUB).  */
// #define R_ARM_ALU_PC_G2                61        /* PC relative (ADD, SUB).  */
// #define R_ARM_LDR_PC_G1                62        /* PC relative (LDR,STR,LDRB,STRB).  */
// #define R_ARM_LDR_PC_G2                63        /* PC relative (LDR,STR,LDRB,STRB).  */
// #define R_ARM_LDRS_PC_G0        64        /* PC relative (STR{D,H},
//                                            LDR{D,SB,H,SH}).  */
// #define R_ARM_LDRS_PC_G1        65        /* PC relative (STR{D,H},
//                                            LDR{D,SB,H,SH}).  */
// #define R_ARM_LDRS_PC_G2        66        /* PC relative (STR{D,H},
//                                            LDR{D,SB,H,SH}).  */
// #define R_ARM_LDC_PC_G0                67        /* PC relative (LDC, STC).  */
// #define R_ARM_LDC_PC_G1                68        /* PC relative (LDC, STC).  */
// #define R_ARM_LDC_PC_G2                69        /* PC relative (LDC, STC).  */
// #define R_ARM_ALU_SB_G0_NC        70        /* Program base relative (ADD,SUB).  */
// #define R_ARM_ALU_SB_G0                71        /* Program base relative (ADD,SUB).  */
// #define R_ARM_ALU_SB_G1_NC        72        /* Program base relative (ADD,SUB).  */
// #define R_ARM_ALU_SB_G1                73        /* Program base relative (ADD,SUB).  */
// #define R_ARM_ALU_SB_G2                74        /* Program base relative (ADD,SUB).  */
// #define R_ARM_LDR_SB_G0                75        /* Program base relative (LDR,
//                                            STR, LDRB, STRB).  */
// #define R_ARM_LDR_SB_G1                76        /* Program base relative
//                                            (LDR, STR, LDRB, STRB).  */
// #define R_ARM_LDR_SB_G2                77        /* Program base relative
//                                            (LDR, STR, LDRB, STRB).  */
// #define R_ARM_LDRS_SB_G0        78        /* Program base relative
//                                            (LDR, STR, LDRB, STRB).  */
// #define R_ARM_LDRS_SB_G1        79        /* Program base relative
//                                            (LDR, STR, LDRB, STRB).  */
// #define R_ARM_LDRS_SB_G2        80        /* Program base relative
//                                            (LDR, STR, LDRB, STRB).  */
// #define R_ARM_LDC_SB_G0                81        /* Program base relative (LDC,STC).  */
// #define R_ARM_LDC_SB_G1                82        /* Program base relative (LDC,STC).  */
// #define R_ARM_LDC_SB_G2                83        /* Program base relative (LDC,STC).  */
// #define R_ARM_MOVW_BRelocations_NC        84        /* Program base relative 16
//                                            bit (MOVW).  */
// #define R_ARM_MOVT_BRelocations                85        /* Program base relative high
//                                            16 bit (MOVT).  */
// #define R_ARM_MOVW_BRelocations                86        /* Program base relative 16
//                                            bit (MOVW).  */
// #define R_ARM_THM_MOVW_BRelocations_NC        87        /* Program base relative 16
//                                            bit (Thumb32 MOVW).  */
// #define R_ARM_THM_MOVT_BRelocations        88        /* Program base relative high
//                                            16 bit (Thumb32 MOVT).  */
// #define R_ARM_THM_MOVW_BRelocations        89        /* Program base relative 16
//                                            bit (Thumb32 MOVW).  */
// #define R_ARM_TLS_GOTDESC        90
// #define R_ARM_TLS_CALL                91
// #define R_ARM_TLS_DESCSEQ        92        /* TLS relaxation.  */
// #define R_ARM_THM_TLS_CALL        93
// #define R_ARM_PLT32_ABS                94
// #define R_ARM_GOT_ABS                95        /* GOT entry.  */
// #define R_ARM_GOT_PRelocations                96        /* PC relative GOT entry.  */
// #define R_ARM_GOT_BRelocations12        97        /* GOT entry relative to GOT
//                                            origin (LDR).  */
// #define R_ARM_GOTOFF12                98        /* 12 bit, GOT entry relative
//                                            to GOT origin (LDR, STR).  */
// #define R_ARM_GOTRelocations_AddendsX                99
// #define R_ARM_GNU_VTENTRY        100
// #define R_ARM_GNU_VTINHERIT        101
// #define R_ARM_THM_PC11                102        /* PC relative & 0xFFE (Thumb16 B).  */
// #define R_ARM_THM_PC9                103        /* PC relative & 0x1FE
//                                            (Thumb16 B/B<cond>).  */
// #define R_ARM_TLS_GD32                104        /* PC-rel 32 bit for global dynamic
//                                            thread local data */
// #define R_ARM_TLS_LDM32                105        /* PC-rel 32 bit for local dynamic
//                                            thread local data */
// #define R_ARM_TLS_LDO32                106        /* 32 bit offset relative to TLS
//                                            block */
// #define R_ARM_TLS_IE32                107        /* PC-rel 32 bit for GOT entry of
//                                            static TLS block offset */
// #define R_ARM_TLS_LE32                108        /* 32 bit offset relative to static
//                                            TLS block */
// #define R_ARM_TLS_LDO12                109        /* 12 bit relative to TLS
//                                            block (LDR, STR).  */
// #define R_ARM_TLS_LE12                110        /* 12 bit relative to static
//                                            TLS block (LDR, STR).  */
// #define R_ARM_TLS_IE12GP        111        /* 12 bit GOT entry relative
//                                            to GOT origin (LDR).  */
// #define R_ARM_ME_TOO                128        /* Obsolete.  */
// #define R_ARM_THM_TLS_DESCSEQ        129
// #define R_ARM_THM_TLS_DESCSEQ16        129
// #define R_ARM_THM_TLS_DESCSEQ32        130
// #define R_ARM_THM_GOT_BRelocations12        131        /* GOT entry relative to GOT
//                                            origin, 12 bit (Thumb32 LDR).  */
// #define R_ARM_IRelocations_AddendsTIVE                160
// #define R_ARM_RXPC25                249
// #define R_ARM_RSBRelocations32                250
// #define R_ARM_THM_RPC22                251
// #define R_ARM_RRelocations32                252
// #define R_ARM_RABS22                253
// #define R_ARM_RPC24                254
// #define R_ARM_RBASE                255
// /* Keep this the last entry.  */
// #define R_ARM_NUM                256
//
// /* C-SKY */
// #define R_CKCORE_NONE               0        /* no reloc */
// #define R_CKCORE_ADDR32             1        /* direct 32 bit (S + A) */
// #define R_CKCORE_PCRelocationsIMM8BY4       2        /* disp ((S + A - P) >> 2) & 0xff   */
// #define R_CKCORE_PCRelocationsIMM11BY2      3        /* disp ((S + A - P) >> 1) & 0x7ff  */
// #define R_CKCORE_PCRelocations32            5        /* 32-bit rel (S + A - P)           */
// #define R_CKCORE_PCRelocationsJSR_IMM11BY2  6        /* disp ((S + A - P) >>1) & 0x7ff   */
// #define R_CKCORE_Relocations_AddendsTIVE           9        /* 32 bit adjust program base(B + A)*/
// #define R_CKCORE_COPY               10        /* 32 bit adjust by program base    */
// #define R_CKCORE_GLOB_DAT           11        /* off between got and sym (S)      */
// #define R_CKCORE_JUMP_SLOT          12        /* PLT entry (S) */
// #define R_CKCORE_GOTOFF             13        /* offset to GOT (S + A - GOT)      */
// #define R_CKCORE_GOTPC              14        /* PC offset to GOT (GOT + A - P)   */
// #define R_CKCORE_GOT32              15        /* 32 bit GOT entry (G) */
// #define R_CKCORE_PLT32              16        /* 32 bit PLT entry (G) */
// #define R_CKCORE_ADDRGOT            17        /* GOT entry in GLOB_DAT (GOT + G)  */
// #define R_CKCORE_ADDRPLT            18        /* PLT entry in GLOB_DAT (GOT + G)  */
// #define R_CKCORE_PCRelocations_IMM26BY2     19        /* ((S + A - P) >> 1) & 0x3ffffff   */
// #define R_CKCORE_PCRelocations_IMM16BY2     20        /* disp ((S + A - P) >> 1) & 0xffff */
// #define R_CKCORE_PCRelocations_IMM16BY4     21        /* disp ((S + A - P) >> 2) & 0xffff */
// #define R_CKCORE_PCRelocations_IMM10BY2     22        /* disp ((S + A - P) >> 1) & 0x3ff  */
// #define R_CKCORE_PCRelocations_IMM10BY4     23        /* disp ((S + A - P) >> 2) & 0x3ff  */
// #define R_CKCORE_ADDR_HI16          24        /* high & low 16 bit ADDR */
//                                         /* ((S + A) >> 16) & 0xffff */
// #define R_CKCORE_ADDR_LO16          25        /* (S + A) & 0xffff */
// #define R_CKCORE_GOTPC_HI16         26        /* high & low 16 bit GOTPC */
//                                         /* ((GOT + A - P) >> 16) & 0xffff */
// #define R_CKCORE_GOTPC_LO16         27        /* (GOT + A - P) & 0xffff */
// #define R_CKCORE_GOTOFF_HI16        28        /* high & low 16 bit GOTOFF */
//                                         /* ((S + A - GOT) >> 16) & 0xffff */
// #define R_CKCORE_GOTOFF_LO16        29        /* (S + A - GOT) & 0xffff */
// #define R_CKCORE_GOT12              30        /* 12 bit disp GOT entry (G) */
// #define R_CKCORE_GOT_HI16           31        /* high & low 16 bit GOT */
//                                         /* (G >> 16) & 0xffff */
// #define R_CKCORE_GOT_LO16           32        /* (G & 0xffff) */
// #define R_CKCORE_PLT12              33        /* 12 bit disp PLT entry (G) */
// #define R_CKCORE_PLT_HI16           34        /* high & low 16 bit PLT */
//                                         /* (G >> 16) & 0xffff */
// #define R_CKCORE_PLT_LO16           35        /* G & 0xffff */
// #define R_CKCORE_ADDRGOT_HI16       36        /* high & low 16 bit ADDRGOT */
//                                         /* (GOT + G * 4) & 0xffff */
// #define R_CKCORE_ADDRGOT_LO16       37        /* (GOT + G * 4) & 0xffff */
// #define R_CKCORE_ADDRPLT_HI16       38        /* high & low 16 bit ADDRPLT */
//                                         /* ((GOT + G * 4) >> 16) & 0xFFFF */
// #define R_CKCORE_ADDRPLT_LO16       39        /* (GOT+G*4) & 0xffff */
// #define R_CKCORE_PCRelocations_JSR_IMM26BY2 40        /* disp ((S+A-P) >>1) & x3ffffff */
// #define R_CKCORE_TOFFSET_LO16       41        /* (S+A-BTEXT) & 0xffff */
// #define R_CKCORE_DOFFSET_LO16       42        /* (S+A-BTEXT) & 0xffff */
// #define R_CKCORE_PCRelocations_IMM18BY2     43        /* disp ((S+A-P) >>1) & 0x3ffff */
// #define R_CKCORE_DOFFSET_IMM18      44        /* disp (S+A-BDATA) & 0x3ffff */
// #define R_CKCORE_DOFFSET_IMM18BY2   45        /* disp ((S+A-BDATA)>>1) & 0x3ffff */
// #define R_CKCORE_DOFFSET_IMM18BY4   46        /* disp ((S+A-BDATA)>>2) & 0x3ffff */
// #define R_CKCORE_GOT_IMM18BY4       48        /* disp (G >> 2) */
// #define R_CKCORE_PLT_IMM18BY4       49        /* disp (G >> 2) */
// #define R_CKCORE_PCRelocations_IMM7BY4      50        /* disp ((S+A-P) >>2) & 0x7f */
// #define R_CKCORE_TLS_LE32           51        /* 32 bit offset to TLS block */
// #define R_CKCORE_TLS_IE32           52
// #define R_CKCORE_TLS_GD32           53
// #define R_CKCORE_TLS_LDM32          54
// #define R_CKCORE_TLS_LDO32          55
// #define R_CKCORE_TLS_DTPMOD32       56
// #define R_CKCORE_TLS_DTPOFF32       57
// #define R_CKCORE_TLS_TPOFF32        58
//
// /* C-SKY elf header definition.  */
// #define EF_CSKY_ABIMASK                    0XF0000000
// #define EF_CSKY_OTHER                    0X0FFF0000
// #define EF_CSKY_PROCESSOR            0X0000FFFF
//
// #define EF_CSKY_ABIV1                    0X10000000
// #define EF_CSKY_ABIV2                    0X20000000
//
// /* C-SKY attributes section.  */
// #define ELF_Section_Header_Type__CSKY_ATTRIBUTES            (ELF_Section_Header_Type__LOPROC + 1)
//
// /* IA-64 specific declarations.  */
//
// /* Processor specific flags for the Ehdr processor_flags field.  */
// #define EF_IA_64_MASKOS                0x0000000f        /* os-specific flags */
// #define EF_IA_64_ABI64                0x00000010        /* 64-bit ABI */
// #define EF_IA_64_ARCH                0xff000000        /* arch. version mask */
//
// /* Processor specific values for the Phdr p_type field.  */
// #define PT_IA_64_ARCHEXT        (PT_LOPROC + 0)        /* arch extension bits */
// #define PT_IA_64_UNWIND                (PT_LOPROC + 1)        /* ia64 unwind bits */
// #define PT_IA_64_HP_OPT_ANOT        (PT_LOOS + 0x12)
// #define PT_IA_64_HP_HSL_ANOT        (PT_LOOS + 0x13)
// #define PT_IA_64_HP_STACK        (PT_LOOS + 0x14)
//
// /* Processor specific flags for the Phdr p_flags field.  */
// #define PF_IA_64_NORECOV        0x80000000        /* spec insns w/o recovery */
//
// /* Processor specific values for the Shdr type field.  */
// #define ELF_Section_Header_Type__IA_64_EXT                (ELF_Section_Header_Type__LOPROC + 0) /* extension bits */
// #define ELF_Section_Header_Type__IA_64_UNWIND        (ELF_Section_Header_Type__LOPROC + 1) /* unwind bits */
//
// /* Processor specific flags for the Shdr flags field.  */
// #define SHF_IA_64_SHORT                0x10000000        /* section near gp */
// #define SHF_IA_64_NORECOV        0x20000000        /* spec insns w/o recovery */
//
// /* Processor specific values for the Dyn d_tag field.  */
// #define DT_IA_64_PLT_RESERVE        (DT_LOPROC + 0)
// #define DT_IA_64_NUM                1
//
// /* IA-64 relocations.  */
// #define R_IA64_NONE                0x00        /* none */
// #define R_IA64_IMM14                0x21        /* symbol + addend, add imm14 */
// #define R_IA64_IMM22                0x22        /* symbol + addend, add imm22 */
// #define R_IA64_IMM64                0x23        /* symbol + addend, mov imm64 */
// #define R_IA64_DIR32MSB                0x24        /* symbol + addend, data4 MSB */
// #define R_IA64_DIR32LSB                0x25        /* symbol + addend, data4 LSB */
// #define R_IA64_DIR64MSB                0x26        /* symbol + addend, data8 MSB */
// #define R_IA64_DIR64LSB                0x27        /* symbol + addend, data8 LSB */
// #define R_IA64_GPRelocations22                0x2a        /* @gprel(sym + add), add imm22 */
// #define R_IA64_GPRelocations64I                0x2b        /* @gprel(sym + add), mov imm64 */
// #define R_IA64_GPRelocations32MSB        0x2c        /* @gprel(sym + add), data4 MSB */
// #define R_IA64_GPRelocations32LSB        0x2d        /* @gprel(sym + add), data4 LSB */
// #define R_IA64_GPRelocations64MSB        0x2e        /* @gprel(sym + add), data8 MSB */
// #define R_IA64_GPRelocations64LSB        0x2f        /* @gprel(sym + add), data8 LSB */
// #define R_IA64_LTOFF22                0x32        /* @ltoff(sym + add), add imm22 */
// #define R_IA64_LTOFF64I                0x33        /* @ltoff(sym + add), mov imm64 */
// #define R_IA64_PLTOFF22                0x3a        /* @pltoff(sym + add), add imm22 */
// #define R_IA64_PLTOFF64I        0x3b        /* @pltoff(sym + add), mov imm64 */
// #define R_IA64_PLTOFF64MSB        0x3e        /* @pltoff(sym + add), data8 MSB */
// #define R_IA64_PLTOFF64LSB        0x3f        /* @pltoff(sym + add), data8 LSB */
// #define R_IA64_FPTR64I                0x43        /* @fptr(sym + add), mov imm64 */
// #define R_IA64_FPTR32MSB        0x44        /* @fptr(sym + add), data4 MSB */
// #define R_IA64_FPTR32LSB        0x45        /* @fptr(sym + add), data4 LSB */
// #define R_IA64_FPTR64MSB        0x46        /* @fptr(sym + add), data8 MSB */
// #define R_IA64_FPTR64LSB        0x47        /* @fptr(sym + add), data8 LSB */
// #define R_IA64_PCRelocations60B                0x48        /* @pcrel(sym + add), brl */
// #define R_IA64_PCRelocations21B                0x49        /* @pcrel(sym + add), ptb, call */
// #define R_IA64_PCRelocations21M                0x4a        /* @pcrel(sym + add), chk.s */
// #define R_IA64_PCRelocations21F                0x4b        /* @pcrel(sym + add), fchkf */
// #define R_IA64_PCRelocations32MSB        0x4c        /* @pcrel(sym + add), data4 MSB */
// #define R_IA64_PCRelocations32LSB        0x4d        /* @pcrel(sym + add), data4 LSB */
// #define R_IA64_PCRelocations64MSB        0x4e        /* @pcrel(sym + add), data8 MSB */
// #define R_IA64_PCRelocations64LSB        0x4f        /* @pcrel(sym + add), data8 LSB */
// #define R_IA64_LTOFF_FPTR22        0x52        /* @ltoff(@fptr(s+a)), imm22 */
// #define R_IA64_LTOFF_FPTR64I        0x53        /* @ltoff(@fptr(s+a)), imm64 */
// #define R_IA64_LTOFF_FPTR32MSB        0x54        /* @ltoff(@fptr(s+a)), data4 MSB */
// #define R_IA64_LTOFF_FPTR32LSB        0x55        /* @ltoff(@fptr(s+a)), data4 LSB */
// #define R_IA64_LTOFF_FPTR64MSB        0x56        /* @ltoff(@fptr(s+a)), data8 MSB */
// #define R_IA64_LTOFF_FPTR64LSB        0x57        /* @ltoff(@fptr(s+a)), data8 LSB */
// #define R_IA64_SEGRelocations32MSB        0x5c        /* @segrel(sym + add), data4 MSB */
// #define R_IA64_SEGRelocations32LSB        0x5d        /* @segrel(sym + add), data4 LSB */
// #define R_IA64_SEGRelocations64MSB        0x5e        /* @segrel(sym + add), data8 MSB */
// #define R_IA64_SEGRelocations64LSB        0x5f        /* @segrel(sym + add), data8 LSB */
// #define R_IA64_SECRelocations32MSB        0x64        /* @secrel(sym + add), data4 MSB */
// #define R_IA64_SECRelocations32LSB        0x65        /* @secrel(sym + add), data4 LSB */
// #define R_IA64_SECRelocations64MSB        0x66        /* @secrel(sym + add), data8 MSB */
// #define R_IA64_SECRelocations64LSB        0x67        /* @secrel(sym + add), data8 LSB */
// #define R_IA64_Relocations32MSB                0x6c        /* data 4 + Relocations */
// #define R_IA64_Relocations32LSB                0x6d        /* data 4 + Relocations */
// #define R_IA64_Relocations64MSB                0x6e        /* data 8 + Relocations */
// #define R_IA64_Relocations64LSB                0x6f        /* data 8 + Relocations */
// #define R_IA64_LTV32MSB                0x74        /* symbol + addend, data4 MSB */
// #define R_IA64_LTV32LSB                0x75        /* symbol + addend, data4 LSB */
// #define R_IA64_LTV64MSB                0x76        /* symbol + addend, data8 MSB */
// #define R_IA64_LTV64LSB                0x77        /* symbol + addend, data8 LSB */
// #define R_IA64_PCRelocations21BI        0x79        /* @pcrel(sym + add), 21bit inst */
// #define R_IA64_PCRelocations22                0x7a        /* @pcrel(sym + add), 22bit inst */
// #define R_IA64_PCRelocations64I                0x7b        /* @pcrel(sym + add), 64bit inst */
// #define R_IA64_IPLTMSB                0x80        /* dynamic reloc, imported PLT, MSB */
// #define R_IA64_IPLTLSB                0x81        /* dynamic reloc, imported PLT, LSB */
// #define R_IA64_COPY                0x84        /* copy relocation */
// #define R_IA64_SUB                0x85        /* Addend and symbol difference */
// #define R_IA64_LTOFF22X                0x86        /* LTOFF22, relaxable.  */
// #define R_IA64_LDXMOV                0x87        /* Use of LTOFF22X.  */
// #define R_IA64_TPRelocations14                0x91        /* @tprel(sym + add), imm14 */
// #define R_IA64_TPRelocations22                0x92        /* @tprel(sym + add), imm22 */
// #define R_IA64_TPRelocations64I                0x93        /* @tprel(sym + add), imm64 */
// #define R_IA64_TPRelocations64MSB        0x96        /* @tprel(sym + add), data8 MSB */
// #define R_IA64_TPRelocations64LSB        0x97        /* @tprel(sym + add), data8 LSB */
// #define R_IA64_LTOFF_TPRelocations22        0x9a        /* @ltoff(@tprel(s+a)), imm2 */
// #define R_IA64_DTPMOD64MSB        0xa6        /* @dtpmod(sym + add), data8 MSB */
// #define R_IA64_DTPMOD64LSB        0xa7        /* @dtpmod(sym + add), data8 LSB */
// #define R_IA64_LTOFF_DTPMOD22        0xaa        /* @ltoff(@dtpmod(sym + add)), imm22 */
// #define R_IA64_DTPRelocations14                0xb1        /* @dtprel(sym + add), imm14 */
// #define R_IA64_DTPRelocations22                0xb2        /* @dtprel(sym + add), imm22 */
// #define R_IA64_DTPRelocations64I        0xb3        /* @dtprel(sym + add), imm64 */
// #define R_IA64_DTPRelocations32MSB        0xb4        /* @dtprel(sym + add), data4 MSB */
// #define R_IA64_DTPRelocations32LSB        0xb5        /* @dtprel(sym + add), data4 LSB */
// #define R_IA64_DTPRelocations64MSB        0xb6        /* @dtprel(sym + add), data8 MSB */
// #define R_IA64_DTPRelocations64LSB        0xb7        /* @dtprel(sym + add), data8 LSB */
// #define R_IA64_LTOFF_DTPRelocations22        0xba        /* @ltoff(@dtprel(s+a)), imm22 */
//
// /* SH specific declarations */
//
// /* Processor specific flags for the ELF header processor_flags field.  */
// #define EF_SH_MACH_MASK                0x1f
// #define EF_SH_UNKNOWN                0x0
// #define EF_SH1                        0x1
// #define EF_SH2                        0x2
// #define EF_SH3                        0x3
// #define EF_SH_DSP                0x4
// #define EF_SH3_DSP                0x5
// #define EF_SH4AL_DSP                0x6
// #define EF_SH3E                        0x8
// #define EF_SH4                        0x9
// #define EF_SH2E                        0xb
// #define EF_SH4A                        0xc
// #define EF_SH2A                        0xd
// #define EF_SH4_NOFPU                0x10
// #define EF_SH4A_NOFPU                0x11
// #define EF_SH4_NOMMU_NOFPU        0x12
// #define EF_SH2A_NOFPU                0x13
// #define EF_SH3_NOMMU                0x14
// #define EF_SH2A_SH4_NOFPU        0x15
// #define EF_SH2A_SH3_NOFPU        0x16
// #define EF_SH2A_SH4                0x17
// #define EF_SH2A_SH3E                0x18
//
// /* SH relocs.  */
// #define        R_SH_NONE                0
// #define        R_SH_DIR32                1
// #define        R_SH_Relocations32                2
// #define        R_SH_DIR8WPN                3
// #define        R_SH_IND12W                4
// #define        R_SH_DIR8WPL                5
// #define        R_SH_DIR8WPZ                6
// #define        R_SH_DIR8BP                7
// #define        R_SH_DIR8W                8
// #define        R_SH_DIR8L                9
// #define        R_SH_SWITCH16                25
// #define        R_SH_SWITCH32                26
// #define        R_SH_USES                27
// #define        R_SH_COUNT                28
// #define        R_SH_ALIGN                29
// #define        R_SH_CODE                30
// #define        R_SH_DATA                31
// #define        R_SH_LABEL                32
// #define        R_SH_SWITCH8                33
// #define        R_SH_GNU_VTINHERIT        34
// #define        R_SH_GNU_VTENTRY        35
// #define        R_SH_TLS_GD_32                144
// #define        R_SH_TLS_LD_32                145
// #define        R_SH_TLS_LDO_32                146
// #define        R_SH_TLS_IE_32                147
// #define        R_SH_TLS_LE_32                148
// #define        R_SH_TLS_DTPMOD32        149
// #define        R_SH_TLS_DTPOFF32        150
// #define        R_SH_TLS_TPOFF32        151
// #define        R_SH_GOT32                160
// #define        R_SH_PLT32                161
// #define        R_SH_COPY                162
// #define        R_SH_GLOB_DAT                163
// #define        R_SH_JMP_SLOT                164
// #define        R_SH_Relocations_AddendsTIVE                165
// #define        R_SH_GOTOFF                166
// #define        R_SH_GOTPC                167
// /* Keep this the last entry.  */
// #define        R_SH_NUM                256
//
// /* S/390 specific definitions.  */
//
// /* Valid values for the processor_flags field.  */
//
// #define EF_S390_HIGH_GPRS    0x00000001  /* High GPRs kernel facility needed.  */
//
// /* Additional s390 relocs */
//
// #define R_390_NONE                0        /* No reloc.  */
// #define R_390_8                        1        /* Direct 8 bit.  */
// #define R_390_12                2        /* Direct 12 bit.  */
// #define R_390_16                3        /* Direct 16 bit.  */
// #define R_390_32                4        /* Direct 32 bit.  */
// #define R_390_PC32                5        /* PC relative 32 bit.        */
// #define R_390_GOT12                6        /* 12 bit GOT offset.  */
// #define R_390_GOT32                7        /* 32 bit GOT offset.  */
// #define R_390_PLT32                8        /* 32 bit PC relative PLT address.  */
// #define R_390_COPY                9        /* Copy symbol at runtime.  */
// #define R_390_GLOB_DAT                10        /* Create GOT entry.  */
// #define R_390_JMP_SLOT                11        /* Create PLT entry.  */
// #define R_390_Relocations_AddendsTIVE                12        /* Adjust by program base.  */
// #define R_390_GOTOFF32                13        /* 32 bit offset to GOT.         */
// #define R_390_GOTPC                14        /* 32 bit PC relative offset to GOT.  */
// #define R_390_GOT16                15        /* 16 bit GOT offset.  */
// #define R_390_PC16                16        /* PC relative 16 bit.        */
// #define R_390_PC16DBL                17        /* PC relative 16 bit shifted by 1.  */
// #define R_390_PLT16DBL                18        /* 16 bit PC rel. PLT shifted by 1.  */
// #define R_390_PC32DBL                19        /* PC relative 32 bit shifted by 1.  */
// #define R_390_PLT32DBL                20        /* 32 bit PC rel. PLT shifted by 1.  */
// #define R_390_GOTPCDBL                21        /* 32 bit PC rel. GOT shifted by 1.  */
// #define R_390_64                22        /* Direct 64 bit.  */
// #define R_390_PC64                23        /* PC relative 64 bit.        */
// #define R_390_GOT64                24        /* 64 bit GOT offset.  */
// #define R_390_PLT64                25        /* 64 bit PC relative PLT address.  */
// #define R_390_GOTENT                26        /* 32 bit PC rel. to GOT entry >> 1. */
// #define R_390_GOTOFF16                27        /* 16 bit offset to GOT. */
// #define R_390_GOTOFF64                28        /* 64 bit offset to GOT. */
// #define R_390_GOTPLT12                29        /* 12 bit offset to jump slot.        */
// #define R_390_GOTPLT16                30        /* 16 bit offset to jump slot.        */
// #define R_390_GOTPLT32                31        /* 32 bit offset to jump slot.        */
// #define R_390_GOTPLT64                32        /* 64 bit offset to jump slot.        */
// #define R_390_GOTPLTENT                33        /* 32 bit rel. offset to jump slot.  */
// #define R_390_PLTOFF16                34        /* 16 bit offset from GOT to PLT. */
// #define R_390_PLTOFF32                35        /* 32 bit offset from GOT to PLT. */
// #define R_390_PLTOFF64                36        /* 16 bit offset from GOT to PLT. */
// #define R_390_TLS_LOAD                37        /* Tag for load insn in TLS code.  */
// #define R_390_TLS_GDCALL        38        /* Tag for function call in general
//                                            dynamic TLS code. */
// #define R_390_TLS_LDCALL        39        /* Tag for function call in local
//                                            dynamic TLS code. */
// #define R_390_TLS_GD32                40        /* Direct 32 bit for general dynamic
//                                            thread local data.  */
// #define R_390_TLS_GD64                41        /* Direct 64 bit for general dynamic
//                                           thread local data.  */
// #define R_390_TLS_GOTIE12        42        /* 12 bit GOT offset for static TLS
//                                            block offset.  */
// #define R_390_TLS_GOTIE32        43        /* 32 bit GOT offset for static TLS
//                                            block offset.  */
// #define R_390_TLS_GOTIE64        44        /* 64 bit GOT offset for static TLS
//                                            block offset. */
// #define R_390_TLS_LDM32                45        /* Direct 32 bit for local dynamic
//                                            thread local data in LE code.  */
// #define R_390_TLS_LDM64                46        /* Direct 64 bit for local dynamic
//                                            thread local data in LE code.  */
// #define R_390_TLS_IE32                47        /* 32 bit address of GOT entry for
//                                            negated static TLS block offset.  */
// #define R_390_TLS_IE64                48        /* 64 bit address of GOT entry for
//                                            negated static TLS block offset.  */
// #define R_390_TLS_IEENT                49        /* 32 bit rel. offset to GOT entry for
//                                            negated static TLS block offset.  */
// #define R_390_TLS_LE32                50        /* 32 bit negated offset relative to
//                                            static TLS block.  */
// #define R_390_TLS_LE64                51        /* 64 bit negated offset relative to
//                                            static TLS block.  */
// #define R_390_TLS_LDO32                52        /* 32 bit offset relative to TLS
//                                            block.  */
// #define R_390_TLS_LDO64                53        /* 64 bit offset relative to TLS
//                                            block.  */
// #define R_390_TLS_DTPMOD        54        /* ID of module containing symbol.  */
// #define R_390_TLS_DTPOFF        55        /* Offset in TLS block.         */
// #define R_390_TLS_TPOFF                56        /* Negated offset in static TLS
//                                            block.  */
// #define R_390_20                57        /* Direct 20 bit.  */
// #define R_390_GOT20                58        /* 20 bit GOT offset.  */
// #define R_390_GOTPLT20                59        /* 20 bit offset to jump slot.  */
// #define R_390_TLS_GOTIE20        60        /* 20 bit GOT offset for static TLS
//                                            block offset.  */
// #define R_390_IRelocations_AddendsTIVE         61      /* STT_GNU_IFUNC relocation.  */
// /* Keep this the last entry.  */
// #define R_390_NUM                62
//
//
// /* CRIS relocations.  */
// #define R_CRIS_NONE                0
// #define R_CRIS_8                1
// #define R_CRIS_16                2
// #define R_CRIS_32                3
// #define R_CRIS_8_PCRelocations                4
// #define R_CRIS_16_PCRelocations                5
// #define R_CRIS_32_PCRelocations                6
// #define R_CRIS_GNU_VTINHERIT        7
// #define R_CRIS_GNU_VTENTRY        8
// #define R_CRIS_COPY                9
// #define R_CRIS_GLOB_DAT                10
// #define R_CRIS_JUMP_SLOT        11
// #define R_CRIS_Relocations_AddendsTIVE                12
// #define R_CRIS_16_GOT                13
// #define R_CRIS_32_GOT                14
// #define R_CRIS_16_GOTPLT        15
// #define R_CRIS_32_GOTPLT        16
// #define R_CRIS_32_GOTRelocations        17
// #define R_CRIS_32_PLT_GOTRelocations        18
// #define R_CRIS_32_PLT_PCRelocations        19
//
// #define R_CRIS_NUM                20
//
//
// /* AMD x86-64 relocations.  */
// #define R_X86_64_NONE                0        /* No reloc */
// #define R_X86_64_64                1        /* Direct 64 bit  */
// #define R_X86_64_PC32                2        /* PC relative 32 bit signed */
// #define R_X86_64_GOT32                3        /* 32 bit GOT entry */
// #define R_X86_64_PLT32                4        /* 32 bit PLT address */
// #define R_X86_64_COPY                5        /* Copy symbol at runtime */
// #define R_X86_64_GLOB_DAT        6        /* Create GOT entry */
// #define R_X86_64_JUMP_SLOT        7        /* Create PLT entry */
// #define R_X86_64_Relocations_AddendsTIVE        8        /* Adjust by program base */
// #define R_X86_64_GOTPCRelocations        9        /* 32 bit signed PC relative
//                                            offset to GOT */
// #define R_X86_64_32                10        /* Direct 32 bit zero extended */
// #define R_X86_64_32S                11        /* Direct 32 bit sign extended */
// #define R_X86_64_16                12        /* Direct 16 bit zero extended */
// #define R_X86_64_PC16                13        /* 16 bit sign extended pc relative */
// #define R_X86_64_8                14        /* Direct 8 bit sign extended  */
// #define R_X86_64_PC8                15        /* 8 bit sign extended pc relative */
// #define R_X86_64_DTPMOD64        16        /* ID of module containing symbol */
// #define R_X86_64_DTPOFF64        17        /* Offset in module's TLS block */
// #define R_X86_64_TPOFF64        18        /* Offset in initial TLS block */
// #define R_X86_64_TLSGD                19        /* 32 bit signed PC relative offset
//                                            to two GOT entries for GD symbol */
// #define R_X86_64_TLSLD                20        /* 32 bit signed PC relative offset
//                                            to two GOT entries for LD symbol */
// #define R_X86_64_DTPOFF32        21        /* Offset in TLS block */
// #define R_X86_64_GOTTPOFF        22        /* 32 bit signed PC relative offset
//                                            to GOT entry for IE symbol */
// #define R_X86_64_TPOFF32        23        /* Offset in initial TLS block */
// #define R_X86_64_PC64                24        /* PC relative 64 bit */
// #define R_X86_64_GOTOFF64        25        /* 64 bit offset to GOT */
// #define R_X86_64_GOTPC32        26        /* 32 bit signed pc relative
//                                            offset to GOT */
// #define R_X86_64_GOT64                27        /* 64-bit GOT entry offset */
// #define R_X86_64_GOTPCRelocations64        28        /* 64-bit PC relative offset
//                                            to GOT entry */
// #define R_X86_64_GOTPC64        29        /* 64-bit PC relative offset to GOT */
// #define R_X86_64_GOTPLT64        30         /* like GOT64, says PLT entry needed */
// #define R_X86_64_PLTOFF64        31        /* 64-bit GOT relative offset
//                                            to PLT entry */
// #define R_X86_64_SIZE32                32        /* Size of symbol plus 32-bit addend */
// #define R_X86_64_SIZE64                33        /* Size of symbol plus 64-bit addend */
// #define R_X86_64_GOTPC32_TLSDESC 34        /* GOT offset for TLS descriptor.  */
// #define R_X86_64_TLSDESC_CALL   35        /* Marker for call through TLS
//                                            descriptor.  */
// #define R_X86_64_TLSDESC        36        /* TLS descriptor.  */
// #define R_X86_64_IRelocations_AddendsTIVE        37        /* Adjust indirectly by program base */
// #define R_X86_64_Relocations_AddendsTIVE64        38        /* 64-bit adjust by program base */
//                                         /* 39 Reserved was R_X86_64_PC32_BND */
//                                         /* 40 Reserved was R_X86_64_PLT32_BND */
// #define R_X86_64_GOTPCRelocationsX        41        /* Load from 32 bit signed pc relative
//                                            offset to GOT entry without REX
//                                            prefix, relaxable.  */
// #define R_X86_64_REX_GOTPCRelocationsX        42        /* Load from 32 bit signed pc relative
//                                            offset to GOT entry with REX prefix,
//                                            relaxable.  */
// #define R_X86_64_NUM                43
//
// /* x86-64 type values.  */
// #define ELF_Section_Header_Type__X86_64_UNWIND        0x70000001 /* Unwind information.  */
//
// /* x86-64 d_tag values.  */
// #define DT_X86_64_PLT                (DT_LOPROC + 0)
// #define DT_X86_64_PLTSZ                (DT_LOPROC + 1)
// #define DT_X86_64_PLTENT        (DT_LOPROC + 3)
// #define DT_X86_64_NUM                4
//
// /* AM33 relocations.  */
// #define R_MN10300_NONE                0        /* No reloc.  */
// #define R_MN10300_32                1        /* Direct 32 bit.  */
// #define R_MN10300_16                2        /* Direct 16 bit.  */
// #define R_MN10300_8                3        /* Direct 8 bit.  */
// #define R_MN10300_PCRelocations32        4        /* PC-relative 32-bit.  */
// #define R_MN10300_PCRelocations16        5        /* PC-relative 16-bit signed.  */
// #define R_MN10300_PCRelocations8        6        /* PC-relative 8-bit signed.  */
// #define R_MN10300_GNU_VTINHERIT        7        /* Ancient C++ vtable garbage... */
// #define R_MN10300_GNU_VTENTRY        8        /* ... collection annotation.  */
// #define R_MN10300_24                9        /* Direct 24 bit.  */
// #define R_MN10300_GOTPC32        10        /* 32-bit PCrel offset to GOT.  */
// #define R_MN10300_GOTPC16        11        /* 16-bit PCrel offset to GOT.  */
// #define R_MN10300_GOTOFF32        12        /* 32-bit offset from GOT.  */
// #define R_MN10300_GOTOFF24        13        /* 24-bit offset from GOT.  */
// #define R_MN10300_GOTOFF16        14        /* 16-bit offset from GOT.  */
// #define R_MN10300_PLT32                15        /* 32-bit PCrel to PLT entry.  */
// #define R_MN10300_PLT16                16        /* 16-bit PCrel to PLT entry.  */
// #define R_MN10300_GOT32                17        /* 32-bit offset to GOT entry.  */
// #define R_MN10300_GOT24                18        /* 24-bit offset to GOT entry.  */
// #define R_MN10300_GOT16                19        /* 16-bit offset to GOT entry.  */
// #define R_MN10300_COPY                20        /* Copy symbol at runtime.  */
// #define R_MN10300_GLOB_DAT        21        /* Create GOT entry.  */
// #define R_MN10300_JMP_SLOT        22        /* Create PLT entry.  */
// #define R_MN10300_Relocations_AddendsTIVE        23        /* Adjust by program base.  */
// #define R_MN10300_TLS_GD        24        /* 32-bit offset for global dynamic.  */
// #define R_MN10300_TLS_LD        25        /* 32-bit offset for local dynamic.  */
// #define R_MN10300_TLS_LDO        26        /* Module-relative offset.  */
// #define R_MN10300_TLS_GOTIE        27        /* GOT offset for static TLS block
//                                            offset.  */
// #define R_MN10300_TLS_IE        28        /* GOT address for static TLS block
//                                            offset.  */
// #define R_MN10300_TLS_LE        29        /* Offset relative to static TLS
//                                            block.  */
// #define R_MN10300_TLS_DTPMOD        30        /* ID of module containing symbol.  */
// #define R_MN10300_TLS_DTPOFF        31        /* Offset in module TLS block.  */
// #define R_MN10300_TLS_TPOFF        32        /* Offset in static TLS block.  */
// #define R_MN10300_SYM_DIFF        33        /* Adjustment for next reloc as needed
//                                            by linker relaxation.  */
// #define R_MN10300_ALIGN                34        /* Alignment requirement for linker
//                                            relaxation.  */
// #define R_MN10300_NUM                35
//
//
// /* M32R relocs.  */
// #define R_M32R_NONE                0        /* No reloc. */
// #define R_M32R_16                1        /* Direct 16 bit. */
// #define R_M32R_32                2        /* Direct 32 bit. */
// #define R_M32R_24                3        /* Direct 24 bit. */
// #define R_M32R_10_PCRelocations                4        /* PC relative 10 bit shifted. */
// #define R_M32R_18_PCRelocations                5        /* PC relative 18 bit shifted. */
// #define R_M32R_26_PCRelocations                6        /* PC relative 26 bit shifted. */
// #define R_M32R_HI16_ULO                7        /* High 16 bit with unsigned low. */
// #define R_M32R_HI16_SLO                8        /* High 16 bit with signed low. */
// #define R_M32R_LO16                9        /* Low 16 bit. */
// #define R_M32R_SDA16                10        /* 16 bit offset in SDA. */
// #define R_M32R_GNU_VTINHERIT        11
// #define R_M32R_GNU_VTENTRY        12
// /* M32R relocs use ELF_Section_Header_Type__Relocations_Addends.  */
// #define R_M32R_16_Relocations_Addends                33        /* Direct 16 bit. */
// #define R_M32R_32_Relocations_Addends                34        /* Direct 32 bit. */
// #define R_M32R_24_Relocations_Addends                35        /* Direct 24 bit. */
// #define R_M32R_10_PCRelocations_Relocations_Addends        36        /* PC relative 10 bit shifted. */
// #define R_M32R_18_PCRelocations_Relocations_Addends        37        /* PC relative 18 bit shifted. */
// #define R_M32R_26_PCRelocations_Relocations_Addends        38        /* PC relative 26 bit shifted. */
// #define R_M32R_HI16_ULO_Relocations_Addends        39        /* High 16 bit with unsigned low */
// #define R_M32R_HI16_SLO_Relocations_Addends        40        /* High 16 bit with signed low */
// #define R_M32R_LO16_Relocations_Addends        41        /* Low 16 bit */
// #define R_M32R_SDA16_Relocations_Addends        42        /* 16 bit offset in SDA */
// #define R_M32R_Relocations_Addends_GNU_VTINHERIT        43
// #define R_M32R_Relocations_Addends_GNU_VTENTRY        44
// #define R_M32R_Relocations32                45        /* PC relative 32 bit.  */
//
// #define R_M32R_GOT24                48        /* 24 bit GOT entry */
// #define R_M32R_26_PLTRelocations        49        /* 26 bit PC relative to PLT shifted */
// #define R_M32R_COPY                50        /* Copy symbol at runtime */
// #define R_M32R_GLOB_DAT                51        /* Create GOT entry */
// #define R_M32R_JMP_SLOT                52        /* Create PLT entry */
// #define R_M32R_Relocations_AddendsTIVE                53        /* Adjust by program base */
// #define R_M32R_GOTOFF                54        /* 24 bit offset to GOT */
// #define R_M32R_GOTPC24                55        /* 24 bit PC relative offset to GOT */
// #define R_M32R_GOT16_HI_ULO        56        /* High 16 bit GOT entry with unsigned
//                                            low */
// #define R_M32R_GOT16_HI_SLO        57        /* High 16 bit GOT entry with signed
//                                            low */
// #define R_M32R_GOT16_LO                58        /* Low 16 bit GOT entry */
// #define R_M32R_GOTPC_HI_ULO        59        /* High 16 bit PC relative offset to
//                                            GOT with unsigned low */
// #define R_M32R_GOTPC_HI_SLO        60        /* High 16 bit PC relative offset to
//                                            GOT with signed low */
// #define R_M32R_GOTPC_LO                61        /* Low 16 bit PC relative offset to
//                                            GOT */
// #define R_M32R_GOTOFF_HI_ULO        62        /* High 16 bit offset to GOT
//                                            with unsigned low */
// #define R_M32R_GOTOFF_HI_SLO        63        /* High 16 bit offset to GOT
//                                            with signed low */
// #define R_M32R_GOTOFF_LO        64        /* Low 16 bit offset to GOT */
// #define R_M32R_NUM                256        /* Keep this the last entry. */
//
// /* MicroBlaze relocations */
// #define R_MICROBLAZE_NONE                0        /* No reloc. */
// #define R_MICROBLAZE_32                 1        /* Direct 32 bit. */
// #define R_MICROBLAZE_32_PCRelocations                2        /* PC relative 32 bit. */
// #define R_MICROBLAZE_64_PCRelocations                3        /* PC relative 64 bit. */
// #define R_MICROBLAZE_32_PCRelocations_LO        4        /* Low 16 bits of PCRelocations32. */
// #define R_MICROBLAZE_64                 5        /* Direct 64 bit. */
// #define R_MICROBLAZE_32_LO                6        /* Low 16 bit. */
// #define R_MICROBLAZE_SRO32                7        /* Read-only small data area. */
// #define R_MICROBLAZE_SRW32                8        /* Read-write small data area. */
// #define R_MICROBLAZE_64_NONE                9        /* No reloc. */
// #define R_MICROBLAZE_32_SYM_OP_SYM        10        /* Symbol Op Symbol relocation. */
// #define R_MICROBLAZE_GNU_VTINHERIT        11        /* GNU C++ vtable hierarchy. */
// #define R_MICROBLAZE_GNU_VTENTRY        12        /* GNU C++ vtable member usage. */
// #define R_MICROBLAZE_GOTPC_64                13        /* PC-relative GOT offset.  */
// #define R_MICROBLAZE_GOT_64                14        /* GOT entry offset.  */
// #define R_MICROBLAZE_PLT_64                15        /* PLT offset (PC-relative).  */
// #define R_MICROBLAZE_Relocations                16        /* Adjust by program base.  */
// #define R_MICROBLAZE_JUMP_SLOT                17        /* Create PLT entry.  */
// #define R_MICROBLAZE_GLOB_DAT                18        /* Create GOT entry.  */
// #define R_MICROBLAZE_GOTOFF_64                19        /* 64 bit offset to GOT. */
// #define R_MICROBLAZE_GOTOFF_32                20        /* 32 bit offset to GOT. */
// #define R_MICROBLAZE_COPY                21        /* Runtime copy.  */
// #define R_MICROBLAZE_TLS                22        /* TLS Reloc. */
// #define R_MICROBLAZE_TLSGD                23        /* TLS General Dynamic. */
// #define R_MICROBLAZE_TLSLD                24        /* TLS Local Dynamic. */
// #define R_MICROBLAZE_TLSDTPMOD32        25        /* TLS Module ID. */
// #define R_MICROBLAZE_TLSDTPRelocations32        26        /* TLS Offset Within TLS Block. */
// #define R_MICROBLAZE_TLSDTPRelocations64        27        /* TLS Offset Within TLS Block. */
// #define R_MICROBLAZE_TLSGOTTPRelocations32        28        /* TLS Offset From Thread Pointer. */
// #define R_MICROBLAZE_TLSTPRelocations32         29        /* TLS Offset From Thread Pointer. */
//
// /* Legal values for d_tag (dynamic entry type).  */
// #define DT_NIOS2_GP             0x70000002 /* Address of _gp.  */
//
// /* Nios II relocations.  */
// #define R_NIOS2_NONE                0        /* No reloc.  */
// #define R_NIOS2_S16                1        /* Direct signed 16 bit.  */
// #define R_NIOS2_U16                2        /* Direct unsigned 16 bit.  */
// #define R_NIOS2_PCRelocations16                3        /* PC relative 16 bit.  */
// #define R_NIOS2_CALL26                4        /* Direct call.  */
// #define R_NIOS2_IMM5                5        /* 5 bit constant expression.  */
// #define R_NIOS2_CACHE_OPX        6        /* 5 bit expression, shift 22.  */
// #define R_NIOS2_IMM6                7        /* 6 bit constant expression.  */
// #define R_NIOS2_IMM8                8        /* 8 bit constant expression.  */
// #define R_NIOS2_HI16                9        /* High 16 bit.  */
// #define R_NIOS2_LO16                10        /* Low 16 bit.  */
// #define R_NIOS2_HIADJ16                11        /* High 16 bit, adjusted.  */
// #define R_NIOS2_BFD_RelocationsOC_32        12        /* 32 bit symbol value + addend.  */
// #define R_NIOS2_BFD_RelocationsOC_16        13        /* 16 bit symbol value + addend.  */
// #define R_NIOS2_BFD_RelocationsOC_8        14        /* 8 bit symbol value + addend.  */
// #define R_NIOS2_GPRelocations                15        /* 16 bit GP pointer offset.  */
// #define R_NIOS2_GNU_VTINHERIT        16        /* GNU C++ vtable hierarchy.  */
// #define R_NIOS2_GNU_VTENTRY        17        /* GNU C++ vtable member usage.  */
// #define R_NIOS2_UJMP                18        /* Unconditional branch.  */
// #define R_NIOS2_CJMP                19        /* Conditional branch.  */
// #define R_NIOS2_CALLR                20        /* Indirect call through register.  */
// #define R_NIOS2_ALIGN                21        /* Alignment requirement for
//                                            linker relaxation.  */
// #define R_NIOS2_GOT16                22        /* 16 bit GOT entry.  */
// #define R_NIOS2_CALL16                23        /* 16 bit GOT entry for function.  */
// #define R_NIOS2_GOTOFF_LO        24        /* %lo of offset to GOT pointer.  */
// #define R_NIOS2_GOTOFF_HA        25        /* %hiadj of offset to GOT pointer.  */
// #define R_NIOS2_PCRelocations_LO        26        /* %lo of PC relative offset.  */
// #define R_NIOS2_PCRelocations_HA        27        /* %hiadj of PC relative offset.  */
// #define R_NIOS2_TLS_GD16        28        /* 16 bit GOT offset for TLS GD.  */
// #define R_NIOS2_TLS_LDM16        29        /* 16 bit GOT offset for TLS LDM.  */
// #define R_NIOS2_TLS_LDO16        30        /* 16 bit module relative offset.  */
// #define R_NIOS2_TLS_IE16        31        /* 16 bit GOT offset for TLS IE.  */
// #define R_NIOS2_TLS_LE16        32        /* 16 bit LE TP-relative offset.  */
// #define R_NIOS2_TLS_DTPMOD        33        /* Module number.  */
// #define R_NIOS2_TLS_DTPRelocations        34        /* Module-relative offset.  */
// #define R_NIOS2_TLS_TPRelocations        35        /* TP-relative offset.  */
// #define R_NIOS2_COPY                36        /* Copy symbol at runtime.  */
// #define R_NIOS2_GLOB_DAT        37        /* Create GOT entry.  */
// #define R_NIOS2_JUMP_SLOT        38        /* Create PLT entry.  */
// #define R_NIOS2_Relocations_AddendsTIVE        39        /* Adjust by program base.  */
// #define R_NIOS2_GOTOFF                40        /* 16 bit offset to GOT pointer.  */
// #define R_NIOS2_CALL26_NOAT        41        /* Direct call in .noat section.  */
// #define R_NIOS2_GOT_LO                42        /* %lo() of GOT entry.  */
// #define R_NIOS2_GOT_HA                43        /* %hiadj() of GOT entry.  */
// #define R_NIOS2_CALL_LO                44        /* %lo() of function GOT entry.  */
// #define R_NIOS2_CALL_HA                45        /* %hiadj() of function GOT entry.  */
//
// /* TILEPro relocations.  */
// #define R_TILEPRO_NONE                0        /* No reloc */
// #define R_TILEPRO_32                1        /* Direct 32 bit */
// #define R_TILEPRO_16                2        /* Direct 16 bit */
// #define R_TILEPRO_8                3        /* Direct 8 bit */
// #define R_TILEPRO_32_PCRelocations        4        /* PC relative 32 bit */
// #define R_TILEPRO_16_PCRelocations        5        /* PC relative 16 bit */
// #define R_TILEPRO_8_PCRelocations        6        /* PC relative 8 bit */
// #define R_TILEPRO_LO16                7        /* Low 16 bit */
// #define R_TILEPRO_HI16                8        /* High 16 bit */
// #define R_TILEPRO_HA16                9        /* High 16 bit, adjusted */
// #define R_TILEPRO_COPY                10        /* Copy relocation */
// #define R_TILEPRO_GLOB_DAT        11        /* Create GOT entry */
// #define R_TILEPRO_JMP_SLOT        12        /* Create PLT entry */
// #define R_TILEPRO_Relocations_AddendsTIVE        13        /* Adjust by program base */
// #define R_TILEPRO_BROFF_X1        14        /* X1 pipe branch offset */
// #define R_TILEPRO_JOFFLONG_X1        15        /* X1 pipe jump offset */
// #define R_TILEPRO_JOFFLONG_X1_PLT 16        /* X1 pipe jump offset to PLT */
// #define R_TILEPRO_IMM8_X0        17        /* X0 pipe 8-bit */
// #define R_TILEPRO_IMM8_Y0        18        /* Y0 pipe 8-bit */
// #define R_TILEPRO_IMM8_X1        19        /* X1 pipe 8-bit */
// #define R_TILEPRO_IMM8_Y1        20        /* Y1 pipe 8-bit */
// #define R_TILEPRO_MT_IMM15_X1        21        /* X1 pipe mtspr */
// #define R_TILEPRO_MF_IMM15_X1        22        /* X1 pipe mfspr */
// #define R_TILEPRO_IMM16_X0        23        /* X0 pipe 16-bit */
// #define R_TILEPRO_IMM16_X1        24        /* X1 pipe 16-bit */
// #define R_TILEPRO_IMM16_X0_LO        25        /* X0 pipe low 16-bit */
// #define R_TILEPRO_IMM16_X1_LO        26        /* X1 pipe low 16-bit */
// #define R_TILEPRO_IMM16_X0_HI        27        /* X0 pipe high 16-bit */
// #define R_TILEPRO_IMM16_X1_HI        28        /* X1 pipe high 16-bit */
// #define R_TILEPRO_IMM16_X0_HA        29        /* X0 pipe high 16-bit, adjusted */
// #define R_TILEPRO_IMM16_X1_HA        30        /* X1 pipe high 16-bit, adjusted */
// #define R_TILEPRO_IMM16_X0_PCRelocations 31        /* X0 pipe PC relative 16 bit */
// #define R_TILEPRO_IMM16_X1_PCRelocations 32        /* X1 pipe PC relative 16 bit */
// #define R_TILEPRO_IMM16_X0_LO_PCRelocations 33        /* X0 pipe PC relative low 16 bit */
// #define R_TILEPRO_IMM16_X1_LO_PCRelocations 34        /* X1 pipe PC relative low 16 bit */
// #define R_TILEPRO_IMM16_X0_HI_PCRelocations 35        /* X0 pipe PC relative high 16 bit */
// #define R_TILEPRO_IMM16_X1_HI_PCRelocations 36        /* X1 pipe PC relative high 16 bit */
// #define R_TILEPRO_IMM16_X0_HA_PCRelocations 37        /* X0 pipe PC relative ha() 16 bit */
// #define R_TILEPRO_IMM16_X1_HA_PCRelocations 38        /* X1 pipe PC relative ha() 16 bit */
// #define R_TILEPRO_IMM16_X0_GOT        39        /* X0 pipe 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X1_GOT        40        /* X1 pipe 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X0_GOT_LO 41        /* X0 pipe low 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X1_GOT_LO 42        /* X1 pipe low 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X0_GOT_HI 43        /* X0 pipe high 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X1_GOT_HI 44        /* X1 pipe high 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X0_GOT_HA 45        /* X0 pipe ha() 16-bit GOT offset */
// #define R_TILEPRO_IMM16_X1_GOT_HA 46        /* X1 pipe ha() 16-bit GOT offset */
// #define R_TILEPRO_MMSTART_X0        47        /* X0 pipe mm "start" */
// #define R_TILEPRO_MMEND_X0        48        /* X0 pipe mm "end" */
// #define R_TILEPRO_MMSTART_X1        49        /* X1 pipe mm "start" */
// #define R_TILEPRO_MMEND_X1        50        /* X1 pipe mm "end" */
// #define R_TILEPRO_SHAMT_X0        51        /* X0 pipe shift amount */
// #define R_TILEPRO_SHAMT_X1        52        /* X1 pipe shift amount */
// #define R_TILEPRO_SHAMT_Y0        53        /* Y0 pipe shift amount */
// #define R_TILEPRO_SHAMT_Y1        54        /* Y1 pipe shift amount */
// #define R_TILEPRO_DEST_IMM8_X1        55        /* X1 pipe destination 8-bit */
// /* Relocs 56-59 are currently not defined.  */
// #define R_TILEPRO_TLS_GD_CALL        60        /* "jal" for TLS GD */
// #define R_TILEPRO_IMM8_X0_TLS_GD_ADD 61        /* X0 pipe "addi" for TLS GD */
// #define R_TILEPRO_IMM8_X1_TLS_GD_ADD 62        /* X1 pipe "addi" for TLS GD */
// #define R_TILEPRO_IMM8_Y0_TLS_GD_ADD 63        /* Y0 pipe "addi" for TLS GD */
// #define R_TILEPRO_IMM8_Y1_TLS_GD_ADD 64        /* Y1 pipe "addi" for TLS GD */
// #define R_TILEPRO_TLS_IE_LOAD        65        /* "lw_tls" for TLS IE */
// #define R_TILEPRO_IMM16_X0_TLS_GD 66        /* X0 pipe 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X1_TLS_GD 67        /* X1 pipe 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X0_TLS_GD_LO 68        /* X0 pipe low 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X1_TLS_GD_LO 69        /* X1 pipe low 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X0_TLS_GD_HI 70        /* X0 pipe high 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X1_TLS_GD_HI 71        /* X1 pipe high 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X0_TLS_GD_HA 72        /* X0 pipe ha() 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X1_TLS_GD_HA 73        /* X1 pipe ha() 16-bit TLS GD offset */
// #define R_TILEPRO_IMM16_X0_TLS_IE 74        /* X0 pipe 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X1_TLS_IE 75        /* X1 pipe 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X0_TLS_IE_LO 76        /* X0 pipe low 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X1_TLS_IE_LO 77        /* X1 pipe low 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X0_TLS_IE_HI 78        /* X0 pipe high 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X1_TLS_IE_HI 79        /* X1 pipe high 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X0_TLS_IE_HA 80        /* X0 pipe ha() 16-bit TLS IE offset */
// #define R_TILEPRO_IMM16_X1_TLS_IE_HA 81        /* X1 pipe ha() 16-bit TLS IE offset */
// #define R_TILEPRO_TLS_DTPMOD32        82        /* ID of module containing symbol */
// #define R_TILEPRO_TLS_DTPOFF32        83        /* Offset in TLS block */
// #define R_TILEPRO_TLS_TPOFF32        84        /* Offset in static TLS block */
// #define R_TILEPRO_IMM16_X0_TLS_LE 85        /* X0 pipe 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X1_TLS_LE 86        /* X1 pipe 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X0_TLS_LE_LO 87        /* X0 pipe low 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X1_TLS_LE_LO 88        /* X1 pipe low 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X0_TLS_LE_HI 89        /* X0 pipe high 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X1_TLS_LE_HI 90        /* X1 pipe high 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X0_TLS_LE_HA 91        /* X0 pipe ha() 16-bit TLS LE offset */
// #define R_TILEPRO_IMM16_X1_TLS_LE_HA 92        /* X1 pipe ha() 16-bit TLS LE offset */
//
// #define R_TILEPRO_GNU_VTINHERIT        128        /* GNU C++ vtable hierarchy */
// #define R_TILEPRO_GNU_VTENTRY        129        /* GNU C++ vtable member usage */
//
// #define R_TILEPRO_NUM                130
//
//
// /* TILE-Gx relocations.  */
// #define R_TILEGX_NONE                0        /* No reloc */
// #define R_TILEGX_64                1        /* Direct 64 bit */
// #define R_TILEGX_32                2        /* Direct 32 bit */
// #define R_TILEGX_16                3        /* Direct 16 bit */
// #define R_TILEGX_8                4        /* Direct 8 bit */
// #define R_TILEGX_64_PCRelocations        5        /* PC relative 64 bit */
// #define R_TILEGX_32_PCRelocations        6        /* PC relative 32 bit */
// #define R_TILEGX_16_PCRelocations        7        /* PC relative 16 bit */
// #define R_TILEGX_8_PCRelocations        8        /* PC relative 8 bit */
// #define R_TILEGX_HW0                9        /* hword 0 16-bit */
// #define R_TILEGX_HW1                10        /* hword 1 16-bit */
// #define R_TILEGX_HW2                11        /* hword 2 16-bit */
// #define R_TILEGX_HW3                12        /* hword 3 16-bit */
// #define R_TILEGX_HW0_LAST        13        /* last hword 0 16-bit */
// #define R_TILEGX_HW1_LAST        14        /* last hword 1 16-bit */
// #define R_TILEGX_HW2_LAST        15        /* last hword 2 16-bit */
// #define R_TILEGX_COPY                16        /* Copy relocation */
// #define R_TILEGX_GLOB_DAT        17        /* Create GOT entry */
// #define R_TILEGX_JMP_SLOT        18        /* Create PLT entry */
// #define R_TILEGX_Relocations_AddendsTIVE        19        /* Adjust by program base */
// #define R_TILEGX_BROFF_X1        20        /* X1 pipe branch offset */
// #define R_TILEGX_JUMPOFF_X1        21        /* X1 pipe jump offset */
// #define R_TILEGX_JUMPOFF_X1_PLT        22        /* X1 pipe jump offset to PLT */
// #define R_TILEGX_IMM8_X0        23        /* X0 pipe 8-bit */
// #define R_TILEGX_IMM8_Y0        24        /* Y0 pipe 8-bit */
// #define R_TILEGX_IMM8_X1        25        /* X1 pipe 8-bit */
// #define R_TILEGX_IMM8_Y1        26        /* Y1 pipe 8-bit */
// #define R_TILEGX_DEST_IMM8_X1        27        /* X1 pipe destination 8-bit */
// #define R_TILEGX_MT_IMM14_X1        28        /* X1 pipe mtspr */
// #define R_TILEGX_MF_IMM14_X1        29        /* X1 pipe mfspr */
// #define R_TILEGX_MMSTART_X0        30        /* X0 pipe mm "start" */
// #define R_TILEGX_MMEND_X0        31        /* X0 pipe mm "end" */
// #define R_TILEGX_SHAMT_X0        32        /* X0 pipe shift amount */
// #define R_TILEGX_SHAMT_X1        33        /* X1 pipe shift amount */
// #define R_TILEGX_SHAMT_Y0        34        /* Y0 pipe shift amount */
// #define R_TILEGX_SHAMT_Y1        35        /* Y1 pipe shift amount */
// #define R_TILEGX_IMM16_X0_HW0        36        /* X0 pipe hword 0 */
// #define R_TILEGX_IMM16_X1_HW0        37        /* X1 pipe hword 0 */
// #define R_TILEGX_IMM16_X0_HW1        38        /* X0 pipe hword 1 */
// #define R_TILEGX_IMM16_X1_HW1        39        /* X1 pipe hword 1 */
// #define R_TILEGX_IMM16_X0_HW2        40        /* X0 pipe hword 2 */
// #define R_TILEGX_IMM16_X1_HW2        41        /* X1 pipe hword 2 */
// #define R_TILEGX_IMM16_X0_HW3        42        /* X0 pipe hword 3 */
// #define R_TILEGX_IMM16_X1_HW3        43        /* X1 pipe hword 3 */
// #define R_TILEGX_IMM16_X0_HW0_LAST 44        /* X0 pipe last hword 0 */
// #define R_TILEGX_IMM16_X1_HW0_LAST 45        /* X1 pipe last hword 0 */
// #define R_TILEGX_IMM16_X0_HW1_LAST 46        /* X0 pipe last hword 1 */
// #define R_TILEGX_IMM16_X1_HW1_LAST 47        /* X1 pipe last hword 1 */
// #define R_TILEGX_IMM16_X0_HW2_LAST 48        /* X0 pipe last hword 2 */
// #define R_TILEGX_IMM16_X1_HW2_LAST 49        /* X1 pipe last hword 2 */
// #define R_TILEGX_IMM16_X0_HW0_PCRelocations 50        /* X0 pipe PC relative hword 0 */
// #define R_TILEGX_IMM16_X1_HW0_PCRelocations 51        /* X1 pipe PC relative hword 0 */
// #define R_TILEGX_IMM16_X0_HW1_PCRelocations 52        /* X0 pipe PC relative hword 1 */
// #define R_TILEGX_IMM16_X1_HW1_PCRelocations 53        /* X1 pipe PC relative hword 1 */
// #define R_TILEGX_IMM16_X0_HW2_PCRelocations 54        /* X0 pipe PC relative hword 2 */
// #define R_TILEGX_IMM16_X1_HW2_PCRelocations 55        /* X1 pipe PC relative hword 2 */
// #define R_TILEGX_IMM16_X0_HW3_PCRelocations 56        /* X0 pipe PC relative hword 3 */
// #define R_TILEGX_IMM16_X1_HW3_PCRelocations 57        /* X1 pipe PC relative hword 3 */
// #define R_TILEGX_IMM16_X0_HW0_LAST_PCRelocations 58 /* X0 pipe PC-rel last hword 0 */
// #define R_TILEGX_IMM16_X1_HW0_LAST_PCRelocations 59 /* X1 pipe PC-rel last hword 0 */
// #define R_TILEGX_IMM16_X0_HW1_LAST_PCRelocations 60 /* X0 pipe PC-rel last hword 1 */
// #define R_TILEGX_IMM16_X1_HW1_LAST_PCRelocations 61 /* X1 pipe PC-rel last hword 1 */
// #define R_TILEGX_IMM16_X0_HW2_LAST_PCRelocations 62 /* X0 pipe PC-rel last hword 2 */
// #define R_TILEGX_IMM16_X1_HW2_LAST_PCRelocations 63 /* X1 pipe PC-rel last hword 2 */
// #define R_TILEGX_IMM16_X0_HW0_GOT 64        /* X0 pipe hword 0 GOT offset */
// #define R_TILEGX_IMM16_X1_HW0_GOT 65        /* X1 pipe hword 0 GOT offset */
// #define R_TILEGX_IMM16_X0_HW0_PLT_PCRelocations 66 /* X0 pipe PC-rel PLT hword 0 */
// #define R_TILEGX_IMM16_X1_HW0_PLT_PCRelocations 67 /* X1 pipe PC-rel PLT hword 0 */
// #define R_TILEGX_IMM16_X0_HW1_PLT_PCRelocations 68 /* X0 pipe PC-rel PLT hword 1 */
// #define R_TILEGX_IMM16_X1_HW1_PLT_PCRelocations 69 /* X1 pipe PC-rel PLT hword 1 */
// #define R_TILEGX_IMM16_X0_HW2_PLT_PCRelocations 70 /* X0 pipe PC-rel PLT hword 2 */
// #define R_TILEGX_IMM16_X1_HW2_PLT_PCRelocations 71 /* X1 pipe PC-rel PLT hword 2 */
// #define R_TILEGX_IMM16_X0_HW0_LAST_GOT 72 /* X0 pipe last hword 0 GOT offset */
// #define R_TILEGX_IMM16_X1_HW0_LAST_GOT 73 /* X1 pipe last hword 0 GOT offset */
// #define R_TILEGX_IMM16_X0_HW1_LAST_GOT 74 /* X0 pipe last hword 1 GOT offset */
// #define R_TILEGX_IMM16_X1_HW1_LAST_GOT 75 /* X1 pipe last hword 1 GOT offset */
// #define R_TILEGX_IMM16_X0_HW3_PLT_PCRelocations 76 /* X0 pipe PC-rel PLT hword 3 */
// #define R_TILEGX_IMM16_X1_HW3_PLT_PCRelocations 77 /* X1 pipe PC-rel PLT hword 3 */
// #define R_TILEGX_IMM16_X0_HW0_TLS_GD 78        /* X0 pipe hword 0 TLS GD offset */
// #define R_TILEGX_IMM16_X1_HW0_TLS_GD 79        /* X1 pipe hword 0 TLS GD offset */
// #define R_TILEGX_IMM16_X0_HW0_TLS_LE 80        /* X0 pipe hword 0 TLS LE offset */
// #define R_TILEGX_IMM16_X1_HW0_TLS_LE 81        /* X1 pipe hword 0 TLS LE offset */
// #define R_TILEGX_IMM16_X0_HW0_LAST_TLS_LE 82 /* X0 pipe last hword 0 LE off */
// #define R_TILEGX_IMM16_X1_HW0_LAST_TLS_LE 83 /* X1 pipe last hword 0 LE off */
// #define R_TILEGX_IMM16_X0_HW1_LAST_TLS_LE 84 /* X0 pipe last hword 1 LE off */
// #define R_TILEGX_IMM16_X1_HW1_LAST_TLS_LE 85 /* X1 pipe last hword 1 LE off */
// #define R_TILEGX_IMM16_X0_HW0_LAST_TLS_GD 86 /* X0 pipe last hword 0 GD off */
// #define R_TILEGX_IMM16_X1_HW0_LAST_TLS_GD 87 /* X1 pipe last hword 0 GD off */
// #define R_TILEGX_IMM16_X0_HW1_LAST_TLS_GD 88 /* X0 pipe last hword 1 GD off */
// #define R_TILEGX_IMM16_X1_HW1_LAST_TLS_GD 89 /* X1 pipe last hword 1 GD off */
// /* Relocs 90-91 are currently not defined.  */
// #define R_TILEGX_IMM16_X0_HW0_TLS_IE 92        /* X0 pipe hword 0 TLS IE offset */
// #define R_TILEGX_IMM16_X1_HW0_TLS_IE 93        /* X1 pipe hword 0 TLS IE offset */
// #define R_TILEGX_IMM16_X0_HW0_LAST_PLT_PCRelocations 94 /* X0 pipe PC-rel PLT last hword 0 */
// #define R_TILEGX_IMM16_X1_HW0_LAST_PLT_PCRelocations 95 /* X1 pipe PC-rel PLT last hword 0 */
// #define R_TILEGX_IMM16_X0_HW1_LAST_PLT_PCRelocations 96 /* X0 pipe PC-rel PLT last hword 1 */
// #define R_TILEGX_IMM16_X1_HW1_LAST_PLT_PCRelocations 97 /* X1 pipe PC-rel PLT last hword 1 */
// #define R_TILEGX_IMM16_X0_HW2_LAST_PLT_PCRelocations 98 /* X0 pipe PC-rel PLT last hword 2 */
// #define R_TILEGX_IMM16_X1_HW2_LAST_PLT_PCRelocations 99 /* X1 pipe PC-rel PLT last hword 2 */
// #define R_TILEGX_IMM16_X0_HW0_LAST_TLS_IE 100 /* X0 pipe last hword 0 IE off */
// #define R_TILEGX_IMM16_X1_HW0_LAST_TLS_IE 101 /* X1 pipe last hword 0 IE off */
// #define R_TILEGX_IMM16_X0_HW1_LAST_TLS_IE 102 /* X0 pipe last hword 1 IE off */
// #define R_TILEGX_IMM16_X1_HW1_LAST_TLS_IE 103 /* X1 pipe last hword 1 IE off */
// /* Relocs 104-105 are currently not defined.  */
// #define R_TILEGX_TLS_DTPMOD64        106        /* 64-bit ID of symbol's module */
// #define R_TILEGX_TLS_DTPOFF64        107        /* 64-bit offset in TLS block */
// #define R_TILEGX_TLS_TPOFF64        108        /* 64-bit offset in static TLS block */
// #define R_TILEGX_TLS_DTPMOD32        109        /* 32-bit ID of symbol's module */
// #define R_TILEGX_TLS_DTPOFF32        110        /* 32-bit offset in TLS block */
// #define R_TILEGX_TLS_TPOFF32        111        /* 32-bit offset in static TLS block */
// #define R_TILEGX_TLS_GD_CALL        112        /* "jal" for TLS GD */
// #define R_TILEGX_IMM8_X0_TLS_GD_ADD 113        /* X0 pipe "addi" for TLS GD */
// #define R_TILEGX_IMM8_X1_TLS_GD_ADD 114        /* X1 pipe "addi" for TLS GD */
// #define R_TILEGX_IMM8_Y0_TLS_GD_ADD 115        /* Y0 pipe "addi" for TLS GD */
// #define R_TILEGX_IMM8_Y1_TLS_GD_ADD 116        /* Y1 pipe "addi" for TLS GD */
// #define R_TILEGX_TLS_IE_LOAD        117        /* "ld_tls" for TLS IE */
// #define R_TILEGX_IMM8_X0_TLS_ADD 118        /* X0 pipe "addi" for TLS GD/IE */
// #define R_TILEGX_IMM8_X1_TLS_ADD 119        /* X1 pipe "addi" for TLS GD/IE */
// #define R_TILEGX_IMM8_Y0_TLS_ADD 120        /* Y0 pipe "addi" for TLS GD/IE */
// #define R_TILEGX_IMM8_Y1_TLS_ADD 121        /* Y1 pipe "addi" for TLS GD/IE */
//
// #define R_TILEGX_GNU_VTINHERIT        128        /* GNU C++ vtable hierarchy */
// #define R_TILEGX_GNU_VTENTRY        129        /* GNU C++ vtable member usage */
//
// #define R_TILEGX_NUM                130
//
// /* RISC-V ELF Flags */
// #define EF_RISCV_RVC                         0x0001
// #define EF_RISCV_FLOAT_ABI                 0x0006
// #define EF_RISCV_FLOAT_ABI_SOFT         0x0000
// #define EF_RISCV_FLOAT_ABI_SINGLE         0x0002
// #define EF_RISCV_FLOAT_ABI_DOUBLE         0x0004
// #define EF_RISCV_FLOAT_ABI_QUAD         0x0006
// #define EF_RISCV_RVE                        0x0008
// #define EF_RISCV_TSO                        0x0010
//
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

// TLS: Thread Local Storage
// GOT: Global Offset Table
// TP:  Thread Pointer
// PC:  Program Counter

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
#define Relocation_RISC_V__Compressed_Branch                              44
#define Relocation_RISC_V__Compressed_Jump                                45
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

//
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
//
// /* ARC specific declarations.  */
//
// /* Processor specific flags for the Ehdr processor_flags field.  */
// #define EF_ARC_MACH_MSK            0x000000ff
// #define EF_ARC_OSABI_MSK    0x00000f00
// #define EF_ARC_ALL_MSK            (EF_ARC_MACH_MSK | EF_ARC_OSABI_MSK)
//
// /* Processor specific values for the Shdr type field.  */
// #define ELF_Section_Header_Type__ARC_ATTRIBUTES        (ELF_Section_Header_Type__LOPROC + 1) /* ARC attributes section.  */
//
// /* ARCompact/ARCv2 specific relocs.  */
// #define R_ARC_NONE                0x0
// #define R_ARC_8                        0x1
// #define R_ARC_16                0x2
// #define R_ARC_24                0x3
// #define R_ARC_32                0x4
//
// #define R_ARC_B22_PCRelocations                0x6
// #define R_ARC_H30                0x7
// #define R_ARC_N8                0x8
// #define R_ARC_N16                0x9
// #define R_ARC_N24                0xA
// #define R_ARC_N32                0xB
// #define R_ARC_SDA                0xC
// #define R_ARC_SECTOFF                0xD
// #define R_ARC_S21H_PCRelocations        0xE
// #define R_ARC_S21W_PCRelocations        0xF
// #define R_ARC_S25H_PCRelocations        0x10
// #define R_ARC_S25W_PCRelocations        0x11
// #define R_ARC_SDA32                0x12
// #define R_ARC_SDA_LDST                0x13
// #define R_ARC_SDA_LDST1                0x14
// #define R_ARC_SDA_LDST2                0x15
// #define R_ARC_SDA16_LD                0x16
// #define R_ARC_SDA16_LD1                0x17
// #define R_ARC_SDA16_LD2                0x18
// #define R_ARC_S13_PCRelocations                0x19
// #define R_ARC_W                        0x1A
// #define R_ARC_32_ME                0x1B
// #define R_ARC_N32_ME                0x1C
// #define R_ARC_SECTOFF_ME        0x1D
// #define R_ARC_SDA32_ME                0x1E
// #define R_ARC_W_ME                0x1F
// #define R_ARC_H30_ME                0x20
// #define R_ARC_SECTOFF_U8        0x21
// #define R_ARC_SECTOFF_S9        0x22
// #define R_AC_SECTOFF_U8                0x23
// #define R_AC_SECTOFF_U8_1        0x24
// #define R_AC_SECTOFF_U8_2        0x25
// #define R_AC_SECTOFF_S9                0x26
// #define R_AC_SECTOFF_S9_1        0x27
// #define R_AC_SECTOFF_S9_2        0x28
// #define R_ARC_SECTOFF_ME_1        0x29
// #define R_ARC_SECTOFF_ME_2        0x2A
// #define R_ARC_SECTOFF_1                0x2B
// #define R_ARC_SECTOFF_2                0x2C
// #define R_ARC_SDA_12                0x2D
// #define R_ARC_SDA16_ST2                0x30
// #define R_ARC_32_PCRelocations                0x31
// #define R_ARC_PC32                0x32
// #define R_ARC_GOTPC32                0x33
// #define R_ARC_PLT32                0x34
// #define R_ARC_COPY                0x35
// #define R_ARC_GLOB_DAT                0x36
// #define R_ARC_JMP_SLOT                0x37
// #define R_ARC_Relocations_AddendsTIVE                0x38
// #define R_ARC_GOTOFF                0x39
// #define R_ARC_GOTPC                0x3A
// #define R_ARC_GOT32                0x3B
// #define R_ARC_S21W_PCRelocations_PLT        0x3C
// #define R_ARC_S25H_PCRelocations_PLT        0x3D
//
// #define R_ARC_JLI_SECTOFF        0x3F
//
// #define R_ARC_TLS_DTPMOD        0x42
// #define R_ARC_TLS_DTPOFF        0x43
// #define R_ARC_TLS_TPOFF                0x44
// #define R_ARC_TLS_GD_GOT        0x45
// #define R_ARC_TLS_GD_LD                0x46
// #define R_ARC_TLS_GD_CALL        0x47
// #define R_ARC_TLS_IE_GOT        0x48
// #define R_ARC_TLS_DTPOFF_S9        0x49
// #define R_ARC_TLS_LE_S9                0x4A
// #define R_ARC_TLS_LE_32                0x4B
// #define R_ARC_S25W_PCRelocations_PLT        0x4C
// #define R_ARC_S21H_PCRelocations_PLT        0x4D
// #define R_ARC_NPS_CMEM16        0x4E
//
// /* OpenRISC 1000 specific relocs.  */
// #define R_OR1K_NONE                0
// #define R_OR1K_32                1
// #define R_OR1K_16                2
// #define R_OR1K_8                3
// #define R_OR1K_LO_16_IN_INSN        4
// #define R_OR1K_HI_16_IN_INSN        5
// #define R_OR1K_INSN_Relocations_26        6
// #define R_OR1K_GNU_VTENTRY        7
// #define R_OR1K_GNU_VTINHERIT        8
// #define R_OR1K_32_PCRelocations                9
// #define R_OR1K_16_PCRelocations                10
// #define R_OR1K_8_PCRelocations                11
// #define R_OR1K_GOTPC_HI16        12
// #define R_OR1K_GOTPC_LO16        13
// #define R_OR1K_GOT16                14
// #define R_OR1K_PLT26                15
// #define R_OR1K_GOTOFF_HI16        16
// #define R_OR1K_GOTOFF_LO16        17
// #define R_OR1K_COPY                18
// #define R_OR1K_GLOB_DAT                19
// #define R_OR1K_JMP_SLOT                20
// #define R_OR1K_Relocations_AddendsTIVE                21
// #define R_OR1K_TLS_GD_HI16        22
// #define R_OR1K_TLS_GD_LO16        23
// #define R_OR1K_TLS_LDM_HI16        24
// #define R_OR1K_TLS_LDM_LO16        25
// #define R_OR1K_TLS_LDO_HI16        26
// #define R_OR1K_TLS_LDO_LO16        27
// #define R_OR1K_TLS_IE_HI16        28
// #define R_OR1K_TLS_IE_LO16        29
// #define R_OR1K_TLS_LE_HI16        30
// #define R_OR1K_TLS_LE_LO16        31
// #define R_OR1K_TLS_TPOFF        32
// #define R_OR1K_TLS_DTPOFF        33
// #define R_OR1K_TLS_DTPMOD        34
//

#endif // ELF_H
