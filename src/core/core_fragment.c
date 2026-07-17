internal Fragment *
Fragments__push_empty_fragment(Fragments *fragments, U32 location)
{
        Fragment *fragment = Arena__push_struct_m(fragments->arena, Fragment);
        fragment->location = location;

        SLL_queue_push_m(fragments->first, fragments->last, fragment);
        fragments->count += 1;

        return fragment;
}

// TODO(medium): still very NOT happy with this equivalent to `frag_grow`, and it is duplicated, delicate logic of the
// `Fragments__push`.
internal void
Fragments__ensure(Fragments *fragments, U32 size)
{
        U64  capacity_left             = fragments->arena->reserved_size - fragments->arena->offset;
        B32  arena_block_new_needed    = capacity_left < size;

        assert_always_m(capacity_left < fragments->arena->reserved_size && "underflow");
        if (arena_block_new_needed)
        {
                // Fill the capacity left of the arena block, so that we're sure to have a new block later.
                Arena *block_before = fragments->arena->current;
                Arena__push_array_m(fragments->arena, U8, capacity_left);
                assert_always_m(fragments->arena->current == block_before);
        }
        return;
}

// Push `size` bytes into the fragment, returning a pointer to zeroed data of the same size.
internal U8 *
Fragments__push(Fragments *fragments, U32 location, U32 size)
{
        U8  *result                    = 0;
        U64  capacity_left             = fragments->arena->reserved_size - fragments->arena->offset;
        B32  arena_block_new_needed    = capacity_left < size;
        B32  fragment_seal_last_needed = arena_block_new_needed && fragments->last;
        B32  fragment_new_needed       = arena_block_new_needed || fragments->last == 0;
        B32  fragment_new_created      = 0;

        assert_always_m(capacity_left < fragments->arena->reserved_size && "underflow");
        assert_always_m(fragments->last->relax_state == Relax_State__None && "cannot push to sealed fragment");

        if (arena_block_new_needed)
        {
                // Fill the capacity left of the arena block, so that we're sure to have a new block later.
                Arena *block_before = fragments->arena->current;
                Arena__push_array_m(fragments->arena, U8, capacity_left);
                assert_always_m(fragments->arena->current == block_before);
        }

        if (fragment_seal_last_needed)
        {
                // We have to "seal" the current fragment, and switch to another arena block.
                // We that also the new fragment header is on the new arena.
                Fragment__wane(fragments->last);
        }

        if (fragment_new_needed)
        {
                Fragments__push_empty_fragment(fragments, location);
                fragment_new_created = 1;
        }

        result = Arena__push_array_m(fragments->arena, U8, size);

        if (fragment_new_created)
        {
                fragments->last->data = result;
        }

        fragments->last->data_size += size;

        return result;
}

// Push a variable amount of bytes, capped by `Fragment__data_variable_size_max`, into the fragment.
// This operations seals the current fragment with the provided information, and creates a blank one.
internal void
Fragments__variable
(
        Fragments   *fragments,
        U32          location,
        Relax_Info   relax_info,
        Relax_State  relax_state,
        U8          *data_variable,
        U8           data_variable_size
)
{
        data_variable_size = min_m(data_variable_size, Fragment__data_variable_size_max);
        Fragment *sealed = fragments->last;

        memory_copy(sealed->data_variable_buffer, data_variable, data_variable_size);
        sealed->data_variable_size  = data_variable_size;
        sealed->relax_info          = relax_info;
        sealed->relax_state         = relax_state;

        // We have to create another fragment since variable data seal it.
        Fragments__push_empty_fragment(fragments, location);

        return;
}

// Seal the current fragment with a fill pattern.
internal void
Fragments__fill(Fragments *fragments, U32 location, Fill fill)
{
        Relax_Info relax_info = { .fill_expression = fill.repeat };
        U8 *pattern = (U8 *)&fill.pattern;
        Fragments__variable
        (
                fragments,
                location,
                relax_info,
                Relax_State__Fill,
                pattern,
                fill.pattern_size
        );
        return;
}

internal void
Fragments__align(Fragments *fragments, U32 location, Alignment alignment)
{
        assert_always_m(pow_2_is_m(alignment.boundary) || !alignment.boundary);

        // TODO(check-gas): GNU as does some special handling of the absolute section. Since no variable-sized data exist on the
        // absolute section, it can be expanded to match the required alignment right away.

        Relax_Info relax_info =
        {
                .alignment =
                {
                        .boundary = alignment.boundary,
                        .write_size_max = alignment.write_size_max
                }
        };

        U8 *pattern = (U8 *)&alignment.pattern;
        Fragments__variable
        (
                fragments,
                location,
                relax_info,
                Relax_State__Align,
                pattern,
                alignment.pattern_size
        );

        return;
}

internal void
Fragment__wane(Fragment *fragment)
{
        assert_always_m(fragment->relax_state == Relax_State__None);
        fragment->data_variable_size = 0;
        fragment->relax_state        = Relax_State__Fill;
        fragment->relax_info         = (Relax_Info){0};
}
