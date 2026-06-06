// ELF_Section
// ELF_Section_from_Directive_Kind(Directive_Kind kind)
// {
// 	ELF_Section section = 0;
// 	switch (kind)
// 	{
// 	case Directive_Kind__Text:           { section = ELF_Section__Text;           } break;
// 	case Directive_Kind__Data:           { section = ELF_Section__Data;           } break;
// 	case Directive_Kind__Read_Only_Data: { section = ELF_Section__Read_Only_Data; } break;
// 	case Directive_Kind__BSS:            { section = ELF_Section__BSS;            } break;
// 	default: {} break;
// 	}
// 	return section;
// }
//
// internal void
// Object_File_Section_initialize(Object_File_Section *section, ELF_Section section_index, Arena *arena)
// {
// 	U8 *data = arena ? (U8 *)Arena__push_zero_m(arena) : (U8 *)0;
//
// 	String8 buffer =
// 	{
// 		.data  = data,
// 		.count = 0,
// 	};
// 	*section = (Object_File_Section)
// 	{
// 		.arena         = arena,
// 		.buffer        = buffer,
// 		.section_index = section_index,
// 		.offset        = 0,
// 		.alignment     = ELF_Section_alignments[section_index],
//
// 	};
//
// 	return;
// }
//
// internal Object_File_Section *
// Object_File_Section_create_all(Arena *arena, U32 input_size)
// {
// 	Object_File_Section *sections = Arena__push_array_m(arena, Object_File_Section, ELF_Section__COUNT);
//
// 	U32 index = 0;
// 	for (;;)
// 	{
// 		B32 break_should = index >= ELF_Section__COUNT;
// 		if (break_should)
// 		{
// 			break;
// 		}
//
// 		Arena *arena_dedicated = 0;
// 		B32 section_empty = ELF_Section__None || index == ELF_Section__BSS;
// 		if (!section_empty)
// 		{
// 			arena_dedicated = Arena__allocate_m(.reserve_size = input_size, .flags = Arena_Flags__Chaining_Disabled);
// 		}
//
// 		Object_File_Section_initialize(&sections[index], index, arena_dedicated);
//
// 		index += 1;
// 	}
// 	return sections;
// }
//
// internal void
// Object_File_Section_align(Object_File_Section *section, U8 alignment)
// {
// 	U32 mask         = alignment - 1;
// 	B32 power_two_or_zero_is = (alignment & (mask)) == 0 || alignment == 0;
// 	assert_always_m(power_two_or_zero_is && "alignment must be a power of two, or zero (no-op)");
//
// 	U32 offset_alignment_distance = section->offset & mask;
// 	// We mask again to handle the case where distance is zero, without branches.
// 	U32 padding = (alignment - offset_alignment_distance) & mask;
//
// 	// Examples:
// 	//
// 	// alignment                                      = 0b0100 (4)
// 	// mask                                           = 0b0011 (3)
// 	// offset                                         = 0b0111 (7)
// 	// offset_alignment_distance                      = 0b0111 & 0b0011 = 0b0011 (3)
// 	// alignment - offset_alignment_distance          = 0b0100 - 0b0011 = 0b0001 (1)
// 	//
// 	// alignment                                      = 0b0000_1000 (8)
// 	// mask                                           = 0b0000_0111 (7)
// 	// offset                                         = 0b0001_0000 (16)
// 	// offset_alignment_distance                      = 0b0001_0000 & 0b0000_0111 = 0b0000_0000 (0)
// 	// padding                                        = (0b0000_1000 - 0b0000_0000) & 0b0000_0111 = 0b0000_0000 (0)
//
// 	padding = padding & ((alignment == 0) - 1);
//
// 	Arena__push_array_m(section->arena, U8, padding);
// 	section->offset += padding;
// 	section->buffer.count += padding;
//
// 	section->alignment = max_m(section->alignment, alignment);
// 	return;
// }
//
// // TODO(low): maybe avoid this duplication?
// U32
// Object_File_Section_write_byte(Object_File_Section *section, U8 value, U64 count)
// {
// 	U32 offset_old = section->offset;
// 	U32 offset_new = offset_old + count;
//
// 	Arena__push_array_m(section->arena, U8, count);
//
// 	memory_fill(section->buffer.data + section->offset, value, count);
// 	section->offset = offset_new;
// 	section->buffer.count += count;
// 	Object_File_Section_align(section, section->alignment);
//
// 	return offset_old;
// }
//
// // TODO(low): this is kinda bad, not very generic either.
// U32
// Object_File_Section_write_instruction(Object_File_Section *section, U32 instruction_encoding)
// {
// 	U32 offset_old = section->offset;
// 	U32 offset_new = offset_old + sizeof(instruction_encoding);
//
// 	U32 *value_pointer = Arena__push_struct_m(section->arena, U32);
// 	*value_pointer = instruction_encoding;
//
// 	section->offset = offset_new;
// 	section->buffer.count += sizeof(instruction_encoding);
// 	Object_File_Section_align(section, section->alignment);
//
// 	return offset_old;
// }
//
// // Write and align, returning the offset where data has been written.
// U32
// Object_File_Section_write_bytes(Object_File_Section *section, U8 *data, U64 count)
// {
// 	U32 offset_old = section->offset;
// 	U32 offset_new = offset_old + count;
//
// 	Arena__push_array_m(section->arena, U8, count);
//
// 	memory_copy(section->buffer.data + section->offset, data, count);
// 	section->offset = offset_new;
// 	section->buffer.count += count;
// 	Object_File_Section_align(section, section->alignment);
//
// 	return offset_old;
// }
//
// U32
// Object_File_Section_write(Object_File_Section *section, void *data, U64 size, U64 count)
// {
// 	U32 result = Object_File_Section_write_bytes(section, (U8 *)data, size * count);
// 	return result;
// }
//
// U32
// Object_File_Section_relocation_write(Object_File_Section *section, ELF64_Relocation_Addend *relocation)
// {
// 	U32 result = Object_File_Section_write_bytes(section, (U8 *)relocation, sizeof(*relocation));
// 	return result;
// }

