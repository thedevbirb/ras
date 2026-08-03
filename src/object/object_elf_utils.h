#ifndef OBJECT_ELF_UTILS_H
#define OBJECT_ELF_UTILS_H

#define ELF_Section_Header_Flags__cstring  "aeowxEGMST"
global const String8 ELF_Section_Header_Flags__string8 = String8__literal(ELF_Section_Header_Flags__cstring);

#define ELF_Section_Header_Flags__default ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE
#define ELF_Section_Header_Type__default  ELF_Section_Header_Type__Program_Data

// TODO(low, check-gas): incomplete compared to what GNU as does.
// TODO(low, check-gas): probably some of these flags are incompatible and some warnings/errors should be emitted
internal ELF_Section_Header_Flags
ELF_Section_Header_Flags__parse
(
        String8 string
)
{
        U8  *data = string.data;
        U64  index = 0;
        ELF_Section_Header_Flags result = 0;

        for (;;)
        {
                if (index >= string.count)
                {
                        break;
                }

                switch (data[index])
                {
                case 'a':
                {
                  result |= ELF_Section_Header_Flags__ALLOC;
                } break;
                case 'e':
                {
                  result |= ELF_Section_Header_Flags__EXCLUDE;

                } break;
                case 'o':
                {
                  result |= ELF_Section_Header_Flags__LINK_ORDER;

                } break;
                case 'w':
                {
                  result |= ELF_Section_Header_Flags__WRITE;

                } break;
                case 'x':
                {
                  result |= ELF_Section_Header_Flags__EXECINSTR;

                } break;
                case 'G':
                {
                  result |= ELF_Section_Header_Flags__GROUP;

                } break;
                case 'M':
                {
                  result |= ELF_Section_Header_Flags__MERGE;

                } break;
                case 'S':
                {
                  result |= ELF_Section_Header_Flags__STRINGS;

                } break;
                case 'T':
                {
                  result |= ELF_Section_Header_Flags__TLS;

                } break;
                default:
                {
                        result = ELF_Section_Header_Flags__Invalid;
                } break;
                }

                index += 1;
        }

        return result;
}

internal U32
ELF_Section_Header_Type__from_String8(String8 string)
{
        ELF_Section_Header_Type result = 0;

        if      (String8__match_exact(string, String8__literal("progbits")))
        {
                result = ELF_Section_Header_Type__Program_Data;
        }
        else if (String8__match_exact(string, String8__literal("nobits")))
        {
                result = ELF_Section_Header_Type__No_Data;
        }
        else if (String8__match_exact(string, String8__literal("note")))
        {
                result = ELF_Section_Header_Type__Notes;
        }
        else if (String8__match_exact(string, String8__literal("init_array")))
        {
                result = ELF_Section_Header_Type__INIT_ARRAY;
        }
        else if (String8__match_exact(string, String8__literal("fini_array")))
        {
                result = ELF_Section_Header_Type__FINI_ARRAY;
        }
        else if (String8__match_exact(string, String8__literal("preinit_array")))
        {
                result = ELF_Section_Header_Type__PREINIT_ARRAY;
        }
        else
        {
                result = ELF_Section_Header_Type__Invalid;
        }

        return result;
}

internal void
ELF_identifier_fill(U8 identifier[ELF_id_size])
{
        identifier[ELF_ID_Magic__Index]     = ELF_ID_Magic__0;
        identifier[ELF_ID_Magic__Index + 1] = ELF_ID_Magic__1;
        identifier[ELF_ID_Magic__Index + 2] = ELF_ID_Magic__2;
        identifier[ELF_ID_Magic__Index + 3] = ELF_ID_Magic__3;

        identifier[ELF_ID_Class__Index]     = ELF_ID_Class__64;
        identifier[ELF_ID_Data__Index]      = ELF_ID_Data__2LSB;
        identifier[ELF_ID_Version__Index]   = ELF_ID_Version__Current;

        identifier[ELF_ID_OS_ABI__Index]    = ELF_ID_OS_ABI__Linux;

        return;
}

