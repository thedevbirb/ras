#ifndef FIXUP_H
#define FIXUP_H

typedef struct Fixup Fixup;
struct Fixup
{
	Fixup    *next;
	Fragment *fragment;

	U32 expression_index;
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


#endif // FIXUP_H