internal B32
Section__zero_is(Section *s)
{
	B32 result = memory_match_struct(s, &Section__zero) == 0;
	return result;
}

Sections_Trie *
Sections_Trie_Chunk_List__push(Sections_Trie_Chunk_List *chunks, Arena *arena, U64 capacity)
{
	if (chunks->last == 0 || chunks->last->count >= chunks->last->capacity)
	{
		Sections_Trie_Chunk *chunk_new = Arena__push_struct_m(arena, Sections_Trie_Chunk);
		chunk_new->nodes = Arena__push_array_m(arena, Sections_Trie, capacity);
		chunk_new->capacity = capacity;

		SLL_queue_push_m(chunks->first, chunks->last, chunk_new);
		chunks->count += 1;
	}

	Sections_Trie_Chunk *chunk_last = chunks->last;
	Sections_Trie *result = &chunk_last->nodes[chunk_last->count];
	chunk_last->count += 1;

	return result;
}

Sections_Trie *
Sections_Trie__get(Sections_Trie *trie, U64 hash, String8 name)
{
	Sections_Trie *result = 0;
	Sections_Trie *trie_current = trie;
	U64 hash_shifted = hash;
	for (;;)
	{
		B32 trie_current_zero = trie_current == 0;
		B32 found = !trie_current_zero && String8__match_exact(trie_current->section.name, name);
		if (found)
		{
			result = trie_current;
		}

		B32 break_should = trie_current_zero || found;
		if (break_should)
		{
			break;
		}

		trie_current = trie_current->children[(hash_shifted >> 62)];
		hash_shifted = hash_shifted << 2;
	}

	return result;
}

Sections_Trie *
Sections_Trie__get_or_default(Sections_Trie *trie, Arena *arena, Sections_Trie_Chunk_List *chunks, U64 hash, String8 name)
{
	B32 initialized = 0;
	B32 match = 0;

	Sections_Trie *trie_current = trie;
	U64 hash_shifted = hash;
	for (;;)
	{
		if (trie_current == 0)
		{
			Sections_Trie *trie_new = Sections_Trie_Chunk_List__push(chunks, arena, Sections_Trie_Chunk__capacity_default);
			memory_zero_array(trie_new->children);
			trie_current = trie_new;
			initialized = 1;
		}

		if (!initialized && String8__match_exact(trie_current->section.name, name))
		{
			match = 1;
		}

		B32 break_should = initialized || match;
		if (break_should)
		{
			break;
		}

		trie_current = trie_current->children[(hash_shifted >> 62)];
		hash_shifted = hash_shifted << 2;
	}

	return trie_current;
}


internal Sections_Table *
Sections_Table__default(void)
{
	Arena *arena = Arena__allocate_m();
	Sections_Table *sections_table = Arena__push_struct_m(arena, Sections_Table);
	sections_table->arena  = arena;
	sections_table->chunks = Arena__push_struct_m(arena, Sections_Trie_Chunk_List);
	return sections_table;
}

internal Section *
Sections_Table__get(Sections_Table *sections_table, String8 name)
{
	U64 hash = FNV_hash(name);
	Sections_Trie *trie = Sections_Trie__get(sections_table->root, hash, name);

	Section *result = trie ? &trie->section : 0;
	return result;
}

internal Section *
Sections_Table__get_or_default(Sections_Table *sections_table, String8 name)
{
	U64 hash = FNV_hash(name);
	Sections_Trie *trie = Sections_Trie__get_or_default(sections_table->root, sections_table->arena, sections_table->chunks, hash, name);

	B32 zero_is = Section__zero_is(&trie->section);
	if (zero_is)
	{
		trie->section = (Section)
		{
			.arena = Arena__allocate_m(),
			.name  = name,
			.index = sections_table->index_next,
		};
		sections_table->index_next += 1;
	}

	return &trie->section;
}


internal void
Sections_Table__add_common(Sections_Table *sections_table)
{
	String8 nil  = String8__literal("");
	String8 text = String8__literal(".text");
	String8 data = String8__literal(".data");
	String8 bss  = String8__literal(".bss");

	Sections_Table__get_or_default(sections_table, nil);
	Sections_Table__get_or_default(sections_table, text);
	Sections_Table__get_or_default(sections_table, data);
	Sections_Table__get_or_default(sections_table, bss);

	return;
}

// Sections_Trie *
// sections_trie_push(Arena *arena, Sections_Trie_Chunk_List *chunks, Sections_Trie **trie_ptr, U64 hash, Section *value)
// {
// 	B32 initialized = 0;
// 	B32 match = 0;
//
// 	Sections_Trie **trie_current = trie_ptr;
// 	U64 hash_shifted = hash;
// 	for (;;)
// 	{
// 		if (*trie_current == 0)
// 		{
// 			Sections_Trie *trie_new = sections_trie_chunk_list_push(arena, chunks, Sections_Trie_Chunk__capacity_default);
// 			trie_new->section = *value;
// 			memory_zero_array(trie_new->children);
// 			*trie_current = trie_new;
// 			initialized = 1;
// 		}
//
// 		if (!initialized && (*trie_current)->key && String8__match_exact(*(*trie_current)->key, value->key))
// 		{
// 			match = 1;
// 		}
//
// 		B32 break_should = initialized || match;
// 		if (break_should)
// 		{
// 			break;
// 		}
//
// 		trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
// 		hash_shifted = hash_shifted << 2;
// 	}
//
// 	return *trie_current;
// }
