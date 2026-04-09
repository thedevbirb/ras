#ifndef SECTION_H
#define SECTION_H


// TODO: make some stuff into elf if applicable

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

global const U8 ELF_Section_from_Directive_Kind[Directive_Kind__COUNT] =
{
	[Directive_Kind__None]           = 0,
	[Directive_Kind__Section]        = 0,
	[Directive_Kind__Text]           = ELF_Section__Text,
	[Directive_Kind__Data]           = ELF_Section__Data,
	[Directive_Kind__Read_Only_Data] = ELF_Section__Read_Only_Data,
	[Directive_Kind__BSS]            = ELF_Section__BSS,
	[Directive_Kind__Globl]          = 0,
	[Directive_Kind__Byte]           = 0,
	[Directive_Kind__Word_Half]      = 0,
	[Directive_Kind__Word]           = 0,
	[Directive_Kind__Word_Double]    = 0,
	[Directive_Kind__Ascii]          = 0,
	[Directive_Kind__Asciz]          = 0,
	[Directive_Kind__Equality]       = 0,
	[Directive_Kind__Align]          = 0,
};

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

typedef struct Object_File_Section Object_File_Section;
struct Object_File_Section
{
	Arena       *arena;
	String8      buffer;
	ELF_Section  section_index;
	U32          offset; // Also known as "location counter", but it's just a byte offset.
	U8	     alignment;
};

internal void
Object_File_Section_initialize(Object_File_Section *section, ELF_Section section_index, Arena *arena)
{
	U8 *data = arena ? Arena_push_zero_m(arena) : 0;

	String8 buffer =
	{
		.data  = data,
		.count = 0,
	};
	*section = (Object_File_Section)
	{
		.arena         = arena,
		.buffer        = buffer,
		.section_index = section_index,
		.offset        = 0,
		.alignment     = ELF_Section_alignments[section_index],

	};

	return;
}

internal Object_File_Section *
Object_File_Section_create_all(Arena *arena, U32 input_size)
{
	Object_File_Section *sections = Arena_push_array_m(arena, Object_File_Section, ELF_Section__COUNT);

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= ELF_Section__COUNT;
		if (break_should)
		{
			break;
		}

		Arena *arena_dedicated = 0;
		B32 section_empty = ELF_Section__None || index == ELF_Section__BSS;
		if (!section_empty)
		{
			arena_dedicated = Arena_alloc_m(.reserve_size = input_size, .flags = Arena_Flags__No_Chain);
		}

		Object_File_Section_initialize(&sections[index], index, arena_dedicated);

		index += 1;
	}
	return sections;
}

internal void
Object_File_Section_align(Object_File_Section *section, U8 alignment)
{
	U32 mask         = alignment - 1;
	B32 power_two_or_zero_is = (alignment & (mask)) == 0 || alignment == 0;
	assert_always_m(power_two_or_zero_is && "alignment must be a power of two, or zero (no-op)");

	U32 offset_alignment_distance = section->offset & mask;
	// We mask again to handle the case where distance is zero, without branches.
	U32 padding = (alignment - offset_alignment_distance) & mask;

	// Examples:
	//
	// alignment                                      = 0b0100 (4)
	// mask                                           = 0b0011 (3)
	// offset                                         = 0b0111 (7)
	// offset_alignment_distance                      = 0b0111 & 0b0011 = 0b0011 (3)
	// alignment - offset_alignment_distance          = 0b0100 - 0b0011 = 0b0001 (1)
	//
	// alignment                                      = 0b0000_1000 (8)
	// mask                                           = 0b0000_0111 (7)
	// offset                                         = 0b0001_0000 (16)
	// offset_alignment_distance                      = 0b0001_0000 & 0b0000_0111 = 0b0000_0000 (0)
	// padding                                        = (0b0000_1000 - 0b0000_0000) & 0b0000_0111 = 0b0000_0000 (0)

	padding = padding & ((alignment == 0) - 1);

	Arena_push_array_m(section->arena, U8, padding);
	section->offset += padding;
	section->buffer.count += padding;

	section->alignment = max_m(section->alignment, alignment);
	return;
}

// Write and align, returning the offset where data has been written.
U32
Object_File_Section_write(Object_File_Section *section, U8 *data, U64 count)
{
	U32 offset_old = section->offset;
	U32 offset_new = offset_old + count + 1;

	Arena_push_array_m(section->arena, U8, count);

	os_memory_copy(section->buffer.data + section->offset, data, count);
	// Extra space for null-termination.
	section->offset = offset_new;
	section->buffer.count += count;
	Object_File_Section_align(section, section->alignment);

	return offset_old;
}

#endif // SECTION_H
