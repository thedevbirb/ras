# comment
.section .text
.global _start
.local helper
.equ SIZE, 4 << 2

_start:
    lui a0, %hi(msg)
    addi a0, a0, %lo(msg)
    li a1, 0xff | (1 << 3)
    li a2, -~SIZE
    la t0, helper
    beqz a0, 1f
    j _start
1:
    addi a0, a0, +'A'
helper:
    .byte 0b1010, 0x2F, 077, 42
    .half . - _start
    ret

.section .rodata
msg:
    .string "hello\n"
    .asciz "world"

.section .data
.align 3
table:
    .word msg, _start
    .dword 0xDEAD_BEEF
    .skip 16
    .zero 8

.section .bss
.comm buffer, 64, 8
