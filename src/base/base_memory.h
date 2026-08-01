#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

// @per_os_implementation Platform Memory Allocation
internal void *memory_reserve(U64 size);
internal B32   memory_commit(void *pointer, U64 size);
internal void  memory_decommit(void *pointer, U64 size);
internal void  memory_release(void *pointer, U64 size);

internal U64 page_size(void);

#endif // BASE_MEMORY_H
