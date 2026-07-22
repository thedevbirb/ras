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
//
// Helpful references:
//
// 1. https://refspecs.linuxfoundation.org/elf/elf.pdf
// 2. https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/elf.h

// INVARIANT: the arena is used write to the fragment list only.
typedef struct Section Section;
struct Section
{
        Arena               *arena;
        Fragments            fragments;
        Fixups               fixups;
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

typedef struct Sections_Trie_Chunk Sections_Trie_Chunk;
struct Sections_Trie_Chunk
{
        Sections_Trie_Chunk *next;
        Sections_Trie       *nodes;
        U64 count;
        U64 capacity;
};

global U64 Sections_Trie_Chunk__capacity_default = 4096;

typedef struct Sections_Trie_Chunk_List Sections_Trie_Chunk_List;
struct Sections_Trie_Chunk_List
{
        U64 count;
        Sections_Trie_Chunk *first;
        Sections_Trie_Chunk *last;
};



// Assumptions:
//
// 1. It's append only. In an assembler sections are only created, and might be modified.
typedef struct Sections_Table Sections_Table;
struct Sections_Table
{
        Arena                    *arena;
        Sections_Trie            *root;
        Sections_Trie_Chunk_List *chunks;
        Section                  *current;
        Section                  *undefined;
        Section                  *absolute;
        // TODO(common): unsupported for now;
        Section                  *common;
        U32                       index_next;
};

internal Sections_Table *
Sections_Table__default(void);

internal void
Sections_Table__add_common(Sections_Table *);

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
