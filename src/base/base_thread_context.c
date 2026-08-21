thread_local_static Thread_Context *thread_context_local = 0;

internal Thread_Context *
Thread_Context_alloc(void)
{
	Arena *arena = Arena__allocate_m();
	/* The thread context itself is stored in the arena. */
	Thread_Context *thread_context = Arena__push_array_m(arena, Thread_Context, 1);
	thread_context->arenas[0] = arena;
	thread_context->arenas[1] = Arena__allocate_m();

	return thread_context;
}

internal void
Thread_Context_deallocate(Thread_Context *thread_context)
{
	Arena__deallocate(thread_context->arenas[1]);
	Arena__deallocate(thread_context->arenas[0]);
}

internal void
Thread_Context_select(Thread_Context *thread_context)
{
	thread_context_local = thread_context;
	return;
}

internal Thread_Context *
Thread_Context_selected(void)
{
	return thread_context_local;
}

// Scratch Arenas
//
// The idea of scratch arenas is having a stack-like allocation pattern for data that wouldn't fit
// the stack or to not pollute the main arena with scope-lived data. Every thread features a certain
// number (two is sufficient for most use-cases) of scratch arenas that can be used.

// If a function does NOT have an arena parameter, then this can be used as follows:
//
// ```
// int foo()
// {
//         Arena *arena = Arena_Temporary__begin(Thread_Context_scratch_get(0, 0));
//         // Do your stuff here
//         Arena_Temporary__end(arena);
// }
// ```
//
// In this case, it would simply return the 0-th arena in the `Thread_Context`.
// If your function needs to an allocation that outlives the scope of the function function, it takes an `Arena *`
// parameter. If the provided arena is itself a scratch arena, then we might get the same one and free it. Example:
//
// ```
// void example(void)
// {
//         Arena__Temp scratch = Arena_Temporary__begin(Thread_Context_scratch_get(0, 0));
//         void *result = 0;
//         // Suppose this scope being a function `void *inner(Arena *arena)` called as `inner(scratch.arena)`.
//         {
//                 // BAD: arena == scratch
//                 Arena__Temp scratch = Arena_Temporary__begin(Thread_Context_scratch_get(0, 0));
//                 result = Arena__push_array_zero(arena, U8, 1024);
//                 // `result` points to dropped data!
//                 Arena_Temporary__end(scratch);
//         }
//         // other code...
//         Arena_Temporary__end(scratch);
//         return;
// }
// ```
//
// For this reason, another rule must be adopted. When `get_scratch` is called, it must take any
// arenas being used for persistent allocations, to ensure a different one is returned, to avoid to
// the mixed usage mentioned above. This leads to the API defined below.
//
// If only a single "persistent" arena is present at any point in any codepath (e.g. a caller never
// passes in two arenas), then you will not need more than two scratch arenas. Those two scratch
// arenas can be used for arbitrarily-deep call stacks, because each frame in any call stack will
// alternate between using a single arena for persistent allocations, and the other for scratch
// allocations.
internal Arena *
Thread_Context_scratch_get(Arena **conflicts, U64 count)
{
	Thread_Context *thread_context = Thread_Context_selected();
	Arena *result = 0;
	Arena **arena_ptr = thread_context->arenas;

	for (U64 i = 0; i < array_count_m(thread_context->arenas); i += 1, arena_ptr +=1)
	{
		Arena **conflict_ptr = conflicts;
		B32 has_conflict = 0;
		for (U64 j = 0; j < count; j += 1, conflict_ptr += 1)
		{
			if (*arena_ptr == *conflict_ptr)
			{
				has_conflict = 1;
				break;
			}
		}
		if (!has_conflict)
		{
			result = *arena_ptr;
			break;
		}
	}

	return result;
}
