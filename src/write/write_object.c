#include "core/core_fixup.h"
#include "core/core_section.h"
#include "core/core_symbol.h"
#include "write_elf.h"
#include "write_section.h"
#include "write_object.h"

internal void
write_object_file
(
        Arena           *arena,
        Diagnostics     *diagnostics,
        Expressions     *expressions,
        Symbols_Table   *symbols_table,
        Fixups          *fixups
)
{
        // Section__create_riscv_attributes(sections_table);
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
                for each_node_m(section->fragments.first, fragment)
                {
                        Fragment__convert_to_fill(fragment, section, expressions, arena, fixups);
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
                        diagnostic->message    = String8__format(arena, "section '%*s' size (%u bytes) is not a multiple of its entry size (%u bytes)",
                                                                 section->symbol->name->count, section->symbol->name->data, section->elf.size, section->elf.entry_size);
                        diagnostic->location   = section->symbol->location;
                }
        }

        for each_node_m(symbols_table->first, symbol)
        {
                Symbol_Ref__resolve(symbol, diagnostics, Resolve_Level__Finalize);
        }
        Expressions__finalize(expressions, diagnostics);
        Fixups__resolve(fixups, diagnostics);
}
