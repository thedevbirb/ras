#ifndef RISCV_H
#define RISCV_H

// @section integer registers (x0-x31)

#define REG_ZERO  0   // x0  - hard-wired zero
#define REG_RA    1   // x1  - return address
#define REG_SP    2   // x2  - stack pointer
#define REG_GP    3   // x3  - global pointer
#define REG_TP    4   // x4  - thread pointer
#define REG_T0    5   // x5  - temporary
#define REG_T1    6   // x6  - temporary
#define REG_T2    7   // x7  - temporary
#define REG_S0    8   // x8  - saved register / frame pointer
#define REG_FP    8   // x8  - frame pointer (alias for s0)
#define REG_S1    9   // x9  - saved register
#define REG_A0    10  // x10 - function argument / return value
#define REG_A1    11  // x11 - function argument / return value
#define REG_A2    12  // x12 - function argument
#define REG_A3    13  // x13 - function argument
#define REG_A4    14  // x14 - function argument
#define REG_A5    15  // x15 - function argument
#define REG_A6    16  // x16 - function argument
#define REG_A7    17  // x17 - function argument
#define REG_S2    18  // x18 - saved register
#define REG_S3    19  // x19 - saved register
#define REG_S4    20  // x20 - saved register
#define REG_S5    21  // x21 - saved register
#define REG_S6    22  // x22 - saved register
#define REG_S7    23  // x23 - saved register
#define REG_S8    24  // x24 - saved register
#define REG_S9    25  // x25 - saved register
#define REG_S10   26  // x26 - saved register
#define REG_S11   27  // x27 - saved register
#define REG_T3    28  // x28 - temporary
#define REG_T4    29  // x29 - temporary
#define REG_T5    30  // x30 - temporary
#define REG_T6    31  // x31 - temporary

// @section floating-point registers (f0-f31)

#define REG_FT0   0   // f0  - temporary
#define REG_FT1   1   // f1  - temporary
#define REG_FT2   2   // f2  - temporary
#define REG_FT3   3   // f3  - temporary
#define REG_FT4   4   // f4  - temporary
#define REG_FT5   5   // f5  - temporary
#define REG_FT6   6   // f6  - temporary
#define REG_FT7   7   // f7  - temporary
#define REG_FS0   8   // f8  - saved register
#define REG_FS1   9   // f9  - saved register
#define REG_FA0   10  // f10 - function argument / return value
#define REG_FA1   11  // f11 - function argument / return value
#define REG_FA2   12  // f12 - function argument
#define REG_FA3   13  // f13 - function argument
#define REG_FA4   14  // f14 - function argument
#define REG_FA5   15  // f15 - function argument
#define REG_FA6   16  // f16 - function argument
#define REG_FA7   17  // f17 - function argument
#define REG_FS2   18  // f18 - saved register
#define REG_FS3   19  // f19 - saved register
#define REG_FS4   20  // f20 - saved register
#define REG_FS5   21  // f21 - saved register
#define REG_FS6   22  // f22 - saved register
#define REG_FS7   23  // f23 - saved register
#define REG_FS8   24  // f24 - saved register
#define REG_FS9   25  // f25 - saved register
#define REG_FS10  26  // f26 - saved register
#define REG_FS11  27  // f27 - saved register
#define REG_FT8   28  // f28 - temporary
#define REG_FT9   29  // f29 - temporary
#define REG_FT10  30  // f30 - temporary
#define REG_FT11  31  // f31 - temporary

// @section RV32I base instruction set - match and mask
//
// MATCH = the fixed bits that identify the instruction
// MASK  = which bits in the 32-bit word are fixed (1 = fixed, 0 = operand)
//
// To check: (instruction & MASK) == MATCH

// U-type
#define MATCH_LUI          0x00000037
#define MASK_LUI           0x0000007F
#define MATCH_AUIPC        0x00000017
#define MASK_AUIPC         0x0000007F

// J-type
#define MATCH_JAL          0x0000006F
#define MASK_JAL           0x0000007F

// I-type (jump)
#define MATCH_JALR         0x00000067
#define MASK_JALR          0x0000707F

// B-type (branches)
#define MATCH_BEQ          0x00000063
#define MASK_BEQ           0x0000707F
#define MATCH_BNE          0x00001063
#define MASK_BNE           0x0000707F
#define MATCH_BLT          0x00004063
#define MASK_BLT           0x0000707F
#define MATCH_BGE          0x00005063
#define MASK_BGE           0x0000707F
#define MATCH_BLTU         0x00006063
#define MASK_BLTU          0x0000707F
#define MATCH_BGEU         0x00007063
#define MASK_BGEU          0x0000707F

