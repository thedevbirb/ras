internal Options
Options__default(void)
{
        Options result =
        {
                .xlen             = XLEN_64,
                .abi_xlen         = XLEN_64,
                .relax            = 1,
                .machine_abi      = String8__literal("lp64d"),

                .attributes =
                {
                        .architecture = String8__literal("rv64i"),
                }
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
