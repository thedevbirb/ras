#ifndef CORE_FRAGMENT_H
#define CORE_FRAGMENT_H

// Forward declaration for pointer use.
typedef struct Expression_Node Expression_Node;

typedef U8 Relax_State;
enum
{
        // Used by listing code.
        //
        // This describes either an unitiliased or open fragment, which is able to ingest more bytes.
        // Sealing a fragment is marked by changing this state to something else. For fixed-size fragments, inspired by
        // GNU as, this is achieved by marking the fragment in a `Relax_State__Fill` of zero size, with no pattern,
        // instead of having a dedicated variant for it.
        //
        // TODO(medium): the above is achieved by calling something like `frag_wane`, which is currently unimplemented.
        Relax_State__None,
        // Describes a `.fill <repeat>, <size>, <pattern>` directive:
        // - `expression_node` contains the <repeat> expression.
        // - `size_variable` contains <size>.
        // - the variable bytes of the fragment contain the pattern.
        //
        // NOTE: GNU as further distinguishes between `rs_fill` and `rs_space`/`rs_space_nop`. In essence, `rs_fill` is
        // guaranteed to have a constant <repeat> count, while `rs_space`/`rs_space_nop` do contain expressions inside
        // the symbol field. However we don't need this discrimination, and we can just check whether the symbol field
        // constains a constant expression or not, and move on.
        Relax_State__Fill,
        // Describes a fill directive in a code section. Should be used in a `nops/.nops` directive.
        Relax_State__Fill_Nop,
        // Describe an `.align` directive:
        // - `expression_node` contains the constant expression with the power of two used for alignment.
        // - `subtype` contains the maximum number of bytes to skip when aligning, or zero if there is no maximum.
        // - `size_variable` contains the size of the byte pattern.
        Relax_State__Align,
        // Describe an `.align` directive, in a _code_ section. The fill pattern is handled by the backend.
        // - `expression_node` contains the constant expression with the power of two used for alignment.
        // - `subtype` contains the maximum number of bytes to skip when aligning, or zero if there is no maximum.
        // - `size_variable` contains the size of the byte pattern, but this is machine dependent.
        Relax_State__Align_Code,
        // Machine specific relaxable instruction (e.g. branches, etc.).
        Relax_State__Machine,
        Relax_State__COUNT,
};


// The fragment structure represents a portion of assembly code. The most generic way to view it is a container of some
// known number of bytes, followed by some variable number of bytes.
//
// Code is made up of instructions, which can be _relaxable_, meaning that they can expand (or shrink) in size to
// accomodate for example large jumps, whose distance cannot be known upfront until all symbols are resolved. This gives
// the variable tail of the fragment mentioned earlier.
//
// Fragments are composed of an header (the `Fragment` struct) along with opaque bytes which contain the actual
// instruction encoding. They're intended to be stored contiguously in memory, i.e. something like `fragment ++ data`.
typedef struct Fragment Fragment;
struct Fragment
{
        // The next fragment in the chain.
        Fragment         *next;
        U64               object_file_offset;
        // For relaxation algorithm.
        U64               object_file_offset_last;
        // Variably-sized fragments are tied to instructions which can expand and most probably
        // have an expression attached to them.
        Expression_Node *expression_node;
        // Variably-sized fragments are tied to instructions which can expand and most probably
        // have an expression attached to them.
        S64              expression_constant;
        // The location in the source code where this fragment has been created.
        U32              location;
        // The fixed number of bytes we have.
        U32              size_fixed;
        // Opaque 32 bits to store additional information contextual to the `type`.
        U32              subtype;
        // The variable number of bytes we have past the fixed ones.
        U8               size_variable;
        Relax_State      type;
};

typedef struct Fragment_List Fragment_List;
struct Fragment_List
{
        U64 count;
        Fragment *first;
        Fragment *last;
};

internal U64
Arena__ensure_contiguous_for_size(Arena *arena, U32 size);

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size.
//
// NOTE: since we're using an arena allocator, it might happen that the current chunk hasn't enough reserved size for it.
// In such case, we need to "seal" the current fragment and switch to another arena block, creating a new one.
internal U8 *
Fragment_List__push(Fragment_List *fragments, Arena *arena, U32 location, U32 size);

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size and increasing the fragment
// `size_fixed`.
internal U8 *
Fragment_List__fixed(Fragment_List *fragments, Arena *arena, U32 location, U32 size);

// Push a variable (`size_variable`) amount of bytes, capped by `size_max`, into the fragment.
// This effectively pushes `size_max` bytes into it, while accounting the size currently used by the relaxable
// instruction.
//
// This operations seals the current fragment with the provided information, and creates a blank one.
internal U8 *
Fragment_List__variable
(
        Fragment_List   *fragments,
        Arena           *arena,
        U32              location,
        U32              size_max,
        U32              size_variable,
        Expression_Node *expression_node,
        S64              expression_constant,
        U32              subtype,
        Relax_State      type
);

// Seal the current fragment with a fill pattern.
internal void
Fragment_List__fill(Fragment_List *fragments, Arena *arena, U32 location, Expression_Node *repeat_expression_node, S64 pattern, U8 size);

internal void
Fragment_List__align(Fragment_List *fragments, Arena *arena, U32 location, U32 power_of_two, U8 pattern, U8 alignment_max);

#endif // CORE_FRAGMENT_H
