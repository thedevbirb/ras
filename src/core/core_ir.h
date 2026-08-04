#ifndef CORE_IR_H
#define CORE_IR_H

// Contains the primary in-memory data structures of the assembler, i.e. its "IR" (internal representation):
// expressions, symbols, fragments, fixups and sections.

// Forward declarations of the core types.
typedef struct Expression     Expression;
typedef struct Symbol_Ref     Symbol_Ref;
typedef struct Fragment       Fragment;
typedef struct Fixup          Fixup;
typedef struct Section        Section;

//-----------------------------------------------------------------------------
// @Expression
//-----------------------------------------------------------------------------

// Enumeration which  can be used both for _both_ parsing information and evaluation information.
//
// Consider the expression `1 + 2`, which creates a tree rooted in `+`.
// Such root node will have `Expression_Kind__Add` regarding parsing information,
// since the token underlying the node contains a plus sign.
//
// However, when the expression is evaluated the root can be folded to a constant expression
// which value is `3`, and so we would track it as a `Expression_Kind__Constant` expression.
// The use of this enumeration for evaluation purposes is akin to GNU as `operatorT`.
//
// Similarly, in an expression like `undefined_1 - undefined_2`, since both symbols are unknown, the evaluation of the
// `-` node is `Evaluation_Kind__Subtract`, meaning that the expression evaluates to itself, a "subtract" expression.
//
// As such, after evaluating an expression, using `expression_evaluate` or `Symbol_Ref__resolve`, the
// `Expression.evaluation` field should NOT be zero, and in the worst unresolvable case equal to the `Expression.kind`
// field.
typedef enum Expression_Kind
{
        // The expression hasn't been parser/evaluated
        Expression_Kind__None,

        // Leaf nodes
        Expression_Kind__Constant,
        Expression_Kind__Symbol,

        // Unary operators
        Expression_Kind__Negate,            // -x
        Expression_Kind__Bitwise_Not,       // ~x
        Expression_Kind__Logical_Not,       // !x

        // Binary arithmetic
        Expression_Kind__Add,               // +
        Expression_Kind__Subtract,          // -
        Expression_Kind__Multiply,          // *
        Expression_Kind__Divide,            // /
        Expression_Kind__Modulo,            // %

        // Binary bitwise
        Expression_Kind__Bitwise_Or,        // |
        Expression_Kind__Bitwise_Xor,       // ^
        Expression_Kind__Bitwise_And,       // &
        Expression_Kind__Shift_Left,        // <<
        Expression_Kind__Shift_Right,       // >>

        // Binary comparison
        Expression_Kind__Equal,             // ==
        Expression_Kind__Not_Equal,         // !=
        Expression_Kind__Less_Than,         // <
        Expression_Kind__Less_Equal,        // <=
        Expression_Kind__Greater_Than,      // >
        Expression_Kind__Greater_Equal,     // >=

        // Binary logical
        Expression_Kind__Logical_And,       // &&
        Expression_Kind__Logical_Or,        // ||

        Expression_Kind__COUNT,
}
Expression_Kind;

internal B32 Expression_Kind__unary_is(Expression_Kind kind);
internal B32 Expression_Kind__equality_is(Expression_Kind kind);
internal B32 Expression_Kind__comparison_is(Expression_Kind kind);

internal Expression_Kind Expression_Kind__from_Token_Kind_binary(Token_Kind kind);
internal Expression_Kind Expression_Kind__from_Token_Kind_unary(Token_Kind kind);

internal S64 unary_evaluate(Expression_Kind kind, S64 a);
internal S64 operation_evaluation(Expression_Kind kind, S64 a, S64 b);

// Binding power levels for Pratt parsing, ordered lowest to highest.
// Even numbers: gaps allow left/right binding power distinction if needed.
typedef enum Binding_Power
{
        Binding_Power__None           =   0,
        Binding_Power__Logical_Or     =   2,
        Binding_Power__Logical_And    =   4,
        Binding_Power__Bitwise_Or     =   6,
        Binding_Power__Bitwise_Xor    =   8,
        Binding_Power__Bitwise_And    =  10,
        Binding_Power__Equality       =  12,
        Binding_Power__Comparison     =  14,
        Binding_Power__Shift          =  16,
        Binding_Power__Additive       =  18,
        Binding_Power__Multiplicative =  20,
        Binding_Power__Unary          = 100,
}
Binding_Power;

