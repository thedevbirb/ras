// -----------------------------------------------------------------------------
// Range operations
// -----------------------------------------------------------------------------

internal B32
Range1_U32__contains(Range1_U32 range, U32 x)
{
	B32 result = range.min <= x && x < range.max;
	return result;
}

