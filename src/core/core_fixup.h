#ifndef CORE_FIXUP_H
#define CORE_FIXUP_H

typedef struct Fixup Fixup;
struct Fixup
{
        Fixup    *next;
        Fragment *fragment;

        U32 expression_index;
        // Offset in the fragment data where the result must be written to.
        U32 encoding_offset;

        U16 relocation_type;
        U8  size;
        U8  flags;
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

