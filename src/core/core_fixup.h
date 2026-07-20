#ifndef CORE_FIXUP_H
#define CORE_FIXUP_H

// Forward declaration for pointer use.
typedef struct Expression Expression;

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

typedef struct Fixup_List Fixup_List;
struct Fixup_List
{
        U64 count;
        Fixup *first;
        Fixup *last;
};

typedef struct Fixups Fixups;
struct Fixups
{
        Arena      *arena;
        Fixup_List  list;
};

internal Fixup *
Fixups__push(Fixups *fixups)
{
        Fixup *fixup = Arena__push_struct_m(fixups->arena, Fixup);
        SLL_queue_push_m(fixups->list.first, fixups->list.last, fixup);
        return fixup;
}

#endif // CORE_FIXUP_H

