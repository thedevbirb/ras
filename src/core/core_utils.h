#ifndef CORE_UTILS_H
#define CORE_UTILS_H

// TODO(refactor): this is a bin of standalone utils I don't know where to put. I don't like utils files in general.

#define shift_right_mask_m(x, shift, bits)  (((U64)(x) >> (shift)) & ((1ULL << (bits)) - 1))

internal void
cursor_write(U8 **cursor, U8 *source, U64 size);

#define cursor_write_struct_m(cursor, source) cursor_write((cursor), (U8 *)(source), sizeof((source)))

#endif // CORE_UTILS_H



