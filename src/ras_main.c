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

#include "object/object_include.h"

#include "language/language_include.h"
#include "core/core_include.h"
#include "syntax/syntax_include.h"
#include "lexer.h"
#include "parser/parser_include.h"
// #include "core/core_resolver.h"

#include <base/base_include.c>

#include "core/core_include.c"
#include "lexer.c"
#include "language/language_include.c"
#include "parser/parser_include.c"
// #include "core/core_resolver.c"

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

	String8 filename = String8__from_cstring(argument_vector[0]);
	printf("filename: %s\n", filename.data);
	int file_descriptor = open((char *)filename.data, O_RDONLY);
	assert_always_m(file_descriptor > 0 && "failed to find input file");

	struct stat file_in_statistics;
	assert_always_m(fstat(file_descriptor, &file_in_statistics) == 0 && "failed to call fstat on input file");
	assert_always_m(file_in_statistics.st_size >= 0 && "file size is negative");
	U64 file_in_size = (U64)file_in_statistics.st_size;

	Arena *arena = Arena__allocate_m();
	U8 *input_data_mapped = mmap(NULL, file_in_size, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
	assert_always_m(input_data_mapped != MAP_FAILED && "failed to mmap file contents");

	String8 input = { .data = input_data_mapped, .count = file_in_size };

	Source source =
	{
		.data = input.data,
		.name = filename.data,

		.count = input.count,
		.name_count = filename.count,
	};
	U32 source_index = 0;

	// ----
	// Sections_Table creation
	// ---

	Sections_Table *sections_table = Sections_Table__default();
	Sections_Table__add_common(sections_table);
	Section *section = Sections_Table__get(sections_table, String8__literal(".text"));

	Arena *arena_symbols_table = Arena__allocate_m();
	Symbols_Table *symbols_table = Arena__push_struct_m(arena_symbols_table, Symbols_Table);
	symbols_table->arena = arena_symbols_table;
	symbols_table->chunks = Arena__push_struct_m(arena, Symbols_Trie_Chunk_List);

	Expressions expressions = {0};
	Expressions__initialize(&expressions, arena, 12);

	Diagnostic_List diagnostics = {0};

	statement_read
		(
			arena,
			&source,
			&source_index,
			section,
			&diagnostics,
			&expressions,
			sections_table,
			symbols_table
		);

	if (diagnostics.first)
	{
		Diagnostic *current = diagnostics.first;
		for (;;)
		{
			diagnostic_print(current, &source, arena);
			current = current->next;

			if (!current)
			{
				break;
			}
		}
		exit(1);
	}

	return 0;
}
