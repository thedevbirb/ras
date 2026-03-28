#ifndef SYMBOL_HASHMAP_H
#define SYMBOL_HASHMAP_H

#define SYMBOL_HASHMAP_INITIAL_CAP 64
#define SYMBOL_HASHMAP_LOAD_MAX    70 // percent

typedef struct Symbol_Entry Symbol_Entry;
struct Symbol_Entry
{
	String8      key;
	ELF64_Symbol value;
	B32 used;
};

typedef struct Symbols_Table Symbols_Table;
struct Symbols_Table
{
	Symbol_Entry *entries;

	U32 capacity; // always power of two
	U32 count;

	Arena *arena;
};

////////////////////////////////
// API

internal void
Symbols_Table_initialize(Symbols_Table *map, Arena *arena);

internal Symbol_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key);

internal B32
Symbols_Table_put(Symbols_Table *map, String8 key, ELF64_Symbol value);

#endif // SYMBOL_HASHMAP_H

#ifdef HASHMAP_IMPLEMENTATION

////////////////////////////////
// Hash (FNV-1a)

internal U32
Symbols_Table_hash(String8 key)
{
	U32 hash = 2166136261u;

	for (U64 i = 0; i < key.count; i++)
	{
		hash ^= key.data[i];
		hash *= 16777619u;
	}

	return hash;
}

////////////////////////////////
// Init

void
Symbols_Table_initialize(Symbols_Table *map, Arena *arena)
{
	map->arena    = arena;
	map->capacity = SYMBOL_HASHMAP_INITIAL_CAP;
	map->count    = 0;

	map->entries = Arena_push_array_m(arena, Symbol_Entry, map->capacity);
	os_memory_zero(map->entries, sizeof(Symbol_Entry) * map->capacity);

	return;
}

////////////////////////////////
// Internal: Find Slot
//
// IMPORTANT:
// - This is where collision handling happens
// - Uses linear probing
// - Returns:
//     *slot_out = index of match OR insertion point
//     returns true if key found, false if empty slot
//

internal B32
Symbols_Table_find_slot(Symbols_Table *map, String8 key, U32 *slot_out)
{
	assert_always_m(map->capacity && "uninitialized map");

	U32 hash  = Symbols_Table_hash(key);
	U32 index = hash & (map->capacity - 1);

	B32 key_found = 0;

	for (;;)
	{
		Symbol_Entry *entry = &map->entries[index];

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

		// --- COLLISION HANDLING ---
		// Linear probing step
		index = (index + 1) & (map->capacity - 1);
	}

	return key_found;
}

////////////////////////////////
// Internal: Grow
//
// Rehashes all entries into new table
//

internal void
Symbols_Table_grow(Symbols_Table *map)
{
	U32 capacity_new = map->capacity * 2;

	Symbol_Entry *entries_new = Arena_push_array_m(map->arena, Symbol_Entry, capacity_new);
	os_memory_zero(entries_new, sizeof(Symbol_Entry) * capacity_new);

	Symbol_Entry *entries_old = map->entries;
	U32 capacity_old          = map->capacity;

	map->entries  = entries_new;
	map->capacity = capacity_new;
	map->count    = 0;

	// Reinsert entries (rehash required!)
	U32 index = 0;
	for (;;)
	{
		B32 break_should = index >= capacity_old;
		if (break_should)
		{
			break;
		}

		Symbol_Entry *entry = &entries_old[index];
		if (entry->used)
		{
			Symbols_Table_put(map, entry->key, entry->value);
		}
		index += 1;
	}
	return;
}

////////////////////////////////
// Put

// Return true if a value has been updated.
B32
Symbols_Table_put(Symbols_Table *map, String8 key, ELF64_Symbol value)
{
	assert_always_m(map->entries && "uninitialized hashmap");

	// Grow if load factor exceeded
	if ((map->count * 100) >= (map->capacity * SYMBOL_HASHMAP_LOAD_MAX))
	{
		Symbols_Table_grow(map);
	}

	U32 slot = 0;
	B32 found = Symbols_Table_find_slot(map, key, &slot);

	Symbol_Entry *entry = &map->entries[slot];

	// We can write these fields regardless of whether it has been found or not.
	entry->value = value;
	entry->used  = 1;
	entry->key   = key;   // NOTE: assumes key lifetime >= hashmap
	entry->value = value;
	map->count  += !found;

	return found;
}

////////////////////////////////
// Get

Symbol_Entry *
Symbols_Table_get(Symbols_Table *map, String8 key)
{
	Symbol_Entry *result = 0;

	if (map->entries)
	{
		U32 slot = 0;
		B32 found = Symbols_Table_find_slot(map, key, &slot);

		if (found)
		{
			result = &map->entries[slot];
		}
	}

	return result;
}

#endif // HASHMAP_IMPLEMENTATION
