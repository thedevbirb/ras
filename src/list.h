#ifndef LIST_H
#define LIST_H

#define list_capacity_initial 64

#define list_declare_m(type)                                                                                        \
                                                                                                                    \
typedef struct type##_List type##_List;                                                                             \
struct type##_List                                                                                                  \
{                                                                                                                   \
	type  *data;                                                                                                \
	Arena *arena;                                                                                               \
	U32    capacity;                                                                                            \
	U32    count;                                                                                               \
};                                                                                                                  \
                                                                                                                    \
void                                                                                                                \
type##_List_initialize(type##_List *list, Arena *arena);                                                            \
                                                                                                                    \
void                                                                                                                \
type##_List_push(type##_List *list, type element);                                                                  \

// implementation

#define list_implement_m(type)                                                                                      \
                                                                                                                    \
internal void                                                                                                       \
type##_List_grow(type##_List *list)                                                                                 \
{                                                                                                                   \
	U32 capacity_new = list->capacity * 2;                                                                      \
	type *data = Arena_push_array_m(list->arena, type, capacity_new);                                           \
	os_memory_copy(data, list->data, list->count * sizeof(type));                                               \
                                                                                                                    \
	list->data     = data;                                                                                      \
	list->capacity = capacity_new;                                                                              \
                                                                                                                    \
	return;                                                                                                     \
}                                                                                                                   \
                                                                                                                    \
void                                                                                                                \
type##_List_initialize(type##_List *list, Arena *arena)                                                             \
{                                                                                                                   \
	type *data = Arena_push_array_m(arena, type, list_capacity_initial);                                        \
                                                                                                                    \
	type##_List list_initialized =                                                                              \
	{                                                                                                           \
		.data     = data,                                                                                   \
		.arena    = arena,                                                                                  \
		.capacity = list_capacity_initial,                                                                  \
		.count    = 0,                                                                                      \
	};                                                                                                          \
                                                                                                                    \
	*list = list_initialized;                                                                                   \
                                                                                                                    \
	return;                                                                                                     \
}                                                                                                                   \
                                                                                                                    \
void                                                                                                                \
type##_List_push(type##_List *list, type element)                                                                   \
{                                                                                                                   \
	assert_always_m(list->arena && "list uninitialized");                                                       \
	B32 full = list->count == list->capacity;                                                                   \
	if (full)                                                                                                   \
	{                                                                                                           \
		type##_List_grow(list);                                                                             \
	}                                                                                                           \
                                                                                                                    \
	list->data[list->count] = element;                                                                          \
	list->count += 1;                                                                                           \
                                                                                                                    \
	return;                                                                                                     \
}                                                                                                                   \

#endif // LIST_H

