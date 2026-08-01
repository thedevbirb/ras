#ifndef BASE_MATH_H
#define BASE_MATH_H

// align_pow_2_m
//
// Rounds `x` up to the next multiple of `b`, where `b` must be a power of two.
//
// This is commonly used for memory alignment (e.g. aligning sizes or addresses
// to 8, 16, 64 bytes, etc.).
//
// How it works:
// - Adding (b - 1) ensures that any value not already aligned crosses
//   into the next alignment boundary.
// - Masking with ~(b - 1) clears the lower bits, snapping the result
//   down to the nearest aligned multiple.
//
// Examples:
//   align_pow_2_m(13, 8)  -> 16
//   align_pow_2_m(16, 8)  -> 16
//   align_pow_2_m(17, 8)  -> 24
//
// Requirements:
// - `b` MUST be a power of two (1, 2, 4, 8, 16, ...).
// - Behavior is undefined if `b` is not a power of two.
//
// Bit-level example (x = 13, b = 8):
//
//   x        = 13  =  0000 1101
//   b        =  8  =  0000 1000
//   b - 1    =  7  =  0000 0111
//
//   x + b - 1      =  0000 1101
//                    +0000 0111
//                    ----------
//                     0001 0100  (20)
//
//   ~(b - 1)       = ~0000 0111
//                    ----------
//                     1111 1000
//
//   (x + b - 1)
//   & ~(b - 1)     =  0001 0100
//                    &1111 1000
//                    ----------
//                     0001 0000  (16)
//
// Result: 16 (next multiple of 8)
//
// Notes:
// - Works for unsigned integers.
// - Beware of overflow if `x + b - 1` exceeds the integer type’s max value.
#define align_pow_2_m(x,b) (((x) + (b) - 1) & (~((b) - 1)))
#define pow_2_is_m(x)      (((x & ~x) == 0) && x)

#define max_m(a,b) (((a)>(b))?(a):(b))
#define min_m(a,b) (((a)<(b))?(a):(b))

#define abs_s32_m(v) (S64)abs(v)
#define abs_s64_m(v) (S64)llabs(v)
#define abs_f32_m(v) (F32)fabsf(v)
#define abs_f64_m(v) (F64)fabs(v)

typedef union Vec2_U32 Vec2_U32;
union Vec2_U32
{
	struct
	{
		U32 x;
		U32 y;
	};
	U32 v[2];
};

typedef union Vec3_U32 Vec3_U32;
union Vec3_U32
{
	struct
	{
		U32 x;
		U32 y;
		U32 z;
	};
	U32 v[3];
};

typedef union Vec2_F32 Vec2_F32;
union Vec2_F32
{
	struct
	{
		F32 x;
		F32 y;
	};
	F32 v[2];
};

typedef union Vec3_F32 Vec3_F32;
union Vec3_F32
{
	struct
	{
		F32 x;
		F32 y;
		F32 z;
	};
	F32 v[3];
};

typedef union Vec4_F32 Vec4_F32;
union Vec4_F32
{
	struct
	{
		F32 x;
		F32 y;
		F32 z;
		F32 w;
	};
	struct
	{
		Vec2_F32 xy;
		Vec2_F32 zw;
	};
	F32 v[4];
};

// -----------------------------------------------------------------------------
// Ranges
// -----------------------------------------------------------------------------

// All ranges are semi-open internals. For example, Range1_U8 { a, b } corresponds
// to [a, b) where a is included and b is excluded.

// 1-dimensional ranges

typedef union Range1_U8 Range1_U8;
union Range1_U8
{
	struct
	{
		U8 min;
		U8 max;
	};
	U8 v[2];
};

typedef union Range1_U16 Range1_U16;
union Range1_U16
{
	struct
	{
		U16 min;
		U16 max;
	};
	U16 v[2];
};

typedef union Range1_U32 Range1_U32;
union Range1_U32
{
	struct
	{
		U32 min;
		U32 max;
	};
	U32 v[2];
};

// -----------------------------------------------------------------------------
// Range operations
// -----------------------------------------------------------------------------

internal B32 Range1_U32__contains(Range1_U32 range, U32 x);

#endif // BASE_MATH_H
