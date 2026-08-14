#ifndef CORE_OPTIONS_H
#define CORE_OPTIONS_H

#define XLEN_32 32
#define XLEN_64 64

typedef struct Options Options;
struct Options
{
        // Register size, either 32-bit or 64
        U8  xlen;
        // Pointer size, either 32-bit or 64
        U8  abi_xlen;
        B32 compressed;
        // Limit the number of registers
        B32 embedded;
        B32 position_indipendent_code;
        B32 relax;

        String8 input_file;
        String8 output_file;

        String8 machine_abi;

        U32 elf_header_flags;

        RISCV_Attributes attributes;
};

internal Options
Options__default(void);

internal U32
ELF_Header_Flags__from_Options(Options *options);

#endif // CORE_OPTIONS_H

