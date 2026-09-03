#ifndef RISCV_EXTENSIONS_H
#define RISCV_EXTENSIONS_H

// RISC-V ISA extension parsing, modeled after bfd/elfxx-riscv.c (`riscv_parse_subset` and friends).
// Parses a `-march` string such as "rv64imfdc_zba_zbb" into the base XLEN plus the list of enabled extensions, and
// answers "is this extension enabled?" queries.

// An extension token extracted from the ISA string: its name (e.g. "i", "m", "zba") plus its version.
// A version of 0 means it was not specified in the string (e.g. "zba2p1" -> major 2, minor 1; "m" -> major 0, minor 0).
typedef struct RISCV_Extension RISCV_Extension;
struct RISCV_Extension
{
        String8 name;
        U8      major;
        U8      minor;
};

// Upper bound on the number of extensions in a single ISA string, including those added implicitly by the parser (g, e,
// i, zicsr, zifencei, zmmul, zaamo, zalrsc, ...).
#define RISCV_Extensions__max 64

// The parsed result of a `-march` string.
typedef struct RISCV_Extensions RISCV_Extensions;
struct RISCV_Extensions
{
        U64     count;
        U64     max;
        RISCV_Extension *data;
};

global const RISCV_Extension RISCV_Extension__defaults[];

typedef struct RISCV_Implicit_Extension RISCV_Implicit_Extension;
struct RISCV_Implicit_Extension
{
        String8  extension;
        // Comma-separated list of implied extensions.
        String8  implicit;
        // The rule fires only when `requires` is also present; empty means always.
        String8  requires;
        // Which XLEN the rule applies to: 0 = any, 32 = RV32 only, 64 = RV64 only.
        // Mirrors the `check_implicit_for_*` callbacks of bfd/elfxx-riscv.c
        // (e.g. `c` implies `zcf` only on RV32, `zcd` only when `d` is present).
        U8       xlen;
};

// Parse an ISA string. On success, fills `extensions` and returns an empty String8. On failure, returns a description
// of the problem (allocated in `arena`).
//
// Mirrors bfd/elfxx-riscv.c `riscv_parse_subset`.
internal String8
RISCV_Extensions__parse(Arena *arena, RISCV_Extensions *extensions, String8 string, U8 xlen);

// Find an extension entry by name (e.g. to inspect its version), or return 0.
internal const RISCV_Extension *
RISCV_Extensions__find(const RISCV_Extension *extension, U64 count, String8 name);

// Add the implicit extensions implied by the explicit ones.
//
// Mirrors bfd/elfxx-riscv.c `riscv_parse_add_implicit_subsets`.
internal void
RISCV_extensions_add_implicit(RISCV_Extensions *extensions, U8 xlen);

// Add a single extension at its canonical position, unless it is already
// present.  Keeps the list canonically sorted at all times.
internal void
RISCV_extensions_add(RISCV_Extensions *extensions, String8 name, U8 major, U8 minor);

// Check the parsed extensions against each other and the XLEN. Returns an empty String8 when there is no conflict, or a
// description of the first conflict otherwise.
//
// Mirrors bfd/elfxx-riscv.c `riscv_parse_check_conflicts`
internal String8
RISCV_extensions_check_conflicts(Arena *arena, RISCV_Extensions *extensions, U8 xlen);

// Apply a GNU-as-style `.option arch` change string to `extensions`.
//
//   "+c,+zbb1p0"   incrementally add extensions (optional NpM versions)
//   "rv64imafd"    absolute re-parse of a full ISA string
//
// The XLEN is never changed: `.option` operates on the live extension list
// only (the ELF class and ABI stay fixed at the declared target), so an
// absolute ISA string must match `xlen`.  Removal (`-ext`) is not supported:
// GNU as has deprecated it.
//
// Mirrors bfd/elfxx-riscv.c `riscv_update_subset`.
internal String8
RISCV_Extensions__update(Arena *arena, RISCV_Extensions *extensions, String8 string, U8 xlen);

// Whether the opcode class `class` is enabled by the parsed extensions.
//
// Mirrors bfd/elfxx-riscv.c `riscv_multi_subset_supports`.
internal B32
RISCV_extensions_supports_class(const RISCV_Extensions *extensions, OPC class);

#endif // RISCV_EXTENSIONS_H
