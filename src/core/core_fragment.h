#ifndef CORE_FRAGMENT_H
#define CORE_FRAGMENT_H

typedef U8 Fragment_Kind;
enum
{
	Fragment_Kind__None,
	Fragment_Kind__Fill,
	Fragment_Kind__Align,
	Fragment_Kind__Machine,
	Fragment_Kind__COUNT,
};



typedef struct Fragment Fragment;
struct Fragment
{
	Fragment *next;
	// Multi-purpose field, for fill is it the repeat count.
	U32 offset;
	U32 location;
	U32 size_fixed;
	U8  size_variable;
	Fragment_Kind  kind;
};
assert_static_m(sizeof(Fragment) % 8 == 0, Fragment__sizeof_check);

typedef struct Fragment_List Fragment_List;
struct Fragment_List
{
	U64 count;
	Fragment *first;
	Fragment *last;
};

internal U8 *
Fragment_List__push(Fragment_List *fragments, Arena *arena, U32 location, U32 size)
{
	U8 *result = 0;

	U64 capacity_left = arena->reserved_size - arena->offset;
	if (capacity_left < size)
	{
		// We have to "seal" the current fragment, and switch to another arena block.
		// We that also the new fragment header is on the new arena. To do so, we first fill the rest of the
		// arena.
		//
		// TODO: maybe some other steps are needed.
		Arena *block_before = arena->current;
		Arena__push_array_m(arena, U8, capacity_left);
		assert_always_m(arena->current == block_before);
	}

	if (capacity_left < size || fragments->last == 0)
	{
		// We have to "seal" the current fragment, and switch to another arena block.
		// We that also the new fragment header is on the new arena.
		//
		// TODO: maybe some other steps are needed.
		Fragment *fragment = Arena__push_struct_m(arena, Fragment);
		fragment->location = location;
		SLL_queue_push_m(fragments->first, fragments->last, fragment);
		fragments->count += 1;
	}
	else
	{
		result = Arena__push_array_m(arena, U8, size);
	}

	return result;
}

internal U8 *
Fragment_List__push_fixed(Fragment_List *fragments, Arena *arena, U32 location, U32 size)
{
	U8 *data = Fragment_List__push(fragments, arena, location, size);
	fragments->last->size_fixed += size;
	return data;
}

internal U8 *
Fragment_List__push_variable(Fragment_List *fragments, Arena *arena, U32 location, U32 size_max, U32 size_variable)
{
	U8 *data = Fragment_List__push(fragments, arena, location, size_max);
	fragments->last->size_variable = size_variable;
	// TODO: a lot more fields to add here.

	// We have to create another fragment since variable data seal it.
	Fragment *fragment = Arena__push_struct_m(arena, Fragment);
	fragment->location = location;
	SLL_queue_push_m(fragments->first, fragments->last, fragment);
	fragments->count += 1;

	return data;
}

// NOTE:
//
// 1. It's fine if `size` is passed as zero if it cannot be decided immediately, e.g. `.zero label2-label1`. However,
// make sure you create an appropriate fixup.
internal U8 *
Fragment_List__push_fill(Fragment_List *fragments, Arena *arena, U32 location, U32 repeat, S64 pattern, U8 size)
{
	U8 *data = Fragment_List__push(fragments, arena, location, size);
	memory_copy(data, (U8 *)&pattern, size);

	Fragment *fragment = fragments->last;
	fragment->offset   = repeat;
	fragment->kind     = Fragment_Kind__Fill;

	// We have to create another fragment since the fill seals it.
	Fragment *fragment_new = Arena__push_struct_m(arena, Fragment);
	fragment_new->location = location;
	SLL_queue_push_m(fragments->first, fragments->last, fragment_new);
	fragments->count += 1;
	return 0;
}


#endif // CORE_FRAGMENT_H

