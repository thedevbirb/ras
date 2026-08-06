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
String8__skip_chop(String8 token_string)
{

        String8 result = {0};
        result = String8__skip(token_string, 1);
        result = String8__chop(result, 1);
        return result;
}

internal String8
String8__substring(String8 string, U64 count)
{
        String8 result = { .data = string.data, .count = min_m(string.count, count) };
        return result;
}

internal String8
String8__prefix(String8 string, U64 count)
{
        U64 count_clamped = min_m(string.count, count);
        String8 result = String8__new(string.data, count_clamped);
        return result;
}

// -----------------------------------------------------------------------------
// String8_Node_List Constructors
// -----------------------------------------------------------------------------

internal void
String8_Node_List__push(String8_Node_List *list, String8_Node *node)
{
        SLL_queue_push_m(list->first, list->last, node);
        list->count += 1;
        list->string_count_total += node->string.count;
        return;
}

// -----------------------------------------------------------------------------
// Matching
// -----------------------------------------------------------------------------

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

internal B32
String8__match_prefix(String8 source, String8 expected)
{
        String8 prefix = String8__prefix(source, expected.count);
        B32 result = String8__match_exact(prefix, expected);
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

//------------------------------------------------------------------------------
// Serial
//------------------------------------------------------------------------------

internal void
String8__serial_write(String8 *string, U8 *data, U64 size)
{
        U64 size_capped = min_m(string->count, size);
        memory_copy(string->data, data, size_capped);
        string->data  += size_capped;
        string->count -= size_capped;
        return;
}

//------------------------------------------------------------------------------
// Other
//------------------------------------------------------------------------------

internal U64
String8__escaped_size(String8 string)
{
        U8  *data  = string.data;
        U64  size  = 0;
        U64  index = 0;

        for (;;)
        {
                B32 break_should = index >= string.count;
                if (break_should)
                {
                        break;
                }

                size += 1;

                if (data[index] == '\\')
                {
                        index += 1;
                        if (data[index] == 'x')
                        {
                                // E.g. \x1a
                                //       ^--- cursor is here
                                // We know from lexing the first one is guaranteed to be valid
                                index += 2;
                                // E.g. \x1a
                                //         ^--- cursor is here
                                if (hex_table[data[index]] != hex_table_invalid)
                                {
                                        index += 1;
                                }
                        }
                        else if (U8__octal_prefix(data[index]))
                        {
                                // E.g. \377
                                //       ^--- cursor is here
                                index += 1;
                                if (U8__octal(data[index]))
                                {
                                        // E.g. \377
                                        //        ^--- cursor is here
                                        index += 1;
                                }

                                if (U8__octal(data[index]))
                                {
                                        // E.g. \377
                                        //         ^--- cursor is here
                                        index += 1;
                                }
                        }
                        else
                        {
                                index += 1;
                        }
                }
                else
                {
                        index += 1;
                }
        }

        return size;
}

// Base hashing

internal U32
FNV_hash_U32(String8 string)
{
        U32 hash = 2166136261u;

        U32 index = 0;
        for (;;)
        {
                B32 break_should = index >= string.count;
                if (break_should)
                {
                        break;
                }

                hash ^= (U8)string.data[index];
                hash *= 16777619u;

                index += 1;
        }

        return hash;
}

internal U64
FNV_hash_U64(String8 string)
{
        U64 hash = 2166136261ull;

        U64 index = 0;
        for (;;)
        {
                B32 break_should = index >= string.count;
                if (break_should)
                {
                        break;
                }

                hash ^= (U8)string.data[index];
                hash *= 16777619ull;

                index += 1;
        }

        return hash;
}
