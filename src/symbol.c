void
Symbols_Table_initialize(Symbols_Table *map, Arena *arena)
{
	map->arena    = arena;
	map->capacity = 64;
	map->count    = 0;
	map->entries  = Arena__push_array_m(arena, Symbols_Table_Entry, map->capacity);
	map->slots    = Arena__push_array_m(arena, U32, map->capacity);
	memory_zero(map->entries, sizeof(Symbols_Table_Entry) * map->capacity);
}

internal void
Symbols_Table_grow(Symbols_Table *map)
{
	U32 capacity_new = map->capacity * 2;

	// We perform a copy of just the metadata and the pointers, not the heap allocated data.
	Symbols_Table map_old;
	memory_copy(&map_old, map, sizeof(Symbols_Table));

	Symbols_Table_Entry *entries_new = Arena__push_array_m(map->arena, Symbols_Table_Entry, capacity_new);
	U32 *slots_new = Arena__push_array_m(map->arena, U32, capacity_new);

	map->entries   = entries_new;
	map->slots     = slots_new;
	map->capacity  = capacity_new;
	map->count     = 0;

	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= map_old.capacity;
		if (break_should) { break; }

		// Re-insert in order of insertion.
		U32 slot_old = map_old.slots[index];
		Symbols_Table_Entry *entry_old = &map_old.entries[slot_old];
		if (entry_old->flags & Symbol_Flags__Written)
		{
			Symbols_Table_Entry *entry = Symbols_Table_get(map, entry_old->key);
			*entry = *entry_old;

			map->slots[map->count] = entry->index;
			map->count  += 1;
		}
		index += 1;
	}

	return;
}

Symbols_Table_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key)
{
	assert_m(map->arena);
	assert_m(map->capacity < map->count);

	U32 hash         = Hashmap_hash(key);
	U32 index_modulo = hash & (map->capacity - 1);
	B32 key_found    = 0;
	B32 empty        = 0;
	B32 break_should = 0;

	Symbols_Table_Entry *entry = 0;

	for (;;)
	{
		entry = &map->entries[index_modulo];

		empty = entry->flags == Symbol_Flags__None;
		key_found = !empty
			 && entry->key.count == key.count
			 && memory_match(entry->key.data, key.data, key.count) == 0;

		break_should = empty || key_found;
		if (break_should)
		{
			break;
		}

		index_modulo = (index_modulo + 1) & (map->capacity - 1);
	}

	// Ensure these are always set.
	entry->index = index_modulo;
	entry->key   = key;

	return entry;
}

// Reserve is essentially like `get`, but it grows in case capacity is low. To be used when you want to "put". Uses the
// arena set during initialization for allocation if necessary.
Symbols_Table_Entry *
Symbols_Table_reserve(Symbols_Table *map, String8 key)
{
	assert_always_m(map->entries && "uninitialized hashmap");

	Symbols_Table_Entry *entry = 0;
	B32 overwritten = 0;

	if ((map->count * 100) >= (map->capacity * 70))
	{
		Symbols_Table_grow(map);
	}

	entry       = Symbols_Table_get(map, key);
	overwritten = entry->flags & Symbol_Flags__Written;

	map->slots[map->count] = entry->index & ~(overwritten - 1);
	map->count            += overwritten;

	entry->flags |= Symbol_Flags__Written;

	return entry;
}
