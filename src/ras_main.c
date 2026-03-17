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

#include "lexer.h"

#include <base/base_include.c>
#include <os/os_include.c>

#include "lexer.c"

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

	U8 *data = mmap(NULL, file_in_statistics.st_size, PROT_READ, MAP_PRIVATE, file_in_descriptor, 0);
	assert_always_m(data != MAP_FAILED && "failed to mmap file contents");

	arguments_shift(&argument_count, &argument_vector);

	String8 input = { .data = data, .count = file_in_statistics.st_size };

	Arena *arena = Arena_alloc_m();
	Token_Array token_array = LE_tokenize(&input, arena);

	Lexer_Error error = token_array.error;
	if (error.kind)
	{
		fprintf(stderr, "\x1B[1m%s:%d:%d:\x1B[0m \x1B[1;31merror:\x1B[0m ", file_in_path, error.row_index + 1, error.column_index + 1);
		if (error.kind == Lexer_Error_Kind__String_Multiline_Unsupported)
		{
			fprintf(stderr, "unsupported multine comment\n");
		}
		// fprintf(stderr, "%5d | %s\n");
	}

	// const char *file_path_out = argument_vector[0];
	// printf("file path out: %s\n", file_path_out);
	// int file_out_descriptor = open(file_path_out, O_CREAT|O_WRONLY|O_TRUNC, 0644);
	// assert_always_m(file_out_descriptor > 0, "failed to create file output");

	return 0;
}
