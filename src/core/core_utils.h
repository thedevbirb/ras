#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#define U32_ULEB128_encoding_size_max 5

internal U8 ULEB128__encoded_size(U32 source);
internal U8 ULEB128__from_U32(U32 source, U8 buffer[U32_ULEB128_encoding_size_max]);

// Memory-map the whole file for reading. Panics on failure. Overallocates by 8 bytes to allow not checking bounds always.
internal U8 * mmap_file(S32 file_descriptor, U64 file_in_size);

// Create/truncate the file to `size` bytes and memory-map it for writing. Panics on failure.
internal U8 * mmap_file_output(S32 file_descriptor, U64 size);

// Copy `text` into `out`, decoding GNU-as escape sequences. Returns the number of bytes written.
internal U32 bytes_escaped_fill(String8 text, U8 *out, U32 write_max);

#endif // CORE_UTILS_H