// I-type (loads)
#define MATCH_LB           0x00000003
#define MASK_LB            0x0000707F
#define MATCH_LH           0x00001003
#define MASK_LH            0x0000707F
#define MATCH_LW           0x00002003
#define MASK_LW            0x0000707F
#define MATCH_LBU          0x00004003
#define MASK_LBU           0x0000707F
#define MATCH_LHU          0x00005003
#define MASK_LHU           0x0000707F

// S-type (stores)
#define MATCH_SB           0x00000023
#define MASK_SB            0x0000707F
#define MATCH_SH           0x00001023
#define MASK_SH            0x0000707F
#define MATCH_SW           0x00002023
#define MASK_SW            0x0000707F

// I-type (arithmetic immediate)
#define MATCH_ADDI         0x00000013
#define MASK_ADDI          0x0000707F
#define MATCH_SLTI         0x00002013
#define MASK_SLTI          0x0000707F
#define MATCH_SLTIU        0x00003013
#define MASK_SLTIU         0x0000707F
#define MATCH_XORI         0x00004013
#define MASK_XORI          0x0000707F
#define MATCH_ORI          0x00006013
#define MASK_ORI           0x0000707F
#define MATCH_ANDI         0x00007013
#define MASK_ANDI          0x0000707F
#define MATCH_SLLI         0x00001013
#define MASK_SLLI          0xFC00707F
#define MATCH_SRLI         0x00005013
#define MASK_SRLI          0xFC00707F
#define MATCH_SRAI         0x40005013
#define MASK_SRAI          0xFC00707F

// R-type (arithmetic)
#define MATCH_ADD          0x00000033
#define MASK_ADD           0xFE00707F
#define MATCH_SUB          0x40000033
#define MASK_SUB           0xFE00707F
#define MATCH_SLL          0x00001033
#define MASK_SLL           0xFE00707F
#define MATCH_SLT          0x00002033
#define MASK_SLT           0xFE00707F
#define MATCH_SLTU         0x00003033
#define MASK_SLTU          0xFE00707F
#define MATCH_XOR          0x00004033
#define MASK_XOR           0xFE00707F
#define MATCH_SRL          0x00005033
#define MASK_SRL           0xFE00707F
#define MATCH_SRA          0x40005033
#define MASK_SRA           0xFE00707F
#define MATCH_OR           0x00006033
#define MASK_OR            0xFE00707F
#define MATCH_AND          0x00007033
#define MASK_AND           0xFE00707F

// I-type (system)
#define MATCH_FENCE        0x0000000F
#define MASK_FENCE         0x0000707F
#define MATCH_ECALL        0x00000073
#define MASK_ECALL         0xFFFFFFFF
#define MATCH_EBREAK       0x00100073
#define MASK_EBREAK        0xFFFFFFFF

// @section RV64I extension (64-bit specific instructions)

// I-type (loads, 64-bit)
#define MATCH_LWU          0x00006003
#define MASK_LWU           0x0000707F
#define MATCH_LD           0x00003003
#define MASK_LD            0x0000707F

// S-type (stores, 64-bit)
#define MATCH_SD           0x00003023
#define MASK_SD            0x0000707F

// I-type (arithmetic immediate, 64-bit word)
#define MATCH_ADDIW        0x0000001B
#define MASK_ADDIW         0x0000707F
#define MATCH_SLLIW        0x0000101B
#define MASK_SLLIW         0xFE00707F
#define MATCH_SRLIW        0x0000501B
#define MASK_SRLIW         0xFE00707F
#define MATCH_SRAIW        0x4000501B
#define MASK_SRAIW         0xFE00707F

// R-type (arithmetic, 64-bit word)
#define MATCH_ADDW         0x0000003B
#define MASK_ADDW          0xFE00707F
#define MATCH_SUBW         0x4000003B
#define MASK_SUBW          0xFE00707F
#define MATCH_SLLW         0x0000103B
#define MASK_SLLW          0xFE00707F
#define MATCH_SRLW         0x0000503B
#define MASK_SRLW          0xFE00707F
#define MATCH_SRAW         0x4000503B
#define MASK_SRAW          0xFE00707F

#endif // RISCV_H