internal Binding_Power Binding_Power_from_Token_Kind(Token_Kind kind);

// An `Expression` contains information about both a parsed expression and its evaluation, where the latter can
// mutate as more information is providing during multiple evaluation rounds, like during the relaxation process.
typedef struct Expression Expression;
struct Expression
{
        Expression *next;
        // Location tracking. Consider `1 + 2` as an example.

        // Points to the location of the "root" token of the expression. For example, if the node is `+`, it would point
        // to its location.
        U32        location;
        // The location range of this expression. For example, if the node is `+` it would cover the whole subexpression
        // `1 + 2`.
        Range1_U32 location_range;

        // Evaluation-related fields, in the relocation friendly format `<symbol> + <addend>`.

        // The value of a constant expression, or an offset to be applied to `Expression.symbol`.
        S64              integer_value;
        // The symbol this expression evaluates to.
        Symbol_Ref      *symbol;

        // Parsing-related fields. Pointers to child expression nodes.

        Expression      *left;
        Expression      *right;

        // Parsing
        Expression_Kind  kind;
        Expression_Kind  evaluation;
};

typedef struct Expressions Expressions;
struct Expressions
{
        U64         count;
        Expression *first;
        Expression *last;
};


Expression *
Expressions_push_empty(Expressions *expressions, Arena *arena);

// Create a constant expression.
internal Expression *
Expressions__push_constant(Expressions *expressions, Arena *arena, S64 value);

// Create an expression based on a single symbol
internal Expression *
Expression__push_symbol(Expressions *expressions, Arena *arena, Symbol_Ref *symbol);

// Evaluate all expressions while finalizing symbols. See `Symbol_Ref__resolve`/`Symbols_Table__finalize`.
internal void
Expressions__finalize(Expressions *expressions, Diagnostics *diagnostics);

//-----------------------------------------------------------------------------
// @Symbols
//-----------------------------------------------------------------------------

#define INTERNAL_SYMBOL_PREFIX ".L"
#define DOT_SYMBOL_NAME (INTERNAL_SYMBOL_PREFIX "\x01")
#define DOT_SYMBOL_HASH 0
#define FAKE_LABEL_NAME (INTERNAL_SYMBOL_PREFIX "0 ")

global String8 dot_symbol_string = String8__literal(DOT_SYMBOL_NAME);
global String8 fake_label_string = String8__literal(FAKE_LABEL_NAME);

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
internal S64
Symbol_Ref__resolve(Symbol_Ref *symbol, Diagnostics *diagnostics, Resolve_Level level);

//-----------------------------------------------------------------------------
// @Fragment
//-----------------------------------------------------------------------------

// The state of the relaxable code contained within a fragment.
typedef U8 Relax_State;
enum
{
        // An open fragment which able to ingest more bytes by calling push operations on `Fragments`.
        // When this changes, push operations MUST be forbidden to correct byte ordering.
        Relax_State__None = 0,
        // Describes a fill pattern, consisting of:
        // - A repeat expression which if, omitted, defaults to zero.
        // - A pattern used to write the fill bytes, along with its size. If the size is smaller than the pattern bytes,
        //   the pattern is truncated. It's up to the caller to check it.
        //
        // The pattern will be saved in the variable data portion of the fragment.
        // A zero-ed version of this is used to seal fragments once full.
        Relax_State__Fill,
        // Describe an `.align` directive, constisting of:
        // - A alignment boundary, in bytes.
        // - A pattern used to write the fill bytes, along with its size. If the size is smaller than the pattern bytes,
        //   the pattern is truncated. It's up to the caller to check it.
        // - A maximum size to write for the alignment;
        //
        // The pattern will be saved in the variable data portion of the fragment.
        Relax_State__Align,
        // The fragment is relaxable due to a conditional (branch) or unconditional jump
        Relax_State__Jump,
};

// Buffer reserved for variable data. In RISC-V, every relax state would have at most 8 bytes of variable data:
// 1. Fill/Align pattern is at most what fits in a U64/S64.
// 2. A jump can expand to at most two uncompressed instructions.
#define Fragment__data_variable_size_max 8

