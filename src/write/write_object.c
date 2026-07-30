#include "core/core_fixup.h"
#include "core/core_section.h"
#include "core/core_symbol.h"
#include "write_section.h"
#include "write_object.h"

#include <unistd.h>

internal U64
write_object_file
(
        Arena           *arena,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        S32              file_descriptor_out
)
{
        Symbol_Ref *symbol_riscv_attributes = Symbols_Table__create_section_riscv_attributes(symbols_table);
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
                // bug this is skipped, no dll
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
        printf("relaxation completed in %u passes\n", relaxation_passes);

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
                Section__resolve_fixups(section, symbols_table->arena, diagnostics);
        }

        // Add the mandatory ending sections last: `.symtab`, `.strtab`, `.shstrtab`.
        Symbol_Ref *symbol_symtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".symtab"));
        Symbols_Table__create_section(symbols_table, symbol_symtab);
        symbol_symtab->section->elf.type = ELF_Section_Header_Type__Symbols_Table;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_symtab->section);

        Symbol_Ref *symbol_strtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".strtab"));
        Symbols_Table__create_section(symbols_table, symbol_strtab);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_strtab->section);
        symbol_strtab->section->elf.type = ELF_Section_Header_Type__Strings_Table;
        // Must contain at least the initial null byte.
        symbol_strtab->section->elf.size = 1;

        Symbol_Ref *symbol_shstrtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".shstrtab"));
        Symbols_Table__create_section(symbols_table, symbol_shstrtab);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_shstrtab->section);
        symbol_shstrtab->section->elf.type = ELF_Section_Header_Type__Strings_Table;
        // Must contain at least the initial null byte.
        symbol_shstrtab->section->elf.size = 1;

        // Count the null-section as well.
        U64 object_file_size = sizeof(ELF64_Header) + sizeof(ELF64_Section_Header);

        // TODO: I'm probably missing writting null section and symbol.

        // 1. Create `.rela<section>`s.
        // 2. Compute section indexes.
        // 3. Track section headers string table offset values.
        // 4. Compute `.shstrtab` size.
        // 5. Increase the total `object_file_size`.
        for each_node_m(symbols_table->section_first, section)
        {
                if (section->fixups.unresolved > 0)
                {
                        Arena_Temporary scratch = Arena_Temporary__begin(arena);
                        String8 name = String8__format(scratch.arena, ".rela%.*s", String8__varg(*section->symbol->name));
                        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);
                        Arena_Temporary__end(scratch);

                        Symbols_Table__create_section(symbols_table, symbol);
                        DLL_insert_m(symbols_table->section_first, symbols_table->section_last, section, symbol->section);
                        symbol->section->elf.type = ELF_Section_Header_Type__Relocations;
                        symbol->section->elf.size = sizeof(ELF64_Relocation_Addend) * section->fixups.unresolved;
                }

                section->index                   = section->previous ? section->index + 1 : 1;
                section->elf.string_table_offset = symbol_shstrtab->section->elf.size;

                U32 c_string_size = section->symbol->name->count + 1;
                symbol_shstrtab->section->elf.size += c_string_size;
                object_file_size += section->elf.size;
        }

        // Count the undefined section as well.
        U32 section_header_table_size = (1 + symbols_table->sections_count) * sizeof(ELF64_Section_Header);

        // 1. Track symbols string table offsets.
        // 2. Compute string table size.
        // 3. Count total symbols.
        U32 symbols_to_keep = 0;
        for each_node_m(symbols_table->first, symbol)
        {
                B32 skip =   symbol->flags & Symbol_Flags__Removed
                        ||   symbol->flags & Symbol_Flags__Redefined
                        || !(symbol->flags & Symbol_Flags__Used);

                if (!skip)
                {
                        symbol->string_table_offset = symbol_strtab->section->elf.size;
                        U32 c_string_size = symbol->name->count + 1;
                        symbol_strtab->section->elf.size += c_string_size;
                        symbols_to_keep += 1;
                        symbol->index = symbols_to_keep;
                }
        }

        // Final size is here.
        U32 symbols_table_size = symbols_to_keep * sizeof(ELF64_Symbol);
        object_file_size += symbols_table_size + symbol_strtab->section->elf.size;
        U32 section_header_table_file_offset = object_file_size - (sizeof(ELF64_Header) * symbols_table->sections_count);

        ELF64_Header elf_header =
        {
                .identifier                        = {0},
                .object_file_type                  = ELF_Type__Relocatable,
                .architecture                      = ELF_Machine__RISCV,
                .object_file_version               = ELF_ID_Version__Current,
                .entry_point_virtual_address       = 0,
                .program_header_table_file_offset  = 0,
                .section_header_table_file_offset  = section_header_table_file_offset,
                .processor_flags                   = 0,
                .header_size                       = sizeof(ELF64_Header),
                .program_header_table_entry_size   = 0,
                .program_header_table_entry_count  = 0,
                .section_header_table_entry_size   = section_header_table_size,
                .section_header_table_entry_count  = symbols_table->sections_count,
                .section_header_string_table_index = symbols_table->section_last->index
        };
        ELF_identifier_fill(elf_header.identifier);

        U8 *file_out = mmap_file_output(file_descriptor_out, object_file_size);
        U8 *file_out_cursor = file_out;

        cursor_write_struct_m(&file_out_cursor, &elf_header);

        // Write all sections that have fragments, and their relocations. Write the section header string table.
        U32 section_header_string_table_offset = section_header_table_file_offset - symbol_shstrtab->section->elf.size;
        U8 *section_header_string_table_cursor = file_out + section_header_string_table_offset;
        section_header_string_table_cursor[0] = 0; section_header_string_table_cursor += 1;
        for each_node_m(symbols_table->section_first, section)
        {
                for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
                {
                        cursor_write(&file_out_cursor, fragment->data,          fragment->data_size);
                        cursor_write(&file_out_cursor, fragment->data_variable, fragment->data_variable_size);
                }

                B32 relocation_is = section->previous && section->previous->fixups.unresolved > 0;
                if (relocation_is)
                {
                        assert_always_m(section->fragments.first == &Fragment__nil);
                        for each_node_m(section->previous->fixups.first, fixup)
                        {
                                if (!(fixup->flags & Fixup_Flags__Done))
                                {
                                        U32 symbol_index = fixup->expression->symbol->index;
                                        ELF64_Relocation_Addend relocation =
                                        {
                                                .offset = fixup->fragment->object_file_offset + fixup->offset,
                                                .info = ELF64_Relocation_info_m(symbol_index, fixup->relocation_type),
                                                .addend = fixup->expression->integer_value,
                                        };

                                        cursor_write_struct_m(&file_out_cursor, &relocation);
                                }
                        }
                }

                U32 c_string_size = section->symbol->name->count + 1;
                cursor_write(&section_header_string_table_cursor, section->symbol->name->data, c_string_size);
        }

        // Write symbols table and string table. First byte must be null.
        U8 *string_table_cursor = file_out_cursor + symbols_table_size;
        string_table_cursor[0] = 0; string_table_cursor += 1;
        for each_node_m(symbols_table->first, symbol)
        {
                B32 skip =   symbol->flags & Symbol_Flags__Removed
                        ||   symbol->flags & Symbol_Flags__Redefined
                        || !(symbol->flags & Symbol_Flags__Used);

                if (!skip)
                {
                        ELF64_Symbol elf_symbol =
                        {
                                .string_table_offset = symbol->string_table_offset,
                                .type_and_binding    = ELF_Symbol_info_m(symbol->binding, symbol->type),
                                .visibility          = symbol->visibility,
                                .section_index       = symbol->section->index,
                                .value               = symbol->value,
                                .size                = symbol->size
                        };
                        cursor_write_struct_m(&file_out_cursor, &elf_symbol);
                        U32 c_string_size = symbol->name->count + 1;
                        cursor_write(&string_table_cursor, symbol->name->data, c_string_size);
                }
                assert_always_m(file_out_cursor <= string_table_cursor);
        }

        munmap(file_out, object_file_size);
        return object_file_size;
}
