# `ras` - a from-scratch RISC-V assembler

`ras` is a RISC-V 64-bit assembler written entirely in C, with zero dependencies other than `libc`.
It reads GNU syntax assembly and produces ELF64 relocatable object files that can be ingested by a
linker. The assembler has limited instruction set support (only base `I` extension, at the time of
writing), although it is designed with some friendliness towards supporting multiple extensions.

The design of the assembler is greatly based on GNU `as`, referred also to as `gas`, which has been
an indispendable source of inspiration other than a source of truth for how a production assembler
works.

This is clearly alpha software, not throughly tested, made primary for learning. Future developments
might change this.

## Building and running

Building `ras` is purposefully extremely simple: you just need a supported C compiler (`gcc` or
`clang` only). First, you build the [`nob`](https://github.com/tsoding/nob.h) build file, and then
you run it to build `ras`.

Example usage:

```sh
cc -o nob nob.c
./nob
./build/ras hello.s -o hello.o
readelf -h -S hello.o
```