typedef struct Relax_Info_Jump Relax_Info_Jump;
struct Relax_Info_Jump
{
        Expression *expression;
        // The fixup to be be adjusted after relaxation.
        Fixup      *fixup;
        U8          compressed_is;
        U8          unconditional_is;
        U8          instructions_total_size;
        U8          pad;
};

// Minimum union that provides just enough ergonomics of access.
typedef union Relax_Info Relax_Info;
union Relax_Info
{
        Relax_Info_Jump jump;
        Expression *fill_expression;
        struct
        {
                U32 boundary;
                U32 write_size_max;
        } alignment;
        U8 opaque[24];
};
assert_static_m(sizeof(Relax_Info) == 24, Relax_Info__sizeof_check);

// The fragment structure represents a portion of assembly code. The most generic way to view it is a container of some
// known number of bytes, followed by some variable number of bytes.
//
// Code is made up of instructions, which can be _relaxable_, meaning that they can expand (or shrink) in size to
// accomodate for example large jumps, whose final distance cannot be known upfront. This gives the variable tail of the
// fragment mentioned earlier.
//
// Fragments are composed of an header (the `Fragment` struct) along with opaque bytes which contain the actual
// instruction encoding. They're intended to be stored contiguously in memory, i.e. something like `fragment ++ data`.
typedef struct Fragments Fragments;
struct Fragment
{
        // The next fragment in the chain.
        Fragment         *next;
        // The data of the fragment.
        U8               *data;
        // Offset within the section
        U64               object_file_offset;
        // For relaxation algorithm.
        U64               object_file_offset_last;

        // The location in the source code where this fragment has been created.
        U32              location;

        // The size of `Fragment.data`.
        U32              data_size;

        Relax_Info       relax_info;
        Relax_State      relax_state;

        U8 data_variable[Fragment__data_variable_size_max];
        // How much of the variable buffer is used.
        U8 data_variable_size;
};

typedef struct Fragments Fragments;
struct Fragments
{
        // NOTE: exclusive use of this arena
        Arena    *arena;
        U64       count;
        Fragment *first;
        Fragment *last;
};

typedef struct Fill Fill;
struct Fill
{
        Expression *repeat;
        U64         pattern;
        U8          pattern_size;
};

typedef struct Alignment Alignment;
struct Alignment
{
        U64 pattern;
        U32 boundary;
        U32 write_size_max;
        U8  pattern_size;
};

// Append a new fragment at the end of the queue, and return it.
internal Fragment *
Fragments__push_empty_fragment(Fragments *fragments, U32 location);

// Ensure the next `size` bytes pushed will fit within the same fragment.
internal void
Fragments__ensure(Fragments *fragments, U32 size);

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size.
internal U8 *
Fragments__push(Fragments *fragments, U32 location, U32 size);

// Push a variable amount of bytes, capped by `Fragment__data_variable_size_max`, into the fragment. This operations
// seals the current fragment with the provided information and returns it, then creates a blank one.
internal Fragment *
Fragments__variable
(
        Fragments   *fragments,
        U32          location,
        Relax_Info   relax_info,
        Relax_State  relax_state,
        U8          *data_variable,
        U8           data_variable_size
);

// Seal the current fragment with a fill pattern.
internal void
Fragments__fill(Fragments *fragments, U32 location, Fill fill);

internal void
Fragments__align(Fragments *fragments, U32 location, Alignment alignment);

// Seal the current fragment by making it a fixed-size fill variant, with no tail.
internal void
Fragment__wane(Fragment *fragment);

internal void
Fragment__convert_to_fill(Fragment *fragment, Section *section, Expressions *expressions, Arena *arena);

// Compute the total size of the instructions needed to relax a jump fragment.
internal U8
Fragment__jump_instructions_total_size(Fragment *fragment, Section *section);

//-----------------------------------------------------------------------------
// @Fixup
//-----------------------------------------------------------------------------

#define Fixup__8_Bit  (Relocation_RISC_V__COUNT + 0)
#define Fixup__16_Bit (Relocation_RISC_V__COUNT + 1)

