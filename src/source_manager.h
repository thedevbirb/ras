#ifndef SOURCE_MANAGER_H
#define SOURCE_MANAGER_H

// Logic to track sources of text.
//
// Text can come from files, buffers and macros.

// Contains the indexes where a sources line start.
typedef struct Source_Lines Source_Lines;
struct Source_Lines
{
	Xar_Metadata metadata;
	Xar_Header   header;
	U64         *chunks[14];
};

internal void
Source_Lines__initialize(Source_Lines *source_lines, Arena *arena)
{
	xar_initialize_m(source_lines, 12);
	// Ensure the first entry is zero.
	xar_push_m(source_lines, arena);
	return;
}

// Find the line (starting from zero) which contains text
// at the specified offset.
//
// It starts from the end.
//
// NOTE: make this a binary search if the source is big enough.
internal U64
Source_Lines__search(Source_Lines *source_lines, U64 offset)
{

	U64 count = source_lines->header.count;
	U64 start = U64_max;
	U32 index = count;

	for (;;)
	{
		B32 break_should = start < offset || index == 0;
		if (break_should)
		{
			break;
		}

		index -= 1;
		start = *(U64 *)xar_get_m(source_lines, index);
	}

	return index;
}

typedef struct Source Source;
struct Source
{
	String8   input;
	String8   name;
	// Start in the virtual address space created for all sources.
	U64       start_offset_logical;
	// Where the text of this source has been pulled in.
	U64       expansion_offset_logical;
	// Where the text of this source has originated. For files, this equals `expansion_offset_logical`,
	// for macros, this helps differentiate between origin location and expansion location.
	U64       origin_offset_logical;

	Source_Lines lines;
};

internal void
Source__initialize(Source *source, String8 input, String8 name, Arena *arena)
{
	source->input = input;
	source->name  = name;
	Source_Lines__initialize(&source->lines, arena);
	return;
}

internal B32
Source__macro_is(Source *source)
{
	B32 result = source->expansion_offset_logical != source->origin_offset_logical;
	return result;
}

internal U64
Source__location(Source *source, U64 index)
{
	U64 result = index + source->start_offset_logical;
	return result;
}


typedef struct Source_Manager Source_Manager;
struct Source_Manager
{
	Xar_Metadata metadata;
	Xar_Header   header;
	Source      *chunks[14];
};

internal Source *
Source_Manager__find(Source_Manager *source_manager, U64 offset)
{
	Source *result = 0;
	U32 index = 0;
	B32 found = 0;
	for (;;)
	{
		B32 break_should = found || index >= source_manager->header.count;
		if (break_should)
		{
			break;
		}

		Source *source = xar_get_m(source_manager, index);
		found = source->start_offset_logical <= offset && offset <= source->start_offset_logical + source->input.count;
	}

	return result;
}

// Adds the source file
// internal void
// Source_Manager__file_add(Source_Manager *source_manager, Source *source, Arena *arena)
// {
// 	U64 start_offset_logical = 0;
// 	if (source_manager->header.count > 0)
// 	{
// 		U64 index = source_manager->header.count - 1;
// 		Source *last = xar_get_m(source_manager, index);
// 		start_offset_logical = last->start_offset_logical + last->input.count;
// 	}
//
// 	Source *result = xar_push_m(source_manager, arena);
// 	*result = *source;
// 	result->origin_offset_logical = result->expansion_offset_logical;
// 	result->start_offset_logical  = start_offset_logical;
// }

#endif // SOURCE_MANAGER_H

