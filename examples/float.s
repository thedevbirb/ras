# F extension: single-precision floating-point, complete instruction set.
# ABI register names (ft0-fs1, fa0-fa7, fs2-fs11, ft8-ft11) and numeric (f0-f31).

# Loads / stores.
flw fa0, 0(a0)
flw f1, 4(sp)
flw ft7, 2047(t0)
flw f31, -2048(s1)
flw fa5, %lo(.LC0)(a5)
fsw fa1, 8(a1)
fsw f2, 12(s2)
fsw ft8, -1(t1)
fsw f30, 16(t2)

# Move between GPR and FPR.
fmv.x.w a0, fa0
fmv.x.s t0, ft1
fmv.w.x fa1, a1
fmv.s.x ft2, t2
fmv.x.w s0, fs0
fmv.w.x fs1, s1

# Move / negate / absolute value (sign-injection aliases).
fmv.s fa2, fa3
fneg.s fa4, fa5
fabs.s fa6, fa7
fmv.s ft0, ft0
fneg.s fs0, fs1
fabs.s f8, f9

# Sign injection.
fsgnj.s fa0, fa1, fa2
fsgnjn.s fa3, fa4, fa5
fsgnjx.s fa6, fa7, ft0
fsgnj.s f0, f1, f2
fsgnjn.s f3, f4, f5
fsgnjx.s f6, f7, f8

# Arithmetic (default rounding = dyn, and explicit rounding modes).
fadd.s fa0, fa1, fa2
fadd.s fa0, fa1, fa2, rne
fadd.s fa0, fa1, fa2, rtz
fadd.s fa0, fa1, fa2, rdn
fadd.s fa0, fa1, fa2, rup
fadd.s fa0, fa1, fa2, rmm
fadd.s fa0, fa1, fa2, dyn
fsub.s fa0, fa1, fa2
fsub.s fa0, fa1, fa2, rtz
fmul.s fa0, fa1, fa2
fmul.s fa0, fa1, fa2, rdn
fdiv.s fa0, fa1, fa2
fdiv.s fa0, fa1, fa2, rup
fmin.s fa0, fa1, fa2
fmax.s fa0, fa1, fa2
fmin.s f0, f1, f2
fmax.s f0, f1, f2

# Square root.
fsqrt.s fa0, fa1
fsqrt.s fa0, fa1, rtz
fsqrt.s f0, f1

# Fused multiply-add.
fmadd.s fa0, fa1, fa2, fa3
fmadd.s fa0, fa1, fa2, fa3, rne
fmadd.s fa0, fa1, fa2, fa3, rtz
fnmadd.s fa0, fa1, fa2, fa3
fnmadd.s fa0, fa1, fa2, fa3, rdn
fmsub.s fa0, fa1, fa2, fa3
fmsub.s fa0, fa1, fa2, fa3, rup
fnmsub.s fa0, fa1, fa2, fa3
fnmsub.s fa0, fa1, fa2, fa3, rmm
fmadd.s f0, f1, f2, f3
fnmadd.s f4, f5, f6, f7
fmsub.s f8, f9, f10, f11
fnmsub.s f12, f13, f14, f15

# Convert float -> int.
fcvt.w.s a0, fa0
fcvt.w.s a0, fa0, rtz
fcvt.wu.s a0, fa0
fcvt.wu.s a0, fa0, rdn
fcvt.l.s a0, fa0
fcvt.l.s a0, fa0, rup
fcvt.lu.s a0, fa0
fcvt.lu.s a0, fa0, rmm
fcvt.w.s x0, f0
fcvt.l.s x1, f1

# Convert int -> float.
fcvt.s.w fa0, a0
fcvt.s.w fa0, a0, rne
fcvt.s.wu fa0, a0
fcvt.s.wu fa0, a0, rtz
fcvt.s.l fa0, a0
fcvt.s.l fa0, a0, rdn
fcvt.s.lu fa0, a0
fcvt.s.lu fa0, a0, rup
fcvt.s.w f0, x0
fcvt.s.l f1, x1

# Classify.
fclass.s a0, fa0
fclass.s t0, ft1

# Compare.
feq.s a0, fa0, fa1
flt.s a0, fa0, fa1
fle.s a0, fa0, fa1
feq.s t0, f0, f1
flt.s t0, f0, f1
fle.s t0, f0, f1
# Swapped-operand aliases.
fgt.s a0, fa1, fa0
fge.s a0, fa1, fa0
fgt.s t0, f1, f0
fge.s t0, f1, f0

# Data for the %lo relocation above.
.data
.LC0:
        .word 1374389535
