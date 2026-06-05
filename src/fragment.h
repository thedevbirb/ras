#ifndef FRAGMENT_H
#define FRAGMENT_H

typedef struct Fragment Fragment;
struct Fragment
{
	Fragment *next;
	// Where it started
	U32 location;
	U64 offset;

	U32 size_fixed;
	U8  size_variable;
};

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
Fragment_List__push_fixed(Fragment_List *fragments, Arena *arena, U32 size, U32 location)
{
	U8 *data = Fragment_List__push(fragments, arena, location, size);
	fragments->last->size_fixed += size;
	return data;
}

internal U8 *
Fragment_List__push_variable(Fragment_List *fragments, Arena *arena, U32 size_max, U32 size_variable, U32 location)
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


#endif // FRAGMENT_H