typedef U8 Fixup_Flags;
enum
{
	Fixup_Flags__None         = 0 << 0,
        // Whether the fix has been applied, and emitting a relocation for this `Fixup` is not needed. Note that this
        // doesn't prevent the emission of a `Relocation_RISC_V__Relax`, since the latter is an hint to the linker that
        // further shrinking could be done.
        Fixup_Flags__Done         = 1 << 0,
        Fixup_Flags__PC_Relative  = 1 << 1,
};


// TODO(medium): fixups are currently a bit of lifetime soup between `Fragment` and `Expression`. Since everything has
// almost static duration, this is not a problem, however it's something to track.
//
// Again, it should be in the same arena of the Symbols_Table.
struct Fixup
{
        // DLL needed to insert `Relocation_RISC_V__Relax` in between.
        Fixup       *next;
        Fixup       *previous;

        Fragment    *fragment;
        // Section     *section;

        Expression  *expression;
        // Offset in the fragment where data should be written. If bigger than `Fragment.data_size`, then it's in the
        // variable buffer.
        U32          offset;
        U16          relocation_type;
        // Size of the patch to be written.
        U8           fragment_write_size;
        U8           flags;
};

// Information relative to a `Relocation_RISC_V__PC_Relative_High_20` fixup.
typedef struct PC_Relative_High PC_Relative_High;
struct PC_Relative_High
{
        PC_Relative_High *next;
        Expression       *expression;
        Section          *section;
        U64               object_file_offset;
};

internal PC_Relative_High *
PC_Relative_High__find(PC_Relative_High *pc_relative_high, Section *section, U64 object_file_offset)
{
        PC_Relative_High *current = pc_relative_high;
        PC_Relative_High *result  = 0;
        for (;;)
        {
                if (result || !current)
                {
                        break;
                }

                result = current->section == section
                      && current->object_file_offset == object_file_offset
                       ? current : 0;

                current = current->next;
        }

        return result;
}

typedef struct Fixups Fixups;
struct Fixups
{
        U64    count;
        U64    unresolved;
        Fixup *first;
        Fixup *last;

        // A stack containing information about `Relocation_RISC_V__PC_Relative_High_20` fixups.
        PC_Relative_High *pc_relative_high;
};

// Fixup handling functions. These should be used after sections have been relaxed, and after all symbols and
// expressions have been `Symbol_Ref__finalize`d.

internal void
Fixup__apply_constant(Fixup *fixup, U32 patch_to_or_into_encoding);

internal void
Fixup__apply_jump(Fixup *fixup, U32(*encoding_callback)(S64), B32(*valid_immediate_callback)(S64), Diagnostics *diagnostics);

internal void
Fixup__apply(Fixup *fixup, Section *section, Arena *arena, Diagnostics *diagnostics);

internal void
Fixups__resolve(Section *section, Arena *arena, Diagnostics *diagnostics);

//-----------------------------------------------------------------------------
// @Section
//-----------------------------------------------------------------------------

global String8 section_name_text      = String8__literal(".text");
global String8 section_name_data      = String8__literal(".data");
global String8 section_name_bss       = String8__literal(".bss");
global String8 section_name_undefined = String8__literal("");
global String8 section_name_absolute  = String8__literal("*ABSOLUTE*");
global String8 section_name_common    = String8__literal("*COMMON*");

// A data structure modelling an object file section, in memory.
typedef struct Section Section;
struct Section
{
        Section             *previous;
        Section             *next;
        Symbol_Ref          *symbol;
        Fixups               fixups;
        Fragments            fragments;

        // https://gabi.xinuos.com/v42/elf/03-sheader.html#special-sections
        B32 special;

        // Output fields.

        // This can be reliably set only close to object file writing, unless the section is either  absolute, or
        // common. As such, use this for section comparison only against those. Otherwise, compare section pointers.
        U32                  index;
        ELF64_Section_Header elf;
};

internal void
Section__add_instruction_fixed
(
        Section *section,
        Fixup   *fixup,

        U32      encoding,
        U8       encoding_size,
        U32      location
);

internal void Section__finish(Section *section);
internal B32  Section__relax(Section *section, Arena *arena, Diagnostics *diagnostics);

//-----------------------------------------------------------------------------
// @Globals
//-----------------------------------------------------------------------------

// Sentinel fragment terminating every fragment chain.
global Fragment Fragment__nil = { .next = &Fragment__nil };

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

#endif // CORE_IR_H
