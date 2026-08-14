.text
.globl _start
_start:
    # VALID: normalize to -2048, the 12-bit lower bound
    addi x1, x0, 0xfffff800
    # VALID: normalize to -256
    addi x1, x0, 0xffffff00
    # VALID: normalize to -1
    addi x1, x0, 0xffffffff
    # VALID: already fits without normalization
    addi x1, x0, 0x7ff
    addi x1, x0, -1
    addi x1, x0, -2048
    # VALID: load/store offsets normalize the same way
    lw   x2, 0xfffff800(x3)
    lw   x2, 0xffffff00(x3)
    sw   x4, 0xfffff800(x5)
    sw   x4, 0xffffff00(x5)

    # INVALID: normalize to -4096, still does not fit 12 bits (rejected)
    # addi x1, x0, 0xfffff000
    # INVALID: normalize to -2147483648, does not fit (rejected)
    # addi x1, x0, 0x80000000
    # INVALID: 0x7fffffff (2147483647) stays positive, does not fit (rejected)
    # addi x1, x0, 0x7fffffff
    # INVALID: 0xfff / 0x800 are out of the signed 12-bit range (rejected)
    # addi x1, x0, 0xfff
    # addi x1, x0, 0x800
    # addi x1, x0, -2049
    # lb   x6, 0xfff(x7)
