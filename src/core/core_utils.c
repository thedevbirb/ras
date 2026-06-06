// TODO: this is a bin of standalone utils I don't know where to put. I don't like utils files in general.

internal B32
U8__octal_prefix(U8 byte)
{
	B32 result = '0' <= byte && byte < '4';
	return result;
}

internal B32
U8__octal(U8 byte)
{
	B32 result = '0' <= byte && byte < '7';
	return result;
}

internal U32
hash_FNV_1a(String8 string)
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
commas_until_newline(U8 *data, U64 count)
{
	U64 result  = 0;
	U64 index   = 0;
	U8  current = 0;
	for (;;)
	{
		if (current == '\n' || index >= count)
		{
			break;
		}
		current = data[index];
		result += current == ',';
		index += 1;
	}
	return result;
}

// Returns the size of the literal string, as if were a byte slice, escaping characters.
// Assumes a string with valid escape sequences.
//
// Example: String8.data = [",\,n,h,e,l,l,o,\,n,"] -> 7
internal U32
String8__escaped_size(String8 string)
{
	assert_always_m(string.count >= 2);
	U32 size = 0;
	U32 index = 1;
	U32 count = string.count - 2; // No ".
	U8 *data = string.data;

	for (;;)
	{
		B32 break_should = index >= count;
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
				index += 2;
			}
		}
		else
		{
			index += 1;
		}
	}

	return size;
}

internal void
bytes_escaped_fill(String8 text, U8 *out, U32 write_max)
{
	U32  index         = 0;
	U32  bytes_written = 0;
	U8  *data = text.data;

	for (;;)
	{
		B32 break_should = bytes_written >= write_max;
		if (break_should)
		{
			break;
		}

		U8 byte = 0;

		if (data[index] == '\\')
		{
			index += 1;
			if (data[index] == 'x')
			{
				// E.g. \x1a
				//       ^--- cursor is here
				index += 1;
				// E.g. \x1a
				//        ^--- cursor is here
				byte = data[index];
				index += 1;
				U8 value = hex_table[data[index]];
				if (value != hex_table_invalid)
				{
					// E.g. \x1a
					//         ^--- cursor is here
					byte = byte * 16 + data[index];
					index += 1;
				}
			}
			else if (U8__octal_prefix(data[index]))
			{
				// E.g. \377
				//       ^--- cursor is here
				byte = data[index];
				index += 1;
				if (U8__octal(data[index]))
				{
					// E.g. \377
					//        ^--- cursor is here
					byte = byte * 8 + data[index];
					index += 1;
				}
				if (U8__octal(data[index]))
				{
					// E.g. \377
					//         ^--- cursor is here
					byte = byte * 8 + data[index];
					index += 1;
				}
			}
			else
			{
				byte = escape_table[data[index]];
				assert_always_m(byte != escape_value_invalid);
				index += 2;
			}
		}
		else
		{
			byte = data[index];
		}


		out[bytes_written] = byte;
		bytes_written += 1;
	}

	return;
}

// TODO: actually modify this for U64, it has been retrofitted from U32 implementation.
internal U64
FNV_hash(String8 key)
{
	U64 hash = 2166136261ull;

	for (U64 i = 0; i < key.count; i++)
	{
		hash ^= key.data[i];
		hash *= 16777619ull;
	}

	return hash;
}

internal U8
number_to_digits_count(U64 number)
{
	U8 digits = 1;
	for (;;)
	{
		number /= 10;
		B32 break_should = number == 0;
		if (break_should)
		{
			break;
		}
		digits += 1;
	}
	return digits;
}

internal U8
label_symbol_prefix_set(U8 *data, U8 label_numeric_value)
{
	data[0] = '.';
	data[1] = 'L';
	data[2] = '0' + label_numeric_value;
	data[3] = 0x02;
	return 4;
}

internal B32
S64_bits_range_in(S64 signed_integer, U8 bits)
{
	S64 limit_low  = -(1LL << (bits - 1));
	S64 limit_high =  (1LL << (bits - 1)) - 1;
	B32 result = - limit_low < signed_integer && signed_integer && limit_high;
	return result;
}

// Completely branchless, freestanding implementation.
internal U8
count_trailing_zeros(U64 x)
{
	// If zero, set it to one. This would give a result of no trailing zeros.
	U8 is_zero = (x == 0);
	x |= (U64)is_zero;

	// Branchless binary search of lowest set bit. Each mask checks whether the right most 2^count bits are zeros,
	// and computes how much we can shift, hence how many trailing zeros found.
	U8  count = 0;
	U32 shift = 0;

	shift = ((x & 0x00000000FFFFFFFFULL) == 0) << 5;  count += shift; x >>= shift;
	shift = ((x & 0x000000000000FFFFULL) == 0) << 4;  count += shift; x >>= shift;
	shift = ((x & 0x00000000000000FFULL) == 0) << 3;  count += shift; x >>= shift;
	shift = ((x & 0x000000000000000FULL) == 0) << 2;  count += shift; x >>= shift;
	shift = ((x & 0x0000000000000003ULL) == 0) << 1;  count += shift; x >>= shift;
	shift =  (x & 0x0000000000000001ULL) == 0;        count += shift;

	// If x was originally zero, bump the result from 0 to 64. Otherwise 0.
	count += is_zero << 6;
	return count;
}

