#ifndef CORE_SYMBOL_H
#define CORE_SYMBOL_H

#define DOT_SYMBOL_NAME ".L0\x01"
#define DOT_SYMBOL_HASH 0

global String8 dot_symbol_string = { .data = (U8 *)DOT_SYMBOL_NAME, .count = sizeof(DOT_SYMBOL_NAME) };

// TODO: I don't like that this is the only file that depends on object/ directory due to ELF64_Symbol.

// TODO: review some of this variants, they're taken from GAS but not always used.
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

	// Whether this symbol has been replaced with another `.set` directive, and as such it should NOT be written in
	// the object file, but should be kept because some fixups depends on it.
	Symbol_Flags__Redefined                   = 1 << 12,
}
Symbol_Flags;

typedef struct Symbol_Ref Symbol_Ref;
struct Symbol_Ref
{
	Fragment *fragment;
	ELF64_Symbol  elf;
	// Where the symbol has been declared.
	U32 location;
	// The index of the expression which defines its value, if known.
	U32 expression_index;
	Symbol_Flags flags;
};


typedef struct Symbol Symbol;
struct Symbol
{
	String8     name;
	Symbol_Ref  value;
};

typedef struct Symbols_Trie Symbols_Trie;
struct Symbols_Trie
{
	String8       name;
	Symbol_Ref    symbol;
	Symbols_Trie *children[4];
};

typedef struct Symbols_Trie_Chunk Symbols_Trie_Chunk;
struct Symbols_Trie_Chunk
{
	Symbols_Trie_Chunk *next;
	Symbols_Trie       *nodes;
	U64 count;
	U64 capacity;
};

#define Symbols_Trie_Chunk__capacity_default 4096

typedef struct Symbols_Trie_Chunk_List Symbols_Trie_Chunk_List;
struct Symbols_Trie_Chunk_List
{
	U64 count;
	Symbols_Trie_Chunk *first;
	Symbols_Trie_Chunk *last;
};

typedef struct Symbols_Table Symbols_Table;
struct Symbols_Table
{
	Arena        *arena;
	Symbols_Trie *root;
	Symbols_Trie_Chunk_List *chunks;
};


#endif // CORE_SYMBOL_H
