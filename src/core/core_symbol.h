#ifndef CORE_SYMBOL_H
#define CORE_SYMBOL_H

#define INTERNAL_SYMBOL_PREFIX ".L"
#define DOT_SYMBOL_NAME (INTERNAL_SYMBOL_PREFIX "\x01")
#define DOT_SYMBOL_HASH 0
#define FAKE_LABEL_NAME (INTERNAL_SYMBOL_PREFIX "0 ")

global String8 dot_symbol_string = { .data = (U8 *)DOT_SYMBOL_NAME, .count = sizeof(DOT_SYMBOL_NAME) };

// Forward declaration for pointer use.
typedef struct Expression Expression;

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
        Symbol_Flags__Finalized                   = 1 << 2,

        // Whether the symbol value is currently being resolved (used to detect loops in symbol dependencies).
        Symbol_Flags__Resolving                  = 1 << 3,

        // Whether the symbol value is used in a relocation. This is used to ensure that symbols used in relocations are
        // written out, even if they are local and would otherwise not be.
        Symbol_Flags__Relocation                 = 1 << 4,

        // Whether the symbol is used as an operand or in an expression.
        Symbol_Flags__Used                       = 1 << 5,

        // Whether the symbol can be re-defined.
        Symbol_Flags__Volatile                   = 1 << 6 ,

        // Whether the symbol is a forward reference, and whether such has been determined.
        Symbol_Flags__Forward_Reference          = 1 << 7,
        Symbol_Flags__Forward_Reference_Resolved = 1 << 8,

        // The symbol is volatile and has been re-defined.
        Symbol_Flags__Redefined                   = 1 << 9,

        // Explicitly mark this symbol as to be omitted from the final symbol table.
        Symbol_Flags__Skip                   = 1 << 12,
}
Symbol_Flags;


typedef struct Symbol_Ref Symbol_Ref;
struct Symbol_Ref
{
        Symbol_Ref       *next;
        // This is a reference to `Symbol_Trie.name`
        String8          *name;
        Section          *section;
        Fragment         *fragment;
        // The expression which defines its value, if appropriate.
        Expression       *expression;
        // ELF value for this symbol, which can mean an offset for labels.
        U64 value;
        U64 size;
        Symbol_Flags      flags;

        U32 index;
        U32 string_table_offset;
        U32 location;
        // Only 4 bits of it will be read.
        U8 type;
        // Only 4 bits of it will be read.
        U8 binding;
        U8 visibility;
};

global Symbol_Ref Symbol_Ref__zero = {0};

global Symbol_Ref Symbol_Ref__undefined;
global Section Section__undefined =
{
        .symbol    = &Symbol_Ref__undefined,
        .fragments =
        {
                .first = &Fragment__nil,
                .last  = &Fragment__nil
        }
};
global Symbol_Ref Symbol_Ref__undefined =
{
        .name     = &section_name_undefined,
        .section  = &Section__undefined,
        .fragment = &Fragment__nil,
        .type     = 0,
        .flags    = Symbol_Flags__Used
};

global Symbol_Ref Symbol_Ref__common;
global Section Section__common =
{
        .symbol    = &Symbol_Ref__common,
        .fragments =
        {
                .first = &Fragment__nil,
                .last  = &Fragment__nil
        }
};
global Symbol_Ref Symbol_Ref__common =
{
        .section  = &Section__common,
        .fragment = &Fragment__nil,
        .type     = STT_SECTION
};

global Symbol_Ref Symbol_Ref__absolute;
global Section Section__absolute =
{
        .symbol    = &Symbol_Ref__absolute,
        .index     = ELF_Section_Index__Absolute,
        .fragments =
        {
                .first = &Fragment__nil,
                .last  = &Fragment__nil
        }
};
global Symbol_Ref Symbol_Ref__absolute =
{
        .name     = &section_name_absolute,
        .section  = &Section__absolute,
        .fragment = &Fragment__nil,
        .type     = STT_SECTION
};

