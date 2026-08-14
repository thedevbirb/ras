internal Options
Options__default(void)
{
        Options result =
        {
                .xlen             = XLEN_64,
                .abi_xlen         = XLEN_64,
                .relax            = 1,
                .machine_abi      = String8__literal("lp64d"),
                .elf_header_flags = EF_RISCV_FLOAT_ABI_DOUBLE,

                .attributes =
                {
                        .architecture = String8__literal("rv64i"),
                }
        };

        return result;
}

internal U32
ELF_Header_Flags__from_Options(Options *options)
{
        // TODO(low): make this configurable, but in practice this is a rv64i assembler for now.
        U32 flags = EF_RISCV_FLOAT_ABI_DOUBLE;
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