typedef struct Section_Descriptor Section_Descriptor;
struct Section_Descriptor
{
        String8 name;
        U32     type;
        U64     flags;
};

global const Section_Descriptor Section_Descriptor__default = { String8__literal(""), ELF_Section_Header_Type__default, ELF_Section_Header_Flags__default };

// Reference: https://gabi.xinuos.com/v42/elf/03-sheader.html#special-sections
global const Section_Descriptor Section_Descriptor__table[] =
{
{ String8__literal(".bss"),              ELF_Section_Header_Type__No_Data,             ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".comment"),          ELF_Section_Header_Type__Program_Data,        0                                                                                                 },
{ String8__literal(".data"),             ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".data1"),            ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".debug"),            ELF_Section_Header_Type__Program_Data,        0                                                                                                 },
{ String8__literal(".dynamic"),          ELF_Section_Header_Type__Dynamic_Linking,     ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".dynstr"),           ELF_Section_Header_Type__Strings_Table,       ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".dynsym"),           ELF_Section_Header_Type__DYNSYM,              ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".fini"),             ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR                             },
{ String8__literal(".fini_array"),       ELF_Section_Header_Type__FINI_ARRAY,          ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".got"),              ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".hash"),             ELF_Section_Header_Type__Symbols_Hash_Table,  ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".init"),             ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR                             },
{ String8__literal(".init_array"),       ELF_Section_Header_Type__INIT_ARRAY,          ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".interp"),           ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".line"),             ELF_Section_Header_Type__Program_Data,        0                                                                                                 },
{ String8__literal(".note"),             ELF_Section_Header_Type__Notes,               0                                                                                                 },
{ String8__literal(".plt"),              ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR                             },
{ String8__literal(".preinit_array"),    ELF_Section_Header_Type__PREINIT_ARRAY,       ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE                                 },
{ String8__literal(".rodata"),           ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".rodata1"),          ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC                                                                   },
{ String8__literal(".shstrtab"),         ELF_Section_Header_Type__Strings_Table,       0                                                                                                 },
{ String8__literal(".strtab"),           ELF_Section_Header_Type__Strings_Table,       0                                                                                                 },
{ String8__literal(".symtab"),           ELF_Section_Header_Type__Symbols_Table,       0                                                                                                 },
{ String8__literal(".symtab_shndx"),     ELF_Section_Header_Type__SYMTAB_SHNDX,        0                                                                                                 },
{ String8__literal(".tbss"),             ELF_Section_Header_Type__No_Data,             ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE | ELF_Section_Header_Flags__TLS },
{ String8__literal(".tdata"),            ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE | ELF_Section_Header_Flags__TLS },
{ String8__literal(".tdata1"),           ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__WRITE | ELF_Section_Header_Flags__TLS },
{ String8__literal(".text"),             ELF_Section_Header_Type__Program_Data,        ELF_Section_Header_Flags__ALLOC | ELF_Section_Header_Flags__EXECINSTR                             },
{ String8__literal(".riscv.attributes"), ELF_Section_Header_Type__RISCV_Attributes,    0                                                                                                 }
};

typedef struct Section_Descriptor_Match Section_Descriptor_Match;
struct Section_Descriptor_Match
{
        Section_Descriptor descriptor;
        B32                match;
};

internal Section_Descriptor const *
Section_Descriptor__lookup(String8 name)
{
        U8 index = 0;
        Section_Descriptor const *result = 0;
        Section_Descriptor const *current = 0;
        for (;;)
        {
                B32 break_should = result || index >= array_count_m(Section_Descriptor__table);
                if (break_should)
                {
                        break;
                }

                current = &Section_Descriptor__table[index];
                result = String8__match_exact(current->name, name) ? current : 0;
                index += 1;
        }

        if (!result) { result = &Section_Descriptor__default; }

        return result;
}

#endif // OBJECT_ELF_UTILS_H
