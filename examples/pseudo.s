.text
    .globl _start
_start:
    addi a0, zero, 42
    li   t0, 7
    li   t1, 100000
    jal  ra, target
    add  a1, t0, t1
target:
    ret

    .data
msg:
    .asciz "hello"
    .word  0xdeadbeef

    .text
back_in_text:
    call target
