// Symbol trie utilities.

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
                        String8 name_duplicated = String8__duplicate(arena, name);
                        trie_new->name = name_duplicated;
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

// Label numeric utilities

internal Label_Numeric *
label_numeric_chunk_list_push(Arena *arena, Label_Numeric_Chunk_List *chunks, U64 capacity)
{
        if (chunks->last == 0 || chunks->last->count >= chunks->last->capacity)
        {
                Label_Numeric_Chunk *chunk_new = Arena__push_struct_m(arena, Label_Numeric_Chunk);
                chunk_new->nodes = Arena__push_array_m(arena, Label_Numeric, capacity);
                chunk_new->capacity = capacity;

                SLL_queue_push_m(chunks->first, chunks->last, chunk_new);
                chunks->count += 1;
        }

        Label_Numeric_Chunk *chunk_last = chunks->last;
        Label_Numeric *result = &chunk_last->nodes[chunk_last->count];
        chunk_last->count += 1;

        return result;
}

internal Label_Numeric *
label_numeric_get_or_default(Arena *arena, Label_Numeric_Chunk_List *chunks, U64 capacity, U32 label)
{
        B32 found = 0;
        Label_Numeric_Chunk *current = chunks->first;
        Label_Numeric *result = 0;

        for (;;)
        {
                B32 break_should = found || current == 0;
                if (break_should)
                {
                        break;
                }

                U32 index = 0;
                for (;;)
                {
                        B32 break_should_inner = found || index >= current->count;
                        if (break_should_inner)
                        {
                                break;
                        }
                        result = &(current->nodes[index]);
                        found = result->number == label;

                        index += 1;
                }

                current = current->next;
        }

        if (!found)
        {
                result = label_numeric_chunk_list_push(arena, chunks, capacity);
        }

        return result;
}

internal String8
label_numeric_string(Arena *arena, Label_Numeric label)
{
        String8 result = String8__format(arena, ".L%u\x02%u", label.number, label.instances);
        return result;
}

// Symbols Table API

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

// Get or create a default symbol given its name.
internal Symbol_Ref *
Symbols_Table__get(Symbols_Table *symbols_table, String8 name)
{

        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node = symbols_trie_get(symbols_table->root, hash, name);

        Symbol_Ref *result = node ? &node->symbol : 0;
        return result;
}

// Get or create a default symbol given its name.
internal Symbol_Ref *
Symbols_Table__get_or_default(Symbols_Table *symbols_table, String8 name)
{

        U64 hash = FNV_hash_U64(name);
        Symbols_Trie *node = symbols_trie_get_or_default(symbols_table->arena, symbols_table->chunks, &symbols_table->root, hash, name);

        return &node->symbol;
}

// Return the global dot symbol trie.
internal Symbols_Trie *
Symbols_Table__dot(Symbols_Table *symbols_table)
{

        Symbols_Trie *result = symbols_trie_get(symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_string);
        return result;
}

internal void
Symbol_Ref__update_section(Symbol_Ref *symbol, Section *section)
{
        symbol->fragment          = section->fragment_list.last;
        symbol->elf.value         = section->fragment_list.last->size_fixed;
        symbol->elf.section_index = section->index;

        return;
}

internal Symbol_Ref *
Symbols_Table__clone(Symbols_Table *symbols_table, Symbol_Ref *symbol, String8 name)
{
        Symbols_Trie *clone = symbols_trie_chunk_list_push(symbols_table->arena, symbols_table->chunks, Symbols_Trie_Chunk__capacity_default);
        clone->name   = String8__duplicate(symbols_table->arena, name);
        clone->symbol = *symbol;
        return &clone->symbol;
}

internal Symbol_Ref *
Symbols_Table__create(Symbols_Table *symbols_table, String8 name)
{
        U64 hash = FNV_hash_U64(name);
        // Symbols_Trie *last = Symbols_Table__last(symbols_table);
        Symbols_Trie *node = symbols_trie_create(symbols_table->arena, symbols_table->chunks, &symbols_table->root, hash, name);
        // ELF-SPECIFIC: Update string table offset field.
        // if (node->symbol.elf.string_table_offset == 0)
        // {
        //      if (last)
        //      {
        //              node->symbol.elf.string_table_offset = last->symbol.elf.string_table_offset + last->name.count;
        //      }
        // }
        return &node->symbol;
}

internal Label_Numeric *
Symbols_Table__label_numeric_get_or_default(Symbols_Table *symbols_table, U32 number)
{
        Label_Numeric *label = label_numeric_get_or_default(symbols_table->arena, symbols_table->chunks_label, Label_Numeric_Chunk__capacity_default, number);
        return label;
}

// Creates a new symbols table with a dedicated arena allocator and by creating the global dot symbol.
internal Symbols_Table *
Symbols_Table__new(void)
{
        Arena *arena_symbols_table = Arena__allocate_m();
        Symbols_Table *symbols_table = Arena__push_struct_m(arena_symbols_table, Symbols_Table);

        symbols_table->arena        = arena_symbols_table;
        symbols_table->chunks_label = Arena__push_struct_m(arena_symbols_table, Label_Numeric_Chunk_List);
        symbols_table->chunks       = Arena__push_struct_m(arena_symbols_table, Symbols_Trie_Chunk_List);

        // Initialization steps

        // 1. Create global dot
        String8 dot_symbol_name = String8__duplicate(symbols_table->arena, dot_symbol_string);
        symbols_trie_create(symbols_table->arena, symbols_table->chunks, &symbols_table->root, DOT_SYMBOL_HASH, dot_symbol_name);
        // 2. Place numeric labels 0..9 at beginning of chunks.
        U8 index = 0;
        for (;;)
        {
                if (index == 10)
                {
                        break;
                }
                Label_Numeric *label = label_numeric_chunk_list_push(symbols_table->arena, symbols_table->chunks_label, Label_Numeric_Chunk__capacity_default);
                label->number = index;

                index += 1;
        }


        return symbols_table;
}
