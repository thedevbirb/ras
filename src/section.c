ELF_Section
ELF_Section_from_Directive_Kind(Directive_Kind kind)
{
	ELF_Section section = 0;
	switch (kind)
	{
	case Directive_Kind__Text:           { section = ELF_Section__Text;           } break;
	case Directive_Kind__Data:           { section = ELF_Section__Data;           } break;
	case Directive_Kind__Read_Only_Data: { section = ELF_Section__Read_Only_Data; } break;
	case Directive_Kind__BSS:            { section = ELF_Section__BSS;            } break;
	default: {} break;
	}
	return section;
}

internal void
Object_File_Section_initialize(Object_File_Section *section, ELF_Section section_index, Arena *arena)
{
	U8 *data = arena ? Arena_push_zero_m(arena) : 0;

	String8 buffer =
	{
		.data  = data,
		.count = 0,
	};
	*section = (Object_File_Section)
	{
		.arena         = arena,
		.buffer        = buffer,
		.section_index = section_index,
		.offset        = 0,
		.alignment     = ELF_Section_alignments[section_index],

	};

	return;
}

internal Object_File_Section *
Object_File_Section_create_all(Arena *arena, U32 input_size)
{
	Object_File_Section *sections = Arena_push_array_m(arena, Object_File_Section, ELF_Section__COUNT);

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= ELF_Section__COUNT;
		if (break_should)
		{
			break;
		}

		Arena *arena_dedicated = 0;
		B32 section_empty = ELF_Section__None || index == ELF_Section__BSS;
		if (!section_empty)
		{
			arena_dedicated = Arena_alloc_m(.reserve_size = input_size, .flags = Arena_Flags__No_Chain);
		}

		Object_File_Section_initialize(&sections[index], index, arena_dedicated);

		index += 1;
	}
	return sections;
}

internal void
Object_File_Section_align(Object_File_Section *section, U8 alignment)
{
	U32 mask         = alignment - 1;
	B32 power_two_or_zero_is = (alignment & (mask)) == 0 || alignment == 0;
	assert_always_m(power_two_or_zero_is && "alignment must be a power of two, or zero (no-op)");

	U32 offset_alignment_distance = section->offset & mask;
	// We mask again to handle the case where distance is zero, without branches.
	U32 padding = (alignment - offset_alignment_distance) & mask;

	// Examples:
	//
	// alignment                                      = 0b0100 (4)
	// mask                                           = 0b0011 (3)
	// offset                                         = 0b0111 (7)
	// offset_alignment_distance                      = 0b0111 & 0b0011 = 0b0011 (3)
	// alignment - offset_alignment_distance          = 0b0100 - 0b0011 = 0b0001 (1)
	//
	// alignment                                      = 0b0000_1000 (8)
	// mask                                           = 0b0000_0111 (7)
	// offset                                         = 0b0001_0000 (16)
	// offset_alignment_distance                      = 0b0001_0000 & 0b0000_0111 = 0b0000_0000 (0)
	// padding                                        = (0b0000_1000 - 0b0000_0000) & 0b0000_0111 = 0b0000_0000 (0)

	padding = padding & ((alignment == 0) - 1);

	Arena_push_array_m(section->arena, U8, padding);
	section->offset += padding;
	section->buffer.count += padding;

	section->alignment = max_m(section->alignment, alignment);
	return;
}

// TODO: maybe avoid this duplication?
U32
Object_File_Section_write_byte(Object_File_Section *section, U8 value, U64 count)
{
	U32 offset_old = section->offset;
	U32 offset_new = offset_old + count;

	Arena_push_array_m(section->arena, U8, count);

	os_memory_set(section->buffer.data + section->offset, value, count);
	section->offset = offset_new;
	section->buffer.count += count;
	Object_File_Section_align(section, section->alignment);

	return offset_old;
}

// TODO: this is kinda bad, not very generic either.
U32
Object_File_Section_write_instruction(Object_File_Section *section, U32 instruction_encoding)
{
	U32 offset_old = section->offset;
	U32 offset_new = offset_old + sizeof(instruction_encoding);

	U32 *value_pointer = Arena_push_struct_m(section->arena, U32);
	*value_pointer = instruction_encoding;

	section->offset = offset_new;
	section->buffer.count += sizeof(instruction_encoding);
	Object_File_Section_align(section, section->alignment);

	return offset_old;
}

// Write and align, returning the offset where data has been written.
U32
Object_File_Section_write_bytes(Object_File_Section *section, U8 *data, U64 count)
{
	U32 offset_old = section->offset;
	U32 offset_new = offset_old + count;

	Arena_push_array_m(section->arena, U8, count);

	os_memory_copy(section->buffer.data + section->offset, data, count);
	section->offset = offset_new;
	section->buffer.count += count;
	Object_File_Section_align(section, section->alignment);

	return offset_old;
}

U32
Object_File_Section_write(Object_File_Section *section, void *data, U64 size, U64 count)
{
	U32 result = Object_File_Section_write_bytes(section, (U8 *)data, size * count);
	return result;
}

U32
Object_File_Section_relocation_write(Object_File_Section *section, ELF64_Relocation_Addend *relocation)
{
	U32 result = Object_File_Section_write_bytes(section, (U8 *)relocation, sizeof(*relocation));
	return result;
}
