internal Options
Options__default(Arena *arena)
{
        RISCV_Extension *extensions_data = Arena__push_array_m(arena, RISCV_Extension, RISCV_Extensions__max);
        RISCV_Extensions extensions =
        {
                .count = 0,
                .max   = RISCV_Extensions__max,
                .data  = extensions_data,
        };

        // Match GNU as's default ISA: the `g` group (i, m, a, f, d, zicsr, zifencei) plus the implicit dependencies
        // (zmmul from m, zaamo/zalrsc from a).
        RISCV_Extensions__parse(arena, &extensions, String8__literal("g"), XLEN_64);

        Options result =
        {
                .xlen             = XLEN_64,
                .abi_xlen         = XLEN_64,
                .relax            = 1,
                .machine_abi      = String8__literal("lp64d"),

                .extensions = extensions
        };
        result.elf_header_flags = ELF_Header_Flags__from_Options(&result);

        return result;
}

internal U32
ELF_Header_Flags__from_Options(Options *options)
{
        U32 flags = 0;
        if (String8__match_suffix(options->machine_abi, String8__literal("f")))
        {
                flags |= EF_RISCV_FLOAT_ABI_SINGLE;
        }
        else if (String8__match_suffix(options->machine_abi, String8__literal("d")))
        {
                flags |= EF_RISCV_FLOAT_ABI_DOUBLE;
        }
        if (options->compressed)
        {
                flags |= EF_RISCV_RVC;
        }
        if (options->embedded)
        {
                flags |= EF_RISCV_RVE;
        }

        return flags;
}


internal String8
Options__architecture_parse(Options *options, String8 architecture, Arena *arena)
{
        options->extensions.count = 0;
        String8 error = {0};

        String8 rv32 = String8__literal("rv32");
        String8 rv64 = String8__literal("rv64");

        // Must begin with rv32 or rv64.
        if (String8__match_prefix(architecture, rv32))
        {
                options->xlen = 32;
        }
        else if (String8__match_prefix(architecture, rv64))
        {
                options->xlen = 64;
        }
        else
        {
                error = error.count ? error : String8__format(arena, "ISA string `%.*s' must begin with rv32 or rv64", String8__varg(architecture));
        }

        String8 extensions = String8__skip(architecture, rv32.count);
        String8 error_parse = RISCV_Extensions__parse(arena, &options->extensions, extensions, options->xlen);
        error = error.count ? error : error_parse;

        options->embedded   = !!RISCV_Extensions__find(options->extensions.data, options->extensions.count, String8__literal("e"));
        options->compressed = !!RISCV_Extensions__find(options->extensions.data, options->extensions.count, String8__literal("c"));

        return error;
}

internal String8
architecture_string(const RISCV_Extensions *extensions, U8 xlen, Arena *arena, B32 symbol)
{
        // Compute total char count first, skipping the base `i` when `e` is
        // present (RV32E), mirroring GNU as's `riscv_arch_str1`.

        // [$x]rv32/rv64
        U64 count = (!!symbol * 2) + 4;
        U64 emitted_count = 0;
        U64 count_index = 0;
        B32 count_previous_is_e = 0;
        for (;;)
        {
                if (count_index >= extensions->count)
                {
                        break;
                }

                RISCV_Extension extension = extensions->data[count_index];
                B32 is_i = String8__match_exact(extension.name, String8__literal("i"));
                B32 skip = count_previous_is_e && is_i;
                if (!skip)
                {
                        count += extension.name.count;
                        // <major>p<minor>
                        count += 3;
                        emitted_count += 1;
                }
                count_previous_is_e = String8__match_exact(extension.name, String8__literal("e"));

                count_index += 1;
        }
        B32 add_underscores = emitted_count > 0;
        if (add_underscores)
        {
                count += emitted_count - 1;
        }

        // Null-terminated buffer
        U8 *data = Arena__push_array_m(arena, U8, count + 1);
        String8 result = String8__new(data, count);
        String8 cursor = result;

        if (symbol)
        {
                String8__serial_write(&cursor, (U8 *)"$x", 2);
        }

        String8__serial_write(&cursor, (U8 *)"rv", 2);
        const char *xlen_cstring = xlen == 64 ? "64" : "32";
        String8__serial_write(&cursor, (U8 *)xlen_cstring, 2);

        U8 p          = 'p';
        U8 underscore = '_';

        U64 write_index = 0;
        B32 wrote_any = 0;
        B32 write_previous_is_e = 0;
        for (;;)
        {
                if (write_index >= extensions->count)
                {
                        break;
                }

                RISCV_Extension extension = extensions->data[write_index];
                B32 is_i = String8__match_exact(extension.name, String8__literal("i"));
                B32 skip = write_previous_is_e && is_i;
                if (!skip)
                {
                        if (wrote_any)
                        {
                                String8__serial_write_m(&cursor, &underscore);
                        }
                        String8__serial_write(&cursor, extension.name.data, extension.name.count);

                        U8 major_character = extension.major + '0';
                        U8 minor_character = extension.minor + '0';

                        String8__serial_write_m(&cursor, &major_character);
                        String8__serial_write_m(&cursor, &p);
                        String8__serial_write_m(&cursor, &minor_character);
                        wrote_any = 1;
                }
                write_previous_is_e = String8__match_exact(extension.name, String8__literal("e"));

                write_index += 1;
        }

        return result;
}
