#ifndef CORE_FIXUP_H
#define CORE_FIXUP_H


// For reference, here's how GNU as implements them:
//
// fixS *
// fix_new (fragS *frag,			/* Which frag?  */
// 	 unsigned long where,		/* Where in that frag?  */
// 	 unsigned long size,		/* 1, 2, or 4 usually.  */
// 	 symbolS *add_symbol,		/* X_add_symbol.  */
// 	 offsetT offset,		/* X_add_number.  */
// 	 int pcrel,			/* TRUE if PC-relative relocation.  */
// 	 RELOC_ENUM r_type		/* Relocation type.  */)
// {

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

