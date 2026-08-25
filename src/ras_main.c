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
#include "riscv/riscv_include.h"

#include "core/core_include.h"
#include "lexer.h"
#include "parser/parser_include.h"
#include "write/write_include.h"

// .c files

#include "base/base_include.c"

#include "object/object_include.c"
#include "riscv/riscv_include.c"

#include "core/core_include.c"
#include "lexer.c"
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
        fprintf(stderr, "usage: ras <filepath_in> -o <filepath_out>\n");
}

global const String8 march_option_prefix = String8__literal("-march=");
global const String8 mabi_option_prefix  = String8__literal("-mabi=");

// TODO(low): probably not good way to diagnostics, also not good to exit immediately each time.
internal Options
Options__parse(S32 *argument_count, char **argument_vector, Arena *arena)
{

        arguments_shift(argument_count, &argument_vector);
        if (*argument_count < 2)
        {
                usage_print();
                exit(1);
        }

        Arena_Temporary scratch = Arena__scratch_begin_m(&arena, 1);
        Options options = Options__default(arena);

        String8 architecture = {0};
        String8 abi          = {0};

        for (;;)
        {
                if (*argument_count == 0)
                {
                        break;
                }

                String8 argument = String8__from_cstring(*argument_vector);
                if (String8__match_prefix(argument, march_option_prefix))
                {
                        architecture = String8__skip(argument, march_option_prefix.count);

                }
                else if (String8__match_prefix(argument, mabi_option_prefix))
                {
                        abi = String8__skip(argument, mabi_option_prefix.count);
                }
                else if (String8__match_exact(argument, String8__literal("-o")))
                {
                        if (options.output_file.count != 0)
                        {
                                fprintf(stderr, "output file already provided: %*s\n", String8__varg(options.output_file));
                                exit(1);
                        }

                        arguments_shift(argument_count, &argument_vector);
                        if (argument_count == 0)
                        {
                                fprintf(stderr, "expected outfile file after -o flag\n");
                                exit(1);
                        }

                        String8 output_file = String8__from_cstring(*argument_vector);
                        options.output_file = output_file;
                }
                else if (String8__match_exact(argument, String8__literal("-fpic"))
                         || String8__match_exact(argument, String8__literal("-fPIC")))
                {
                        options.position_indipendent_code = 1;
                }
                else if (String8__match_exact(argument, String8__literal("-fno-pic")))
                {
                        options.position_indipendent_code = 0;
                }
                else if (String8__match_exact(argument, String8__literal("--help")))
                {
                        usage_print();
                        exit(0);
                }
                else
                {
                        // TODO(low): actually support more than one file
                        if (options.input_file.count != 0)
                        {
                                fprintf(stderr, "input file already provided: %*s\n", String8__varg(options.input_file));
                                exit(1);
                        }

                        options.input_file = argument;
                }

                arguments_shift(argument_count, &argument_vector);
        }

        if (abi.count)
        {
                B32 match = String8__match_exact(abi, String8__literal("ilp32"))
                        || String8__match_exact(abi, String8__literal("ilp32f"))
                        || String8__match_exact(abi, String8__literal("ilp32d"))
                        || String8__match_exact(abi, String8__literal("lp64"))
                        || String8__match_exact(abi, String8__literal("lp64f"))
                        || String8__match_exact(abi, String8__literal("lp64d"));
                // TODO(low): not very clean
                if (!match)
                {
                        fprintf(stderr, "invalid abi, expected 'ilp32', 'ilp32f', 'ilp32d', 'lp64', 'lp64f' or 'lp64d', found: %*s\n", String8__varg(abi));
                        exit(1);
                }

                options.machine_abi = abi;
                options.abi_xlen = abi.data[3] == '3' ? XLEN_32 : XLEN_64;
        }

        if (architecture.count)
        {
                String8 error = Options__architecture_parse(&options, architecture, scratch.arena);
                if (error.count)
                {
                        fprintf(stderr, "%*s\n", String8__varg(error));
                        exit(1);
                }

                if (!abi.count)
                {
                        // Default ABI follows the ISA width: ilp32 for rv32, lp64 for rv64.
                        options.machine_abi = options.xlen == XLEN_32 ? String8__literal("ilp32") : String8__literal("lp64");
                        options.abi_xlen = options.xlen;
                }
        }

        if (options.abi_xlen != options.xlen)
        {
                fprintf(stderr, "can't have %d-bit ABI on %d-bit ISA\n", options.abi_xlen, options.xlen);
                exit(1);
        }

        options.elf_header_flags = ELF_Header_Flags__from_Options(&options);

        Arena__scratch_end_m(scratch);
        return options;
}

