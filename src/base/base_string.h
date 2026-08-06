#ifndef BASE_STRING_H
#define BASE_STRING_H

// A byte pointer with its length.
//
// Note that most of the helper functions related to String* will allocate a null-terminated string, but the count field
// will not take into account that extra byte.
typedef struct String8 String8;
struct String8 {
	U64  count;
	U8  *data;
};

typedef struct String8_Node String8_Node;
struct String8_Node
{
        String8_Node *next;
        String8       string;
};

typedef struct String8_Node_List String8_Node_List;
struct String8_Node_List
{
        String8_Node *first;
        String8_Node *last;
        U64           count;
        U64           string_count_total;
};


// ----------------------------------------------------------------------------
// C-strings
// ----------------------------------------------------------------------------

internal U64 cstring8_count(const char *cstring);
internal String8 String8__from_cstring(const char *cstring);

// -----------------------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------------------

// Useful to create strings inside packed tables
#define String8__inline_m(s) (U8 *)s, (sizeof(s) - 1)

// To be used for example with formatting etc.
#define String8__literal(s)    (String8){ .data = (U8 *)(s), .count = sizeof((s)) - 1 }

internal String8 String8__new(U8 *data, U64 count);

internal String8 String8__skip(String8 string, U64 amount);
internal String8 String8__chop(String8 string, U64 amount);

// Used for example to remove quotes
internal String8 String8__skip_chop(String8 token_string);
internal String8 String8__substring(String8 string, U64 count);
internal String8 String8__prefix(String8 string, U64 count);

// -----------------------------------------------------------------------------
// String8_Node_List Constructors
// -----------------------------------------------------------------------------

internal void
String8_Node_List__push(String8_Node_List *list, String8_Node *node);

// -----------------------------------------------------------------------------
// Matching
// -----------------------------------------------------------------------------

typedef enum String_Match_Flags
{
	String_Match_Flag__Case_Insensitive = (1 << 0),
	String_Match_Flag_Right_Side_Sloppy = (1 << 1),
	String_Match_Flag_Slash_Insensitive = (1 << 2),
}
String_Match_Flags;

internal B32 String8__match_exact(String8 a, String8 b);

// -----------------------------------------------------------------------------
// Formatting
// -----------------------------------------------------------------------------

#define String8__varg(s) (int)((s).count), ((s).data)

#if defined(stdin) && defined(va_arg)
internal String8
String8__format_v(Arena *arena, char *format, va_list arguments);

internal String8
String8__format(Arena *arena, char *format, ...);
#endif

#endif // BASE_STRING_H

//------------------------------------------------------------------------------
// Serial
//------------------------------------------------------------------------------

// Treat `String8` as a serialization cursor

// Write data into the serial buffer. Partial data will be written if it doesn't fit.
internal void
String8__serial_write(String8 *string, U8 *data, U64 size);

#define String8__serial_write_m(string, pointer) String8__serial_write(string, (U8 *)pointer, sizeof(*pointer))

//------------------------------------------------------------------------------
// Other
//------------------------------------------------------------------------------

// Returns the size of the literal string, as if were a byte slice, escaping characters.
// Assumes a string with valid escape sequences.
//
// Example: String8.data = [\,n,h,e,l,l,o,\,n] -> 7
internal U64 String8__escaped_size(String8 string);

// -----------------------------------------------------------------------------
// Basic hash functions
// -----------------------------------------------------------------------------

internal U32 FNV_hash_U32(String8 string);
internal U64 FNV_hash_U64(String8 string);
