#ifndef PARSER_EXPRESSION_H
#define PARSER_EXPRESSION_H

typedef enum Expression_Flags
{
	Expression_Flags__None           = 0,
        Expression_Flags__Defer_Dot      = 1,
        // TODO(medium): this is a patch that I don't like much.
        Expression_Flags__Data_Directive = 2,
	Expression_Flags__COUNT,
}
Expression_Flags;

internal Expression *
expression_parse_with_flags
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Symbols_Table      *symbols_table,
        Diagnostics        *diagnostics,
        Expression_Flags    flags
);

internal Expression *
expression_parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Symbols_Table      *symbols_table,
        Diagnostics        *diagnostics
);

// NOTE: both LLVM and GNU as have a precise way of handle relocation operators. They must appear at the beginning of
// the expression, and everything else is absorbed by it. Examples:
//
// - `addi x1, x0, %lo(foo) + 1` is equivalent to `addi x1, x0, %lo(foo + 1)`.
// - `addi x1, x0, 1 + %lo(foo)` is invalid.
//
// If a certain instruction supports a relocation prefix, this should be called before parsing its expression.
internal void
try_parse_relocation_prefix
(
        Token_Cursor             *cursor,
        Diagnostics              *diagnostics,
        // Machine-dependent
        U16                      *relocation_out,
        Relocation_Operator_List  relocation_match_list
);

#endif // PARSER_EXPRESSION_H

