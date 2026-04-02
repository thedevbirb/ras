# Comprehensive RISC-V assembly test file

.section .text
.global _start
.global main
.local helper
.local loop_body

.equ STACK_SIZE, 4 << 4
.equ UART_BASE, 0x10000000
.equ UART_TX, UART_BASE + 0x00
.equ UART_STATUS, UART_BASE + 0x04
.equ READY_BIT, 1 << 5
.equ MASK, 0xFF & ~(0x3 << 4)

_start:
    la sp, stack_top
    li gp, 0x10000800
    addi sp, sp, -STACK_SIZE
    call main
    li a7, 93
    # ecall

main:
    addi sp, sp, -32
    sw ra, 28(sp)
    sw s0, 24(sp)
    sw s1, 20(sp)
    addi s0, sp, 32

    lui a0, %hi(msg)
    addi a0, a0, %lo(msg)
    li a1, 0xff | (1 << 3)
    li a2, -~STACK_SIZE
    la t0, helper
    mv s1, zero

    beqz a0, 1f
    jal ra, helper
    mv s1, a0
1:
    addi a0, s1, 'A'
    li t1, 0xDEADBEEF
    srai t2, t1, 16
    andi t2, t2, MASK
    beq a0, a1, 2f
    bne a1, a2, 1f
    j 2f
1:
    addi a0, a0, 1
    bge a0, a1, 3f
    j 1b

2:
    add t2, t1, a0
    bnez t2, 3f
    j 1b

3:
    la a0, table
    lw t3, 0(a0)
    lw t4, 4(a0)
    add t5, t3, t4
    sw t5, 8(a0)

    li t0, 10
    mv t1, zero
4:
    # mul t2, t0, t0
    add t1, t1, t2
    addi t0, t0, -1
    bnez t0, 4b

    li t3, UART_BASE
5:
    lw t4, 4(t3)
    andi t4, t4, READY_BIT
    beqz t4, 5b
    li t5, 'O'
    sw t5, 0(t3)
5:
    lw t4, 4(t3)
    andi t4, t4, READY_BIT
    beqz t4, 5b
    li t5, 'K'
    sw t5, 0(t3)

    la a0, result_buf
    sw t1, 0(a0)

    mv a0, s1
    lw ra, 28(sp)
    lw s0, 24(sp)
    lw s1, 20(sp)
    addi sp, sp, 32
    ret

helper:
    addi sp, sp, -16
    sw ra, 12(sp)

    beqz a0, 1f
    la t0, lut
    andi t1, a0, 0xF
    slli t1, t1, 2
    add t0, t0, t1
    lw a0, 0(t0)
    j 2f
1:
    li a0, -1
2:
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

loop_body:
    slli t0, a0, 3
    add t0, t0, a1
    # ld t1, 0(t0)
    addi t1, t1, 1
    # sd t1, 0(t0)
    ret

.section .rodata
msg:
    .ascii "\x1B[1mStatus: "
    .asciz "ready\x1B[0m"
    .string "hello\n"
    .asciz "world\t\033[31mred\033[0m"
.equ MSG_LEN, . - msg

lut:
    .word 0, 1, 1, 2, 3, 5, 8, 13
    .word 21, 34, 55, 89, 144, 233, 377, 610

.section .data
.align 3
table:
    .word 100, 200, 0
    .dword 0xCAFEBABEDEADBEEF
    .half 0xBEEF, 0xCAFE, 0x1234
    .byte 0b1010, 0x2F, 077, 42, 'Z', '\n'
    .skip 5
    .zero 3

.align 2
values:
    .word STACK_SIZE
    .word MASK
    .word MSG_LEN
    .word UART_BASE + 0x100

result_buf:
    .skip 32

.section .bss
.comm buffer, 64, 8
.comm scratch, 256, 16

stack:
    .skip 4096
stack_top:
