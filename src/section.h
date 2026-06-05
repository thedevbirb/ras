#ifndef SECTION_H
#define SECTION_H


// TODO: make some stuff into elf if applicable

// NOTE: programs can have a lot of sections. In particular compilers may emit one section per function
// `-ffunction-section` that allows them to eliminate dead functions at link time. This means sections should be stored
// in an hashmap. Moreover, each section can grow its data.

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
	[ELF_Section__RISCV_Attributes]  = 1
};

global const U8 ELF_Section_relocations[ELF_Section__COUNT] =
{
	[ELF_Section__Text]             = ELF_Section__Relocations_Text,
	[ELF_Section__Data]             = ELF_Section__Relocations_Data,
	[ELF_Section__Read_Only_Data]   = ELF_Section__Relocations_Read_Only_Data,
};

internal U32
Hashmap_hash(String8 key)
{
	U32 hash = 2166136261u;

	for (U64 i = 0; i < key.count; i++)
	{
		hash ^= key.data[i];
		hash *= 16777619u;
	}

	return hash;
}

// A data structure modelling an object file section, in memory.
//
// Helpful references:
//
// 1. https://refspecs.linuxfoundation.org/elf/elf.pdf
// 2. https://sourceware.org/git/?p=glibc.git;a=blob;f=elf/elf.h

// typedef struct Object_File_Section Object_File_Section;
// struct Object_File_Section
// {
// 	Arena       *arena;
// 	String8      buffer;
// 	ELF_Section  section_index;
// 	U32          offset; // Also known as "location counter", but it's just a byte offset.
// 	U8	     alignment;
// };

// INVARIANT: the arena is used write to the fragment list only.
typedef struct Section Section;
struct Section
{
	Arena         *arena;
	Fragment_List  fragment_list;
	Fixup_List     fixup_list;
	String8        name;
	ELF_Section    index;
	U8             alignment;

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
	U32                       index_next;
};

internal Sections_Table *
Sections_Table__default(void);

internal void
Sections_Table__add_common(Sections_Table *);

// internal void
// Object_File_Section_initialize(Object_File_Section *section, ELF_Section section_index, Arena *arena);
//
// internal Object_File_Section *
// Object_File_Section_create_all(Arena *arena, U32 input_size);
//
// internal void
// Object_File_Section_align(Object_File_Section *section, U8 alignment);
//
// // Write and align, returning the offset where data has been written.
// U32
// Object_File_Section_write_bytes(Object_File_Section *section, U8 *data, U64 count);
//
// U32
// Object_File_Section_write(Object_File_Section *section, void *data, U64 size, U64 count);
//
// U32
// Object_File_Section_relocation_write(Object_File_Section *section, ELF64_Relocation_Addend *relocation);

#endif // SECTION_H
