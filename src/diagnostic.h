#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

typedef enum Diagnostic_Kind
{
	Diagnostic_Kind__Error,
	Diagnostic_Kind__Warning,
	Diagnostic_Kind__Hint,
	Diagnostic_Kind__COUNT,
}
Diagnostic_Kind;

typedef struct Diagnostic_Fix Diagnostic_Fix;
struct Diagnostic_Fix
{
	Vec2_U32 range;
	String8  text;
};

// A singly-linked double-ended list (queue) of diagnostics. I've chosen a queue because I just want to append
// them as we go, with only a very light container struct to manage them.

typedef struct Diagnostic Diagnostic;
struct Diagnostic
{
	Diagnostic     *next;
	String8         message;
	// Logical location where the diagnostic has been emitted.
	U32             location;
	// Squiggly ('~') ranges to put under the line of the provided logical location.
	Vec2_U32        ranges[4];
	// Additional lines with text fixes.
	Diagnostic_Fix  fixes[4];
	Diagnostic_Kind kind;
};

// TODO: may need to support:
//
// 1. Configuration, e.g. transform warning in errors.
// 2. Maybe counter of errors to abort on too many of them?
typedef struct Diagnostic_List Diagnostic_List;
struct Diagnostic_List
{
	Diagnostic *first;
	Diagnostic *last;
	U32         count;
};

// TODO: support multiple sources.
internal void
diagnostic_print
(
	Diagnostic *diagnostic,
	Source     *source,
	Arena      *arena
);

#endif // DIAGNOSTIC_H

