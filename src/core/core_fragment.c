internal U64
Arena__ensure_contiguous_for_size(Arena *arena, U32 size)
{
        U64 capacity_left = arena->reserved_size - arena->offset;
        if (capacity_left < size)
        {
                // Fill the capacity left of the arena block, so that we're sure to have a new block later.
                Arena *block_before = arena->current;
                Arena__push_array_m(arena, U8, capacity_left);
                assert_always_m(arena->current == block_before);
        }
        return capacity_left;
}

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size.
//
// NOTE: since we're using an arena allocator, it might happen that the current chunk hasn't enough reserved size for it.
// In such case, we need to "seal" the current fragment and switch to another arena block, creating a new one.
internal U8 *
Fragment_List__push(Fragment_List *fragments, Arena *arena, U32 location, U32 size)
{
        U8 *result = 0;

        U64 capacity_left = Arena__ensure_contiguous_for_size(arena, size);
        if (capacity_left < size || fragments->last == 0)
        {
                // We have to "seal" the current fragment, and switch to another arena block.
                // We that also the new fragment header is on the new arena.
                Fragment *fragment = Arena__push_struct_m(arena, Fragment);
                fragment->location = location;
                SLL_queue_push_m(fragments->first, fragments->last, fragment);
                fragments->count += 1;
        }
        else
        {
                result = Arena__push_array_m(arena, U8, size);
        }

        return result;
}

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size and increasing the fragment
// `size_fixed`.
internal U8 *
Fragment_List__fixed(Fragment_List *fragments, Arena *arena, U32 location, U32 size)
{
        U8 *data = Fragment_List__push(fragments, arena, location, size);
        fragments->last->size_fixed += size;
        return data;
}


// Push a variable (`size_variable`) amount of bytes, capped by `size_max`, into the fragment.
// This effectively pushes `size_max` bytes into it, while accounting the size currently used by the relaxable
// instruction.
//
// This operations seals the current fragment with the provided information, and creates a blank one.
internal U8 *
Fragment_List__variable
(
        Fragment_List *fragments,
        Arena         *arena,
        U32            location,
        U32            size_max,
        U32            size_variable,
        U32            expression_index,
        U32            subtype,
        Relax_State    type
)
{
        assert_always_m(size_variable <= size_max);
        U8 *data = Fragment_List__push(fragments, arena, location, size_max);
        Fragment *sealed = fragments->last;

        sealed->size_variable    = size_variable;
        sealed->expression_index = expression_index;
        sealed->subtype          = subtype;
        sealed->type             = type;

        // We have to create another fragment since variable data seal it.
        Fragment *fragment = Arena__push_struct_m(arena, Fragment);
        fragment->location = location;
        SLL_queue_push_m(fragments->first, fragments->last, fragment);
        fragments->count += 1;

        return data;
}

// Seal the current fragment with a fill pattern.
internal void
Fragment_List__fill(Fragment_List *fragments, Arena *arena, U32 location, U32 repeat_expression_index, S64 pattern, U8 size)
{
        assert_always_m(size <= 8);
        U8 *data = Fragment_List__variable
        (
                fragments,
                arena,
                location,
                size,
                size,
                repeat_expression_index,
                0,
                Relax_State__Fill
        );
        memory_copy(data, (U8 *)&pattern, size);
        return;
}

internal void
Fragment_List__align(Fragment_List *fragments, Arena *arena, U32 location, U32 alignment_expression_index, U8 pattern, U8 alignment_max)
{
        // NOTE: double check more precisely how GNU as overloads some fields, and then decide whether to keep the same
        // layout or not.
        // TODO(check-gas): GNU as does some special handling of the absolute section. Since no variable-sized data exist on the
        // absolute section, it can be expanded to match the required alignment right away.

        U8 size_max = 1;
        U8 size_variable = 1;
        U8 *data = Fragment_List__variable
        (
                fragments,
                arena,
                location,
                size_max,
                size_variable,
                alignment_expression_index,
                alignment_max,
                Relax_State__Fill
        );
        data[0] = pattern;

        return;
}
