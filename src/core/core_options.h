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
        // The extensions parsed from `-march` (XLEN + enabled extensions).
        RISCV_Extensions extensions;

        // The architecture string derived from `-march` at parse time.
        //
        // It is cached because `.option arch` (and, indirectly, `.option rvc`) mutate `extensions` afterwards, and the
        // emitted `.riscv.attributes` architecture tag must keep the parse-time value.
        String8 architecture;

        // Intrusive SLL stack of `.option push` / `.option pop` snapshots
        Options *next;
};

internal Options
Options__default(Arena *arena);

// internal void
// Options__architecture_parse(Options *options, String8 architecture);

// Return the architecture string created from extensions and xlen.
//
// If `symbol` is true, returns the `$xrv32`-like string for .symtab
internal String8
architecture_string(const RISCV_Extensions *extensions, U8 xlen, Arena *arena, B32 symbol);

internal U32
ELF_Header_Flags__from_Options(Options *options);

#endif // CORE_OPTIONS_H