S32
main(S32 argument_count, char **argument_vector)
{
        Thread_Context *thread_context = Thread_Context_alloc();
        Thread_Context_select(thread_context);

        Arena *arena = Arena__allocate_m();
        Options options = Options__parse(&argument_count, argument_vector, arena);

        S32 file_descriptor_in = open((char *)options.input_file.data, O_RDONLY);
        if (file_descriptor_in <= 0)
        {
                fprintf(stderr, "failed to find input file");
                exit(1);
        }

        S32 file_descriptor_out = open((char *)options.output_file.data, O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (file_descriptor_out <= 0)
        {
                fprintf(stderr, "failed to open output file");
                exit(1);
        }

        struct stat file_in_statistics;
        assert_always_m(fstat(file_descriptor_in, &file_in_statistics) == 0 && "failed to call fstat on input file");
        assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
        U64 file_in_size = (U64)file_in_statistics.st_size;

        // TODO(medium): non-trivial lifetime relationship between the source and diagnostics.
        U8 *input_data_mapped = mmap_file(file_descriptor_in, file_in_size + 4);
        String8 input = { .data = input_data_mapped, .count = file_in_size };

        Source source =
        {
                .data = input.data,
                .name = options.input_file.data,

                .count = input.count,
                .name_count = options.input_file.count,
        };

        Arena         *arena_symbols_table  = Arena__allocate_m();
        Symbols_Table *symbols_table        = Arena__push_struct_m(arena_symbols_table, Symbols_Table);
                       arena = arena_symbols_table;

        // Create the dot symbol first for faster lookup in the trie
        Symbol_Ref *symbol_dot = Symbols_Table__get_or_default(symbols_table, dot_symbol_string, arena);

        Symbol_Ref *symbol_text = Symbols_Table__get_or_default(symbols_table, section_name_text, arena);
        Symbols_Table__create_section(symbols_table, symbol_text, arena, Arena_Parameters__default);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_text->section);
        symbol_text->section->elf.alignment = options.compressed ? 2 : 4;
        symbols_table->section_current = symbol_text->section;

        Symbol_Ref *symbol_data = Symbols_Table__get_or_default(symbols_table, section_name_data, arena);
        Symbols_Table__create_section(symbols_table, symbol_data, arena, Arena_Parameters__default);
        symbol_data->section->elf.alignment = 8;
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_data->section);

        Symbol_Ref *symbol_bss = Symbols_Table__get_or_default(symbols_table, section_name_bss, arena);
        Symbols_Table__create_section(symbols_table, symbol_bss, arena, Arena_Parameters__default);
        DLL_push_back_m(symbols_table->section_first, symbols_table->section_last, symbol_bss->section);
        symbol_bss->section->elf.alignment = 8;

        Symbol_Ref__update_section(symbol_dot, symbol_text->section);

        Expressions expressions = {0};

        Diagnostics *diagnostics = Diagnostics__new(Arena__allocate_m());

        Token_Cursor cursor = { .source = &source, .source_index = 0 };

        statements_read(arena, &cursor, diagnostics, &expressions, symbols_table, &options);

        B32 exit_status = 0;

        for each_node_m(diagnostics->first, diagnostic)
        {
                exit_status |= diagnostic->kind == Diagnostic_Kind__Error;
                Diagnostic__print(diagnostic, &source, arena);
                DLL_remove_m(diagnostics->first, diagnostics->last, diagnostic);
        }
        if (exit_status)
        {
                exit(1);
        }

        U64 size = write_object_file(arena, diagnostics, &expressions, symbols_table, &options, file_descriptor_out);
        unused_m(size);

        // fprintf(stderr, "written %llu bytes of object file\n", size);

        for each_node_m(diagnostics->first, diagnostic)
        {
                exit_status |= diagnostic->kind == Diagnostic_Kind__Error;
                Diagnostic__print(diagnostic, &source, arena);
                DLL_remove_m(diagnostics->first, diagnostics->last, diagnostic);
        }
        if (exit_status)
        {
                exit(1);
        }

        close(file_descriptor_in);
        close(file_descriptor_out);

        return exit_status;
}
