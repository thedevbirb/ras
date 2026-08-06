#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#define U32_ULEB128_encoding_size_max 5

internal U8 ULEB128__encoded_size(U32 source);
internal U8 ULEB128__from_U32(U32 source, U8 buffer[U32_ULEB128_encoding_size_max]);

#endif // CORE_UTILS_H



