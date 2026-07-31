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

        for each_symbol_m(symbols_table, symbol)
        {
                Symbol_Ref__resolve(symbol, diagnostics, Resolve_Level__Finalize);
        }
        Expressions__finalize(expressions, diagnostics);
        for each_node_m(symbols_table->section_first, section)
        {
                Section__resolve_fixups(section, symbols_table->arena, diagnostics);
        }

        // Let's avoid off-by-one errors.
        Symbols_Table__ensure_undefined_present(symbols_table);

        // Add the mandatory ending sections last: `.symtab`, `.strtab`, `.shstrtab`.
        Symbol_Ref *symbol_symtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".symtab"));
        Symbols_Table__create_section(symbols_table, symbol_symtab);
        symbol_symtab->section->elf.entry_size = sizeof(ELF64_Symbol);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_symtab->section);

        Symbol_Ref *symbol_strtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".strtab"));
        Symbols_Table__create_section(symbols_table, symbol_strtab);
        symbol_strtab->section->elf.entry_size       = 1;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_strtab->section);

        Symbol_Ref *symbol_shstrtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".shstrtab"));
        Symbols_Table__create_section(symbols_table, symbol_shstrtab);
        symbol_strtab->section->elf.entry_size       = 1;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_shstrtab->section);

        U64 object_file_size = sizeof(ELF64_Header);

        // 1. Create `.rela<section>`s.
        // 2. Compute section indexes.
        // 3. Track section headers string table offset values.
        // 4. Compute `.shstrtab` size.
        // 5. Increase the total `object_file_size`.
        for each_node_m(symbols_table->section_first, section)
        {
                section->index = section->previous ? section->previous->index + 1 : 0;

                if (section->fixups.unresolved > 0)
                {
                        Arena_Temporary scratch = Arena_Temporary__begin(arena);
                        String8 name = String8__format(scratch.arena, ".rela%.*s", String8__varg(*section->symbol->name));
                        Symbol_Ref *symbol = Symbols_Table__get_or_default(symbols_table, name);
                        Arena_Temporary__end(scratch);

                        Symbols_Table__create_section(symbols_table, symbol);
                        DLL_insert_m(symbols_table->section_first, symbols_table->section_last, section, symbol->section);
                        symbol->section->elf.type       = ELF_Section_Header_Type__Relocations;
                        symbol->section->elf.size       = sizeof(ELF64_Relocation_Addend) * section->fixups.unresolved;
                        symbol->section->elf.entry_size = sizeof(ELF64_Relocation_Addend);
                        // Fill info: https://gabi.xinuos.com/v42/elf/03-sheader.html#the-sh-link-and-sh-info-fields
                        symbol->section->elf.info       = section->index;
                }

                section->elf.string_table_offset = symbol_shstrtab->section->elf.size;
                U32 c_string_size = section->symbol->name->count + 1;
                symbol_shstrtab->section->elf.size += c_string_size;
                object_file_size += section->elf.size;
        }
        symbol_symtab->section->elf.link = symbol_strtab->section->index;

        // 1. Track symbols string table offsets.
        // 2. Compute string table size.
        // 3. Count total symbols.
        U32 symbols_to_keep = 0;
        for each_symbol_m(symbols_table, symbol)
        {
                B32 skip =   symbol->flags & Symbol_Flags__Removed
                        ||   symbol->flags & Symbol_Flags__Redefined
                        || !(symbol->flags & Symbol_Flags__Used);

                if (!skip)
                {
                        symbols_to_keep += 1;
                        symbol->index = symbols_to_keep;

                        if (symbol->type != STT_SECTION)
                        {
                                symbol->string_table_offset = symbol_strtab->section->elf.size;
                                U32 c_string_size = symbol->name->count + 1;
                                symbol_strtab->section->elf.size += c_string_size;
                        }
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
                .section_header_table_entry_size   = sizeof(ELF64_Section_Header),
                .section_header_table_entry_count  = symbols_table->sections_count,
                .section_header_string_table_index = symbols_table->section_last->index
        };
        ELF_identifier_fill(elf_header.identifier);

        U8 *file_out = mmap_file_output(file_descriptor_out, object_file_size);
        U8 *file_out_cursor = file_out;
        cursor_write_struct_m(&file_out_cursor, &elf_header);

        // Write all sections along with their headers, and their relocations. Write the section header string table.
        U8 *section_header_table_cursor = file_out + section_header_table_file_offset;
        U32 section_header_string_table_offset = section_header_table_file_offset - symbol_shstrtab->section->elf.size;
        U8 *section_header_string_table_cursor = file_out + section_header_string_table_offset;
        for each_node_m(symbols_table->section_first, section)
        {
                for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
                {
                        cursor_write(&file_out_cursor, fragment->data,          fragment->data_size);
                        cursor_write(&file_out_cursor, fragment->data_variable, fragment->data_variable_size);
                }

                Section *previous = section->previous;
                B32 relocation_is = previous && previous->fixups.unresolved > 0;
                if (relocation_is)
                {
                        // Fill link: https://gabi.xinuos.com/v42/elf/03-sheader.html#the-sh-link-and-sh-info-fields
                        section->elf.link = symbol_symtab->section->elf.link;
                        assert_always_m(section->fragments.first == &Fragment__nil);
                        for each_node_m(previous->fixups.first, fixup)
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

                // Offset is from the start of the file, so we must include the ELF header.
                section->elf.offset = previous ? previous->elf.offset + previous->elf.size : sizeof(ELF64_Header);

                U32 c_string_size = section->symbol->name->count + 1;
                cursor_write(&section_header_string_table_cursor, section->symbol->name->data, c_string_size);

                // Write the section header
                cursor_write_struct_m(&section_header_table_cursor, &section->elf);
        }

        // After all sections data we have the symbols table and the strings table.
        U8 *string_table_cursor = file_out_cursor + symbols_table_size;
        for each_symbol_m(symbols_table, symbol)
        {
                // TODO(high): there is probably some mismanagement of the dot symbol.
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

                        if (symbol->type != STT_SECTION)
                        {
                                U32 c_string_size = symbol->name->count + 1;
                                cursor_write(&string_table_cursor, symbol->name->data, c_string_size);
                        }
                }
                assert_always_m(file_out_cursor <= string_table_cursor);
        }

        assert_always_m(section_header_string_table_offset == symbol_shstrtab->section->elf.offset && ".shstrab mismatch");

        munmap(file_out, object_file_size);
        return object_file_size;
}
