#ifndef CORE_INITIALIZE_H
#define CORE_INITIALIZE_H

global U8 hex_table[256];
global U8 hex_table_invalid = 0xFF;

internal void
Initialize_hex_table(void);

// List of characters that can appear after a number in a lexically valid program.
//
// NOTE: Float literals are not supported.
global U8 numeric_suffix_table[256];

internal void
Initialize_numeric_suffix_table(void);

global U8 escape_value_invalid = 0xFF;
global U8 escape_table[256];

internal void
Initialize_escape_table(void);

internal void
Initialize(void);

#endif // CORE_INITIALIZE_H

