internal Symbols_Trie *
symbols_trie_chunk_list_push(Arena *arena, Symbols_Trie_Chunk_List *chunks, U64 capacity)
{
	if (chunks->last == 0 || chunks->last->count >= chunks->last->capacity)
	{
		Symbols_Trie_Chunk *chunk_new = Arena__push_struct_m(arena, Symbols_Trie_Chunk);
		chunk_new->nodes = Arena__push_array_m(arena, Symbols_Trie, capacity);
		chunk_new->capacity = capacity;

		SLL_queue_push_m(chunks->first, chunks->last, chunk_new);
		chunks->count += 1;
	}

	Symbols_Trie_Chunk *chunk_last = chunks->last;
	Symbols_Trie *result = &chunk_last->nodes[chunk_last->count];
	chunk_last->count += 1;

	return result;
}

// TODO: check whether get, get_or_default and create can be unified in a single implementation with "modes".

internal Symbols_Trie *
symbols_trie_get(Symbols_Trie *trie, U64 hash, String8 name)
{
	Symbols_Trie *result = 0;
	Symbols_Trie *trie_current = trie;
	U64 hash_shifted = hash;
	for (;;)
	{
		B32 trie_current_zero = trie_current == 0;
		B32 found = !trie_current_zero && String8__match_exact(trie_current->name, name);
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

// NOTE: we need a reference to the root pointer so that in case it's null we can change it.
internal Symbols_Trie *
symbols_trie_get_or_default(Arena *arena, Symbols_Trie_Chunk_List *chunks, Symbols_Trie **root, U64 hash, String8 name)
{
	B32 initialized = 0;
	B32 match = 0;

	Symbols_Trie **trie_current = root;
	U64 hash_shifted = hash;
	for (;;)
	{
		if (*trie_current == 0)
		{
			Symbols_Trie *trie_new = symbols_trie_chunk_list_push(arena, chunks, Symbols_Trie_Chunk__capacity_default);
			trie_new->name = name;
			memory_zero_array(trie_new->children);
			*trie_current = trie_new;
			initialized = 1;
		}

		if (!initialized && String8__match_exact((*trie_current)->name, name))
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

// Always create a new symbol, by overriding a current definition if exist, without dropping it.
internal Symbols_Trie *
symbols_trie_create(Arena *arena, Symbols_Trie_Chunk_List *chunks, Symbols_Trie **root, U64 hash, String8 name)
{
	B32 initialized = 0;

	Symbols_Trie **trie_current = root;
	U64 hash_shifted = hash;
	for (;;)
	{
		if (*trie_current == 0)
		{
			Symbols_Trie *trie_new = symbols_trie_chunk_list_push(arena, chunks, Symbols_Trie_Chunk__capacity_default);
			trie_new->name = name;
			memory_zero_array(trie_new->children);
			*trie_current = trie_new;
			initialized = 1;
		}

		if (!initialized && String8__match_exact((*trie_current)->name, name))
		{
			// We've found an existing definition. Ensure we don't write it in the object file.
			(*trie_current)->symbol.flags |= Symbol_Flags__Redefined;
		}

		B32 break_should = initialized;
		if (break_should)
		{
			break;
		}

		trie_current = &(*trie_current)->children[(hash_shifted >> 62)];
		hash_shifted = hash_shifted << 2;
	}

	return *trie_current;
}

internal Symbols_Trie *
Symbols_Table__last(Symbols_Table *symbols_table)
{
	Symbols_Trie *result = 0;
	if (symbols_table->root)
	{
		Symbols_Trie_Chunk *chunk_last = symbols_table->chunks->last;
		// Valid because there is at least the root.
		result = &chunk_last->nodes[chunk_last->count - 1];
	}

	return result;
}


// TODO: if created, the section index should also be set accordingly.
internal Symbol_Ref *
Symbols_Table__get_or_default(Symbols_Table *symbols_table, String8 name)
{

	U64 hash = FNV_hash_U64(name);
	Symbols_Trie *node = symbols_trie_get_or_default(symbols_table->arena, symbols_table->chunks, &symbols_table->root, hash, name);

	return &node->symbol;
}

// Return the global dot symbol.
internal Symbol_Ref *
Symbols_Table__dot(Symbols_Table *symbols_table)
{

	Symbols_Trie *result = symbols_trie_get(symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_string);
	return &result->symbol;
}

// Create the global dot symbol. This should be done right after symbol table initialization and called once.
internal Symbol_Ref *
Symbols_Table__dot_create(Symbols_Table *symbols_table)
{
	local_persist B32 created = 0;
	assert_m(!created);
	Symbols_Trie *result = symbols_trie_create(symbols_table->arena, symbols_table->chunks, &symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_string);
	created = 1;
	// ELF-SPECIFIC: Update string table offset field.
	// if (node->symbol.elf.string_table_offset == 0)
	// {
	// 	if (last)
	// 	{
	// 		node->symbol.elf.string_table_offset = last->symbol.elf.string_table_offset + last->name.count;
	// 	}
	// }
	return &result->symbol;
}

// Update the global dot symbol with the data of the provided section and create a snapshot of it.
internal Symbol_Ref *
Symbols_Table__dot_snapshot(Symbols_Table *symbols_table, Section *section)
{
	Symbol_Ref *dot = Symbols_Table__dot(symbols_table);
	// Update it.
	dot->fragment          = section->fragment_list.last;
	dot->elf.value         = section->fragment_list.last->size_fixed;
	dot->elf.section_index = section->index;

	Symbols_Trie *snapshot = symbols_trie_chunk_list_push(symbols_table->arena, symbols_table->chunks, Symbols_Trie_Chunk__capacity_default);
	snapshot->name   = dot_symbol_string;
	snapshot->symbol = *dot;

	return &snapshot->symbol;
}

internal Symbol_Ref *
Symbols_Table__create(Symbols_Table *symbols_table, String8 name)
{
	U64 hash = FNV_hash_U64(name);
	Symbols_Trie *last = Symbols_Table__last(symbols_table);
	Symbols_Trie *node = symbols_trie_create(symbols_table->arena, symbols_table->chunks, &symbols_table->root, hash, name);
	// ELF-SPECIFIC: Update string table offset field.
	if (node->symbol.elf.string_table_offset == 0)
	{
		if (last)
		{
			node->symbol.elf.string_table_offset = last->symbol.elf.string_table_offset + last->name.count;
		}
	}
	return &node->symbol;
}

// Creates a new symbols table with a dedicated arena allocator and by creating the global dot symbol.
internal Symbols_Table *
Symbols_Table__new(void)
{
	Arena *arena_symbols_table = Arena__allocate_m();
	Symbols_Table *symbols_table = Arena__push_struct_m(arena_symbols_table, Symbols_Table);

	symbols_table->arena  = arena_symbols_table;
	symbols_table->chunks = Arena__push_struct_m(arena_symbols_table, Symbols_Trie_Chunk_List);

	Symbols_Table__dot_create(symbols_table);
	return symbols_table;
}
