#ifndef PARSER_EXPRESSION_H
#define PARSER_EXPRESSION_H

typedef enum Expression_Flags
{
	Expression_Flags__None      = 0,
        Expression_Flags__Defer_Dot = 1,
	Expression_Flags__COUNT,
}
Expression_Flags;

internal Expression_Node *
expression_parse_with_flags
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Section            *section,
        Diagnostic_List    *diagnostics,
        Expression_Flags    flags
);

internal Expression_Node *
expression_parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Section            *section,
        Diagnostic_List    *diagnostics
);

internal Expression_Node *
expression_parse_with_relocation
(
        Arena                    *arena,
        Token_Cursor             *cursor,
        Expressions              *expressions,
        Symbols_Table            *symbols_table,
        Section                  *section,
        Diagnostic_List          *diagnostics,
        // Machine-dependent
        U16                      *relocation_out,
        Relocation_Operator_List  relocation_match_list
);

#endif // PARSER_EXPRESSION_H

