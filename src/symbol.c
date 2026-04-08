internal void
Symbols_Table_initialize(Symbols_Table *map, Arena *arena)
{
	map->arena    = arena;
	map->capacity = 64;
	map->count    = 0;
	map->entries  = Arena_push_array_m(arena, Symbols_Table_Entry, map->capacity);
	map->slots    = Arena_push_array_m(arena, U32, map->capacity);
	os_memory_zero(map->entries, sizeof(Symbols_Table_Entry) * map->capacity);
}

internal B32
Symbols_Table_find_slot(Symbols_Table *map, String8 key, U32 *slot_out)
{
	assert_always_m(map->capacity && "uninitialized map");

	U32 hash  = Hashmap_hash(key);
	U32 index = hash & (map->capacity - 1);
	B32 key_found = 0;

	for (;;)
	{
		Symbols_Table_Entry *entry = &map->entries[index];

		B32 empty = !entry->used;
		key_found = !empty &&
		            entry->key.count == key.count &&
		            os_memory_match(entry->key.data, key.data, key.count) == 0;

		B32 break_should = empty || key_found;
		if (break_should)
		{
			*slot_out = index;
			break;
		}

		index = (index + 1) & (map->capacity - 1);
	}

	return key_found;
}

internal void
Symbols_Table_grow(Symbols_Table *map)
{
	U32 capacity_new = map->capacity * 2;

	// We perform a copy of just the metadata and the pointers, not the heap allocated data.
	Symbols_Table map_old;
	os_memory_copy(&map_old, map, sizeof(Symbols_Table));

	Symbols_Table_Entry *entries_new = Arena_push_array_m(map->arena, Symbols_Table_Entry, capacity_new);
	U32 *slots_new = Arena_push_array_m(map->arena, U32, capacity_new);

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
		if (entry_old->used)
		{
			// Find a new slot, and put it there.
			U32 slot = 0;
			B32 found = Symbols_Table_find_slot(map, entry_old->key, &slot);
			assert_always_m(!found && "map contains duplicates");

			Symbols_Table_Entry *entry = &map->entries[slot];
			entry->key   = entry_old->key;
			entry->value = entry_old->value;
			entry->used  = 1;
			entry->index = map->count;

			map->slots[map->count] = slot;
			map->count  += 1;
		}
		index += 1;
	}

	return;
}

internal Vec2_U32 // Slot, found
Symbols_Table_put(Symbols_Table *map, String8 key, ELF64_Symbol value)
{
	assert_always_m(map->entries && "uninitialized hashmap");

	if ((map->count * 100) >= (map->capacity * 70))
	{
		Symbols_Table_grow(map);
	}

	U32 slot  = 0;
	U32 found = (U32)Symbols_Table_find_slot(map, key, &slot);

	value.string_table_offset = map->string_table_section_size;

	Symbols_Table_Entry *entry = &map->entries[slot];
	entry->key   = key;
	entry->value = value;
	entry->used  = 1;
	entry->index = map->count;

	if (!found)
	{
		map->slots[map->count] = slot;
		map->count  += 1;
		map->string_table_section_size = key.count + 1; // null-termination
	}

	Vec2_U32 slot_and_found = { .x = slot, .y = found};

	return slot_and_found;
}

// TODO: decide whether to return the non-pointer version. In practice, entry will be at least zero-initialized.

internal Symbols_Table_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key)
{
	Symbols_Table_Entry *result = 0;

	if (map->entries)
	{
		U32 slot  = 0;
		B32 found = Symbols_Table_find_slot(map, key, &slot);

		if (found)
		{
			result = &map->entries[slot];
		}
	}

	return result;
}