typedef struct Label_Numeric Label_Numeric;
struct Label_Numeric
{
        Label_Numeric *next;
        U32            number;
        U32            instances;
};

typedef struct Symbol_Numeric Symbol_Numeric;
struct Symbol_Numeric
{
        Symbol_Ref    *symbol;
        Label_Numeric *label;
};

typedef struct Symbols_Trie Symbols_Trie;
struct Symbols_Trie
{
        String8       name;
        Symbol_Ref    symbol;
        Symbols_Trie *children[4];
};

typedef struct Symbols_Table Symbols_Table;
struct Symbols_Table
{
        // A dedicated arena for every data, including symbols names, that are saved here.
        Arena                    *arena;
        Symbols_Trie             *root;

        // Symbol_Ref               *first;
        // Symbol_Ref               *last;

        Symbol_Ref               *first;
        Symbol_Ref               *last;

        Label_Numeric            *label_numeric_first;
        Label_Numeric            *label_numeric_last;

        // Sections are symbols of type `STT_SECTION`, with their additional `Section` data, so they belong here.

        // The underlying doubly-linked list collection.
        Section                  *section_first;
        Section                  *section_last;

        Section                  *section_current;

        U32                       count;
        U32                       sections_count;

};

#define each_symbol_m(st, element) \
        (Symbol_Ref *element = (DLL_join_npz_m(0, st->local_last, st->global_first, next, previous), st->local_first); element; element = element->next)

internal Symbols_Trie *
symbols_trie_get(Symbols_Trie *trie, U64 hash, String8 name);

internal Symbols_Trie *
symbols_trie_get_or_default(Arena *arena, Symbols_Trie **root, U64 hash, String8 name);

internal Symbols_Trie *
symbols_trie_create(Arena *arena, Symbols_Trie **root, U64 hash, String8 name);

internal String8
label_numeric_string(Arena *arena, Label_Numeric label);

internal B32
Symbol_Ref__internal_is(Symbol_Ref *symbol);

// Symbols Table API

internal Symbol_Ref *
Symbols_Table__create_internal(Symbols_Table *symbols_table, Section *section);

// Create a section associated to the provided symbol.
internal void
Symbols_Table__create_section(Symbols_Table *symbols_table, Symbol_Ref *symbol);

internal Symbol_Ref *
Symbols_Table__create_section_riscv_attributes(Symbols_Table *symbols_table);

internal Symbol_Ref *
Symbols_Table__get(Symbols_Table *symbols_table, String8 name);

// Get or create a default symbol given its name. If it doesn't exist, the symbol is attached to the undefined section
// symbol, if available.
internal Symbol_Ref *
Symbols_Table__get_or_default(Symbols_Table *symbols_table, String8 name);

// Get or create a default numeric symbol (e.g. `1b`/`1f`). See `Symbols_Table__get_or_default`.
internal Symbol_Numeric
Symbols_Table__get_or_default_numeric(Symbols_Table *symbols_table, U32 number, B32 forward);

// Return the global dot symbol trie. It is recommended to always update the dot symbol before it's usage.
// See `Symbol_Ref__update_section`.
internal Symbols_Trie *
Symbols_Table__dot(Symbols_Table *symbols_table);

// Update `Section` and `Fragment` information of the given symbol.
internal void
Symbol_Ref__update_section(Symbol_Ref *symbol, Section *section);

// Create a clone of the given symbol.
internal Symbol_Ref *
Symbols_Table__clone(Symbols_Table *symbols_table, Symbol_Ref *symbol);

// Resolution levels for `Symbol_Ref__resolve`.
typedef enum Resolve_Level
{
	Resolve_Level__None     = 0,
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
Symbol_Ref__resolve(Symbol_Ref *symbol, Diagnostics *diagnostics, Resolve_Level level);

#endif // CORE_SYMBOL_H
