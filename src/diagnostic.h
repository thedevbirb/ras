#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#ifndef DIAGNOSTICS_ERRORS_MAX
#define DIAGNOSTICS_ERRORS_MAX 64
#endif

#ifndef DIAGNOSTICS_WARNINGS_MAX
#define DIAGNOSTICS_WARNINGS_MAX 128
#endif

typedef struct Diagnostic Diagnostic;
struct Diagnostic
{
	String8      filename;
	String8      message_kind;
	U32          row_index;
	U32          column_index_begin;
	U32          column_index_end;
	U32          input_index_start;
	U32	     variant;
};


typedef struct Diagnostics Diagnostics;
struct Diagnostics
{
	Diagnostic *errors;
	Diagnostic *warnings;
	U8	    errors_count;
	U8	    warnings_count;
};

internal void
Diagnostics__push_error(Diagnostics *diagnostics, Diagnostic *error)
{
	if (diagnostics->errors_count < DIAGNOSTICS_ERRORS_MAX)
	{
		diagnostics->errors[diagnostics->errors_count] = *error;
		diagnostics->errors_count += 1;
	}
	return;
}

void
Diagnostic__print(Diagnostic *diagnostic, String8 *input);

#endif // DIAGNOSTIC_H

