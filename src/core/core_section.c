internal B32
Section__zero_is(Section *s)
{
	B32 result = memory_match_struct(s, &Section__zero);
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
Sections_Trie__get_or_default(Sections_Trie **root, Arena *arena, Sections_Trie_Chunk_List *chunks, U64 hash, String8 name)
{
	B32 initialized = 0;
	B32 match = 0;

	Sections_Trie **trie_current = root;
	U64 hash_shifted = hash;
	for (;;)
	{
		if (*trie_current == 0)
		{
			Sections_Trie *trie_new = Sections_Trie_Chunk_List__push(chunks, arena, Sections_Trie_Chunk__capacity_default);
			memory_zero_array(trie_new->children);
			*trie_current = trie_new;
			initialized = 1;
		}

		if (!initialized && String8__match_exact((*trie_current)->section.name, name))
		{
			match = 1;
		}

		B32 break_should = initialized || match;
		if (break_should)
		{
			break;
		}

		trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
		hash_shifted = hash_shifted << 2;
	}

	return *trie_current;
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
	U64 hash = FNV_hash_U64(name);
	Sections_Trie *trie = Sections_Trie__get(sections_table->root, hash, name);

	Section *result = trie ? &trie->section : 0;
	return result;
}

// Return the section or create one with appropriate defaults, meaning:
//
// 1. Set the provided name.
// 2. Set the section index field.
// 3. Add an empty fragment to it.
internal Section *
Sections_Table__get_or_default(Sections_Table *sections_table, String8 name)
{
	U64 hash = FNV_hash_U64(name);
	Sections_Trie *trie = Sections_Trie__get_or_default(&sections_table->root, sections_table->arena, sections_table->chunks, hash, name);

	B32 zero_is = Section__zero_is(&trie->section);
	if (zero_is)
	{
		Arena *arena = Arena__allocate_m();
		trie->section = (Section)
		{
			.arena = arena,
			.name  = name,
			.index = sections_table->index_next,
		};
		sections_table->index_next += 1;
		Fragment *fragment = Arena__push_struct_m(arena, Fragment);
		Fragment_List *fragment_list = &trie->section.fragment_list;
		SLL_queue_push_m(fragment_list->first, fragment_list->last, fragment);
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

	// TODO: modify flags etc.
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
