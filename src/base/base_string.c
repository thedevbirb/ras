internal B32
String8__match_exact(String8 a, String8 b)
{
	B32 match = 0;
	if (a.count == b.count)
	{
		match = memory_match(a.data, b.data, a.count) == 0;
	}

	return match;
}

// -------------------------------------------------------------------------
// C-strings
// -------------------------------------------------------------------------


internal U64
cstring8_count(const char *cstring)
{
	U64 count = 0;
	char *p = (char *)(void *)cstring;
	for (; *p; p++);
	count = p - cstring;
	return count;
}

internal String8
String8__from_cstring(const char *cstring)
{
	U64 count = cstring8_count(cstring);
	String8 result =
	{
		.data  = (U8 *)cstring,
		.count = count,
	};
	return result;
}

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------

internal String8
String8__new(U8 *data, U64 count)
{
	String8 result = { .data = data, .count = count };
	return result;
}

internal String8
String8__duplicate(Arena *arena, String8 string)
{
	U8 *data = Arena__push_array_no_zero_m(arena, U8, string.count);
	memory_copy(data, string.data, string.count);
	String8 result = { .data = data, .count = string.count };
	return result;
}

internal String8
String8__duplicate_null_terminated(Arena *arena, String8 string)
{
	U8 *data = Arena__push_array_no_zero_m(arena, U8, string.count + 1);
	memory_copy(data, string.data, string.count);
        data[string.count] = 0;
	String8 result = { .data = data, .count = string.count };
        return result;
}

internal String8
String8__skip(String8 string, U64 amount)
{
	U64 skip = min_m(amount, string.count);
	String8 result = { .data = string.data + skip, .count = string.count - skip };
	return result;
}

internal String8
String8__chop(String8 string, U64 amount)
{
	U64 chop = min_m(amount, string.count);
	String8 result = { .data = string.data, .count = string.count - chop };
	return result;
}

internal String8
String8__substring(String8 string, U64 count)
{
        String8 result = { .data = string.data, .count = min_m(string.count, count) };
        return result;
}

// ----------------------------------------------------------------------------
// Formatting
// ----------------------------------------------------------------------------

#if defined(stdin) && defined(va_arg)
internal String8
String8__format_v(Arena *arena, char *format, va_list arguments)
{
	va_list arguments_copy;
	va_copy(arguments_copy, arguments);

	U32 bytes_needed = vsnprintf(0, 0, format, arguments) + 1;
	String8 result = {0};

	result.data  = Arena__push_array_no_zero_m(arena, U8, bytes_needed);
	result.count = vsnprintf((char*)result.data, bytes_needed, format, arguments_copy);
	result.data[result.count] = 0;

	va_end(arguments_copy);
	return result;
}

internal String8
String8__format(Arena *arena, char *format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	String8 result = String8__format_v(arena, format, arguments);
	va_end(arguments);
	return result;
}
#endif
