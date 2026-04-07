#ifndef PRIMITIVES_H
#define PRIMITIVES_H

internal U8 Input_count_extra = 8;

// Input is a String8 where we assume ZII, so we can ensure to not panic on out of bounds access. To do so, Input is
// slighly over-allocated. In practice, when an input is allocated `Input_count_extra` elements are reserved.
//
// This simplifies a lot of code because no particular branching is needed for checking out of bounds.
typedef struct Input Input;
struct Input
{
	U8 *data;
	U64 count;
};

internal Input
Input_new(U64 count, Arena *arena)
{
	assert_always_m(count < U64_max && "cannot allocate U64_max bytes");
	U64 count_extra = count + Input_count_extra;
	U8 *data = Arena_push_array_m(arena, U8, count_extra);

	Input input =
	{
		.data = data,
		.count = count,
	};

	return input;
}

#endif // PRIMITIVES_H

