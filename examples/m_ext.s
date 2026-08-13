# M extension: the complete RV64M instruction set.
#
# RV64M adds multiplication and division:
#   - 64-bit: mul, mulh, mulhsu, mulhu, div, divu, rem, remu
#   - 32-bit (word) variants: mulw, divw, divuw, remw, remuw

mul a0, a1, a2
mulh x3, x4, x5
mulhsu s0, s1, s2
mulhu s3, s4, s5
div t0, t1, t2
divu t3, t4, t5
rem t6, x0, x1
remu x31, x30, x29

mulw a0, a1, a2
divw a3, a4, a5
divuw a6, a7, t0
remw t1, t2, t3
remuw t4, t5, t6

# Register arguments.
mul a0, s1, t2
divu x0, x1, x31
mulhsu s1, s2, s3
remuw t6, t5, t4

# Edge registers.
mul x0, x0, x0
mul x31, x31, x31
