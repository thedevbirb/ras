#include "core/core_fixup.h"
#include "core/core_section.h"
#include "core/core_symbol.h"
#include "write_elf.h"
#include "write_section.h"
#include "write_object.h"

#include <unistd.h>

internal void
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

        for each_node_m(symbols_table->section_first, section)
        {
                for each_node_z_m(section->fragments.first, fragment, &Fragment__nil)
                {
                        Fragment__convert_to_fill(fragment, section, expressions, symbols_table->arena);
                }
        }

        for each_node_m(symbols_table->section_first, section)
        {
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


        // We have to create more sections now:
        //
        // 1. relocation sections, e.g. .rela.text after corresponding sections.
        // 2. symtab
        // 3. strtab
        // 4. shstrtab

        // Create the `.rela<section>` sections, and place them after the `<section>`.
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
                }
        }

        Symbol_Ref *symbol_symtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".symtab"));
        Symbols_Table__create_section(symbols_table, symbol_symtab);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_symtab->section);
        symbol_symtab->section->elf.type = ELF_Section_Header_Type__Symbols_Table;

        Symbol_Ref *symbol_strtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".strtab"));
        Symbols_Table__create_section(symbols_table, symbol_strtab);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_strtab->section);
        symbol_strtab->section->elf.type = ELF_Section_Header_Type__Strings_Table;

        Symbol_Ref *symbol_shstrtab = Symbols_Table__get_or_default(symbols_table, String8__literal(".shstrtab"));
        Symbols_Table__create_section(symbols_table, symbol_shstrtab);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_shstrtab->section);
        symbol_shstrtab->section->elf.type = ELF_Section_Header_Type__Strings_Table;
}
