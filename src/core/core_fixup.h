#ifndef CORE_FIXUP_H
#define CORE_FIXUP_H

// Forward declaration for pointer use.
typedef struct Expression Expression;

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
typedef struct Fixup Fixup;
struct Fixup
{
        Fixup       *next;
        Fixup       *previous;

        Fragment    *fragment;

        Expression  *expression;
        // Pointer to location in the fragment fixed or variable data where the patch should be written.
        U8          *fragment_write_area;

        U16          section_index;
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
        U16               section_index;
        U64               object_file_offset;
};

internal PC_Relative_High *
PC_Relative_High__find(PC_Relative_High *pc_relative_high, U16 section_index, U64 object_file_offset)
{
        PC_Relative_High *current = pc_relative_high;
        PC_Relative_High *result  = 0;
        for (;;)
        {
                if (result || !current)
                {
                        break;
                }

                result = current->section_index == section_index
                      && current->object_file_offset == object_file_offset
                       ? current : 0;

                current = current->next;
        }

        return result;
}

typedef struct Fixups Fixups;
struct Fixups
{
        Arena *arena;

        U64    count;
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
Fixup__apply(Fixup *fixup, Fixups *fixups, Diagnostics *diagnostics);

internal void
Fixups__resolve(Fixups *fixups, Diagnostics *diagnostics);

#endif // CORE_FIXUP_H

