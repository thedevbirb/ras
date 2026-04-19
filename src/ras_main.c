#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

// TODO(medium): move this into appropriate place.
#define label_numeric_max 10

#include <base/base_include.h>
#include <os/os_include.h>

#include <generated/instruction_hashes.h>

#include "initialize.h"
#include "primitives.h"
#include "elf.h"

#include "language/language_include.h"
#include "symbol.h"
#include "section.h"

#include "diagnostic.h"
#include "lexer.h"
#include "expression.h"
#include "statement.h"
#include "parser/parser_include.h"
#include "resolver.h"

#include <base/base_include.c>
#include <os/os_include.c>

#include "initialize.c"
#include "utils.c"
#include "language/language_include.c"
#include "symbol.c"
#include "section.c"

#include "diagnostic.c"
#include "lexer.c"
#include "expression.c"
#include "statement.c"
#include "parser/parser_include.c"
#include "resolver.c"

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
	printf("file_in_path: %s\n", file_in_path);
	int file_in_descriptor = open(file_in_path, O_RDONLY);
	assert_always_m(file_in_descriptor > 0 && "failed to find input file");

	struct stat file_in_statistics;
	assert_always_m(fstat(file_in_descriptor, &file_in_statistics) == 0 && "failed to call fstat on input file");
	assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
	U32 file_in_size = U32_cast_safe((U64)file_in_statistics.st_size); // 4 GiB max size

	// TODO: re-think allocations. All memory must be reserved upfront, as a basic yet realistic function of input
	// size. Think like a (real) architect.

	Arena *arena = Arena_alloc_m();

	Arena *arena_statements = Arena_alloc_m(.reserve_size = file_in_size * 8, .flags = Arena_Flags__No_Chain);
	Statements statements;
	Statements_initialize(&statements, arena_statements);

	Arena *arena_expressions = Arena_alloc_m(.reserve_size = file_in_size, .flags = Arena_Flags__No_Chain);
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

	// TODO: while some of them can be created in advance, in practice arbitrary sections can be created on demand
	// using the `.section` directive, so I should change around this.
	Object_File_Section *sections = Object_File_Section_create_all(arena, file_in_size);

	Statement *statement_context_parser = Arena_push_struct_m(arena, Statement);

	arguments_shift(&argument_count, &argument_vector);

	Lexer lexer = Lexer_new(&input, arena);
	Token_Array token_array = Lexer_tokenize(&lexer);

	Lexer_Error lexer_error = token_array.error;
	if (lexer_error.kind)
	{
		Diagnostic diagnostic =
		{
			.input              = &input,
			.file_in_path       = file_in_path,
			.message_kind       = lexer_error_kind_messages[lexer_error.kind],
			.line               = lexer_error.row_index + 1,
			.column_index_begin = lexer_error.column_index_begin,
			.column_index_end   = lexer_error.column_index_end,
			.input_index_start  = token_array.line_start_indexes[lexer_error.row_index],
		};

		Diagnostic_print(&diagnostic);
		exit(1);
	}

	Parser parser =
	{
		.arena          = arena,
		.input          = &input,
		.tokens         = token_array.tokens,
		.statements     = &statements,
		.symbols_table  = &symbols_table,
		.expressions    = &expressions,

		.statement_context = statement_context_parser,

		.section_current_index = ELF_Section__Text,

		.token_current = token_array.tokens[0],
		.token_count   = token_array.token_count,
		.token_index   = 0,
		.end_reached   = 0 >= token_array.token_count
	};

	Parser_parse(&parser);
	Parser_Error parser_error = parser.error;
	if (parser_error.kind)
	{
		Diagnostic diagnostic =
		{
			.input              = &input,
			.file_in_path       = file_in_path,
			.message_kind       = Parser_Error_Kind_messages[parser_error.kind],
			.line               = parser_error.row_index + 1,
			.column_index_begin = parser_error.column_index_begin,
			.column_index_end   = parser_error.column_index_end,
			.input_index_start  = token_array.line_start_indexes[parser_error.row_index],
		};

		Diagnostic_print(&diagnostic);
		exit(1);
	}

	Vec2_U32 *labels_numeric_statement_index = Arena_push_array_m(arena, Vec2_U32, label_numeric_max);

	Resolver resolver =
	{
		.arena         = arena,
		.input         = &input,
		.tokens        = token_array.tokens,
		.statements    = &statements,
		.symbols_table = &symbols_table,
		.expressions   = &expressions,

		.labels_numeric_statement_index = labels_numeric_statement_index,

		.sections = sections,

		.statement_current      = &statements.data[0],
		.statement_index        = 0,
		.statements_end_reached = 0 >= statements.count,

		.error         = {0},
		.sections_offset = {0},
		.section_current_index = ELF_Section__Text,
	};

	Resolver_relax(&resolver);

	Resolver_Error resolver_error = resolver.error;
	if (resolver_error.kind)
	{
		Diagnostic diagnostic =
		{
			.input              = &input,
			.file_in_path       = file_in_path,
			.message_kind       = Resolver_Error_Kind_messages[resolver_error.kind],
			.line               = resolver_error.row_index + 1,
			.column_index_begin = resolver_error.column_index_begin,
			.column_index_end   = resolver_error.column_index_end,
			.input_index_start  = token_array.line_start_indexes[resolver_error.row_index],
		};

		Diagnostic_print(&diagnostic);
		exit(1);
	}

	Resolver_encode(&resolver);

	// const char *file_path_out = argument_vector[0];
	// printf("file path out: %s\n", file_path_out);
	// int file_out_descriptor = open(file_path_out, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	// assert_always_m(file_out_descriptor > 0, "failed to create file output");

	return 0;
}
