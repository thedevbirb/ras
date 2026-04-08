#ifndef SYMBOL_H
#define SYMBOL_H

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
	String8       key;
	ELF64_Symbol  value;
	B32	      used;
	// The index in the Symbols_Table.entries in which this entry has been inserted.
	U32           index;
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
Symbols_Table_initialize(Symbols_Table *map, Arena *arena);

internal B32
Symbols_Table_find_slot(Symbols_Table *map, String8 key, U32 *slot_out);

internal void
Symbols_Table_grow(Symbols_Table *map);

internal Vec2_U32 // Slot, found
Symbols_Table_put(Symbols_Table *map, String8 key, ELF64_Symbol value);

// TODO: decide whether to return the non-pointer version. In practice, entry will be at least zero-initialized.

internal Symbols_Table_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key);

#endif // SYMBOL_H

