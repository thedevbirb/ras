#ifndef HASHMAP_H
#define HASHMAP_H

#define HASHMAP_INITIAL_CAP 64
#define HASHMAP_LOAD_MAX    70 // percent

////////////////////////////////
// Hash (FNV-1a)

internal U32
Hashmap_hash(String8 key)
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
// Generic Hashmap Declaration
//
// Usage:
//   hashmap_declare_m(Symbols_Table, Symbol_Entry, ELF64_Symbol)
//   hashmap_declare_m(Strings_Table, String_Entry, String8)
//

#define hashmap_declare_m(table_name, entry_name, value_type)                                 \
                                                                                              \
typedef struct entry_name entry_name;                                                         \
struct entry_name                                                                             \
{                                                                                             \
	String8    key;                                                                       \
	value_type value;                                                                     \
	B32        used;                                                                      \
};                                                                                            \
                                                                                              \
typedef struct table_name table_name;                                                         \
struct table_name                                                                             \
{                                                                                             \
	entry_name *entries;                                                                  \
	U32 capacity;                                                                         \
	U32 count;                                                                            \
	Arena *arena;                                                                         \
};                                                                                            \
                                                                                              \
internal void                                                                                 \
table_name##_initialize(table_name *map, Arena *arena);                                       \
internal entry_name *                                                                         \
table_name##_get(table_name *map, String8 key);                                               \
internal B32                                                                                  \
table_name##_put(table_name *map, String8 key, value_type value);                             \

////////////////////////////////
// Generic Hashmap Implementation

#define hashmap_implement_m(table_name, entry_name, value_type)                               \
                                                                                              \
internal void                                                                                 \
table_name##_initialize(table_name *map, Arena *arena)                                        \
{                                                                                             \
	map->arena    = arena;                                                                \
	map->capacity = HASHMAP_INITIAL_CAP;                                                  \
	map->count    = 0;                                                                    \
	map->entries  = Arena_push_array_m(arena, entry_name, map->capacity);                 \
	os_memory_zero(map->entries, sizeof(entry_name) * map->capacity);                     \
}                                                                                             \
                                                                                              \
internal B32                                                                                  \
table_name##_find_slot(table_name *map, String8 key, U32 *slot_out)                           \
{                                                                                             \
	assert_always_m(map->capacity && "uninitialized map");                                \
                                                                                              \
	U32 hash  = Hashmap_hash(key);                                                        \
	U32 index = hash & (map->capacity - 1);                                               \
	B32 key_found = 0;                                                                    \
                                                                                              \
	for (;;)                                                                              \
	{                                                                                     \
		entry_name *entry = &map->entries[index];                                     \
                                                                                              \
		B32 empty = !entry->used;                                                     \
		key_found = !empty &&                                                         \
		            entry->key.count == key.count &&                                  \
		            os_memory_match(entry->key.data, key.data, key.count) == 0;       \
                                                                                              \
		B32 break_should = empty || key_found;                                        \
		if (break_should)                                                             \
		{                                                                             \
			*slot_out = index;                                                    \
			break;                                                                \
		}                                                                             \
                                                                                              \
		index = (index + 1) & (map->capacity - 1);                                    \
	}                                                                                     \
                                                                                              \
	return key_found;                                                                     \
}                                                                                             \
                                                                                              \
internal void                                                                                 \
table_name##_grow(table_name *map)                                                            \
{                                                                                             \
	U32 capacity_new = map->capacity * 2;                                                 \
                                                                                              \
	entry_name *entries_new = Arena_push_array_m(map->arena, entry_name, capacity_new);   \
	os_memory_zero(entries_new, sizeof(entry_name) * capacity_new);                       \
                                                                                              \
	entry_name *entries_old = map->entries;                                               \
	U32 capacity_old        = map->capacity;                                              \
                                                                                              \
	map->entries  = entries_new;                                                          \
	map->capacity = capacity_new;                                                         \
	map->count    = 0;                                                                    \
                                                                                              \
	U32 i = 0;                                                                            \
	for (;;)                                                                              \
	{                                                                                     \
		B32 break_should = i >= capacity_old;                                         \
		if (break_should) { break; }                                                  \
                                                                                              \
		entry_name *entry = &entries_old[i];                                          \
		if (entry->used)                                                              \
		{                                                                             \
			table_name##_put(map, entry->key, entry->value);                      \
		}                                                                             \
		i += 1;                                                                       \
	}                                                                                     \
}                                                                                             \
                                                                                              \
internal B32                                                                                  \
table_name##_put(table_name *map, String8 key, value_type value)                              \
{                                                                                             \
	assert_always_m(map->entries && "uninitialized hashmap");                             \
                                                                                              \
	if ((map->count * 100) >= (map->capacity * HASHMAP_LOAD_MAX))                         \
	{                                                                                     \
		table_name##_grow(map);                                                       \
	}                                                                                     \
                                                                                              \
	U32 slot  = 0;                                                                        \
	B32 found = table_name##_find_slot(map, key, &slot);                                  \
                                                                                              \
	entry_name *entry = &map->entries[slot];                                              \
	entry->key   = key;                                                                   \
	entry->value = value;                                                                 \
	entry->used  = 1;                                                                     \
	map->count  += !found;                                                                \
                                                                                              \
	return found;                                                                         \
}                                                                                             \
                                                                                              \
internal entry_name *                                                                         \
table_name##_get(table_name *map, String8 key)                                                \
{                                                                                             \
	entry_name *result = 0;                                                               \
                                                                                              \
	if (map->entries)                                                                     \
	{                                                                                     \
		U32 slot  = 0;                                                                \
		B32 found = table_name##_find_slot(map, key, &slot);                          \
                                                                                              \
		if (found)                                                                    \
		{                                                                             \
			result = &map->entries[slot];                                         \
		}                                                                             \
	}                                                                                     \
                                                                                              \
	return result;                                                                        \
}

#endif // HASHMAP_H
