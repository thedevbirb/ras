// Safe casts

internal  U8  U8_cast_safe(U16 x)          { assert_always_m(x <= U8_max);        U8 result =  (U8)x; return result; }
internal U16 U16_cast_safe(U32 x)          { assert_always_m(x <= U16_max);      U16 result = (U16)x; return result; }
internal U32 U32_cast_safe(U64 x)          { assert_always_m(x <= U32_max);      U32 result = (U32)x; return result; }
internal S32 S32_cast_safe(S64 x)          { assert_always_m(x <= S32_max);      S32 result = (S32)x; return result; }
internal S64 S64_from_U64_cast_safe(U64 x) { assert_always_m(x <= (U64)S64_max); S64 result = (S64)x; return result; }
internal U64 U64_from_S64_cast_safe(S64 x) { assert_always_m(x >= 0);            U64 result = (U64)x; return result; }

// Bit-patterns

internal B32
S64_bits_range_in(S64 signed_integer, U8 bits)
{
        S64 limit_low  = -(1LL << (bits - 1));
        S64 limit_high =  (1LL << (bits - 1)) - 1;
        B32 result = limit_low <= signed_integer && signed_integer <= limit_high;
        return result;
}

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)

internal U64 count_bits_set32(U32 val) { return __popcnt(val); }
internal U64 count_bits_set64(U64 val) { return __popcnt64(val); }

internal U64 ctz32(U32 mask) { unsigned long idx; _BitScanForward(&idx,   mask); return idx;      }
internal U64 ctz64(U64 mask) { unsigned long idx; _BitScanForward64(&idx, mask); return idx;      }
internal U64 clz32(U32 mask) { unsigned long idx; _BitScanReverse(&idx,   mask); return 31 - idx; }
internal U64 clz64(U64 mask) { unsigned long idx; _BitScanReverse64(&idx, mask); return 63 - idx; }

#elif COMPILER_CLANG || COMPILER_GCC

internal U64 count_bits_set32(U32 val) { return __builtin_popcount(val);   }
internal U64 count_bits_set64(U64 val) { return __builtin_popcountll(val); }

internal U64 ctz32(U32 val) { return __builtin_ctz(val);   }
internal U64 clz32(U32 val) { return __builtin_clz(val);   }
internal U64 ctz64(U64 val) { return __builtin_ctzll(val); }
internal U64 clz64(U64 val) { return __builtin_clzll(val); }
internal U64 msb64(U64 val) { return 63 - clz64(val);      }
internal U64 msb32(U64 val) { return 31 - clz64(val);      }

#else
#error "Bit intrinsic functions not defined for this compiler."
#endif

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

// Completely branchless, freestanding implementation.
internal U8
count_leading_zeros(U64 x)
{
        // If zero, set it to one. This would give a result of no trailing zeros.
        U8 is_zero = (x == 0);
        x |= (U64)is_zero;

        // Branchless binary search of highest set bit. Each mask checks whether the left most 2^count bits are zeros,
        // and computes how much we can shift, hence how many leading zeros found.
        U8  count = 0;
        U32 shift = 0;

        // The difference with the trailing zero version is simply the mask and how we shift the copied value in input.

        shift = ((x & 0xFFFFFFFF00000000ULL) == 0) << 5;  count += shift; x <<= shift;
        shift = ((x & 0xFFFFFFFFFFFF0000ULL) == 0) << 4;  count += shift; x <<= shift;
        shift = ((x & 0xFFFFFFFFFFFFFF00ULL) == 0) << 3;  count += shift; x <<= shift;
        shift = ((x & 0xFFFFFFFFFFFFFFF0ULL) == 0) << 2;  count += shift; x <<= shift;
        shift = ((x & 0xFFFFFFFFFFFFFFF3ULL) == 0) << 1;  count += shift; x <<= shift;
        shift =  (x & 0xFFFFFFFFFFFFFFF1ULL) == 0;        count += shift;

        // If x was originally zero, bump the result from 0 to 64. Otherwise 0.
        count += is_zero << 6;
        return count;
}

// Base64 decoding table: maps a base64 character to its 6-bit value, 0xFF for invalid.
global const U8 base64_table[256] =
{
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

