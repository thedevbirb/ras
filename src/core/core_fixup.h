#ifndef CORE_FIXUP_H
#define CORE_FIXUP_H

// Forward declaration for pointer use.
typedef struct Expression Expression;

typedef U8 Fixup_Flags;
enum
{
	Fixup_Flags__None         = 0 << 0,
        Fixup_Flags__PC_Relative  = 1 << 0,
        Fixup_Flags__Done         = 1 << 1,
};


typedef struct Fixup Fixup;
struct Fixup
{
        Fixup       *next;
        Fragment    *fragment;

        Expression  *expression;
        // Pointer to location in the fragment fixed or variable data where the patch should be written.
        U8          *fragment_write_area;

        U16          relocation_type;
        // Size of the patch to be written.
        U8           fragment_write_size;
        U8           flags;
};

typedef struct Fixups Fixups;
struct Fixups
{
        Arena *arena;
        U64    count;
        Fixup *first;
        Fixup *last;
};

internal Fixup *
Fixups__push(Fixups *fixups)
{
        Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
        SLL_queue_push_m(fixups->first, fixups->last, fixup);
        return fixup;
}

// Requires `fixup` to be an element of `fixups`
internal Fixup *
Fixups__push_at(Fixups *fixups, Fixup *fixup)
{
        // Add in the middle of the queue. This logic could be moved into the base layer.
        Fixup *result    = Arena__push_struct_m(fixups->arena, Fixup);
        Fixup *temporary = fixup->next;

        fixup->next    = result;
        result->next   = temporary;
        fixups->count += 1;

        return result;
}

#endif // CORE_FIXUP_H

