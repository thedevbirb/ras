#ifndef BASE_ARENA_H
#define BASE_ARENA_H

typedef U8 Arena_Flags;
enum
{
	// Whether the arena is allowed to grow, by chaining multiple arenas together.
	Arena_Flags__Chaining_Disabled = (1 << 0),
	// Whether is it possible to request large pages to the operative system. NOT supported yet
	Arena_Flags__Large_Pages       = (1 << 1),
};

typedef struct Arena_Parameters Arena_Parameters;
struct Arena_Parameters
{
	Arena_Flags flags;
	// The (initial) reserve size specifies the amount of bytes that can be used by this arena
	// before another one should be chained, provided the flag `Arena_Flags__Chaining_Disabled` is
	// not provided.
	U64 reserve_size;
	// The (initial) commit size specifies the memory immediately available when the arena
	// is allocated. The committed size can then grow by requesting more pages, capped by the
	// reserve size.
	U64 commit_size;
	const char *allocation_site_file;
	B32 allocation_site_line;
};

#define ARENA_HEADER_SIZE_MAX 128

// An optionally growable arena allocator, implemented as an intrusive, backward singly-linked list.
typedef struct Arena Arena;
struct Arena
{
	Arena *previous;
	Arena *current;
	Arena_Flags flags;
	// The size of memory in bytes to commit at arena allocation time.
	U64 commit_size;
	// The size of memory to be reserved at arena allocation time.
	U64 reserve_size;
	// Offset of this arena's base from the chain start, in terms of cumulated reserved size.
	U64 base_offset;
	// Current allocation offset within this arena.
	U64 offset;
	// The amount of bytes committed for this arena
	U64 committed_size;
	// The amount of bytes reserved for this arena
	U64 reserved_size;
#if defined(ARENA_FREE_LIST)
	// The arena free list is a stack of empty arena blocks to use when pushing data, instead of allocating new
	// blocks. A block is pushed there when enough data is popped from the arena chain.
	Arena *free_last;
#endif
	const char *allocation_site_file;
	S32 allocation_site_line;
};
assert_static_m(sizeof(Arena) <= ARENA_HEADER_SIZE_MAX, Arena__size_check);


// An arena which can be used as a scratch buffer.
typedef struct Arena_Temporary Arena_Temporary;
struct Arena_Temporary
{
	Arena *arena;
	U64 offset;
};

// Globals

global U64 Arena__reserve_size_default  = 64 << 20; // 64 MiB
global U64 Arena__commit_size_default   = 64 << 10; // 64 KiB
global Arena_Flags Arena__flags_default = 0;

// Allocate a new arena using the provided parameters.
Arena *
Arena__allocate(Arena_Parameters *parameters);

// Deallocate all the memory reserved for this arena.
void
Arena__deallocate(Arena *arena);

// Push the provided number of bytes into the arena, with the specified alignment. Optionally choose
// to zero the memory requested.
//
// It is recommended to use the `Arena__push_*` family of macros instead of directly calling this
// function.
internal void *
Arena__push(Arena *arena, U64 size, U64 align, B32 zero);

internal void
Arena__pop(Arena *arena, U64 size);

void
Arena__clear(Arena *arena);

Arena_Temporary
Arena_Temporary__begin(Arena *arena);

void
Arena_Temporary__end(Arena_Temporary tmp);

#define Arena__allocate_m__(...) \
	Arena__allocate(&(Arena_Parameters) \
	{ \
		.reserve_size         = Arena__reserve_size_default, \
		.commit_size          = Arena__commit_size_default, \
		.flags                = Arena__flags_default, \
		.allocation_site_file = __FILE__, \
		.allocation_site_line = __LINE__, \
		__VA_ARGS__ \
	})

#if defined(COMPILER_CLANG)
#define Arena__allocate_m(...) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	Arena__allocate_m__(__VA_ARGS__) \
	_Pragma("clang diagnostic pop")
#elif defined(COMPILER_GCC)
#define Arena__allocate_m(...) \
	({ \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Woverride-init\"") \
		Arena *_arena_tmp = Arena__allocate_m__(__VA_ARGS__); \
		_Pragma("GCC diagnostic pop") \
		_arena_tmp; \
	})
#else
#define Arena__allocate_m(...) Arena__allocate_m__(__VA_ARGS__)
#endif

#define Arena__push_array_no_zero_aligned_m(arena, Type, count, align) (Type *)Arena__push((arena), sizeof(Type)*(count), (align), (0))
#define Arena__push_array_aligned_m(arena, Type, count, align)         (Type *)Arena__push((arena), sizeof(Type)*(count), (align), (1))
#define Arena__push_array_no_zero_m(arena, Type, count)                        Arena__push_array_no_zero_aligned_m(arena, Type, count, max_m(8, cc_align_of(Type)))
#define Arena__push_array_m(arena, Type, count)                                Arena__push_array_aligned_m(arena, Type, count, max_m(8, cc_align_of(Type)))

#define Arena__push_struct_no_zero_aligned_m(arena, Type, align) Arena__push_array_no_zero_aligned_m(arena, Type, 1, align)
#define Arena__push_struct_aligned_m(arena, Type, align)         Arena__push_array_aligned_m(arena, Type, 1, align)
#define Arena__push_struct_no_zero_m(arena, Type)                Arena__push_array_no_zero_m(arena, Type, 1)
#define Arena__push_struct_m(arena, Type)                        Arena__push_array_m(arena, Type, 1)

#endif /* BASE_ARENA_H */
