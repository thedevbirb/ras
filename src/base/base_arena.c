Arena *
Arena__allocate(Arena_Parameters *parameters)
{
	// Round up reserve/commit size to operative system page size.
	U64 page_sz = page_size();
	U64 reserve_size = align_pow_2_m(parameters->reserve_size, page_sz);
	U64 commit_size  = align_pow_2_m(parameters->commit_size,  page_sz);

	// Ensure reserve is greater than or equal the committed size.
	reserve_size = max_m(reserve_size, commit_size);

	void *base = memory_reserve(reserve_size);
	memory_commit(base, commit_size);

	asan_poison(base, commit_size);
	asan_unpoison(base, ARENA_HEADER_SIZE_MAX);

	Arena *arena = (Arena *)base;
	arena->previous             = 0;
	arena->current              = arena;
	arena->flags                = parameters->flags;
	arena->commit_size          = commit_size;
	arena->reserve_size         = reserve_size;
	arena->base_offset          = 0;
	arena->offset               = ARENA_HEADER_SIZE_MAX;
	arena->committed_size       = commit_size;
	arena->reserved_size        = reserve_size;
	arena->allocation_site_file = parameters->allocation_site_file;
	arena->allocation_site_line = parameters->allocation_site_line;

	return arena;
}

void
Arena__deallocate(Arena *arena)
{
	Arena *current  = arena->current;
	Arena *previous = 0;
	for (;;)
	{
		B32 break_should = current == 0;
		if (break_should)
		{
			break;
		}

		previous = current->previous;
		memory_release(current, current->reserved_size);
		current = previous;
	}

	return;
}

internal void *
Arena__push(Arena *arena, U64 size, U64 align, B32 zero)
{
	Arena *current = arena->current;
	assert_always_m(pow_2_is_m(align));

	U64 offset_aligned           = align_pow_2_m(current->offset, align);
	U64 offset_aligned_plus_size = offset_aligned + size;
	// Overflow checks.
	assert_always_m(offset_aligned <= U64_max - size);

	// Chain another arena if necessary and allowed.
	if (current->reserved_size < offset_aligned_plus_size && !(arena->flags & Arena_Flags__Chaining_Disabled))
	{
		Arena *new_block = 0;
#if defined(ARENA_FREE_LIST)
		// Walk backwards the free list stack to find a block that fits.
		{
			Arena *cursor_previous = 0;
			Arena *cursor          = arena->free_last;
			for (;;)
			{
				B32 break_should = cursor == 0 || new_block != 0;
				if (break_should)
				{
					break;
				}

				U64 offset_aligned = align_pow_2_m(cursor->offset, align);
				assert_always_m(offset_aligned <= U64_max - size);
				if (cursor->reserved_size >= offset_aligned + size)
				{
					new_block = cursor;
					// If found immediately, we simply change the pointer to the top of the
					// stack. Otherwise, we close the gap.
					if (cursor_previous == 0)
					{
						arena->free_last = cursor->previous;
					}
					else
					{
						cursor_previous->previous = cursor->previous;
					}
				}

				cursor_previous = cursor;
				cursor          = cursor->previous;
			}
		}
#endif
		// Allocate another arena.
		if (new_block == 0)
		{
			U64 reserve_size = current->reserve_size;
			U64 commit_size  = current->commit_size;
			if (size + ARENA_HEADER_SIZE_MAX > reserve_size)
			{
				reserve_size = align_pow_2_m(size + ARENA_HEADER_SIZE_MAX, align);
				commit_size  = align_pow_2_m(size + ARENA_HEADER_SIZE_MAX, align);
			}

			new_block = Arena__allocate_m
				(
					.reserve_size         = reserve_size,
					.commit_size          = commit_size,
					.flags                = current->flags,
					.allocation_site_file = current->allocation_site_file,
					.allocation_site_line = current->allocation_site_line
				);
		}

		// Put the new block at the tip of the block list.
		assert_always_m(current->base_offset <= SIZE_MAX - current->reserved_size);
		new_block->base_offset = current->base_offset + current->reserved_size;
		new_block->previous    = current;
		arena->current = new_block;
		current        = arena->current;

		// Update the positions so we know how to zero later. Remember that in the new
		// block the header size is written.
		offset_aligned           = align_pow_2_m(current->offset, align);
		offset_aligned_plus_size = offset_aligned + size;

		assert_always_m(offset_aligned_plus_size <= current->reserved_size);
	}

	// Commit new pages if needed.
	if (current->committed_size < offset_aligned_plus_size)
	{
		// Ensure rounding up to the next multiple of commit_size. Subtract one in case offset_aligned_plus_size
		// already is a multiple of it.
		U64 commit_size_new = offset_aligned_plus_size + current->commit_size - 1;
		commit_size_new -= commit_size_new % current->commit_size;
		commit_size_new = min_m(commit_size_new, current->reserved_size);
		U64 size_to_commit = commit_size_new - current->committed_size;
		void *commit_start_pointer = (U8 *)current + current->committed_size;

		if (arena->flags & Arena_Flags__Large_Pages)
		{
			// TODO: add large page support.
			assert_always_m(memory_commit(commit_start_pointer, size_to_commit));
		}
		else
		{
			assert_always_m(memory_commit(commit_start_pointer, size_to_commit));
		}

		current->committed_size = commit_size_new;
		assert_always_m(current->committed_size >= offset_aligned_plus_size);
	}

	// We check now whether we have to zero the [current->committed_size, offset_aligned_plus_size]
	// range since newly committed pages are guaranteed to be zero-ed by the operative system.
	U64 size_to_zero = 0;
	if (zero)
	{
		// We can't write past the commmitted size.
		U64 offset_clamped = min_m(current->committed_size, offset_aligned_plus_size);
		assert_always_m(offset_clamped >= offset_aligned);
		size_to_zero = offset_clamped - offset_aligned;
	}


	// Push if there is committed memory to write into.
	void *result = 0;
	if (current->committed_size >= offset_aligned_plus_size)
	{
		result = (U8 *)current + offset_aligned;
		current->offset = offset_aligned_plus_size;
		asan_unpoison(result, size);
		memory_zero(result, size_to_zero);
	}

	return result;
}

