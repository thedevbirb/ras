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

#define Register__invalid_number 0xFF

global const Register Register__invalid = { .name = String8__literal(""), .number = Register__invalid_number };

global const Register RISCV_registers[] =
{
        {String8__literal("x0"),   0}, {String8__literal("x1"),   1}, {String8__literal("x2"),   2}, {String8__literal("x3"),   3},
        {String8__literal("x4"),   4}, {String8__literal("x5"),   5}, {String8__literal("x6"),   6}, {String8__literal("x7"),   7},
        {String8__literal("x8"),   8}, {String8__literal("x9"),   9}, {String8__literal("x10"), 10}, {String8__literal("x11"), 11},
        {String8__literal("x12"), 12}, {String8__literal("x13"), 13}, {String8__literal("x14"), 14}, {String8__literal("x15"), 15},
        {String8__literal("x16"), 16}, {String8__literal("x17"), 17}, {String8__literal("x18"), 18}, {String8__literal("x19"), 19},
        {String8__literal("x20"), 20}, {String8__literal("x21"), 21}, {String8__literal("x22"), 22}, {String8__literal("x23"), 23},
        {String8__literal("x24"), 24}, {String8__literal("x25"), 25}, {String8__literal("x26"), 26}, {String8__literal("x27"), 27},
        {String8__literal("x28"), 28}, {String8__literal("x29"), 29}, {String8__literal("x30"), 30}, {String8__literal("x31"), 31},

        {String8__literal("zero"), 0},
        {String8__literal("ra"),   1},
        {String8__literal("sp"),   2},
        {String8__literal("gp"),   3},
        {String8__literal("tp"),   4},
        {String8__literal("fp"),   8},  // fp is alias for s0.

        {String8__literal("t0"),  5},  {String8__literal("t1"),  6},  {String8__literal("t2"),  7}, {String8__literal("t3"), 28},
        {String8__literal("t4"), 29},  {String8__literal("t5"), 30},  {String8__literal("t6"), 31},

        {String8__literal("s0"),  8},  {String8__literal("s1"),  9}, {String8__literal("s2"),  18},  {String8__literal("s3"),  19},
        {String8__literal("s4"), 20},  {String8__literal("s5"), 21}, {String8__literal("s6"),  22},  {String8__literal("s7"),  23},
        {String8__literal("s8"), 24},  {String8__literal("s9"), 25}, {String8__literal("s10"), 26},  {String8__literal("s11"), 27},

        {String8__literal("a0"), 10},  {String8__literal("a1"), 11},  {String8__literal("a2"), 12}, {String8__literal("a3"), 13},
        {String8__literal("a4"), 14},  {String8__literal("a5"), 15},  {String8__literal("a6"), 16}, {String8__literal("a7"), 17},
};

global const Register_List RISCV_register_list = { .data = RISCV_registers, .count = array_count_m(RISCV_registers) };

internal const Register *
Register_List__lookup(Register_List, String8);

#endif // RISCV_REGISTER_H

