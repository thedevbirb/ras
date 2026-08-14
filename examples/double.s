# D extension: double-precision floating-point, complete instruction set.
# ABI register names (ft0-fs1, fa0-fa7, fs2-fs11, ft8-ft11) and numeric (f0-f31).

# Loads / stores.
fld fa0, 0(a0)
fld f1, 8(sp)
fld ft7, 2047(t0)
fld f31, -2048(s1)
fld fa5, %lo(.LC1)(a5)
fsd fa1, 16(a1)
fsd f2, 24(s2)
fsd ft8, -1(t1)
fsd f30, 32(t2)

# Move between GPR and FPR (RV64 only).
fmv.x.d a0, fa0
fmv.d.x fa1, a1
fmv.x.d s0, fs0
fmv.d.x fs1, s1

# Move / negate / absolute value (sign-injection aliases).
fmv.d fa2, fa3
fneg.d fa4, fa5
fabs.d fa6, fa7
fmv.d ft0, ft0
fneg.d fs0, fs1
fabs.d f8, f9

# Sign injection.
fsgnj.d fa0, fa1, fa2
fsgnjn.d fa3, fa4, fa5
fsgnjx.d fa6, fa7, ft0
fsgnj.d f0, f1, f2
fsgnjn.d f3, f4, f5
fsgnjx.d f6, f7, f8

# Arithmetic (default rounding = dyn, and explicit rounding modes).
fadd.d fa0, fa1, fa2
fadd.d fa0, fa1, fa2, rne
fadd.d fa0, fa1, fa2, rtz
fadd.d fa0, fa1, fa2, rdn
fadd.d fa0, fa1, fa2, rup
fadd.d fa0, fa1, fa2, rmm
fadd.d fa0, fa1, fa2, dyn
fsub.d fa0, fa1, fa2
fsub.d fa0, fa1, fa2, rtz
fmul.d fa0, fa1, fa2
fmul.d fa0, fa1, fa2, rdn
fdiv.d fa0, fa1, fa2
fdiv.d fa0, fa1, fa2, rup
fmin.d fa0, fa1, fa2
fmax.d fa0, fa1, fa2
fmin.d f0, f1, f2
fmax.d f0, f1, f2

# Square root.
fsqrt.d fa0, fa1
fsqrt.d fa0, fa1, rtz
fsqrt.d f0, f1

# Fused multiply-add.
fmadd.d fa0, fa1, fa2, fa3
fmadd.d fa0, fa1, fa2, fa3, rne
fmadd.d fa0, fa1, fa2, fa3, rtz
fnmadd.d fa0, fa1, fa2, fa3
fnmadd.d fa0, fa1, fa2, fa3, rdn
fmsub.d fa0, fa1, fa2, fa3
fmsub.d fa0, fa1, fa2, fa3, rup
fnmsub.d fa0, fa1, fa2, fa3
fnmsub.d fa0, fa1, fa2, fa3, rmm
fmadd.d f0, f1, f2, f3
fnmadd.d f4, f5, f6, f7
fmsub.d f8, f9, f10, f11
fnmsub.d f12, f13, f14, f15

# Convert double -> int.
fcvt.w.d a0, fa0
fcvt.w.d a0, fa0, rtz
fcvt.wu.d a0, fa0
fcvt.wu.d a0, fa0, rdn
fcvt.l.d a0, fa0
fcvt.l.d a0, fa0, rup
fcvt.lu.d a0, fa0
fcvt.lu.d a0, fa0, rmm
fcvt.w.d x0, f0
fcvt.l.d x1, f1

# Convert int -> double.
fcvt.d.w fa0, a0
fcvt.d.wu fa0, a0
fcvt.d.l fa0, a0
fcvt.d.l fa0, a0, rne
fcvt.d.lu fa0, a0
fcvt.d.lu fa0, a0, rtz
fcvt.d.w f0, x0
fcvt.d.l f1, x1

# Convert between single and double precision.
fcvt.d.s fa0, fa1
fcvt.s.d fa0, fa1
fcvt.s.d fa0, fa1, rtz
fcvt.d.s f0, f1
fcvt.s.d f0, f1

# Classify.
fclass.d a0, fa0
fclass.d t0, ft1

# Compare.
feq.d a0, fa0, fa1
flt.d a0, fa0, fa1
fle.d a0, fa0, fa1
feq.d t0, f0, f1
flt.d t0, f0, f1
fle.d t0, f0, f1
# Swapped-operand aliases.
fgt.d a0, fa1, fa0
fge.d a0, fa1, fa0
fgt.d t0, f1, f0
fge.d t0, f1, f0

# Data for the %lo relocation above.
.data
.LC1:
        .word 1374389535
        .word 1074339512