internal void
Arena__pop_to(Arena *arena, U64 offset_absolute)
{
	assert_always_m(arena->previous == 0 && "expected to call pop on root arena");

	// Make sure we don't delete the root header.
	U64 offset_absolute_clamped = max_m(ARENA_HEADER_SIZE_MAX, offset_absolute);
	Arena *current = arena->current;
	Arena *previous = 0;

// TODO: perhaps this should be part of the arena configuration and not a compile-time constant?
#if defined(ARENA_FREE_LIST)
	// Update the free list.
	for (;;)
	{
		B32 break_should = current->base_offset <= offset_absolute_clamped;
		if (break_should)
		{
			break;
		}

		// Backup `current->previous`, since current is being pushed to free list.
		previous = current->previous;

		// Delete everything from this block except reserved header size.
		current->offset = ARENA_HEADER_SIZE_MAX;

		// Push into the stack.
		current->previous = arena->free_last;
		arena->free_last = current;
		asan_poison((U8 *)current + ARENA_HEADER_SIZE_MAX, current->reserved_size - ARENA_HEADER_SIZE_MAX);

		current = previous;
	}
#else
	// Walk back the chain of blocks and release them.
	for (;;)
	{
		B32 break_should = current->base_offset <= offset_absolute_clamped;
		if (break_should)
		{
			break;
		}
		previous = current->previous;
		memory_release(current, current->reserved_size);

		current = previous;
	}
#endif // ARENA_FREE_LIST
	arena->current = current;
	assert_always_m(offset_absolute_clamped >= current->base_offset );
	U64 offset_new = max_m(offset_absolute_clamped - current->base_offset, ARENA_HEADER_SIZE_MAX);
	current->offset = offset_new;
	return;
}

internal U64
Arena__offset(Arena *arena)
{
	Arena *current = arena->current;
	U64 offset = current->base_offset + current->offset;
	return offset;
}

// This is no-op if the provided size is greater than what's stored in the arena.
internal void
Arena__pop(Arena *arena, U64 size)
{
	U64 offset     = Arena__offset(arena);
	U64 offset_new = offset;
	if (size < offset)
	{
		offset_new = offset_new - size;
	}
	Arena__pop_to(arena, offset_new);
	return;
}

void
Arena__clear(Arena *arena)
{
	Arena__pop_to(arena, 0);
	return;
}

Arena_Temporary
Arena_Temporary__begin(Arena *arena)
{
	U64 offset = Arena__offset(arena);
	Arena_Temporary tmp = (Arena_Temporary)
	{
		.arena  = arena,
		.offset = offset,
	};
	return tmp;
}

void
Arena_Temporary__end(Arena_Temporary tmp)
{
	Arena__pop_to(tmp.arena, tmp.offset);
	return;
}
