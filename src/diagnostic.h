#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

typedef struct Diagnostic Diagnostic;
struct Diagnostic
{
	Input *input;
	char  *file_in_path;
	char  *message_kind;
	U32    line;
	U32    column_index_begin;
	U32    column_index_end;
	U32    input_index_start;
};

void
Diagnostic_print(Diagnostic *diagnostic);

#endif // DIAGNOSTIC_H

