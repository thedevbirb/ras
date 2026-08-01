// Create a new mapping in the virtual address space of the calling process, letting the
// kernel choose the (page-aligned) address at which to create the mapping.
// The pages may not be accessed, as of `PROT_NONE`. Lastly, the mapping is private, only
// known to the current process and anonymous i.e. not backed by any file and zeroed.
internal void *
memory_reserve(U64 size)
{
	void *result = mmap(NULL, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (result == MAP_FAILED)
	{
		result = 0;
	}
	return result;
}

// Change access protection of the provided section of the calling process memory, by setting it
// readable and writable.
internal B32
memory_commit(void *pointer, U64 size)
{
	B32 result = (mprotect(pointer, size, PROT_READ|PROT_WRITE) == 0);
	return result;
}

internal void
memory_decommit(void *pointer, U64 size)
{
	mprotect(pointer, size, PROT_NONE);
}

// Release the mapping, making the virtual address range available for future mappings.
internal void
memory_release(void *pointer, U64 size)
{
	munmap(pointer, size);
}

internal U64
page_size(void)
{
	U64 result = (U64)sysconf(_SC_PAGESIZE);
	return result;
}
