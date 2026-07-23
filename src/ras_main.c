#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

#include <base/base_include.h>

#include <generated/instruction_hashes.h>

#include "object/object_include.h"

#include "core/core_include.h"
#include "lexer.h"
#include "riscv/riscv_include.h"
#include "parser/parser_include.h"
#include "write/write_include.h"

#include <base/base_include.c>

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
        // if (argument_count < 2)
        // {
        //         usage_print();
        //         exit(1);
        // }

        String8 filename = String8__from_cstring(argument_vector[0]);
        printf("filename: %s\n", filename.data);
        int file_descriptor = open((char *)filename.data, O_RDONLY);
        assert_always_m(file_descriptor > 0 && "failed to find input file");

        struct stat file_in_statistics;
        assert_always_m(fstat(file_descriptor, &file_in_statistics) == 0 && "failed to call fstat on input file");
        assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
        U64 file_in_size = (U64)file_in_statistics.st_size;

        // TODO(medium): non-trivial lifetime relationship between the source and diagnostics.
        Arena *arena = Arena__allocate_m();
        U8 *input_data_mapped = mmap_file(file_descriptor, file_in_size);
        assert_always_m(input_data_mapped != MAP_FAILED && "failed to mmap file contents");

        String8 input = { .data = input_data_mapped, .count = file_in_size };

        Source source =
        {
                .data = input.data,
                .name = filename.data,

                .count = input.count,
                .name_count = filename.count,
        };

        Sections_Table *sections_table = Sections_Table__default();
        Sections_Table__add_common(sections_table);

        Symbols_Table *symbols_table = Symbols_Table__new(sections_table);

        Arena *arena_fixups = Arena__allocate_m();
        Fixups *fixups = Arena__push_struct_m(arena_fixups, Fixups);
        fixups->arena = arena_fixups;

        Expressions expressions = {0};
        Expressions__initialize(&expressions, arena, 12);

        Diagnostics *diagnostics = Diagnostics__new(Arena__allocate_m());

        Token_Cursor cursor = { .source = &source, .source_index = 0 };
        statement_read
        (
                arena,
                &cursor,
                diagnostics,
                &expressions,
                symbols_table,
                sections_table,
                fixups
        );

        B32 exit_status = 0;

        // if (diagnostics.first)
        // {
        //         Diagnostic *current = diagnostics.first;
        //         for (;;)
        //         {
        //                 exit_status |= current->kind == Diagnostic_Kind__Error;
        //                 diagnostic_print(current, &source, arena);
        //                 current = current->next;
        //
        //                 if (!current)
        //                 {
        //                         break;
        //                 }
        //         }
        //
        //         if (exit_status)
        //         {
        //                 exit(1);
        //         }
        // }

        // Start of an equivalent of GNU as `write_object_file`.
        write_object_file
        (
                arena,
                diagnostics,
                &expressions,
                symbols_table,
                sections_table,
                fixups
        );

        if (diagnostics->first)
        {
                Diagnostic *current = diagnostics->first;
                for (;;)
                {
                        exit_status |= current->kind == Diagnostic_Kind__Error;
                        diagnostic_print(current, &source, arena);
                        current = current->next;

                        if (!current)
                        {
                                break;
                        }
                }

                if (exit_status)
                {
                        exit(1);
                }
        }

        return exit_status;
}
