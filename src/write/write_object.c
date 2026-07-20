internal void
write_object_file
(
        Arena                   *arena,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Sections_Table          *sections_table,
        Fixups                  *fixups
)
{
        // NOTE: GNU as creates it after finishing to size all sections, and then after creating it, it if finished and
        // relaxed. While it may result in a couple more iterations, I think we can just do it now.
        Section__create_riscv_attributes(sections_table);
        Sections_Table__finish(sections_table);
        Sections_Table__relax(sections_table, arena, diagnostics);

        // Convert frag to fill
        {
        Sections_Trie_Chunk *chunk = sections_table->chunks->first;
        for (;;)
        {
                if (!chunk)
                {
                        break;
                }

                U32 index = 0;
                for (;;)
                {
                        if (index >= chunk->count)
                        {
                                break;
                        }
                        Section  *section  = &chunk->nodes[index].section;
                        Fragment *fragment = section->fragments.first;
                        for (;;)
                        {
                                if (!fragment)
                                {
                                        break;
                                }

                                Fragment__convert_to_fill(fragment, section, expressions, arena, fixups);
                                fragment = fragment->next;
                        }

                        index += 1;
                }

                chunk = chunk->next;
        }
        }

        // Size sections
        {
        Sections_Trie_Chunk *chunk = sections_table->chunks->first;
        for (;;)
        {
                if (!chunk)
                {
                        break;
                }

                U32 index = 0;
                for (;;)
                {
                        if (index >= chunk->count)
                        {
                                break;
                        }
                        Section  *section       = &chunk->nodes[index].section;
                        Fragment *fragment_last = section->fragments.last;

                        assert_always_m(fragment_last->relax_state == Relax_State__Fill);
                        assert_always_m(fragment_last->data_variable_size == 0);

                        section->elf.size = fragment_last->object_file_offset + fragment_last->data_size;

                        if (section->elf.entry_size && (section->elf.size % section->elf.entry_size))
                        {
                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                diagnostic->kind       = Diagnostic_Kind__Warning;
                                diagnostic->message    = String8__format(arena, "section '%*s' size (%u bytes) is not a multiple of its entry size (%u bytes)",
                                                                         section->name.count, section->name.data, section->elf.size, section->elf.entry_size);
                                diagnostic->location   = section->location;
                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                        }

                        index += 1;
                }


                chunk = chunk->next;
        }
        }

        // TODO(low): another hint for the expressions section :), this can result in some footguns.
        Symbols_Table__finalize(symbols_table, arena, diagnostics);
        Expressions__finalize(expressions, arena, diagnostics);
}
