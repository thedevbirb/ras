#ifndef CORE_SECTION_H
#define CORE_SECTION_H


// TODO(refactor): make some stuff into elf if applicable

typedef enum ELF_Section
{
        ELF_Section__None = 0,
        ELF_Section__Text,
        ELF_Section__Data,
        ELF_Section__Read_Only_Data,
        ELF_Section__BSS,
        ELF_Section__Symbols_Table,
        ELF_Section__Strings_Table,
        ELF_Section__Section_Names,
        ELF_Section__RISCV_Attributes,
        ELF_Section__Relocations_Text,
        ELF_Section__Relocations_Data,
        ELF_Section__Relocations_Read_Only_Data,
        ELF_Section__Absolute,
        ELF_Section__COUNT,
}
ELF_Section;

global const char *ELF_Section_strings[ELF_Section__COUNT] =
{
        [ELF_Section__None]                       = "",
        [ELF_Section__Text]                       = ".text",
        [ELF_Section__Data]                       = ".data",
        [ELF_Section__Read_Only_Data]             = ".rodata",
        [ELF_Section__BSS]                        = ".bss",
        [ELF_Section__Symbols_Table]              = ".symtab",
        [ELF_Section__Strings_Table]              = ".strtab",
        [ELF_Section__Section_Names]              = ".shstrtab",
        [ELF_Section__RISCV_Attributes]           = ".riscv.attributes",
        [ELF_Section__Relocations_Text]           = ".rela.text",
        [ELF_Section__Relocations_Data]           = ".rela.data",
        [ELF_Section__Relocations_Read_Only_Data] = ".rela.rodata",
        [ELF_Section__Absolute]                   = "*ABS*",
        // [ELF_Section__Note_GNU_Stack = ".note.GNU-stack",
};

ELF_Section_Header_Type ELF_Section_Header_Type_from_ELF_Section[ELF_Section__COUNT] =
{
        [ELF_Section__None]                       = ELF_Section_Header_Type__None,
        [ELF_Section__Text]                       = ELF_Section_Header_Type__Program_Data,
        [ELF_Section__Data]                       = ELF_Section_Header_Type__Program_Data,
        [ELF_Section__Read_Only_Data]             = ELF_Section_Header_Type__Program_Data,
        [ELF_Section__BSS]                        = ELF_Section_Header_Type__No_Data,
        [ELF_Section__Symbols_Table]              = ELF_Section_Header_Type__Symbols_Table,
        [ELF_Section__Strings_Table]              = ELF_Section_Header_Type__Strings_Table,
        [ELF_Section__Section_Names]              = ELF_Section_Header_Type__Strings_Table,
        [ELF_Section__Relocations_Text]           = ELF_Section_Header_Type__Relocations,
        [ELF_Section__Relocations_Data]           = ELF_Section_Header_Type__Relocations,
        [ELF_Section__Relocations_Read_Only_Data] = ELF_Section_Header_Type__Relocations,
        [ELF_Section__Absolute]                   = ELF_Section_Header_Type__None,
};

// Default value for section alignments.
global const U8 ELF_Section_alignments[ELF_Section__COUNT] =
{
        [ELF_Section__None]              = 0,
        [ELF_Section__Text]              = 4,
        [ELF_Section__Data]              = 8,
        [ELF_Section__Read_Only_Data]    = 8,
        [ELF_Section__BSS]               = 8,
        [ELF_Section__Relocations_Text]  = 8,
        [ELF_Section__Relocations_Data]  = 8,
        [ELF_Section__Symbols_Table]     = 8,
        [ELF_Section__Strings_Table]     = 1,
        [ELF_Section__Section_Names]     = 1,
        [ELF_Section__RISCV_Attributes]  = 1,
        [ELF_Section__Absolute]          = 0
};

global const U8 ELF_Section_relocations[ELF_Section__COUNT] =
{
        [ELF_Section__Text]             = ELF_Section__Relocations_Text,
        [ELF_Section__Data]             = ELF_Section__Relocations_Data,
        [ELF_Section__Read_Only_Data]   = ELF_Section__Relocations_Read_Only_Data,
};

// A data structure modelling an object file section, in memory.
typedef struct Section Section;
struct Section
{
        Section             *previous;
        Section             *next;

        Fragments            fragments;
        String8              name;
        U32                  location;
        U32                  index;
        ELF64_Section_Header elf;
};

global Section Section__zero = {0};

typedef struct Sections_Trie Sections_Trie;
struct Sections_Trie
{
        Section         section;
        Sections_Trie  *children[4];
};

// Assumptions:
//
// 1. It's append only. In an assembler sections are only created, and might be modified.
typedef struct Sections_Table Sections_Table;
struct Sections_Table
{
        Arena                    *arena;
        Sections_Trie            *root;

        // The underlying doubly-linked list collection. It contains only non-virtual sections, so it excludes the
        // undefined, absolute and common section.
        Section                  *first;
        Section                  *last;

        Section                  *current;
        Section                  *undefined;
        Section                  *absolute;
        // TODO(common): unsupported;
        Section                  *common;
        U32                       count;
};

internal Sections_Table *
Sections_Table__default(void);

internal void
Sections_Table__add_common(Sections_Table *);

internal U32
Sections_Table__count(Sections_Table *sections_table);

// Forward declaration for pointer use.
typedef struct Expression Expression;

internal void
Section__add_jump_instruction
(
        Section           *section,
        U32                encoding,
        U8                 encoding_size,
        U32                location,
        U8                 worst_case_size,
        U8                 best_case_size,
        Expression   *expression,
        U8                 jump_instructions_size
);

internal void
Section__add_instruction_fixed
(
        Section *section,
        Fixup   *fixup,

        U32      encoding,
        U8       encoding_size,
        U32      location
);

internal void
Section__finish(Section *section);

#endif // CORE_SECTION_H
