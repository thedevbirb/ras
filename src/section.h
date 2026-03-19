#ifndef SECTION_H
#define SECTION_H

// A data structure modelling an object file section, in memory.

typedef enum ELF64_Section_Header_Type
{
	ELF64_Section_Header_Type__Null             = 0,
	ELF64_Section_Header_Type__Program_Bits     = 1,
	ELF64_Section_Header_Type__Symbols_Table    = 2,
	ELF64_Section_Header_Type__String_Table     = 3,
	ELF64_Section_Header_Type__Relocations      = 4,
	ELF64_Section_Header_Type__Note             = 7,
	ELF64_Section_Header_Type__No_Bits          = 8,
	ELF64_Section_Header_Type__RISCV_Attributes = 0x70000003
}
ELF64_Section_Header_Type;

typedef enum ELF64_Section
{
	ELF64_Section__Null = 0,
	ELF64_Section__Text,
	ELF64_Section__Data,
	ELF64_Section__Read_Only_Data,
	ELF64_Section__BSS,
	ELF64_Section__Relocations_Text,
	ELF64_Section__Relocations_Data,
	ELF64_Section__Symbols_Table,
	ELF64_Section__String_Table,
	ELF64_Section__Section_Names,
	ELF64_Section__RISCV_Attributes,
	// ELF64_Section__Note_GNU_Stack,
	ELF64_Section__COUNT,
}
ELF64_Section;

global const char *ELF64_Section_strings[ELF64_Section__COUNT] =
{
	[ELF64_Section__Null]             = "",
	[ELF64_Section__Text]             = ".text",
	[ELF64_Section__Data]             = ".data",
	[ELF64_Section__Read_Only_Data]   = ".rodata",
	[ELF64_Section__BSS]              = ".bss",
	[ELF64_Section__Relocations_Text] = ".rela.text",
	[ELF64_Section__Relocations_Data] = ".rela.data",
	[ELF64_Section__Symbols_Table]    = ".symtab",
	[ELF64_Section__String_Table]     = ".strtab",
	[ELF64_Section__Section_Names]    = ".shstrtab",
	[ELF64_Section__RISCV_Attributes] = ".riscv.attributes",
	// [ELF64_Section__Note_GNU_Stack = ".note.GNU-stack",
};

ELF64_Section_Header_Type ELF64_Section_Header_Type_from_ELF64_Section[ELF64_Section__COUNT] =
{
	[ELF64_Section__Null]              = ELF64_Section_Header_Type__Null,
	[ELF64_Section__Text]              = ELF64_Section_Header_Type__Program_Bits,
	[ELF64_Section__Data]              = ELF64_Section_Header_Type__Program_Bits,
	[ELF64_Section__Read_Only_Data]    = ELF64_Section_Header_Type__Program_Bits,
	[ELF64_Section__BSS]               = ELF64_Section_Header_Type__No_Bits,
	[ELF64_Section__Relocations_Text]  = ELF64_Section_Header_Type__Relocations,
	[ELF64_Section__Relocations_Data]  = ELF64_Section_Header_Type__Relocations,
	[ELF64_Section__Symbols_Table]     = ELF64_Section_Header_Type__Symbols_Table,
	[ELF64_Section__String_Table]      = ELF64_Section_Header_Type__String_Table,
	[ELF64_Section__Section_Names]     = ELF64_Section_Header_Type__String_Table,
	[ELF64_Section__RISCV_Attributes]  = ELF64_Section_Header_Type__RISCV_Attributes,
};

// Default value for section alignments.
global const U64 ELF64_Section_alignments[ELF64_Section__COUNT] =
{
	[ELF64_Section__Null]              = 0,
	[ELF64_Section__Text]              = 4,
	[ELF64_Section__Data]              = 8,
	[ELF64_Section__Read_Only_Data]    = 8,
	[ELF64_Section__BSS]               = 8,
	[ELF64_Section__Relocations_Text]  = 8,
	[ELF64_Section__Relocations_Data]  = 8,
	[ELF64_Section__Symbols_Table]     = 8,
	[ELF64_Section__String_Table]      = 1,
	[ELF64_Section__Section_Names]     = 1,
	[ELF64_Section__RISCV_Attributes]  = 1
};

typedef struct Object_File_Section Object_File_Section;
struct Object_File_Section
{
	String8        buffer;
	ELF64_Section  section;
	U32            offset; // Also known as "location counter", but it's just a byte offset.
	U8	       alignment;
};

internal Object_File_Section *
Object_File_Section_create_all(Arena *arena, U32 input_size)
{
	Object_File_Section *sections = Arena_push_array_m(arena, Object_File_Section, ELF64_Section__COUNT);

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= ELF64_Section__COUNT;
		if (break_should)
		{
			break;
		}

		U8 *data = 0;
		B32 section_empty = ELF64_Section__Null || index == ELF64_Section__BSS;
		if (!section_empty)
		{
			data = Arena_push_array_m(arena, U8, input_size);
		}

		Object_File_Section *section = &sections[index];

		section->buffer.data     = data;
		section->buffer.count    = input_size;
		section->section         = index;
		section->alignment       = ELF64_Section_alignments[index];

		index += 1;
	}
	return sections;
}

internal void
Object_File_Section_align(Object_File_Section *section, U8 power_two)
{
	assert_always_m(power_two <= 7 && "unexpectedly large alignment > 128");
	U32 alignment = 1 << power_two;
	U32 mask      = alignment - 1;
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

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= padding;
		if (break_should)
		{
			break;
		}

		section->buffer.data[section->offset] = 0;
		section->offset += 1;
		index += 1;
	}

	section->alignment = max_m(section->alignment, alignment);
	return;
}

#endif // SECTION_H
