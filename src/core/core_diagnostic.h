#ifndef CORE_DIAGNOSTIC_H
#define CORE_DIAGNOSTIC_H

global String8 Diagnostic__previous_declaration_String8 = String8__literal("previous declaration is here");

typedef enum Diagnostic_Kind
{
        Diagnostic_Kind__Error,
        Diagnostic_Kind__Warning,
        Diagnostic_Kind__Hint,
        Diagnostic_Kind__Note,
        Diagnostic_Kind__COUNT,
}
Diagnostic_Kind;

typedef enum Diagnostic_ANSI_Color
{
        Diagnostic_ANSI_Color_Default = 39,
        Diagnostic_ANSI_Color_Red     = 31,
        Diagnostic_ANSI_Color_Green   = 32,
        Diagnostic_ANSI_Color_Yellow  = 33,
        Diagnostic_ANSI_Color_Blue    = 34,
        Diagnostic_ANSI_Color_Magenta = 35,
        Diagnostic_ANSI_Color_Cyan    = 36,
}
Diagnostic_ANSI_Color;

typedef struct Diagnostic_Style Diagnostic_Style;
struct Diagnostic_Style
{
        Diagnostic_ANSI_Color color;
        B32                   bold;
};

internal const char *diagnostic_labels[] =
{
        [Diagnostic_Kind__Error]   = "error",
        [Diagnostic_Kind__Warning] = "warning",
        [Diagnostic_Kind__Hint]    = "hint",
        [Diagnostic_Kind__Note]    = "note",
};

internal Diagnostic_Style diagnostic_styles[] =
{
        [Diagnostic_Kind__Error]   = { .color = Diagnostic_ANSI_Color_Red,     .bold = 1 },
        [Diagnostic_Kind__Warning] = { .color = Diagnostic_ANSI_Color_Yellow,  .bold = 1 },
        [Diagnostic_Kind__Hint]    = { .color = Diagnostic_ANSI_Color_Magenta, .bold = 1 },
        [Diagnostic_Kind__Note]    = { .color = Diagnostic_ANSI_Color_Cyan,    .bold = 1 },
};

global const Diagnostic_Style Diagnostic_Style__default_bold = { .color = Diagnostic_ANSI_Color_Default, .bold = 1 };

typedef struct Diagnostic_Fix Diagnostic_Fix;
struct Diagnostic_Fix
{
        Range1_U32 range;
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
        Range1_U32      ranges[4];
        // Additional lines with text fixes.
        Diagnostic_Fix  fixes[4];
        Diagnostic_Kind kind;
};

// TODO(feature): may need to support:
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

// TODO(feature): support multiple sources.
internal void
diagnostic_print
(
        Diagnostic *diagnostic,
        Source     *source,
        Arena      *arena
);

#endif // CORE_DIAGNOSTIC_H

