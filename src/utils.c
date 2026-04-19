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
