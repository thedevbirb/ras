// TODO: this is a bin of standalone utils I don't know where to put. I don't like utils files in general.

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

// Returns the size of the literal string, as if were a byte slice, escaping characters.
// Assumes a string with valid escape sequences.
//
// Example: String8.data = [",\,n,h,e,l,l,o,\,n,"] -> 7
internal U32
String8_byte_size_escaped(String8 string)
{
	U32 size = 0;
	U32 index = 1;
	U32 count = string.count - 2; // No ".
	for (;;)
	{
		B32 break_should = index >= count;
		if (break_should)
		{
			break;
		}

		size += 1;

		U8 character_outer = string.data[index];
		if (character_outer == '\\')
		{
			index += 1;
			U8 character_escaped = string.data[index];
			B32 hex_prefix   = character_escaped == 'x';
			B32 octal_prefix = 0 <= character_escaped - '0' && character_escaped - '0' < 8;

				if (hex_prefix)
				{
					// E.g. \x1a
					//       ^--- cursor is here
					// We know from lexing the first one is guaranteed to be valid
					index += 2;
					// E.g. \x1a
					//         ^--- cursor is here
					U8 character_inner = string.data[index];
					if (hex_table[character_inner] != hex_table_sentinel_invalid)
					{
						index += 1;
					}
				}
				else if (octal_prefix)
				{
					// E.g. \377
					//       ^--- cursor is here
					index += 1;
					U8 character_inner = string.data[index];
					if (character_inner - '0' < 8)
					{
						index += 1;
					}

					character_inner = string.data[index];
					if (character_inner - '0' < 8)
					{
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

