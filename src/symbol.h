#ifndef SYMBOL_H
#define SYMBOL_H

typedef enum Symbol_Flags
{
	Symbol_Flags__None                       = 0 << 0,

	/* Whether the symbol is a local_symbol.  */
	Symbol_Flags__Local                      = 1 << 0,

	/* Weather symbol has been written.  */
	Symbol_Flags__Written                    = 1 << 1,

	/* Whether symbol value has been completely resolved (used during final pass over symbol table).  */
	Symbol_Flags__Resolved                   = 1 << 2,

	/* Whether the symbol value is currently being resolved (used to detect loops in symbol dependencies).  */
	Symbol_Flags__Resolving                  = 1 << 3,

	/* Whether the symbol value is used in a reloc.  This is used to ensure that symbols used in relocs are written
	 * out, even if they are local and would otherwise not be.  */
	// TODO(high): actually use this, for example in branches.
	Symbol_Flags__Relocation                 = 1 << 4,

	/* Whether the symbol is used as an operand or in an expression.
	   NOTE:  Not all the backends keep this information accurate; backends which use this bit are responsible for
	   setting it when a symbol is used in backend routines.  */
	Symbol_Flags__Used                       = 1 << 5,

	/* Whether the symbol can be re-defined.  */
	Symbol_Flags__Volatile                   = 1 << 6 ,

	/* Whether the symbol is a forward reference, and whether such has
	   been determined.  */
	Symbol_Flags__Forward_Reference          = 1 << 7,
	Symbol_Flags__Forward_Reference_Resolved = 1 << 8,

	/* Whether the symbol has been marked to be removed by a .symver
	   directive.  */
	Symbol_Flags__Removed                    = 1 << 9,

	// Whether the symbol has been declared using a label or directive.
	Symbol_Flags__Declared                   = 1 << 10,

	Symbol_Flags__Dot                        = 1 << 11,
}
Symbol_Flags;

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

typedef struct Symbols_Table_Entry Symbols_Table_Entry;
struct Symbols_Table_Entry
{
	String8       key;
	ELF64_Symbol  elf;
	// The index of the `Statements` structure where this symbol has been declared, or zero is unknown.
	U32	      index_statement;
	// The index in the Symbols_Table.entries in which this entry has been inserted.
	U32           index;
	Symbol_Flags  flags;
};

global Symbols_Table_Entry symbols_table_entry_none = {0};

// Symbols table which also tracks order of insertions via an array of slots.
typedef struct Symbols_Table Symbols_Table;
struct Symbols_Table
{
	Arena   *arena;
	Symbols_Table_Entry *entries;
	U32     *slots;

	U32 capacity;
	U32 count;
	// With 10 labels, this pads well.
	U16 label_numeric_count[label_numeric_max];
};

// Allocates a initial buffer of zero-initialized values for saving symbols in the table.
void
Symbols_Table_initialize(Symbols_Table *map, Arena *arena);

Symbols_Table_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key);

// Akin to a put operation, growing the map if needed.
Symbols_Table_Entry *
Symbols_Table_reserve(Symbols_Table *map, String8 key);

#endif // SYMBOL_H

