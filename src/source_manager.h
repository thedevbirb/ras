#ifndef SOURCE_MANAGER_H
#define SOURCE_MANAGER_H

// Scan backward to find the greatest index whose value is greater than or equal the given needle.
//
// Assumes the array is sorted in ascending order.
internal U32
floor_search(U32 *data, U32 count, U32 needle)
{

	U32 start = U32_max;
	U32 index = count;

	for (;;)
	{
		B32 break_should = start < needle || index == 0;
		if (break_should)
		{
			break;
		}

		index -= 1;
		start = data[index];
	}

	return index;
}

typedef struct Source Source;
struct Source
{
	U8  *data;
	U8  *name;
	U32 *line_start_indexes;

	U32 count;
	U32 name_count;
	U32 line_start_count;

	// Start in the virtual address space created for all sources.
	U32 start_offset_logical;
	// Where the text of this source has been pulled in.
	U32 expansion_offset_logical;
	// Where the text of this source has originated. For files, this equals `expansion_offset_logical`,
	// for macros, this helps differentiate between origin location and expansion location.
	U32 origin_offset_logical;
};

#endif // SOURCE_MANAGER_H

