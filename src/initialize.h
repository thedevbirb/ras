#ifndef INITIALIZE_H
#define INITIALIZE_H

global U8 hex_table[256];

internal void
Initialize_hex_table(void)
{
    os_memory_set((void *)hex_table, 0xFF, sizeof(hex_table));

    hex_table['0'] = 0x00;
    hex_table['1'] = 0x01;
    hex_table['2'] = 0x02;
    hex_table['3'] = 0x03;
    hex_table['4'] = 0x04;
    hex_table['5'] = 0x05;
    hex_table['6'] = 0x06;
    hex_table['7'] = 0x07;
    hex_table['8'] = 0x08;
    hex_table['9'] = 0x09;
    hex_table['a'] = 0x0A;
    hex_table['b'] = 0x0B;
    hex_table['c'] = 0x0C;
    hex_table['d'] = 0x0D;
    hex_table['e'] = 0x0E;
    hex_table['f'] = 0x0F;
    hex_table['A'] = 0x0A;
    hex_table['B'] = 0x0B;
    hex_table['C'] = 0x0C;
    hex_table['D'] = 0x0D;
    hex_table['E'] = 0x0E;
    hex_table['F'] = 0x0F;

    return;
}


// List of characters that can appear after a number in a lexically valid program.
//
// NOTE: Float literals are not supported.
global U8 numeric_suffix_table[256];

internal void
Initialize_numeric_suffix_table(void)
{
    os_memory_zero(numeric_suffix_table, sizeof(numeric_suffix_table));

    numeric_suffix_table['(']  = 1;
    numeric_suffix_table[')']  = 1;
    numeric_suffix_table[';']  = 1,
    numeric_suffix_table['#']  = 1; // Comments.
    numeric_suffix_table['+']  = 1;
    numeric_suffix_table['-']  = 1;
    numeric_suffix_table['*']  = 1;
    numeric_suffix_table['/']  = 1;
    numeric_suffix_table['^']  = 1;
    numeric_suffix_table['<']  = 1;
    numeric_suffix_table['>']  = 1;
    numeric_suffix_table['=']  = 1;
    numeric_suffix_table['!']  = 1;
    numeric_suffix_table['|']  = 1;
    numeric_suffix_table['&']  = 1;
    numeric_suffix_table['%']  = 1;
    numeric_suffix_table['\n'] = 1;
    numeric_suffix_table['\r'] = 1;
    numeric_suffix_table[' ']  = 1;
    numeric_suffix_table['\t'] = 1;

    return;
}


internal void
Initialize(void)
{
	Initialize_hex_table();
	Initialize_numeric_suffix_table();
	return;
}

#endif // INITIALIZE_H

