internal void
write_object_file
(
        Arena                   *arena,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        // Symbols_Table           *symbols_table,
        Sections_Table          *sections_table,
        Fixups                  *fixups
)
{
        // Finish sections
        for (;;)
        {
                Sections_Trie_Chunk *chunk = sections_table->chunks->first;

                U32 index = 0;
                for (;;)
                {
                        if (index >= chunk->count)
                        {
                                break;
                        }
                        Section *section = &chunk->nodes[index].section;
                        Section__finish(section);
                        index += 1;
                }


                if (!chunk->next)
                {
                        break;
                }

                chunk = chunk->next;
        }

        // Relax
        // TODO(medium): max iterations?
        U32 relaxation_passes = 0;
        for (;;)
        {
                relaxation_passes += 1;
                B32 changed = 0;
                for (;;)
                {
                        Sections_Trie_Chunk *chunk = sections_table->chunks->first;

                        U32 index = 0;
                        for (;;)
                        {
                                if (index >= chunk->count)
                                {
                                        break;
                                }
                                Section *section = &chunk->nodes[index].section;
                                B32 relax_changed_address = Section__relax(section, arena, diagnostics);
                                changed |= relax_changed_address;
                                index += 1;
                        }


                        if (!chunk->next)
                        {
                                break;
                        }

                        chunk = chunk->next;
                }

                if (!changed)
                {
                        // Finally done!
                        break;
                }
        }
        printf("relaxation completed in %u passes\n", relaxation_passes);

        // Convert frag to fill
        for (;;)
        {
                Sections_Trie_Chunk *chunk_current = sections_table->chunks->first;

                U32 index = 0;
                for (;;)
                {
                        if (index >= chunk_current->count)
                        {
                                break;
                        }
                        Section  *section  = &chunk_current->nodes[index].section;
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


                if (!chunk_current->next)
                {
                        break;
                }

                chunk_current = chunk_current->next;
        }

        // Size sections
        for (;;)
        {
                Sections_Trie_Chunk *chunk = sections_table->chunks->first;
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


                if (!chunk->next)
                {
                        break;
                }

                chunk = chunk->next;
        }
}
