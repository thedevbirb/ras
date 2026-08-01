// Safe casts

internal  U8  U8_cast_safe(U16 x)          { assert_always_m(x <= U8_max);        U8 result =  (U8)x; return result; }
internal U16 U16_cast_safe(U32 x)          { assert_always_m(x <= U16_max);      U16 result = (U16)x; return result; }
internal U32 U32_cast_safe(U64 x)          { assert_always_m(x <= U32_max);      U32 result = (U32)x; return result; }
internal S32 S32_cast_safe(S64 x)          { assert_always_m(x <= S32_max);      S32 result = (S32)x; return result; }
internal S64 S64_from_U64_cast_safe(U64 x) { assert_always_m(x <= (U64)S64_max); S64 result = (S64)x; return result; }
internal U64 U64_from_S64_cast_safe(S64 x) { assert_always_m(x >= 0);            U64 result = (U64)x; return result; }

// Bit-patterns

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
