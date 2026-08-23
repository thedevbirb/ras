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

The matching ABIs are supported (`ilp32`/`ilp32f`/`ilp32d` for RV32 and `lp64`/`lp64f`/`lp64d` for
RV64). Compressed instructions are still in progress.

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
compiler output using `rv64imafd_zba_zbb_zbc_zbs_zicond_zmmul`) is assembled by `ras` and compared
section-by-section against the object produced by GNU `as`.

As of the latest recorded run, the output is **byte-per-byte identical for the major sections**: 9
of the compared sections match exactly, and the `.text` section (nearly 890 KB of machine code) is
byte-for-byte equal with no differences. The only remaining differences are in the `.rela.*`
relocation sections (`.rela.text`, `.rela.data`, `.rela.rodata`, `.rela.srodata`), which have the
same size but differ in ordering of the symbol indexes.

### Reproducing the test

The following reproduces the parity check from scratch:
amalgamation.

1. Get the SQLite amalgamation. This project targets SQLite 3.53.4, Download
   `sqlite-amalgamation-3530400.zip` from `https://www.sqlite.org/` and unzip it:

   ```sh
   unzip sqlite-amalgamation-3530400.zip
   ```

2. Compile it to RISC-V assembly, with the bare-metal RISC-V toolchain (newlib), matching the
   flags used for the milestone run:

   ```sh
   riscv64-unknown-elf-gcc -S -O2 \
       -DSQLITE_OS_OTHER=1 -DSQLITE_THREADSAFE=0 \
       -march=rv64imafd_zba_zbb_zbc_zbs_zicond_zmmul -mabi=lp64d \
       sqlite3.c -o sqlite3.s
   ```

   (`-DSQLITE_OS_OTHER=1` and `-DSQLITE_THREADSAFE=0` skip the OS/pthread layers that newlib does
   not provide. The march deliberately omits `c`/`a`: `a` is not recognized by this assembler yet,
   and `c` is recognized but compressed instruction emission is still in progress.)

3. Assemble with both assemblers and compare the resulting relocatable objects section by
   section using the `compare_objects.py` script:

   ```sh
   ./build/ras sqlite3.s -o sqlite3_ras.o
   riscv64-unknown-elf-as -march=rv64imafd_zba_zbb_zbc_zbs_zicond_zmmul \
       -mabi=lp64d sqlite3.s -o sqlite3_gnu.o
   python3 compare_objects.py
   ```

   The script prints a per-section `OK`/`DIFF` listing and a `JSON:` summary of the identical and
   differing sections.

Yes, the script is obviously AI-generated. I know this is far from optimal, but creating a separate
C program that does object file equivalence isn't exactly small scope either, and I've decided to
defer it. For now, I've found this enough for finding obvious bugs, given the strong feedback loop
model that an agent can use.
