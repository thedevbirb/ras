#ifndef RISCV_REGISTER_H
#define RISCV_REGISTER_H

typedef struct Register Register;
struct Register
{
        String8 name;
        U8 number;
};

typedef struct Register_List Register_List;
struct Register_List
{
        const Register *data;
        U64 count;
};

global const Register RISCV_registers[] =
{
        {String8__literal("zero"), 0},
        {String8__literal("ra"),   1},
        {String8__literal("sp"),   2},
        {String8__literal("gp"),   3},
        {String8__literal("t0"),   5},
        {String8__literal("t1"),   6},
        {String8__literal("t2"),   7},
        {String8__literal("tp"),   4},
        {String8__literal("fp"),   8},  /* fp is alias for s0. */
        {String8__literal("s0"),   8},
        {String8__literal("s1"),   9},
        {String8__literal("a0"),  10},
        {String8__literal("a1"),  11},
        {String8__literal("a2"),  12},
        {String8__literal("a3"),  13},
        {String8__literal("a4"),  14},
        {String8__literal("a5"),  15},


        {String8__literal("a6"),  16},
        {String8__literal("a7"),  17},
        {String8__literal("s2"),  18},
        {String8__literal("s3"),  19},
        {String8__literal("s4"),  20},
        {String8__literal("s5"),  21},
        {String8__literal("s6"),  22},
        {String8__literal("s7"),  23},
        {String8__literal("s8"),  24},
        {String8__literal("s9"),  25},
        {String8__literal("s10"), 26},
        {String8__literal("s11"), 27},
        {String8__literal("t3"),  28},
        {String8__literal("t4"),  29},
        {String8__literal("t5"),  30},
        {String8__literal("t6"),  31},

        {String8__literal("x0"),   0}, {String8__literal("x1"),   1}, {String8__literal("x2"),   2}, {String8__literal("x3"),   3},
        {String8__literal("x4"),   4}, {String8__literal("x5"),   5}, {String8__literal("x6"),   6}, {String8__literal("x7"),   7},
        {String8__literal("x8"),   8}, {String8__literal("x9"),   9}, {String8__literal("x10"), 10}, {String8__literal("x11"), 11},
        {String8__literal("x12"), 12}, {String8__literal("x13"), 13}, {String8__literal("x14"), 14}, {String8__literal("x15"), 15},

        {String8__literal("x16"), 16}, {String8__literal("x17"), 17}, {String8__literal("x18"), 18}, {String8__literal("x19"), 19},
        {String8__literal("x20"), 20}, {String8__literal("x21"), 21}, {String8__literal("x22"), 22}, {String8__literal("x23"), 23},
        {String8__literal("x24"), 24}, {String8__literal("x25"), 25}, {String8__literal("x26"), 26}, {String8__literal("x27"), 27},
        {String8__literal("x28"), 28}, {String8__literal("x29"), 29}, {String8__literal("x30"), 30}, {String8__literal("x31"), 31},
};

global const Register_List RISCV_register_list = { .data = RISCV_registers, .count = array_count_m(RISCV_registers) };

