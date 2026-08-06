internal U64
write_object_file
(
        Arena           *arena,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        Options         *options,
        S32              file_descriptor_out
)
{
        Symbol_Ref *symbol_riscv_attributes = Symbols_Table__create_section_riscv_attributes(symbols_table, &options->attributes);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_riscv_attributes->section);

        for each_node_m(symbols_table->section_first, section)
        {
                Section__finish(section);
        }

        U32 relaxation_passes = 0;
        for (;;)
        {
                relaxation_passes += 1;
                B32 changed = 0;
                for each_node_m(symbols_table->section_first, section)
                {
                        B32 relax_changed_address = Section__relax(section, arena, diagnostics);
                        changed |= relax_changed_address;
                }

                if (!changed)
                {
                        // Finally done!
                        break;
                }
        }
        unused_m(relaxation_passes);
        // printf("relaxation completed in %u passes\n", relaxation_passes);

        // Convert all fragments to fill variants, and compute section size.
        for each_node_m(symbols_table->section_first, section)
        {
                for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
                {
                        Fragment__convert_to_fill(fragment, section, expressions, symbols_table->arena);
                }

                Fragment *fragment_last = section->fragments.last;

                assert_always_m(fragment_last->relax_state == Relax_State__Fill);
                assert_always_m(fragment_last->data_variable_size == 0);

                section->elf.size = fragment_last->object_file_offset + fragment_last->data_size;

                if (section->elf.entry_size && (section->elf.size % section->elf.entry_size))
                {
                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                        diagnostic->kind       = Diagnostic_Kind__Warning;
                        diagnostic->message    = String8__format(arena, "section '%.*s' size (%u bytes) is not a multiple of its entry size (%u bytes)",
                                                                 String8__varg(*section->symbol->name), section->elf.size, section->elf.entry_size);
                        diagnostic->location   = section->symbol->location;
                }
        }

        for each_node_m(symbols_table->first, symbol)
        {
                Symbol_Ref__resolve(symbol, diagnostics, Resolve_Level__Finalize);
        }
        Expressions__finalize(expressions, diagnostics);
        for each_node_m(symbols_table->section_first, section)
        {
                Section__resolve_fixups(section, symbols_table->arena, options, diagnostics);
        }

        // Let's avoid off-by-one errors.
        Symbols_Table__ensure_undefined_present(symbols_table);

        // Add the mandatory ending sections last: `.symtab`, `.strtab`, `.shstrtab`.
        Symbol_Ref *symbol_symtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".symtab"));
                    symbol_symtab->flags |= Symbol_Flags__Skip;
        Symbols_Table__create_section(symbols_table, symbol_symtab);
        symbol_symtab->section->elf.entry_size = sizeof(ELF64_Symbol);
        symbol_symtab->section->elf.size = 0;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_symtab->section);

        Symbol_Ref *symbol_strtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".strtab"));
                    symbol_strtab->flags |= Symbol_Flags__Skip;
        Symbols_Table__create_section(symbols_table, symbol_strtab);
        symbol_strtab->section->elf.entry_size = 1;
        symbol_symtab->section->elf.size = 0;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_strtab->section);

        Symbol_Ref *symbol_shstrtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".shstrtab"));
                    symbol_shstrtab->flags |= Symbol_Flags__Skip;
        Symbols_Table__create_section(symbols_table, symbol_shstrtab);
        symbol_strtab->section->elf.entry_size = 1;
        symbol_symtab->section->elf.size = 0;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_shstrtab->section);

        // 1. Create `.rela<section>`s.
        // 2. Compute section indexes.
        // 2. Compute section offset.
        // 3. Track section headers string table offset values.
        // 4. Compute `.shstrtab` size.
        for each_node_m(symbols_table->section_first, section)
        {
                section->index      = section->previous ? section->previous->index + 1 : 0;
                section->elf.offset = section->previous
                        ? section->previous->elf.type == ELF_Section_Header_Type__No_Data
                                ? section->previous->elf.offset
                                : section->previous->elf.offset + section->previous->elf.size
                        : sizeof(ELF64_Header);

                if (section->fixups.unresolved > 0)
                {
                        // Create relocation section.
                        Arena_Temporary scratch = Arena_Temporary__begin(arena);
                        String8 name = String8__format(scratch.arena, ".rela%.*s", String8__varg(*section->symbol->name));
                        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);
                        Arena_Temporary__end(scratch);

                        symbol->flags |= Symbol_Flags__Skip;
                        Symbols_Table__create_section(symbols_table, symbol);
                        DLL_insert_m(symbols_table->section_first, symbols_table->section_last, section, symbol->section);
                        symbol->section->elf.type       = ELF_Section_Header_Type__Relocations_Addends;
                        symbol->section->elf.flags      = ELF_Section_Header_Flags__INFO_LINK;
                        symbol->section->elf.size       = sizeof(ELF64_Relocation_Addend) * section->fixups.unresolved;
                        symbol->section->elf.entry_size = sizeof(ELF64_Relocation_Addend);
                        // Fill info: https://gabi.xinuos.com/v42/elf/03-sheader.html#the-sh-link-and-sh-info-fields
                        symbol->section->elf.info       = section->index;
                }

                section->elf.string_table_offset = symbol_shstrtab->section->elf.size;
                U32 c_string_size = section->symbol->name->count + 1;
                symbol_shstrtab->section->elf.size += c_string_size;
        }
        symbol_symtab->section->elf.link = symbol_strtab->section->index;

        // 1. Compute string table size.
        // 2. Count total symbols.
        // 3. Promote undefined symbols to globals.
        U32 symbols_to_keep        = 0;
        U32 symbols_local_to_keep  = 0;
        U32 symbols_global_to_keep = 0;
        for each_node_m(symbols_table->first, symbol)
        {
                B32 keep = Symbol_Ref__keep(symbol);
                if (keep)
                {
                        B32 section_is = symbol->type == STT_SECTION;
                        if (!section_is)
                        {
                                U32 c_string_size = symbol->name->count + 1;
                                symbol_strtab->section->elf.size += c_string_size;
                        }

                        symbols_to_keep += 1;

                        B32 ensure_global = symbol->section == &Section__undefined && symbol != &Symbol_Ref__undefined;
                        if (ensure_global)
                        {
                                symbol->binding = ELF_Symbol_Binding__Global;
                        }

                        if (symbol->binding == ELF_Symbol_Binding__Local)
                        {
                                symbol->index = symbols_local_to_keep;
                                symbols_local_to_keep += 1;
                        }
                        else
                        {
                                symbol->index = symbols_global_to_keep;
                                symbols_global_to_keep += 1;
                        }
                }
                else
                {
                        symbols_table->count = symbols_table->count == 0 ? 0 : symbols_table->count - 1;
                }
        }
        symbol_symtab->section->elf.size = symbols_to_keep * sizeof(ELF64_Symbol);
        symbol_symtab->section->elf.info = symbols_local_to_keep;

        // Adjust `.strtab` and `.shstrtab` offsets now that we now the last sizes.
        symbol_strtab->section->elf.offset += symbol_symtab->section->elf.size;
        symbol_shstrtab->section->elf.offset = symbol_strtab->section->elf.offset + symbol_strtab->section->elf.size;

        // -------------------
        // Final sizes
        // ------------------

        // Add symbols table and section header table
        U64 sections_header_table_size = sizeof(ELF64_Section_Header) * symbols_table->sections_count;
        U64 object_file_size = symbol_shstrtab->section->elf.offset + symbol_shstrtab->section->elf.size + sections_header_table_size;

        U64 section_header_table_file_offset = symbol_shstrtab->section->elf.offset + symbol_shstrtab->section->elf.size;
        ELF64_Header elf_header =
        {
                .identifier                        = {0},
                .object_file_type                  = ELF_Type__Relocatable,
                .architecture                      = ELF_Machine__RISCV,
                .object_file_version               = ELF_ID_Version__Current,
                .entry_point_virtual_address       = 0,
                .program_header_table_file_offset  = 0,
                .section_header_table_file_offset  = section_header_table_file_offset,
                .processor_flags                   = options->elf_header_flags,
                .header_size                       = sizeof(ELF64_Header),
                .program_header_table_entry_size   = 0,
                .program_header_table_entry_count  = 0,
                .section_header_table_entry_size   = sizeof(ELF64_Section_Header),
                .section_header_table_entry_count  = symbols_table->sections_count,
                .section_header_string_table_index = symbols_table->section_last->index
        };
        ELF_identifier_fill(elf_header.identifier);

        // --------------------------------------------------------------------
        // Write to file
        // --------------------------------------------------------------------

        // Cursors

        U8 *file_out = mmap_file_output(file_descriptor_out, object_file_size);
        String8 file_out_cursor                    = String8__new(file_out, object_file_size);
        String8 section_header_table_cursor        = String8__new(file_out + section_header_table_file_offset,     sections_header_table_size);
        String8 section_header_string_table_cursor = String8__new(file_out + symbol_shstrtab->section->elf.offset, symbol_shstrtab->section->elf.size);
        String8 string_table_cursor                = String8__new(file_out + symbol_strtab->section->elf.offset,   symbol_strtab->section->elf.size);
        String8 symbols_table_cursor               = String8__new(file_out + symbol_symtab->section->elf.offset,   symbol_symtab->section->elf.size);


        String8__serial_write_m(&file_out_cursor, &elf_header);

        // Write all sections along with their headers, and their relocations. Write the section header string table.
        for each_node_m(symbols_table->section_first, section)
        {
                B32 data_has = section->elf.type != ELF_Section_Header_Type__No_Data;
                String8 section_cursor = String8__new(file_out + section->elf.offset, section->elf.size);

                if (data_has)
                {
                        for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
                        {
                                // Write fragment data.
                                assert_always_m(fragment->relax_state == Relax_State__Fill);
                                String8__serial_write(&section_cursor, fragment->data, fragment->data_size);

                                // Ensure that variable data, if present (e.g. jump instructions), are written at least once.
                                Expression *expression = fragment->relax_info.fill_expression;
                                U32 repeat_count = expression ? expression->integer_value : 1;
                                U32 index = 0;
                                for (;;)
                                {
                                        if (index >= repeat_count)
                                        {
                                                break;
                                        }
                                        String8__serial_write(&section_cursor, fragment->data_variable, fragment->data_variable_size);
                                        index += 1;
                                }
                        }
                }

                Section *previous = section->previous;
                B32 relocation_is = previous && previous->fixups.unresolved > 0;
                if (relocation_is)
                {
                        // Fill link: https://gabi.xinuos.com/v42/elf/03-sheader.html#the-sh-link-and-sh-info-fields
                        section->elf.link = symbol_symtab->section->index;
                        for each_node_m(previous->fixups.first, fixup)
                        {
                                if (!(fixup->flags & Fixup_Flags__Done))
                                {
                                        U32 index_offset = 0;
                                        Symbol_Ref *fixup_symbol = fixup->expression && fixup->expression->symbol ? fixup->expression->symbol : &Symbol_Ref__undefined;
                                        if (fixup_symbol->binding != ELF_Symbol_Binding__Local)
                                        {
                                                index_offset = symbols_local_to_keep;
                                        }
                                        U32 symbol_index = fixup_symbol->index + index_offset;
                                        S64 addend       = fixup->expression ? fixup->expression->integer_value : 0;
                                        ELF64_Relocation_Addend relocation =
                                        {
                                                .offset = fixup->fragment->object_file_offset + fixup->offset,
                                                .info = ELF64_Relocation_info_m(symbol_index, fixup->relocation_type),
                                                .addend = addend,
                                        };

                                        String8__serial_write_m(&section_cursor, &relocation);
                                }
                        }
                }

                U32 c_string_size = section->symbol->name->count + 1;
                String8__serial_write(&section_header_string_table_cursor, section->symbol->name->data, c_string_size);

                // Write the section header
                String8__serial_write_m(&section_header_table_cursor, &section->elf);
        }

        // After all sections data we have the symbols table and the strings table.
        U32 string_table_offset_tracker = 0;
        for each_node_m(symbols_table->first, symbol)
        {
                // TODO(high): there is probably some mismanagement of the dot symbol.
                B32 keep = Symbol_Ref__keep(symbol);
                B32 local = symbol->binding == ELF_Symbol_Binding__Local;
                if (keep && local)
                {
                        B32 section_is = symbol->type == STT_SECTION;
                        ELF64_Symbol elf_symbol =
                        {
                                .string_table_offset = section_is ? 0 : string_table_offset_tracker,
                                .type_and_binding    = ELF_Symbol_info_m(symbol->binding, symbol->type),
                                .visibility          = symbol->visibility,
                                .section_index       = symbol->section->index,
                                .value               = symbol->value,
                                .size                = symbol->size_expression ? symbol->size_expression->integer_value : 0
                        };
                        String8__serial_write_m(&symbols_table_cursor, &elf_symbol);

                        if (!section_is)
                        {
                                U32 c_string_size = symbol->name->count + 1;
                                String8__serial_write(&string_table_cursor, symbol->name->data, c_string_size);
                                string_table_offset_tracker += c_string_size;
                        }
                }
        }
        for each_node_m(symbols_table->first, symbol)
        {
                // TODO(high): there is probably some mismanagement of the dot symbol.
                B32 keep = Symbol_Ref__keep(symbol);
                B32 non_local = symbol->binding != ELF_Symbol_Binding__Local;
                if (keep && non_local)
                {
                        B32 section_is = symbol->type == STT_SECTION;
                        ELF64_Symbol elf_symbol =
                        {
                                .string_table_offset = section_is ? 0 : string_table_offset_tracker,
                                .type_and_binding    = ELF_Symbol_info_m(symbol->binding, symbol->type),
                                .visibility          = symbol->visibility,
                                .section_index       = symbol->section->index,
                                .value               = symbol->value,
                                .size                = symbol->size_expression ? symbol->size_expression->integer_value : 0
                        };
                        String8__serial_write_m(&symbols_table_cursor, &elf_symbol);

                        if (!section_is)
                        {
                                U32 c_string_size = symbol->name->count + 1;
                                String8__serial_write(&string_table_cursor, symbol->name->data, c_string_size);
                                string_table_offset_tracker += c_string_size;
                        }
                }
        }

        munmap(file_out, object_file_size);
        return object_file_size;
}
