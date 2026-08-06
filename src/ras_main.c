#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include "base/base_include.h"

#include <generated/instruction_hashes.h>

#include "object/object_include.h"

#include "core/core_include.h"
#include "lexer.h"
#include "riscv/riscv_include.h"
#include "parser/parser_include.h"
#include "write/write_include.h"

#include "base/base_include.c"

#include "object/object_include.c"

#include "core/core_include.c"
#include "lexer.c"
#include "riscv/riscv_include.c"
#include "parser/parser_include.c"
#include "write/write_include.c"

// Two's complement.
assert_static_m(-1 == ~0, two_complement);

#ifndef ARCH_LITTLE_ENDIAN
#error "little endian architecture expected"
#endif

void
arguments_shift(int *argument_count, char ***argument_vector)
{
        *argument_count  -= 1;
        *argument_vector += 1;

        return;
}

void
usage_print(void)
{
                fprintf(stderr, "usage: ras <filepath_in> <filepath_out>\n");
}

int
main(int argument_count, char **argument_vector)
{
        Initialize();

        Thread_Context *thread_context = Thread_Context_alloc();
        Thread_Context_select(thread_context);

        arguments_shift(&argument_count, &argument_vector);
        if (argument_count < 2)
        {
                usage_print();
                exit(1);
        }

        String8 filename_in  = String8__from_cstring(argument_vector[0]);
        String8 filename_out = String8__from_cstring(argument_vector[1]);
        // printf("input: %s\n", filename_in.data);
        // printf("output: %s\n", filename_out.data);

        int file_descriptor_in = open((char *)filename_in.data, O_RDONLY);
        assert_always_m(file_descriptor_in > 0 && "failed to find input file");

        int file_descriptor_out = open((char *)filename_out.data, O_RDWR | O_CREAT | O_TRUNC, 0644);
        assert_always_m(file_descriptor_out >= 0 && "failed to open output file");

        struct stat file_in_statistics;
        assert_always_m(fstat(file_descriptor_in, &file_in_statistics) == 0 && "failed to call fstat on input file");
        assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
        U64 file_in_size = (U64)file_in_statistics.st_size;

        // TODO(medium): non-trivial lifetime relationship between the source and diagnostics.
        Arena *arena = Arena__allocate_m();
        U8 *input_data_mapped = mmap_file(file_descriptor_in, file_in_size + 4);
        String8 input = { .data = input_data_mapped, .count = file_in_size };

        Source source =
        {
                .data = input.data,
                .name = filename_in.data,

                .count = input.count,
                .name_count = filename_in.count,
        };

        Arena         *arena_symbols_table  = Arena__allocate_m();
        Symbols_Table *symbols_table        = Arena__push_struct_m(arena_symbols_table, Symbols_Table);
                       symbols_table->arena = arena_symbols_table;

        Symbol_Ref *symbol_text = Symbols_Table__get_or_default(symbols_table, section_name_text);
        Symbols_Table__create_section(symbols_table, symbol_text);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_text->section);
        // Byte strings.
        symbol_text->section->elf.entry_size = 1;
        // Compressed is 2
        symbol_text->section->elf.alignment  = 4;
        symbols_table->section_current = symbol_text->section;
        Symbol_Ref *symbol_data = Symbols_Table__get_or_default(symbols_table, section_name_data);
        Symbols_Table__create_section(symbols_table, symbol_data);
        symbol_data->section->elf.alignment = 8;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_data->section);
        Symbol_Ref *symbol_bss = Symbols_Table__get_or_default(symbols_table, section_name_bss);
        Symbols_Table__create_section(symbols_table, symbol_bss);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_bss->section);
        symbol_bss->section->elf.alignment = 8;

        Expressions expressions = {0};

        Diagnostics *diagnostics = Diagnostics__new(Arena__allocate_m());

        Token_Cursor cursor = { .source = &source, .source_index = 0 };
        Options options = { .relax = 1 };

        statement_read
        (
                arena,
                &cursor,
                diagnostics,
                &expressions,
                symbols_table,
                &options
        );

        B32 exit_status = 0;

        for each_node_m(diagnostics->first, diagnostic)
        {
                exit_status |= diagnostic->kind == Diagnostic_Kind__Error;
                diagnostic_print(diagnostic, &source, arena);
                DLL_remove_m(diagnostics->first, diagnostics->last, diagnostic);

                if (exit_status)
                {
                        exit(1);
                }
        }

        // Start of an equivalent of GNU as `write_object_file`.
        U64 size = write_object_file
        (
                arena,
                diagnostics,
                &expressions,
                symbols_table,
                &options,
                file_descriptor_out
        );

        fprintf(stderr, "written %llu bytes of object file\n", size);

        for each_node_m(diagnostics->first, diagnostic)
        {
                exit_status |= diagnostic->kind == Diagnostic_Kind__Error;
                diagnostic_print(diagnostic, &source, arena);
                DLL_remove_m(diagnostics->first, diagnostics->last, diagnostic);

                if (exit_status)
                {
                        exit(1);
                }
        }

        close(file_descriptor_in);
        close(file_descriptor_out);

        return exit_status;
}
