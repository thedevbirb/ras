#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <base/base_include.h>
#include <os/os_include.h>

#include <generated/instruction_hashes.h>

#include "initialize.h"
#include "constants.h"
#include "hashmap.h"
#include "list.h"

#include "lexer.h"
#include "section.h"
#include "expression.h"
#include "parser.h"

#include <base/base_include.c>
#include <os/os_include.c>

#include "lexer.c"
#include "parser.c"

// Two's complement.
assert_static_m(-1 == ~0, two_complement);

void
arguments_shift(int *argument_count, char ***argument_vector)
{
	*argument_count  -= 1;
	*argument_vector += 1;

	return;
}

void
usage_print()
{
		fprintf(stderr, "usage: ras <filepath_in> <filepath_out>\n");
}

int
main(int argument_count, char **argument_vector)
{
	Initialize();

	arguments_shift(&argument_count, &argument_vector);
	if (argument_count < 2)
	{
		usage_print();
		exit(1);
	}

	const char *file_in_path = argument_vector[0];
	int file_in_descriptor = open(file_in_path, O_RDONLY);
	assert_always_m(file_in_descriptor > 0 && "failed to find input file");

	struct stat file_in_statistics;
	assert_always_m(fstat(file_in_descriptor, &file_in_statistics) == 0 && "failed to call fstat on input file");
	assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
	U32 file_in_size = U32_cast_safe((U64)file_in_statistics.st_size); // 4 GiB max size

	// TODO: specify a reserve and commit size as a function of the input size.
	Arena *arena = Arena_alloc_m();

	// Growable collections with dedicated arenas that don't pay reallocation costs.

	Arena *arena_statements = Arena_alloc_m(.commit_size = file_in_size * 8);
	Statements statements;
	Statements_initialize(&statements, arena_statements);

	Arena *arena_expressions = Arena_alloc_m(.commit_size = file_in_size);
	Expressions expressions;
	Expressions_initialize(&expressions, arena_expressions);

	// Ensure same lifetime between arena and file contents.
	U8 *data_mmap = mmap(NULL, file_in_size, PROT_READ, MAP_PRIVATE, file_in_descriptor, 0);
	assert_always_m(data_mmap != MAP_FAILED && "failed to mmap file contents");
	Input input = Input_new(file_in_size, arena);
	os_memory_copy(input.data, data_mmap, file_in_size);
	munmap(data_mmap, file_in_size);

	Symbols_Table symbols_table = {0};
	Symbols_Table_initialize(&symbols_table, arena);
	Expression_Unevaluated_List expression_unevaluated_list = {0};
	Expression_Unevaluated_List_initialize(&expression_unevaluated_list, arena);

	Object_File_Section *sections             = Object_File_Section_create_all(arena, file_in_size);
	Object_File_Section *section_text         = &sections[ELF64_Section__Text];
	Object_File_Section *section_string_table = &sections[ELF64_Section__String_Table];

	arguments_shift(&argument_count, &argument_vector);

	Lexer lexer = Lexer_new(&input, arena);
	Token_Array token_array = Lexer_tokenize(&lexer);

	Lexer_Error lexer_error = token_array.error;
	if (lexer_error.kind)
	{
		U32 line   = lexer_error.row_index + 1;
		U32 column_begin = lexer_error.column_begin_index + 1;
		fprintf(stderr, "\x1B[1m%s:%d:%d:\x1B[0m \x1B[1;31merror:\x1B[0m ", file_in_path, line, column_begin);
		fprintf(stderr, "%s\n", lexer_error_kind_messages[lexer_error.kind]);
		fprintf(stderr, "%5d | ", line);
		U64 index = token_array.line_start_indexes[lexer_error.row_index];
		for (;;)
		{
			if (input.data[index] == '\n')
			{
				fputc('\n', stderr);
				break;
			}

			fputc(input.data[index], stderr);
			index += 1;
		}

		fprintf(stderr, "      | ");
		index = 0;
		for (;;)
		{
			if (index == lexer_error.column_begin_index)
			{
				fprintf(stderr, "\x1B[1;31m^");
				U32 tilde_index = 0;
				// There is already the caret, otherwise we output an extra tilde.
				U32 tilde_count = lexer_error.column_end_index - lexer_error.column_begin_index;
				for (;;)
				{
					if (tilde_index < tilde_count)
					{
						fputc('~', stderr);
						tilde_index += 1;
					}
					else
					{
						break;
					}
				}
				fprintf(stderr, "\x1B[0m\n");
				break;
			}

			fputc(' ', stderr);
			index += 1;
		}

		exit(1);
	}

	Parser parser =
	{
		.arena         = arena,
		.input         = &input,
		.tokens        = token_array.tokens,
		.statements    = &statements,
		.symbols_table = &symbols_table,
		.expressions   = &expressions,

		.expression_unevaluated_list = &expression_unevaluated_list,

		.sections             = sections,
		.section_current      = section_text,
		.section_string_table = section_string_table,

		.token_current = token_array.tokens[0],
		.token_count   = token_array.token_count,
		.token_index   = 0,
		.end_reached   = 0 >= token_array.token_count
	};

	Parser_parse(&parser);
	Parser_Error parser_error = parser.error;
	if (parser_error.kind)
	{
		U32 line   = parser_error.row_index + 1;
		U32 column_begin = parser_error.column_begin_index + 1;
		fprintf(stderr, "\x1B[1m%s:%d:%d:\x1B[0m \x1B[1;31merror:\x1B[0m ", file_in_path, line, column_begin);
		fprintf(stderr, "%s\n", Parser_Error_Kind_messages[parser_error.kind]);
		fprintf(stderr, "%5d | ", line);
		U64 index = token_array.line_start_indexes[parser_error.row_index];
		for (;;)
		{
			if (input.data[index] == '\n')
			{
				fputc('\n', stderr);
				break;
			}

			fputc(input.data[index], stderr);
			index += 1;
		}

		fprintf(stderr, "      | ");
		index = 0;
		for (;;)
		{
			if (index == parser_error.column_begin_index)
			{
				fprintf(stderr, "\x1B[1;31m^");
				U32 tilde_index = 0;
				// There is already the caret, otherwise we output an extra tilde.
				U32 tilde_count = parser_error.column_end_index - parser_error.column_begin_index;
				for (;;)
				{
					if (tilde_index < tilde_count)
					{
						fputc('~', stderr);
						tilde_index += 1;
					}
					else
					{
						break;
					}
				}
				fprintf(stderr, "\x1B[0m\n");
				break;
			}

			fputc(' ', stderr);
			index += 1;
		}
	}

	// const char *file_path_out = argument_vector[0];
	// printf("file path out: %s\n", file_path_out);
	// int file_out_descriptor = open(file_path_out, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	// assert_always_m(file_out_descriptor > 0, "failed to create file output");

	return 0;
}
