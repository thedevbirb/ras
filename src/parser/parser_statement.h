#ifndef PARSER_STATEMENT_H
#define PARSER_STATEMENT_H

// Read all the statements contained in the source file
internal void
statements_read
(
        Arena             *arena,
        Token_Cursor      *cursor,
        Diagnostics       *diagnostics,
        Expressions       *expressions,
        Symbols_Table     *symbols_table,
        Options           *options
);

#endif // PARSER_STATEMENT_H
