// TODO(refactor): this is a bin of standalone utils I don't know where to put. I don't like utils files in general.

// Very dumb, but that's it.
internal void
ULEB128__from_U32(U32 source, U8 buffer[U32_ULEB128_encoding_size])
{
        U64 result = source;
        memory_copy(buffer, (U8 *)&result, U32_ULEB128_encoding_size);
        return;
}

// GNU as supports these escape sequences inside string literals:
// - \\ — backslash
// - \" — double quote
// - \n — newline (0x0A)
// - \t — tab (0x09)
// - \r — carriage return (0x0D)
// - \0 — null (0x00)
// - \a — bell (0x07)
// - \b — backspace (0x08)
// - \f — form feed (0x0C)
// - \NNN — octal value (1–3 octal digits)
// - \xNN — hex value (1–2 hex digits)
//

internal B32
U8_ascii_escape_sequence_start_is(U8 character)
{
	B32 result =
		character == '\\' ||
		character == '"'  ||
		character == 'n'  ||
		character == 't'  ||
		character == 'r'  ||
		character == '0'  ||
		character == 'a'  ||
		character == 'b'  ||
		character == 'f'  ||
		character == 'N'  ||
		character == 'x';
	return result;
}

internal B32
U8_ascii_digit_is(U8 character)
{
	B32 ascii_digit_is = character >= '0' && character <= '9';
	return ascii_digit_is;
}



// Panics on failure. Overallocates by 8 bytes to allow not checking always bounds.
internal U8 *
mmap_file(S32 file_descriptor, U64 file_in_size)
{
        U8 *result = mmap(NULL, file_in_size + 8, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
        assert_always_m(result != MAP_FAILED && "failed to mmap file contents");
        return result;
}

internal U8 *
mmap_file_output(S32 file_descriptor, U64 size)
{
        // Add file size metadata
        S32 ftruncate_result = ftruncate(file_descriptor, size);
        assert_always_m(ftruncate_result == 0);
        U8 *result = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
        assert_always_m(result != MAP_FAILED && "failed to mmap output file");
        return result;
}

internal void
bytes_escaped_fill(String8 text, U8 *out, U32 write_max)
{
        U32  index         = 0;
        U32  bytes_written = 0;
        U8  *data = text.data;

        for (;;)
        {
                B32 break_should = bytes_written >= write_max;
                if (break_should)
                {
                        break;
                }

                U8 byte = 0;

                if (data[index] == '\\')
                {
                        index += 1;
                        if (data[index] == 'x')
                        {
                                // E.g. \x1a
                                //       ^--- cursor is here
                                index += 1;
                                // E.g. \x1a
                                //        ^--- cursor is here
                                byte = data[index];
                                index += 1;
                                U8 value = hex_table[data[index]];
                                if (value != hex_table_invalid)
                                {
                                        // E.g. \x1a
                                        //         ^--- cursor is here
                                        byte = byte * 16 + data[index];
                                        index += 1;
                                }
                        }
                        else if (U8__octal_prefix(data[index]))
                        {
                                // E.g. \377
                                //       ^--- cursor is here
                                byte = data[index];
                                index += 1;
                                if (U8__octal(data[index]))
                                {
                                        // E.g. \377
                                        //        ^--- cursor is here
                                        byte = byte * 8 + data[index];
                                        index += 1;
                                }
                                if (U8__octal(data[index]))
                                {
                                        // E.g. \377
                                        //         ^--- cursor is here
                                        byte = byte * 8 + data[index];
                                        index += 1;
                                }
                        }
                        else
                        {
                                byte = escape_table[data[index]];
                                assert_always_m(byte != escape_value_invalid);
                                index += 2;
                        }
                }
                else
                {
                        byte = data[index];
                        index += 1;
                }


                out[bytes_written] = byte;
                bytes_written += 1;
        }

        return;
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

internal U64
alignment_distance(U64 address, U32 alignment)
{
        B32 power_of_two = ((alignment & ~alignment) == 0) && alignment;
        assert_m(power_of_two);

        U64 address_aligned = align_pow_2_m(address, alignment);
        assert_m(address_aligned >= address);

        U64 result = address_aligned - address;
        return result;
}