// Floating-point registers (F/D extensions). ABI names follow the RISC-V psABI:
// f0-f7 = ft0-ft7, f8-f9 = fs0-fs1, f10-f17 = fa0-fa7, f18-f27 = fs2-fs11, f28-f31 = ft8-ft11.
global const Register RISCV_fp_registers[] =
{
        {String8__literal("ft0"),   0},
        {String8__literal("ft1"),   1},
        {String8__literal("ft2"),   2},
        {String8__literal("ft3"),   3},
        {String8__literal("ft4"),   4},
        {String8__literal("ft5"),   5},
        {String8__literal("ft6"),   6},
        {String8__literal("ft7"),   7},
        {String8__literal("fs0"),   8},
        {String8__literal("fs1"),   9},
        {String8__literal("fa0"),  10},
        {String8__literal("fa1"),  11},
        {String8__literal("fa2"),  12},
        {String8__literal("fa3"),  13},
        {String8__literal("fa4"),  14},
        {String8__literal("fa5"),  15},
        {String8__literal("fa6"),  16},
        {String8__literal("fa7"),  17},
        {String8__literal("fs2"),  18},
        {String8__literal("fs3"),  19},
        {String8__literal("fs4"),  20},
        {String8__literal("fs5"),  21},
        {String8__literal("fs6"),  22},
        {String8__literal("fs7"),  23},
        {String8__literal("fs8"),  24},
        {String8__literal("fs9"),  25},
        {String8__literal("fs10"), 26},
        {String8__literal("fs11"), 27},
        {String8__literal("ft8"),  28},
        {String8__literal("ft9"),  29},
        {String8__literal("ft10"), 30},
        {String8__literal("ft11"), 31},

        {String8__literal("f0"),   0},
        {String8__literal("f1"),   1},
        {String8__literal("f2"),   2},
        {String8__literal("f3"),   3},
        {String8__literal("f4"),   4},
        {String8__literal("f5"),   5},
        {String8__literal("f6"),   6},
        {String8__literal("f7"),   7},
        {String8__literal("f8"),   8},
        {String8__literal("f9"),   9},
        {String8__literal("f10"), 10},
        {String8__literal("f11"), 11},
        {String8__literal("f12"), 12},
        {String8__literal("f13"), 13},
        {String8__literal("f14"), 14},
        {String8__literal("f15"), 15},
        {String8__literal("f16"), 16},
        {String8__literal("f17"), 17},
        {String8__literal("f18"), 18},
        {String8__literal("f19"), 19},
        {String8__literal("f20"), 20},
        {String8__literal("f21"), 21},
        {String8__literal("f22"), 22},
        {String8__literal("f23"), 23},
        {String8__literal("f24"), 24},
        {String8__literal("f25"), 25},
        {String8__literal("f26"), 26},
        {String8__literal("f27"), 27},
        {String8__literal("f28"), 28},
        {String8__literal("f29"), 29},
        {String8__literal("f30"), 30},
        {String8__literal("f31"), 31},
};

global const Register_List RISCV_fp_register_list = { .data = RISCV_fp_registers, .count = array_count_m(RISCV_fp_registers) };

// Control and status registers (Zicsr extension).
//
// A subset of the RISC-V privileged CSR namespace: the user-, supervisor- and machine-level registers that show up in
// boot code, trap handlers and benchmarking loops, plus the F-extension floating-point CSRs.
typedef struct Register_CS Register_CS;
struct Register_CS
{
        String8 name;
        U16     address;
};

typedef struct Register_CS_List Register_CS_List;
struct Register_CS_List
{
        const Register_CS *data;
        U64                 count;
};

global const Register_CS RISCV_csr_registers[] =
{
        // User / counter CSRs.
        {String8__literal("cycle"),    0xc00},
        {String8__literal("time"),     0xc01},
        {String8__literal("instret"),  0xc02},

        {String8__literal("mcycle"),   0xb00},
        {String8__literal("minstret"), 0xb02},

        // Machine information (read-only).
        {String8__literal("mvendorid"), 0xf11},
        {String8__literal("marchid"),   0xf12},
        {String8__literal("mimpid"),    0xf13},
        {String8__literal("mhartid"),   0xf14},

        // Machine trap setup and handling.
        {String8__literal("mstatus"),       0x300},
        {String8__literal("misa"),          0x301},
        {String8__literal("medeleg"),       0x302},
        {String8__literal("mideleg"),       0x303},
        {String8__literal("mie"),           0x304},
        {String8__literal("mtvec"),         0x305},
        {String8__literal("mcounteren"),    0x306},
        {String8__literal("menvcfg"),       0x30a},
        {String8__literal("mcountinhibit"), 0x320},
        {String8__literal("mscratch"),      0x340},
        {String8__literal("mepc"),          0x341},
        {String8__literal("mcause"),        0x342},
        {String8__literal("mtval"),         0x343},
        {String8__literal("mip"),           0x344},

        // Supervisor trap setup and handling.
        {String8__literal("sstatus"),  0x100},
        {String8__literal("sie"),      0x104},
        {String8__literal("stvec"),    0x105},
        {String8__literal("senvcfg"),  0x10a},
        {String8__literal("sscratch"), 0x140},
        {String8__literal("sepc"),     0x141},
        {String8__literal("scause"),   0x142},
        {String8__literal("stval"),    0x143},
        {String8__literal("sip"),      0x144},
        {String8__literal("satp"),     0x180},

        // F-extension floating-point CSRs.
        {String8__literal("fflags"), 0x001},
        {String8__literal("frm"),    0x002},
        {String8__literal("fcsr"),   0x003},
};

global const Register_CS_List RISCV_csr_register_list = { .data = RISCV_csr_registers, .count = array_count_m(RISCV_csr_registers) };

internal const Register *
Register_List__lookup(Register_List, String8, B32 e_extension_enabled);

internal const Register_CS *
Register_CS_List__lookup(Register_CS_List, String8);

#endif // RISCV_REGISTER_H

