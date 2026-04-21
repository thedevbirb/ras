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

// Return the number of instructions needed to expand an LI pseudo-instruction.
//
// The algorithm proceeds by range analysis:
//
//   - If the value fits in a 12-bit signed range, a single ADDI suffices.
//   - If it fits in a 32-bit signed range, it takes LUI alone (if the low 12 bits are zero) or LUI + ADDIW otherwise.
//     ADDIW (not ADDI) is used because the result is meant to be a 32-bit sign-extended value.
//
// Otherwise, we peel the low 12 bits off as a sign-extended tail (to be spliced back with an ADDI later),
// arithmetic-shift the remainder right by 12, and recurse on the upper portion. Each recursive level contributes one
// SLLI (to shift the upper part back into place) plus one ADDI (to splice in the peeled 12 bits, if non-zero).
//
// Note: after the initial LUI + ADDIW builds the topmost 32-bit chunk, every subsequent low-bit insertion uses plain
// ADDI, not ADDIW. ADDIW would discard the upper 32 bits we just shifted in.
//
// Example: li x1, 0x12345111333555
//
// Peeling (top-down analysis):
//
//   value = 0x12345111333555
//     peel low 12 bits = 0x555, shift right by 12
//   value = 0x12345111333
//     peel low 12 bits = 0x333, shift right by 12
//   value = 0x12345111
//     fits in 32-bit signed -> LUI 0x12345, ADDIW 0x111
//
// Emission (bottom-up assembly, 6 instructions):
//
//   lui   ra, 0x12345    ; ra = 0x0000000012345000
//   addiw ra, ra, 0x111  ; ra = 0x0000000012345111   <- base case
//   slli  ra, ra, 12     ; ra = 0x0000012345111000
//   addi  ra, ra, 0x333  ; ra = 0x0000012345111333   <- splice 0x333
//   slli  ra, ra, 12     ; ra = 0x0012345111333000
//   addi  ra, ra, 0x555  ; ra = 0x0012345111333555   <- splice 0x555
//
// The symmetry is the key insight: each level of peeling on the way down (shift right by 12, record a tail) becomes one
// SLLI + ADDI pair on the way back up (shift left by 12, replay the tail). The base case at the bottom of the recursion
// is the LUI (+ optional ADDIW) that seeds the topmost 32-bit chunk.
//
// A minor refinement: if the upper part has trailing zero bits after peeling, they can be folded into the next SLLI
// (shift by more than 12) without costing extra instructions. This doesn't change the instruction count but explains
// why real emitted sequences sometimes show shifts like 13 or 14 instead of a uniform 12. Similarly, we can short
// circuit on whether we find powers of two.
internal U8
LI_instructions_count(S64 immediate)
{
	U8  instructions_count = 0;
	S64 immediate_low_12  = 0;

	U32 index = 0;

	for (;;)
	{

		B32 range_i_s        = S64_bits_range_in(immediate, IMMEDIATE_NOMINAL_I_S_SIZE_BIT);
		B32 range_i_s_plus_u = S64_bits_range_in(immediate, IMMEDIATE_NOMINAL_I_S_SIZE_BIT + IMMEDIATE_NOMINAL_U_SIZE_BIT);
		B32 break_should     = range_i_s || range_i_s_plus_u;

		if (range_i_s)
		{
			instructions_count += 1;
		}
		else if (range_i_s_plus_u)
		{

			immediate_low_12 = (immediate << 52) >> 52;
			B32 lui_suffices = immediate_low_12 == 0;
			instructions_count += lui_suffices ? 1 : 2;
		}
		else
		{
			immediate_low_12   = (immediate << 52) >> 52;
			immediate          = (immediate - immediate_low_12) >> 12;
			instructions_count += 2;
		}

		if (break_should)
		{
			break;
		}

		index += 1;
		assert_always_m(index < 8 && "infinite loop");
	}

	assert_always_m(instructions_count > 0);

	return instructions_count;
}

