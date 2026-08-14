# Zifencei, Zicntr, Zicond, Zba, Zbc, Zbs, Zbb.
# All instructions, ABI + numeric register names, immediate forms.

# --- Zifencei ---
fence.i

# --- Zicntr (pseudo-ops) ---
rdcycle a0
rdtime t0
rdinstret a1
rdcycle x1
rdtime x2

# --- Zicond ---
czero.eqz a0, a1, a2
czero.nez t0, t1, t2
czero.eqz x0, x1, x2
czero.nez x3, x4, x5

# --- Zba ---
sh1add a0, a1, a2
sh2add t0, t1, t2
sh3add s0, s1, s2
sh1add.uw a0, a1, a2
sh2add.uw t0, t1, t2
sh3add.uw s0, s1, s2
sh1add x0, x1, x2
sh3add.uw x3, x4, x5

# --- Zbc ---
clmul a0, a1, a2
clmulh t0, t1, t2
clmulr s0, s1, s2
clmul x0, x1, x2
clmulh x3, x4, x5
clmulr x6, x7, x8

# --- Zbs ---
bclr a0, a1, a2
bclri a0, a1, 5
bclr a0, a1, 5
bset t0, t1, t2
bseti t0, t1, 63
bset t0, t1, 63
binv s0, s1, s2
binvi s0, s1, 0
binv s0, s1, 0
bext a2, a3, a4
bexti a2, a3, 31
bext a2, a3, 31
bclri x1, x2, 1
bseti x3, x4, 62
binvi x5, x6, 33
bexti x7, x8, 12

# --- Zbb ---
clz a0, a1
ctz t0, t1
cpop s0, s1
clzw a0, a1
ctzw t0, t1
cpopw s0, s1
clz x1, x2
ctzw x3, x4

min a0, a1, a2
minu t0, t1, t2
max s0, s1, s2
maxu a3, a4, a5
min x1, x2, x3
maxu x4, x5, x6

sext.b a0, a1
sext.h t0, t1
zext.h s0, s1
sext.b x1, x2
zext.h x3, x4

andn a0, a1, a2
orn t0, t1, t2
xnor s0, s1, s2
andn x1, x2, x3
orn x4, x5, x6
xnor x7, x8, x9

rol a0, a1, a2
ror t0, t1, t2
rori s0, s1, 40
ror t0, t1, 40
rolw a0, a1, a2
rorw t0, t1, t2
roriw s0, s1, 20
rorw t0, t1, 20
rol x1, x2, x3
roriw x4, x5, 5

rev8 a0, a1
rev8 x1, x2
orc.b t0, t1
orc.b x3, x4
