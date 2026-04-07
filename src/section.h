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
			arena_dedicated = Arena_alloc_m(.commit_size = input_size, .flags = Arena_Flags__No_Chain);
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


// ELF64_Symbol table (.symtab) content invariants:
//
// Entry 0: all zeros (null symbol).
// All STB_LOCAL entries before all STB_GLOBAL/STB_WEAK entries.
// One STT_SECTION entry per section, with st_name = 0, st_value = 0, st_shndx = that section's index. These are local.
// STT_FILE entry (if present): st_shndx = SHN_ABS, st_info = ELF64_ST_INFO(STB_LOCAL, STT_FILE).
// Undefined/external symbols: st_shndx = SHN_UNDEF, st_value = 0.
// Absolute symbols (.equ): st_shndx = SHN_ABS.
// Each entry's st_name is a valid offset into .strtab pointing to a null-terminated string.
//
// When I find a symbol or label I have to both put the value inside the string table, avoiding duplicates and in the
// symbol table.

// The symbols table and the string table are coupled, meaning there should be a one to one correspondence between the
// two. This in-memory representation encodes both, by giving a key-value map which records order of insertion and
// tracks the growing size as null-terminated strings to compute ELF64 symbols string table offset.
//
// At a later stage, this can be used to write in both section to produce the ELF file.
typedef struct Symbols_Table_Entry Symbols_Table_Entry;
struct Symbols_Table_Entry
{
	String8 key;
	ELF64_Symbol  value;
	B32     used;
	// The index in the Symbols_Table.entries in which this entry has been inserted.
	U32     index;
};

global Symbols_Table_Entry symbols_table_entry_none = {0};

// Symbols table which also tracks order of insertions via an array of slots.
typedef struct Symbols_Table Symbols_Table;
struct Symbols_Table
{
	Arena   *arena;
	Symbols_Table_Entry *entries;
	U32     *slots;

	U32 string_table_section_size;
	U32 capacity;
	U32 count;
};

internal void
Symbols_Table_initialize(Symbols_Table *map, Arena *arena)
{
	map->arena    = arena;
	map->capacity = 64;
	map->count    = 0;
	map->entries  = Arena_push_array_m(arena, Symbols_Table_Entry, map->capacity);
	map->slots    = Arena_push_array_m(arena, U32, map->capacity);
	os_memory_zero(map->entries, sizeof(Symbols_Table_Entry) * map->capacity);
}

internal B32
Symbols_Table_find_slot(Symbols_Table *map, String8 key, U32 *slot_out)
{
	assert_always_m(map->capacity && "uninitialized map");

	U32 hash  = Hashmap_hash(key);
	U32 index = hash & (map->capacity - 1);
	B32 key_found = 0;

	for (;;)
	{
		Symbols_Table_Entry *entry = &map->entries[index];

		B32 empty = !entry->used;
		key_found = !empty &&
		            entry->key.count == key.count &&
		            os_memory_match(entry->key.data, key.data, key.count) == 0;

		B32 break_should = empty || key_found;
		if (break_should)
		{
			*slot_out = index;
			break;
		}

		index = (index + 1) & (map->capacity - 1);
	}

	return key_found;
}

internal void
Symbols_Table_grow(Symbols_Table *map)
{
	U32 capacity_new = map->capacity * 2;

	// We perform a copy of just the metadata and the pointers, not the heap allocated data.
	Symbols_Table map_old;
	os_memory_copy(&map_old, map, sizeof(Symbols_Table));

	Symbols_Table_Entry *entries_new = Arena_push_array_m(map->arena, Symbols_Table_Entry, capacity_new);
	U32 *slots_new = Arena_push_array_m(map->arena, U32, capacity_new);

	map->entries   = entries_new;
	map->slots     = slots_new;
	map->capacity  = capacity_new;
	map->count     = 0;

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= map_old.capacity;
		if (break_should) { break; }

		// Re-insert in order of insertion.
		U32 slot_old = map_old.slots[index];
		Symbols_Table_Entry *entry_old = &map_old.entries[slot_old];
		if (entry_old->used)
		{
			// Find a new slot, and put it there.
			U32 slot = 0;
			B32 found = Symbols_Table_find_slot(map, entry_old->key, &slot);
			assert_always_m(!found && "map contains duplicates");

			Symbols_Table_Entry *entry = &map->entries[slot];
			entry->key   = entry_old->key;
			entry->value = entry_old->value;
			entry->used  = 1;
			entry->index = map->count;

			map->slots[map->count] = slot;
			map->count  += 1;
		}
		index += 1;
	}

	return;
}

internal Vec2_U32 // Slot, found
Symbols_Table_put(Symbols_Table *map, String8 key, ELF64_Symbol value)
{
	assert_always_m(map->entries && "uninitialized hashmap");

	if ((map->count * 100) >= (map->capacity * 70))
	{
		Symbols_Table_grow(map);
	}

	U32 slot  = 0;
	U32 found = (U32)Symbols_Table_find_slot(map, key, &slot);

	value.string_table_offset = map->string_table_section_size;

	Symbols_Table_Entry *entry = &map->entries[slot];
	entry->key   = key;
	entry->value = value;
	entry->used  = 1;
	entry->index = map->count;

	if (!found)
	{
		map->slots[map->count] = slot;
		map->count  += 1;
		map->string_table_section_size = key.count + 1; // null-termination
	}

	Vec2_U32 slot_and_found = { .x = slot, .y = found};

	return slot_and_found;
}

// TODO: decide whether to return the non-pointer version. In practice, entry will be at least zero-initialized.

internal Symbols_Table_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key)
{
	Symbols_Table_Entry *result = 0;

	if (map->entries)
	{
		U32 slot  = 0;
		B32 found = Symbols_Table_find_slot(map, key, &slot);

		if (found)
		{
			result = &map->entries[slot];
		}
	}

	return result;
}


#endif // SECTION_H
