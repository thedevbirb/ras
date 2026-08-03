#ifndef CORE_FIXUP_H
#define CORE_FIXUP_H

// Forward declarations for pointer use.
typedef struct Expression Expression;
typedef struct Section Section;

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
typedef struct Fixup Fixup;
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

#endif // CORE_FIXUP_H

