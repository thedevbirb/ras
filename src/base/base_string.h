#ifndef BASE_STRING_H
#define BASE_STRING_H

// A byte pointer with its length.
//
// Note that most of the helper functions related to String* will allocate a null-terminated string, but the count field
// will not take into account that extra byte.
typedef struct String8 String8;
struct String8 {
	U8  *data;
	U64  count;
};

internal B32
String8__match_exact(String8 a, String8 b);

internal B32
U8_ascii_lower_is(U8 character)
{
	B32 result = character >= 'a' && character <= 'z';
	return result;
}

internal B32
U8_ascii_upper_is(U8 character)
{
	B32 result = character >= 'A' && character <= 'Z';
	return result;
}

internal B32
U8_ascii_letter_is(U8 character)
{
	B32 result = U8_ascii_lower_is(character) || U8_ascii_upper_is(character);
	return result;
}

internal B32
U8_ascii_printable_is(U8 character)
{
	// From <SPACE> to <DEL>, excluded.
	B32 result = (character >= 0x20 && character < 0x7f) || character == '\t';
	return result;
}

// GNU as supports these escape sequences inside string literals:
// - \\ — backslash
// - \" — double quote
// - \n — newline (0x0A)
// - \t — tab (0x09)
// - \r — carriage return (0x0D)
// - \0 — null (0x00)
// - \a — bell (0x07)
// - \b — backspace (0x08)
// - \f — form feed (0x0C)
// - \NNN — octal value (1–3 octal digits)
// - \xNN — hex value (1–2 hex digits)
//

internal B32
U8_ascii_escape_sequence_start_is(U8 character)
{
	B32 result =
		character == '\\' ||
		character == '"'  ||
		character == 'n'  ||
		character == 't'  ||
		character == 'r'  ||
		character == '0'  ||
		character == 'a'  ||
		character == 'b'  ||
		character == 'f'  ||
		character == 'N'  ||
		character == 'x';
	return result;
}

internal B32
U8_ascii_digit_is(U8 character)
{
	B32 ascii_digit_is = character >= '0' && character <= '9';
	return ascii_digit_is;
}


typedef enum String_Match_Flags
{
	String_Match_Flag__Case_Insensitive = (1 << 0),
	String_Match_Flag_Right_Side_Sloppy = (1 << 1),
	String_Match_Flag_Slash_Insensitive = (1 << 2),
}
String_Match_Flags;

// ----------------------------------------------------------------------------
// C-strings
// ----------------------------------------------------------------------------

internal U64 cstring8_count(const char *cstring);
internal String8 String8__from_cstring(const char *cstring);

// -----------------------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------------------

// To be used for example with formatting etc.
#define String8__literal(s)    (String8){ .data = (U8 *)(s), .count = sizeof((s)) - 1 }

internal String8
String8__new(U8 *data, U64 count);

internal String8
String8__skip(String8 string, U64 amount);

internal String8
String8__chop(String8 string, U64 amount);

internal String8
String8__substring(String8 string, U64 count);

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
