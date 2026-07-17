#ifndef CORE_SYMBOL_H
#define CORE_SYMBOL_H

#define DOT_SYMBOL_NAME ".L0\x01"
#define DOT_SYMBOL_HASH 0
#define FAKE_LABEL_NAME ".L0 "

// Forward declaration for pointer use.
typedef struct Expression Expression;

global String8 dot_symbol_string = { .data = (U8 *)DOT_SYMBOL_NAME, .count = sizeof(DOT_SYMBOL_NAME) };

// TODO(track): review some of this variants, they're taken from GAS but not always used.
typedef enum Symbol_Flags
{
        Symbol_Flags__None                       = 0 << 0,

        // This comes from GNU as, and as it suggests it marks whether the symbol is a local symbol. It is used because
        // a `symbolS` pointer can be casted into a fully-fledged `struct symbol`.
        Symbol_Flags__Local                      = 1 << 0,

        // Weather symbol has been written.
        Symbol_Flags__Written                    = 1 << 1,

        // Whether symbol value has been completely resolved (used during final pass over symbol table).
        //
        // TODO: I think `Symbol_Flags__Finalized` is a better term.
        Symbol_Flags__Finalized                   = 1 << 2,

        // Whether the symbol value is currently being resolved (used to detect loops in symbol dependencies).
        Symbol_Flags__Resolving                  = 1 << 3,

        // Whether the symbol value is used in a relocation. This is used to ensure that symbols used in relocations are
        // written out, even if they are local and would otherwise not be.
        // TODO(medium): actually use this.
        Symbol_Flags__Relocation                 = 1 << 4,

        // Whether the symbol is used as an operand or in an expression.
        Symbol_Flags__Used                       = 1 << 5,

        // Whether the symbol can be re-defined.
        Symbol_Flags__Volatile                   = 1 << 6 ,

        // Whether the symbol is a forward reference, and whether such has been determined.
        Symbol_Flags__Forward_Reference          = 1 << 7,
        Symbol_Flags__Forward_Reference_Resolved = 1 << 8,

        // Whether the symbol has been marked to be removed by a .symver directive.
        // TODO(refactor): I won't support the .symver directive, perhaps this can be collapsed with
        // `Symbol_Flags__Redefined/Symbol_Flags__Skip`
        Symbol_Flags__Removed                    = 1 << 9,

        // Whether the volatile symbol has been actually redefined, and as such it should NOT be written in the object
        // file, but should be kept because something depends on it.
        //
        // NOTE: GNU as doesn't need this flag because symbols are stored in a doubly linked list and previous version
        // of a symbol are simply removed from it. In our case, symbols are stored in a chunk list, so we need some data
        // to explicitly skip them.
        //
        // TODO(refactor): it may be that calling this flag `Symbol_Flags__Skip` is generic enough for multiple usecases.
        Symbol_Flags__Redefined                   = 1 << 12,
}
Symbol_Flags;

typedef struct Symbol_Ref Symbol_Ref;
struct Symbol_Ref
{
        // This is a reference to `Symbol.name`
        String8          *name;
        Section          *section;
        Fragment         *fragment;
        // The expression which defines its value, if appropriate.
        // Non-null ONLY on symbol definition using `.set`-like directives.
        Expression  *expression;
        // ELF value for this symbol, which can mean an offset for labels.
        U64 value;
        // Where the symbol has been declared.
        U32               location_range;
        Symbol_Flags      flags;

        U32 location;
        U32 string_table_offset;
        // Only 4 bits of it will be read.
        U8 type;
        // Only 4 bits of it will be read.
        U8 binding;
        U8 visibility;

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

typedef struct Label_Numeric Label_Numeric;
struct Label_Numeric
{
        U32 number;
        U32 instances;
};

typedef struct Label_Numeric_Chunk Label_Numeric_Chunk;
struct Label_Numeric_Chunk
{
        Label_Numeric_Chunk *next;
        Label_Numeric *nodes;
        U64 count;
        U64 capacity;
};

typedef struct Label_Numeric_Chunk_List Label_Numeric_Chunk_List;
struct Label_Numeric_Chunk_List
{
        U64 count;
        Label_Numeric_Chunk *first;
        Label_Numeric_Chunk *last;
};

#define Label_Numeric_Chunk__capacity_default 4096

typedef struct Symbols_Table Symbols_Table;
struct Symbols_Table
{
        // A dedicated arena for every data, including symbols names, that are saved here.
        Arena                    *arena;
        Symbols_Trie             *root;
        Symbols_Trie_Chunk_List  *chunks;
        Label_Numeric_Chunk_List *chunks_label;
};

internal Symbol_Ref *
Symbols_Table__internal_label(Symbols_Table *symbols_table, Section *section);

// Resolution levels for `Symbol_Ref__resolve`.
typedef enum Resolve_Level
{
	Resolve_Level__None             = 0,
        // Whether the resolve other symbol definitions encountered as well.
	Resolve_Level__Traverse = 1,
        // Whether symbols should be finalized after this resolution pass. Implies previous options.
	Resolve_Level__Finalize = 2,
}
Resolve_Level;

// Kinda based on GNU `as` `resolve_symbol_value`, although with different assumptions.
//
// 1. Labels don't have an expression. Their value can be read straight into `Symbol_Ref.value`.
// 2. `Symbol_Flags__Finalized` means the simplification pass reached an end, and the value can be read from
//    `Symbol_Ref.value`. Undefined symbols and similar should have value zero.
//
// NOTE that this will be called on every symbol.
// TODO(low): replace `expression_evalute` with this, more general version, by wrapping an expression into a
// stack-allocated symbol, since the core evaluation logic is shared.
internal S64
Symbol_Ref__resolve(Symbol_Ref *symbol, Arena *arena, Diagnostic_List *diagnostics, Resolve_Level level);

#endif // CORE_SYMBOL_H
