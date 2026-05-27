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

#include <generated/instruction_hashes.h>

#include "initialize.h"
#include "elf.h"

#include "language/language_include.h"
#include "symbol.h"
// #include "section.h"

#include "diagnostic.h"
#include "lexer.h"
#include "expression.h"
#include "statement.h"
#include "parser/parser_include.h"
// #include "resolver.h"

#include <base/base_include.c>

#include "initialize.c"
#include "utils.c"
#include "language/language_include.c"
// #include "symbol.c"
// #include "section.c"

#include "diagnostic.c"
#include "lexer.c"
#include "expression.c"
#include "statement.c"
#include "parser/parser_include.c"
// #include "resolver.c"

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

	const char *file_in_path = argument_vector[0];
	printf("file_in_path: %s\n", file_in_path);
	int file_in_descriptor = open(file_in_path, O_RDONLY);
	assert_always_m(file_in_descriptor > 0 && "failed to find input file");

	struct stat file_in_statistics;
	assert_always_m(fstat(file_in_descriptor, &file_in_statistics) == 0 && "failed to call fstat on input file");
	assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
	U64 file_in_size = ((U64)file_in_statistics.st_size);

	Arena *arena = Arena__allocate_m();
	U8 *input_data_mapped = mmap(NULL, file_in_size, PROT_READ, MAP_PRIVATE, file_in_descriptor, 0);
	assert_always_m(input_data_mapped != MAP_FAILED && "failed to mmap file contents");
	String8 input = { .data = input_data_mapped, .count = file_in_size };

	// Arena *arena_statements = Arena__allocate_m(.reserve_size = file_in_size * 8, .flags = Arena_Flags__No_Chain);
	// Statements statements;
	// Statements_initialize(&statements, arena_statements);
	//
	// Arena *arena_expressions = Arena__allocate_m(.reserve_size = file_in_size, .flags = Arena_Flags__No_Chain);
	// Expressions expressions;
	// Expressions_initialize(&expressions, arena_expressions);

	// Symbols_Table symbols_table = {0};
	// Symbols_Table_initialize(&symbols_table, arena);
	//
	// // TODO: while some of them can be created in advance, in practice arbitrary sections can be created on demand
	// // using the `.section` directive, so I should change around this.
	// Object_File_Section *sections = Object_File_Section_create_all(arena, file_in_size);
	//
	// Statement *statement_context_parser = Arena__push_struct_m(arena, Statement);

	// I think diagnostics should be provided as an argument to the tokenize function
	// and track where things actually happen.
	//
	// It can be a struct comprising of a fixed list (most 64 errors, how many warnings tho, max 1024?) of errors,
	// warnings and potentially hints.

	Diagnostics diagnostics =
	{
		.errors   = Arena__push_array_m(arena, Diagnostic, DIAGNOSTICS_ERRORS_MAX),
		.warnings = Arena__push_array_m(arena, Diagnostic, DIAGNOSTICS_WARNINGS_MAX),
	};

	Lexer lexer = { .input = &input };

	Symbols_Trie            symbols_trie            = {0};
	Symbols_Trie_Chunk_List symbols_trie_chunk_list = {0};

	Statements_Xar statements = {0};
	xar_initialize_m(&statements, 12);

	String8 filename = String8__from_cstring(file_in_path);

	Expressions expressions = {0};
	Expressions__initialize(&expressions, arena, 12);

	Statement_Expressions_Xar statement_expressions = {0};
	xar_initialize_m(&statement_expressions, 12);

	Parser_2 parser =
	{
		.filename                  = filename,
		.input                     = input,
		.lexer                     = &lexer,
		.expressions               = &expressions,
		.diagnostics               = &diagnostics,
		.symbols_trie              = &symbols_trie,
		.symbols_trie_chunk_list   = &symbols_trie_chunk_list,
		.statement_expressions     = &statement_expressions,
	};


	for (;;)
	{
		Statement statement = Parser_2__statement(&parser, arena);
		break;
	}


	if (diagnostics.errors_count > 0)
	{
		U8 index = 0;
		for (;;)
		{
			Diagnostic *d = &diagnostics.errors[index];
			Diagnostic__print(d, &input);

			index += 1;
			if (index >= diagnostics.errors_count)
			{
				break;
			}
		}
		exit(1);
	}

	// Parser parser =
	// {
	// 	.arena          = arena,
	// 	.input          = &input,
	// 	.tokens         = token_array.tokens,
	// 	.statements     = &statements,
	// 	.symbols_table  = &symbols_table,
	// 	.expressions    = &expressions,
	//
	// 	.statement_context = statement_context_parser,
	//
	// 	.section_current_index = ELF_Section__Text,
	//
	// 	.token_current = token_array.tokens[0],
	// 	.token_count   = token_array.token_count,
	// 	.token_index   = 0,
	// 	.end_reached   = 0 >= token_array.token_count
	// };
	//
	// Parser_parse(&parser);
	// Parser_Error parser_error = parser.error;
	// if (parser_error.kind)
	// {
	// 	Diagnostic diagnostic =
	// 	{
	// 		.input              = &input,
	// 		.file_in_path       = file_in_path,
	// 		.message_kind       = Parser_Error_Kind_messages[parser_error.kind],
	// 		.line               = parser_error.row_index + 1,
	// 		.column_index_begin = parser_error.column_index_begin,
	// 		.column_index_end   = parser_error.column_index_end,
	// 		.input_index_start  = token_array.line_start_indexes[parser_error.row_index],
	// 	};
	//
	// 	Diagnostic_print(&diagnostic);
	// 	exit(1);
	// }
	//
	// Resolver resolver =
	// {
	// 	.arena         = arena,
	// 	.input         = &input,
	// 	.tokens        = token_array.tokens,
	// 	.statements    = &statements,
	// 	.symbols_table = &symbols_table,
	// 	.expressions   = &expressions,
	//
	// 	.sections = sections,
	//
	// 	.statement_current      = &statements.data[0],
	// 	.statement_index        = 0,
	// 	.statements_end_reached = 0 >= statements.count,
	//
	// 	.error         = {0},
	// 	.sections_offset = {0},
	// 	.section_current_index = ELF_Section__Text,
	// };
	//
	// Resolver_relax(&resolver);
	//
	// Resolver_Error resolver_error = resolver.error;
	// if (resolver_error.kind)
	// {
	// 	Diagnostic diagnostic =
	// 	{
	// 		.input              = &input,
	// 		.file_in_path       = file_in_path,
	// 		.message_kind       = Resolver_Error_Kind_messages[resolver_error.kind],
	// 		.line               = resolver_error.row_index + 1,
	// 		.column_index_begin = resolver_error.column_index_begin,
	// 		.column_index_end   = resolver_error.column_index_end,
	// 		.input_index_start  = token_array.line_start_indexes[resolver_error.row_index],
	// 	};
	//
	// 	Diagnostic_print(&diagnostic);
	// 	exit(1);
	// }
	//
	// Resolver_encode(&resolver);

	// TODO: write file.

	// const char *file_path_out = argument_vector[0];
	// printf("file path out: %s\n", file_path_out);
	// int file_out_descriptor = open(file_path_out, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	// assert_always_m(file_out_descriptor > 0, "failed to create file output");

	return 0;
}
