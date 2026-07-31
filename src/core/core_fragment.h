#ifndef CORE_FRAGMENT_H
#define CORE_FRAGMENT_H

// Forward declaration for pointer use.
typedef struct Expression Expression;

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
        U8 opaque[16];
};
assert_static_m(sizeof(Relax_Info) == 16, Relax_Info__sizeof_check);

// The fragment structure represents a portion of assembly code. The most generic way to view it is a container of some
// known number of bytes, followed by some variable number of bytes.
//
// Code is made up of instructions, which can be _relaxable_, meaning that they can expand (or shrink) in size to
// accomodate for example large jumps, whose final distance cannot be known upfront. This gives the variable tail of the
// fragment mentioned earlier.
//
// Fragments are composed of an header (the `Fragment` struct) along with opaque bytes which contain the actual
// instruction encoding. They're intended to be stored contiguously in memory, i.e. something like `fragment ++ data`.
typedef struct Fragment Fragment;
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

global Fragment Fragment__nil = { .next = &Fragment__nil };

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

// Push a variable amount of bytes, capped by `Fragment__data_variable_size_max`, into the fragment variable buffer.
// This operations seals the current fragment with the provided information, and then creates a blank one.
internal void
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
internal void Fragment__wane(Fragment *fragment);

#endif // CORE_FRAGMENT_H
