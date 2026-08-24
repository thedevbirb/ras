# `ras` - a from-scratch RISC-V assembler

`ras` is a RISC-V assembler written entirely in C, with zero dependencies other than `libc`.
It reads GNU syntax assembly and produces ELF relocatable object files (ELF32 and ELF64) that can
be ingested by a linker. The design of the assembler is greatly based on GNU `as` (also referred to
as `gas`), which has been an indispensable source of inspiration other than a source of truth for
how a production assembler works.

This is clearly alpha software, not thoroughly tested, made primarily for learning. Future
developments might change this.

## Supported architecture

Both major RISC-V base ISAs are supported:

- **RV32** (32-bit registers / XLEN)
- **RV64** (64-bit registers / XLEN)

The matching ABIs are supported (`ilp32`/`ilp32f`/`ilp32d` for RV32 and `lp64`/`lp64f`/`lp64d` for RV64).

### Extensions

`-march=` parses the full ISA string (`rv32`/`rv64` followed by the extension list; prefixed
extensions are separated by `_`) and gates instructions by extension. The recognized extensions
are:

- Base and standard: `I`, `E` (RV32E embedded), `M`, `F`, `D`, `C` (compressed), and the `G` group
  (expands to `I`/`M`/`A`/`F`/`D`/`Zicsr`/`Zifencei`).
- `Z`-extensions: `Zba`, `Zbb`, `Zbc`, `Zbs`, `Zicntr`, `Zicond`, `Zicsr`, `Zifencei`, `Zmmul`.

Implicit dependencies are applied automatically (`G` implies the standard group, `D` implies `F`,
`F` implies `Zicsr`, `M` implies `Zmmul`, ...). `Zicsr` is recognized for parsing but has no
instructions implemented yet; extensions such as `A`, `Q`, `V`, `H` are not recognized by
`-march`.

## Directives

The assembler recognizes the following directives (see `src/parser/parser_directive.h`):

- Sections: `.section`, `.text`, `.data`, `.bss`
- Symbols / binding: `.local`, `.weak`, `.comm`, `.common`, `.globl`, `.global`
- Data emission: `.byte`, `.half`, `.word`, `.dword`, `.ascii`, `.asciz`, `.string`, `.base64`
- Alignment: `.align`, `.p2align`, `.p2alignw`, `.p2alignl`, `.balign`, `.balignw`, `.balignl`
- Symbolic constants: `.equ`, `.set`, `.equiv`, `.eqv`
- Fill / padding: `.fill`, `.skip`, `.space`, `.zero`
- Misc: `.option`, `.type`, `.size`, `.file`, `.attribute`, `.ident`

## Building and running

Building `ras` is purposefully extremely simple: you just need a supported C compiler (`gcc` or
`clang` only). First, you build the [`nob`](https://github.com/tsoding/nob.h) build file, and then
you run it to build `ras`.

Example usage:

```sh
cc -o nob nob.c
./nob
./build/ras hello.s -o hello.o          # default: RV64, lp64d, full extension set
./build/ras -march=rv64im hello.s -o hello.o
./build/ras -march=rv32im -mabi=ilp32 hello.s -o hello.o
./build/ras -march=rv64imfd_zba_zbb_zbc_zbs_zicntr_zicond_zifencei_zmmul hello.s -o hello.o
./build/ras -fPIC -march=rv64im hello.s -o hello.o
readelf -h -S hello.o
```

The command line follows `usage: ras <filepath_in> -o <filepath_out>` and supports `-march=`,
`-mabi=`, `-fpic`/`-fPIC`/`-fno-pic`, `-o`, and `--help`.

## Feature and testing parity with GNU as

Testing this type of software is hard, and doing it well will for sure take more time than to write
this code. A practical approach I've taken to this for now is feature parity with GNU `as` on a
sufficiently large C codebase: the SQLite 3 amalgamation. The GCC-generated `sqlite3.s` (~3.7 MB of
compiler output using `rv64imfd_zba_zbb_zbc_zbs_zicond_zmmul`) is assembled by `ras` and compared
section-by-section against the object produced by GNU `as`.

As of the latest recorded run, the comparison is clean on every semantic check: the `.text`
section (nearly 890 KB of machine code) is byte-for-byte identical, all four `.rela.*` sections
match as sets of (offset, type, addend, referenced-symbol), and the `.symtab` matches as a
multiset of all 26,991 symbols. In practice, sections are equivalent modulo symbol indexes.
Similarly, the string tables may slightly differ just because `ras` doesn't support merging strings.

### Reproducing the test

The following reproduces the parity check from scratch, all from the repository root.

1. Get the SQLite amalgamation. This project targets SQLite 3.53.4. Download
   `sqlite-amalgamation-3530400.zip` from `https://www.sqlite.org/` and unzip it (a copy also
   lives in the repo under `sqlite-amalgamation-3530400/`):

   ```sh
   unzip sqlite-amalgamation-3530400.zip
   ```

2. Build `ras`

   ```sh
   ./nob --release
   ```

3. Compile the amalgamation to RISC-V assembly with the bare-metal RISC-V toolchain (newlib),
   matching the flags used for the milestone run:

   ```sh
   riscv64-unknown-elf-gcc -S -O2 \
       -DSQLITE_OS_OTHER=1 -DSQLITE_THREADSAFE=0 \
       -march=rv64imafd_zba_zbb_zbc_zbs_zicond_zmmul -mabi=lp64d \
       sqlite-amalgamation-3530400/sqlite3.c -o sqlite-amalgamation-3530400/sqlite3.s
   ```

   (`-DSQLITE_OS_OTHER=1` and `-DSQLITE_THREADSAFE=0` skip the OS/pthread layers that newlib does
   not provide.

   Note that you can also check for `riscv32` toolchain as well.

4. Assemble `sqlite3.s` with both assemblers:

   ```sh
   ./build/ras sqlite-amalgamation-3530400/sqlite3.s -o sqlite-amalgamation-3530400/sqlite3_ras.o
   riscv64-unknown-elf-as -march=rv64imafd_zba_zbb_zbc_zbs_zicond_zmmul \
       -mabi=lp64d sqlite-amalgamation-3530400/sqlite3.s -o sqlite-amalgamation-3530400/sqlite3_gnu.o
   ```

   By default, `as` should honor the architecture provided by `.attribute arch, "<arch>"` written by
   GCC, so the `-march` flag can be optional.

   Note that you can also check for `riscv32` toolchain as well.

5. Compare the two relocatable objects. `nob` builds `build/compare_objects` from
   `compare_objects.c` and runs it on the two objects:

   ```sh
   ./nob --compare-objects sqlite-amalgamation-3530400/sqlite3_ras.o sqlite-amalgamation-3530400/sqlite3_gnu.o
   ```

   The comparator prints a per-section `OK`/`DIFF` listing plus a `=== SUMMARY ===` block with the
   counts of identical and differing sections, the list of differences, and the first differing
   byte of `.text`. It takes any two relocatable ELF32 or ELF64 objects, exits `0` when they are
   equivalent, and non-zero when any difference is found.
