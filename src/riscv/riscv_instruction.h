#ifndef RISCV_INSTRUCTION_H
#define RISCV_INSTRUCTION_H

//------------------------------------------------------------------------------
// Compile-time configuration
//------------------------------------------------------------------------------

// TODO(32-bit): both of this should not relied upon too much, and ideally be configurable
// depending on either on (currently unsupported) runtime/compile options.
#define INSTRUCTION_SIZE 4
#define XLEN 64

#define RISCV_IMMEDIATE_BITS         12
#define RISCV_IMMEDIATE_LARGE_BITS   (32 - RISCV_IMMEDIATE_BITS)
#define RISCV_IMMEDIATE_REACH        (1LL << RISCV_IMMEDIATE_BITS)
#define RISCV_IMMEDIATE_LARGE_REACH  (1LL << RISCV_IMMEDIATE_LARGE_BITS)

#define RISCV_JUMP_BITS              RISCV_IMMEDIATE_LARGE_BITS
// Every jump must land on an even (2^1) address
#define RISCV_JUMP_ALIGNMENT_BITS    1
#define RISCV_JUMP_ALIGNMENT         (1     << RISCV_JUMP_ALIGNMENT_BITS)
#define RISCV_JUMP_REACH             ((1ULL << RISCV_JUMP_BITS) * RISCV_JUMP_ALIGN)

#define RISCV_BRANCH_BITS            RISCV_IMMEDIATE_BITS
#define RISCV_BRANCH_ALIGNMENT_BITS  RISCV_JUMP_ALIGNMENT_BITS
#define RISCV_BRANCH_ALIGNMENT       (1 << RISCV_BRANCH_ALIGNMENT_BITS)
#define RISCV_BRANCH_REACH           ((1ULL << RISCV_JUMP_BITS) * RISCV_BRANCH_ALIGNMENT)

#define RISCV_RVC_IMMEDIATE_REACH    (1LL << 6)

//------------------------------------------------------------------------------
// Opcode constants
//------------------------------------------------------------------------------

#define OPCODE_LUI                     0x37
#define OPCODE_AUIPC                   0x17
#define OPCODE_JAL                     0x6F
#define OPCODE_JALR                    0x67
#define OPCODE_BRANCH                  0x63
#define OPCODE_LOAD                    0x03
#define OPCODE_STORE                   0x23
#define OPCODE_I_TYPE                  0x13
#define OPCODE_I_TYPE_W                0x1B
#define OPCODE_R_TYPE                  0x33
#define OPCODE_R_TYPE_W                0x3B
#define OPCODE_FENCE                   0x0F
#define OPCODE_ECALL                   0x73
#define OPCODE_EBREAK                  0x73

//------------------------------------------------------------------------------
// Funct3 constants
//------------------------------------------------------------------------------

#define FUNCT3_JALR                    0x00
#define FUNCT3_BEQ                     0x00
#define FUNCT3_BNE                     0x01
#define FUNCT3_BLT                     0x04
#define FUNCT3_BGE                     0x05
#define FUNCT3_BLTU                    0x06
#define FUNCT3_BGEU                    0x07
#define FUNCT3_LB                      0x00
#define FUNCT3_LH                      0x01
#define FUNCT3_LW                      0x02
#define FUNCT3_LD                      0x03
#define FUNCT3_LBU                     0x04
#define FUNCT3_LHU                     0x05
#define FUNCT3_LWU                     0x06
#define FUNCT3_SB                      0x00
#define FUNCT3_SH                      0x01
#define FUNCT3_SW                      0x02
#define FUNCT3_SD                      0x03
#define FUNCT3_ADDI                    0x00
#define FUNCT3_SLTI                    0x02
#define FUNCT3_SLTIU                   0x03
#define FUNCT3_XORI                    0x04
#define FUNCT3_ORI                     0x06
#define FUNCT3_ANDI                    0x07
#define FUNCT3_SLLI                    0x01
#define FUNCT3_SRLI                    0x05
#define FUNCT3_SRAI                    0x05
#define FUNCT3_ADD                     0x00
#define FUNCT3_SUB                     0x00
#define FUNCT3_SLL                     0x01
#define FUNCT3_SLT                     0x02
#define FUNCT3_SLTU                    0x03
#define FUNCT3_XOR                     0x04
#define FUNCT3_SRL                     0x05
#define FUNCT3_SRA                     0x05
#define FUNCT3_OR                      0x06
#define FUNCT3_AND                     0x07
#define FUNCT3_ECALL                   0x73
#define FUNCT3_EBREAK                  0x73
#define FUNCT3_FENCE                   0x0F
#define FUNCT3_FENCE_TSO               0x0F
#define FUNCT3_PAUSE                   0x0F

// 64-bit
#define FUNCT3_ADDIW                   0x00
#define FUNCT3_SLLIW                   0x01
#define FUNCT3_SRLIW                   0x05
#define FUNCT3_SRAIW                   0x05
#define FUNCT3_ADDW                    0x00
#define FUNCT3_SUBW                    0x00
#define FUNCT3_SLLW                    0x01
#define FUNCT3_SRLW                    0x05
#define FUNCT3_SRAW                    0x05

//------------------------------------------------------------------------------
// Funct6 / Funct7 constants
//------------------------------------------------------------------------------

// NOTE: On 64-bit, the shift amount grows to 6 bits (since registers are 64 bits wide), so shamt uses bits [25:20] and
// funct7 shrinks to a 6-bit funct6 in bits [31:26].
#define FUNCT6_SLLI                    0x00
#define FUNCT6_SRLI                    0x00
#define FUNCT6_SRAI                    0x10

#define FUNCT7_ADD                     0x00
#define FUNCT7_SUB                     0x20
#define FUNCT7_SLL                     0x00
#define FUNCT7_SLT                     0x00
#define FUNCT7_SLTU                    0x00
#define FUNCT7_XOR                     0x00
#define FUNCT7_SRL                     0x00
#define FUNCT7_SRA                     0x20
#define FUNCT7_OR                      0x00
#define FUNCT7_AND                     0x00
// 64-bit
#define FUNCT7_ADDW                    0x00
#define FUNCT7_SUBW                    0x20
#define FUNCT7_SLLW                    0x00
#define FUNCT7_SRLW                    0x00
#define FUNCT7_SRAW                    0x20
#define FUNCT7_SLLIW                   0x00
#define FUNCT7_SRLIW                   0x00
#define FUNCT7_SRAIW                   0x20

//------------------------------------------------------------------------------
// Encoding macros and functions
//------------------------------------------------------------------------------

// All these macros convert from and to 64-bit unsigned integers.
//
// Reminder of C99/C11 integer promotion rules
//
// Integer promotions: if type is smaller than int (char, short, unsigned char etc) -> convert to int, preserving value.
//
// Usual arithmetic conversions for binary operators
// (+, -, *, /, %, &, |, ^, ==, <, etc):
//
//   Same rank:    unsigned wins
//   unsigned int  + int          -> unsigned int
//   unsigned long + int          -> unsigned long
//   long          + unsigned int -> long (if long covers unsigned int), else unsigned long
//   int64_t       + uint32_t     -> int64_t (signed is wider → signed)
//
// Shifts (<<, >>):
//   Promote left operand only. No usual arithmetic conversions.
//   Result type = promoted left operand.

// Yields a zero-mask or one-mask depending on the sign of bit 31.
#define immediate_sign_mask_m(x) (-((U32)(x) >> 31 & 1))

#define sign_extend_m(x, bits) ((S64)((U64)(x) << (64 - (bits))) >> (64 - (bits)))

// --- Standard (32-bit) instruction immediates ---

#define encode_immediate_i_m(x)  (shift_right_mask_m(x,  0,  12) << 20)
#define extract_immediate_i_m(x) ((U64)(shift_right_mask_m(x, 20, 12) | (U64)immediate_sign_mask_m(x) << 12))

#define encode_immediate_u_m(x)  (shift_right_mask_m(x, 12, 20) << 12)
#define extract_immediate_u_m(x) (shift_right_mask_m(x, 12, 20) << 12 | (U64)immediate_sign_mask_m(x) << 32)

// imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
#define encode_immediate_s_m(x) ((shift_right_mask_m(x,  0,   5) <<  7) | shift_right_mask_m(x, 5, 7) << 25)
#define extract_immdiate_s_m(x) ((shift_right_mask_m(x,  7,   5)) | shift_right_mask_m(x, 25, 7) << 5 | immediate_sign_mask_m(x) << 12)
// imm[12|10:5] rs2 rs1 000 imm[4:1|11] <opcode>
#define encode_immediate_b_m(x)                                                         \
(                                                                                       \
        (shift_right_mask_m(x, 11, 1) <<  7) | (shift_right_mask_m(x,  1, 4)  <<  8) |  \
        (shift_right_mask_m(x,  5, 6) << 25) | (shift_right_mask_m(x, 12, 1)  << 31)    \
)
// imm[20|10:1|11|19:12] rd <opcode>
#define encode_immediate_j_m(x)                                                         \
(                                                                                       \
        (shift_right_mask_m(x, 12,  8) << 12) | (shift_right_mask_m(x, 11,  1) << 20) | \
        (shift_right_mask_m(x,  1, 10) << 21) | (shift_right_mask_m(x, 20,  1) << 31)   \
)

#define extract_immediate_s_m(x) sign_extend_m((shift_right_mask_m(x, 7, 5) | (shift_right_mask_m(x, 25, 7) << 5)), 12)
#define extract_immediate_b_m(x)                                           \
        sign_extend_m                                                      \
        (                                                                  \
                (                                                          \
                (shift_right_mask_m(x,  7, 1) << 11) |                     \
                (shift_right_mask_m(x,  8, 4) <<  1) |                     \
                (shift_right_mask_m(x, 25, 6) <<  5) |                     \
                (shift_right_mask_m(x, 31, 1) << 12)                       \
                ),                                                         \
                13                                                         \
        )
#define extract_immediate_j_m(x)                                           \
        sign_extend_m                                                      \
        (                                                                  \
                (                                                          \
                (shift_right_mask_m(x, 12,  8) << 12) |                    \
                (shift_right_mask_m(x, 20,  1) << 11) |                    \
                (shift_right_mask_m(x, 21, 10) <<  1) |                    \
                (shift_right_mask_m(x, 31,  1) << 20)                      \
                ),                                                         \
                21                                                         \
        )

#define validate_immediate_i_m(x) (extract_immediate_i_m(encode_immediate_i_m(x)) == (x))
#define validate_immediate_u_m(x) (extract_immediate_u_m(encode_immediate_u_m(x)) == (x))
#define validate_immediate_s_m(x) (extract_immediate_s_m(encode_immediate_s_m(x)) == (x))
#define validate_immediate_b_m(x) (extract_immediate_b_m(encode_immediate_b_m(x)) == (x))
#define validate_immediate_j_m(x) (extract_immediate_j_m(encode_immediate_j_m(x)) == (x))

// --- Compressed (RVC) instruction immediates ---
// Format names follow the RISC-V specification (Chapter 28).

// CI-format: 16-bit encoding
// [ funct3  |i5|   rd/rs1      |  imm[4:0]     | op]
// Immediate: sign_extend({i5, imm[4:0]}, 6)
#define encode_immediate_ci_m(x)                (shift_right_mask_m(x, 0, 5) << 2 | shift_right_mask_m(x, 5, 1) << 12)
#define extract_immediate_ci_m(x)               sign_extend_m((shift_right_mask_m(x, 2, 5) | (shift_right_mask_m(x, 12, 1) << 5)), 6)
#define validate_immediate_ci_m(x)              (extract_immediate_ci_m(encode_immediate_ci_m(x)) == (x))

// CIW-format: 16-bit encoding
// [ funct3  |       imm[7:0]            | rd' | op]
// Immediate: {imm[7:0]}
#define encode_immediate_ciw_m(x)               (shift_right_mask_m(x, 0, 8) << 5)
#define extract_immediate_ciw_m(x)              (shift_right_mask_m(x, 5, 8))
#define validate_immediate_ciw_m(x)             (extract_immediate_ciw_m(encode_immediate_ciw_m(x)) == (x))

// CL-format: 16-bit encoding
// [ funct3  | i4| i3| i2| rs1'   |i1|i0| rd' | op]
// Immediate: {i4, i3, i2, i1, i0}
#define encode_immediate_cl_m(x)                (shift_right_mask_m(x, 0, 2) << 5 | shift_right_mask_m(x, 2, 3) << 10)
#define extract_immediate_cl_m(x)               (shift_right_mask_m(x, 5, 2) | shift_right_mask_m(x, 10, 3) << 2)
#define validate_immediate_cl_m(x)              (extract_immediate_cl_m(encode_immediate_cl_m(x)) == (x))

// CS-format: 16-bit encoding
// [ funct3  | i2| i1| i0| rs1'   |i4|i3| rs2'| op]
// Immediate: {i4, i3, i2, i1, i0}
#define encode_immediate_cs_m(x)                (shift_right_mask_m(x, 0, 3) << 10 | shift_right_mask_m(x, 3, 2) << 5)
#define extract_immediate_cs_m(x)               (shift_right_mask_m(x, 10, 3) | shift_right_mask_m(x, 5, 2) << 3)
#define validate_immediate_cs_m(x)              (extract_immediate_cs_m(encode_immediate_cs_m(x)) == (x))

// CSS-format: 16-bit encoding
// [ funct3  |      imm[5:0]         |   rs2    | op]
// Immediate: {imm[5:0]}
#define encode_immediate_css_m(x)               (shift_right_mask_m(x, 0, 6) << 7)
#define extract_immediate_css_m(x)              (shift_right_mask_m(x, 7, 6))
#define validate_immediate_css_m(x)             (extract_immediate_css_m(encode_immediate_css_m(x)) == (x))

// CB-format: 16-bit encoding
// [ funct3  |i8|i4|i3| rs1'   |i7|i6|i2|i1|i5| op]
// Immediate: sign_extend({i8, i7:i6, i5, i4:i3, i2:i1, 0}, 9)
#define encode_immediate_cb_m(x)                                                        \
(                                                                                       \
        (shift_right_mask_m(x,  1, 2) <<  3) | (shift_right_mask_m(x,  3, 2) << 10) |   \
        (shift_right_mask_m(x,  5, 1) <<  2) | (shift_right_mask_m(x,  6, 2) <<  5) |   \
        (shift_right_mask_m(x,  8, 1) << 12)                                            \
)
#define extract_immediate_cb_m(x)                                                       \
        sign_extend_m                                                                   \
        (                                                                               \
                (                                                                       \
                (shift_right_mask_m(x,  3, 2) <<  1) |                                  \
                (shift_right_mask_m(x, 10, 2) <<  3) |                                  \
                (shift_right_mask_m(x,  2, 1) <<  5) |                                  \
                (shift_right_mask_m(x,  5, 2) <<  6) |                                  \
                (shift_right_mask_m(x, 12, 1) <<  8)                                    \
                ),                                                                      \
                9                                                                       \
        )
#define validate_immediate_cb_m(x)              (extract_immediate_cb_m(encode_immediate_cb_m(x)) == (x))

// CJ-format: 16-bit encoding
// [ funct3  |i11|i4|i9|i8|i10|i6|i7|i3|i2|i1|i5| op]
// Immediate: sign_extend({i11, i10, i9:i8, i7, i6, i5, i4, i3:i1, 0}, 12)
#define encode_immediate_cj_m(x)                                                        \
(                                                                                       \
        (shift_right_mask_m(x,  1, 3) <<  3) | (shift_right_mask_m(x,  4, 1) << 11) |   \
        (shift_right_mask_m(x,  5, 1) <<  2) | (shift_right_mask_m(x,  6, 1) <<  7) |   \
        (shift_right_mask_m(x,  7, 1) <<  6) | (shift_right_mask_m(x,  8, 2) <<  9) |   \
        (shift_right_mask_m(x, 10, 1) <<  8) | (shift_right_mask_m(x, 11, 1) << 12)     \
)
#define extract_immediate_cj_m(x)                                                       \
        sign_extend_m                                                                   \
        (                                                                               \
                (                                                                       \
                (shift_right_mask_m(x,  3, 3) <<  1) |                                  \
                (shift_right_mask_m(x, 11, 1) <<  4) |                                  \
                (shift_right_mask_m(x,  2, 1) <<  5) |                                  \
                (shift_right_mask_m(x,  7, 1) <<  6) |                                  \
                (shift_right_mask_m(x,  6, 1) <<  7) |                                  \
                (shift_right_mask_m(x,  9, 2) <<  8) |                                  \
                (shift_right_mask_m(x,  8, 1) << 10) |                                  \
                (shift_right_mask_m(x, 12, 1) << 11)                                    \
                ),                                                                      \
                12                                                                      \
        )
#define validate_immediate_cj_m(x)              (extract_immediate_cj_m(encode_immediate_cj_m(x)) == (x))

#ifndef shift_right_mask_m
#define shift_right_mask_m(x, shift, bits)  (((U64)(x) >> (shift)) & ((1ULL << (bits)) - 1))
#endif

// --- Instruction encoding constructors ---
// These build a complete 32-bit instruction word from register/immediate fields.

// R-type: [31:25] funct7 | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_r_encode_m(rd, rs1, rs2, opcode, funct3, funct7)                                           \
         (((U32)(opcode)       & 0x7F) <<  0) |                                                                \
         (((U32)(rd)           & 0x1F) <<  7) |                                                                \
         (((U32)(funct3)       & 0x07) << 12) |                                                                \
         (((U32)(rs1)          & 0x1F) << 15) |                                                                \
         (((U32)(rs2)          & 0x1F) << 20) |                                                                \
         (((U32)(funct7)       & 0x7F) << 25)

// I-type: [31:20] imm[11:0] | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_i_encode_m(rd, rs1, imm, opcode, funct3)                                                   \
        (((U32)(opcode)       &  0x7F) <<  0) |                                                                \
        (((U32)(rd)           &  0x1F) <<  7) |                                                                \
        (((U32)(funct3)       &  0x07) << 12) |                                                                \
        (((U32)(rs1)          &  0x1F) << 15) |                                                                \
        (((U32)(imm)          & 0xFFF) << 20)

#define instruction_i_shift_encode_m(rd, rs1, shamt, opcode, funct3, funct6)                                   \
        instruction_i_encode_m(rd, rs1, ((funct6) << 6) | ((shamt) & 0x3F), opcode, funct3)

#define instruction_i_shift_wide_encode_m(rd, rs1, shamt, opcode, funct3, funct7)                              \
        instruction_i_encode_m(rd, rs1, (funct7 << 5) | (shamt & 0x1F), opcode, funct3)

// S-type: [31:25] imm[11:5] | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] imm[4:0] | [6:0] opcode
#define instruction_s_encode_m(rs2, rs1, imm, opcode, funct3)                                                  \
        (((U32)(opcode)       &  0x7F) <<  0) |                                                                \
        (((U32)(imm)          &  0x1F) <<  7) |                                                                \
        (((U32)(funct3)       &  0x07) << 12) |                                                                \
        (((U32)(rs1)          &  0x1F) << 15) |                                                                \
        (((U32)(rs2)          &  0x1F) << 20) |                                                                \
        (((U32)(imm)          & 0xFE0) << 20)

// B-type: [31] imm[12] | [30:25] imm[10:5] | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:8] imm[4:1] | [7] imm[11] | [6:0] opcode
#define instruction_b_encode_m(rs2, rs1, imm, opcode, funct3)                                                  \
        (((U32)(opcode)              &  0x7F) <<  0) |                                                         \
        (((U32)((imm) >> 11)         &  0x01) <<  7) |                                                         \
        (((U32)((imm) >>  1)         &  0x0F) <<  8) |                                                         \
        (((U32)(funct3)              &  0x07) << 12) |                                                         \
        (((U32)(rs1)                 &  0x1F) << 15) |                                                         \
        (((U32)(rs2)                 &  0x1F) << 20) |                                                         \
        (((U32)((imm) >>  5)         &  0x3F) << 25) |                                                         \
        (((U32)((imm) >> 12)         &  0x01) << 31)

// U-type: [31:12] imm[31:12] | [11:7] rd | [6:0] opcode
#define instruction_u_encode_m(rd, imm, opcode)                                                                \
        (((U32)(opcode)              &  0x7F) <<  0) |                                                         \
        (((U32)(rd)                  &  0x1F) <<  7) |                                                         \
        (((U32)(imm)             & 0xFFFFF) << 12)

// J-type: [31] imm[20] | [30:21] imm[10:1] | [20] imm[11] | [19:12] imm[19:12] | [11:7] rd | [6:0] opcode
#define instruction_j_encode_m(rd, imm, opcode)                                                                \
        (((U32)(opcode)              &  0x7F) <<  0) |                                                         \
        (((U32)(rd)                  &  0x1F) <<  7) |                                                         \
        (((U32)((imm) >> 12)         &  0xFF) << 12) |                                                         \
        (((U32)((imm) >> 11)         &  0x01) << 20) |                                                         \
        (((U32)((imm) >>  1)         &  0x3FF) << 21) |                                                        \
        (((U32)((imm) >> 20)         &  0x01) << 31)

// --- Wrapper functions ---

internal U32 encode_immediate_i(S64 x)     { return encode_immediate_i_m(x);   }
internal U32 encode_immediate_u(S64 x)     { return encode_immediate_u_m(x);   }
internal U32 encode_immediate_s(S64 x)     { return encode_immediate_s_m(x);   }
internal U32 encode_immediate_b(S64 x)     { return encode_immediate_b_m(x);   }
internal U32 encode_immediate_j(S64 x)     { return encode_immediate_j_m(x);   }
internal U32 encode_immediate_ci(S64 x)    { return encode_immediate_ci_m(x);  }
internal U32 encode_immediate_ciw(S64 x)   { return encode_immediate_ciw_m(x); }
internal U32 encode_immediate_cl(S64 x)    { return encode_immediate_cl_m(x);  }
internal U32 encode_immediate_cs(S64 x)    { return encode_immediate_cs_m(x);  }
internal U32 encode_immediate_css(S64 x)   { return encode_immediate_css_m(x); }
internal U32 encode_immediate_cb(S64 x)    { return encode_immediate_cb_m(x);  }
internal U32 encode_immediate_cj(S64 x)    { return encode_immediate_cj_m(x);  }

internal B32 validate_immediate_i(S64 x)   { return (S64)extract_immediate_i_m(encode_immediate_i_m(x)) == x;     }
internal B32 validate_immediate_u(S64 x)   { return (S64)extract_immediate_u_m(encode_immediate_u_m(x)) == x;     }
internal B32 validate_immediate_s(S64 x)   { return (S64)extract_immediate_s_m(encode_immediate_s_m(x)) == x;     }
internal B32 validate_immediate_b(S64 x)   { return (S64)extract_immediate_b_m(encode_immediate_b_m(x)) == x;     }
internal B32 validate_immediate_j(S64 x)   { return (S64)extract_immediate_j_m(encode_immediate_j_m(x)) == x;     }
internal B32 validate_immediate_ci(S64 x)  { return (S64)extract_immediate_ci_m(encode_immediate_ci_m(x)) == x;   }
internal B32 validate_immediate_ciw(S64 x) { return (S64)extract_immediate_ciw_m(encode_immediate_ciw_m(x)) == x; }
internal B32 validate_immediate_cl(S64 x)  { return (S64)extract_immediate_cl_m(encode_immediate_cl_m(x)) == x;   }
internal B32 validate_immediate_cs(S64 x)  { return (S64)extract_immediate_cs_m(encode_immediate_cs_m(x)) == x;   }
internal B32 validate_immediate_css(S64 x) { return (S64)extract_immediate_css_m(encode_immediate_css_m(x)) == x; }
internal B32 validate_immediate_cb(S64 x)  { return (S64)extract_immediate_cb_m(encode_immediate_cb_m(x)) == x;   }
internal B32 validate_immediate_cj(S64 x)  { return (S64)extract_immediate_cj_m(encode_immediate_cj_m(x)) == x;   }

//------------------------------------------------------------------------------
// Predefined instruction encodings
//------------------------------------------------------------------------------

#define ENCODING_C_NOP  0x0001
#define ENCODING_NOP    0x00000013
#define ENCODING_RET    0x00008067
#define ENCODING_ECALL  0x00000073
#define ENCODING_EBREAK 0x00100073
#define ENCODING_PAUSE  0x0100000F
#define ENCODING_TSO    0x8330000F

//------------------------------------------------------------------------------
// RV field masks and shift amounts
//------------------------------------------------------------------------------

#define OP_MASK_OP              0x7f
#define OP_SH_OP                0
#define OP_MASK_RS2             0x1f
#define OP_SH_RS2               20
#define OP_MASK_RS1             0x1f
#define OP_SH_RS1               15
#define OP_MASK_RS3             0x1fU
#define OP_SH_RS3               27
#define OP_MASK_RD              0x1f
#define OP_SH_RD                7
#define OP_MASK_SHAMT           0x3f
#define OP_SH_SHAMT             20
#define OP_MASK_SHAMTW          0x1f
#define OP_SH_SHAMTW            20
#define OP_MASK_RM              0x7
#define OP_SH_RM                12
#define OP_MASK_PRED            0xf
#define OP_SH_PRED              24
#define OP_MASK_SUCC            0xf
#define OP_SH_SUCC              20
#define OP_MASK_AQ              0x1
#define OP_SH_AQ                26
#define OP_MASK_RL              0x1
#define OP_SH_RL                25

#define OP_MASK_CSR             0xfffU
#define OP_SH_CSR               20

#define OP_MASK_FUNCT3          0x7
#define OP_SH_FUNCT3            12
#define OP_MASK_FUNCT7          0x7fU
#define OP_SH_FUNCT7            25
#define OP_MASK_FUNCT2          0x3
#define OP_SH_FUNCT2            25

// RISC-V GAS constants

#define X_RA  1
#define X_T1  6

// MASK_RD / MASK_RS1 / MASK_RS2 / MASK_IMM — field bit masks for match/mask construction
#define MASK_RD     0x00000F80   // bits 11:7
#define MASK_RS1    0x000F8000   // bits 19:15
#define MASK_RS2    0x01F00000   // bits 24:20
#define MASK_IMM    0xFFF00000   // bits 31:20

// Instruction info flags
#define INSN_MACRO       0x80000000UL
#define INSN_ALIAS       0x00000001
#define INSN_BRANCH      0x00000002
#define INSN_JSR         0x00000004
#define INSN_CONDBRANCH  0x00000008
#define INSN_DREF        0x00000010
#define INSN_1_BYTE      0x00000020
#define INSN_2_BYTE      0x00000040
#define INSN_4_BYTE      0x00000080

// Macro identifiers
#define MACRO_CALL  1
#define MACRO_LI    2
#define MACRO_LA    3

//------------------------------------------------------------------------------
// MATCH / MASK constants for each instruction
//------------------------------------------------------------------------------

#define MATCH_ADDI 0x13
#define MASK_ADDI  0x707f
#define MATCH_SLLI_RV32 0x1013
#define MASK_SLLI_RV32  0xfe00707f
#define MATCH_SRLI_RV32 0x5013
#define MASK_SRLI_RV32  0xfe00707f
#define MATCH_SRAI_RV32 0x40005013
#define MASK_SRAI_RV32  0xfe00707f
#define MATCH_FRFLAGS 0x102073
#define MASK_FRFLAGS  0xfffff07f
#define MATCH_FSFLAGS 0x101073
#define MASK_FSFLAGS  0xfff0707f
#define MATCH_FSFLAGSI 0x105073
#define MASK_FSFLAGSI  0xfff0707f
#define MATCH_FRRM 0x202073
#define MASK_FRRM  0xfffff07f
#define MATCH_FSRM 0x201073
#define MASK_FSRM  0xfff0707f
#define MATCH_FSRMI 0x205073
#define MASK_FSRMI  0xfff0707f
#define MATCH_FSCSR 0x301073
#define MASK_FSCSR  0xfff0707f
#define MATCH_FRCSR 0x302073
#define MASK_FRCSR  0xfffff07f
#define MATCH_RDCYCLE 0xc0002073
#define MASK_RDCYCLE  0xfffff07f
#define MATCH_RDTIME 0xc0102073
#define MASK_RDTIME  0xfffff07f
#define MATCH_RDINSTRET 0xc0202073
#define MASK_RDINSTRET  0xfffff07f
#define MATCH_RDCYCLEH 0xc8002073
#define MASK_RDCYCLEH  0xfffff07f
#define MATCH_RDTIMEH 0xc8102073
#define MASK_RDTIMEH  0xfffff07f
#define MATCH_RDINSTRETH 0xc8202073
#define MASK_RDINSTRETH  0xfffff07f
#define MATCH_SCALL 0x73
#define MASK_SCALL  0xffffffff
#define MATCH_SBREAK 0x100073
#define MASK_SBREAK  0xffffffff
#define MATCH_BEQ 0x63
#define MASK_BEQ  0x707f
#define MATCH_BNE 0x1063
#define MASK_BNE  0x707f
#define MATCH_BLT 0x4063
#define MASK_BLT  0x707f
#define MATCH_BGE 0x5063
#define MASK_BGE  0x707f
#define MATCH_BLTU 0x6063
#define MASK_BLTU  0x707f
#define MATCH_BGEU 0x7063
#define MASK_BGEU  0x707f
#define MATCH_JALR 0x67
#define MASK_JALR  0x707f
#define MATCH_JAL 0x6f
#define MASK_JAL  0x7f
#define MATCH_LUI 0x37
#define MASK_LUI  0x7f
#define MATCH_AUIPC 0x17
#define MASK_AUIPC  0x7f
#define MATCH_SLLI 0x1013
#define MASK_SLLI  0xfc00707f
#define MATCH_SLTI 0x2013
#define MASK_SLTI  0x707f
#define MATCH_SLTIU 0x3013
#define MASK_SLTIU  0x707f
#define MATCH_XORI 0x4013
#define MASK_XORI  0x707f
#define MATCH_SRLI 0x5013
#define MASK_SRLI  0xfc00707f
#define MATCH_SRAI 0x40005013
#define MASK_SRAI  0xfc00707f
#define MATCH_ORI 0x6013
#define MASK_ORI  0x707f
#define MATCH_ANDI 0x7013
#define MASK_ANDI  0x707f
#define MATCH_ADD 0x33
#define MASK_ADD  0xfe00707f
#define MATCH_SUB 0x40000033
#define MASK_SUB  0xfe00707f
#define MATCH_SLL 0x1033
#define MASK_SLL  0xfe00707f
#define MATCH_SLT 0x2033
#define MASK_SLT  0xfe00707f
#define MATCH_SLTU 0x3033
#define MASK_SLTU  0xfe00707f
#define MATCH_XOR 0x4033
#define MASK_XOR  0xfe00707f
#define MATCH_SRL 0x5033
#define MASK_SRL  0xfe00707f
#define MATCH_SRA 0x40005033
#define MASK_SRA  0xfe00707f
#define MATCH_OR 0x6033
#define MASK_OR  0xfe00707f
#define MATCH_AND 0x7033
#define MASK_AND  0xfe00707f
#define MATCH_ADDIW 0x1b
#define MASK_ADDIW  0x707f
#define MATCH_SLLIW 0x101b
#define MASK_SLLIW  0xfe00707f
#define MATCH_SRLIW 0x501b
#define MASK_SRLIW  0xfe00707f
#define MATCH_SRAIW 0x4000501b
#define MASK_SRAIW  0xfe00707f
#define MATCH_ADDW 0x3b
#define MASK_ADDW  0xfe00707f
#define MATCH_SUBW 0x4000003b
#define MASK_SUBW  0xfe00707f
#define MATCH_SLLW 0x103b
#define MASK_SLLW  0xfe00707f
#define MATCH_SRLW 0x503b
#define MASK_SRLW  0xfe00707f
#define MATCH_SRAW 0x4000503b
#define MASK_SRAW  0xfe00707f
#define MATCH_LB 0x3
#define MASK_LB  0x707f
#define MATCH_LH 0x1003
#define MASK_LH  0x707f
#define MATCH_LW 0x2003
#define MASK_LW  0x707f
#define MATCH_LD 0x3003
#define MASK_LD  0x707f
#define MATCH_LBU 0x4003
#define MASK_LBU  0x707f
#define MATCH_LHU 0x5003
#define MASK_LHU  0x707f
#define MATCH_LWU 0x6003
#define MASK_LWU  0x707f
#define MATCH_SB 0x23
#define MASK_SB  0x707f
#define MATCH_SH 0x1023
#define MASK_SH  0x707f
#define MATCH_SW 0x2023
#define MASK_SW  0x707f
#define MATCH_SD 0x3023
#define MASK_SD  0x707f
#define MATCH_PAUSE 0x0100000f
#define MASK_PAUSE  0xffffffff
#define MATCH_FENCE 0xf
#define MASK_FENCE  0x707f
#define MATCH_FENCE_I 0x100f
#define MASK_FENCE_I  0x707f
#define MATCH_FENCE_TSO 0x8330000f
#define MASK_FENCE_TSO  0xfff0707f
#define MATCH_ECALL 0x73
#define MASK_ECALL  0xffffffff
#define MATCH_EBREAK 0x100073
#define MASK_EBREAK  0xffffffff
#define MATCH_MUL 0x2000033
#define MASK_MUL  0xfe00707f
#define MATCH_MULH 0x2001033
#define MASK_MULH  0xfe00707f
#define MATCH_MULHSU 0x2002033
#define MASK_MULHSU  0xfe00707f
#define MATCH_MULHU 0x2003033
#define MASK_MULHU  0xfe00707f
#define MATCH_DIV 0x2004033
#define MASK_DIV  0xfe00707f
#define MATCH_DIVU 0x2005033
#define MASK_DIVU  0xfe00707f
#define MATCH_REM 0x2006033
#define MASK_REM  0xfe00707f
#define MATCH_REMU 0x2007033
#define MASK_REMU  0xfe00707f
#define MATCH_MULW 0x200003b
#define MASK_MULW  0xfe00707f
#define MATCH_DIVW 0x200403b
#define MASK_DIVW  0xfe00707f
#define MATCH_DIVUW 0x200503b
#define MASK_DIVUW  0xfe00707f
#define MATCH_REMW 0x200603b
#define MASK_REMW  0xfe00707f
#define MATCH_REMUW 0x200703b
#define MASK_REMUW  0xfe00707f

//------------------------------------------------------------------------------
// Bit-field insertion helpers
//------------------------------------------------------------------------------

typedef U32 insn_t;

// Replace bits MASK << SHIFT of STRUCT with the equivalent bits in
// VALUE << SHIFT.  VALUE is evaluated exactly once.
#define INSERT_BITS(STRUCT, VALUE, MASK, SHIFT) \
  (STRUCT) = (((STRUCT) & ~((insn_t)(MASK) << (SHIFT))) \
              | ((insn_t)((VALUE) & (MASK)) << (SHIFT)))

#define INSERT_OPERAND(field, instruction, value) \
  INSERT_BITS ((instruction).encoding, value, OP_MASK_##field, OP_SH_##field)

#define INSERT_IMM(n, s, INSN, VALUE) \
  INSERT_BITS ((INSN).insn_opcode, VALUE, (1UL<<n) - 1, s)

//------------------------------------------------------------------------------
// Types and declarations
//------------------------------------------------------------------------------

typedef U8 RISCV_Instruction_Class;
enum
{
        RISCV_Instruction_Class__None,
        RISCV_Instruction_Class__I,

        RISCV_Instruction_Class__COUNT,

};
assert_static_m(RISCV_Instruction_Class__COUNT < U8_max, RISCV_Instruction_Class__count_check);

typedef U16 OP_Argument;
enum
{
        OP_Argument__None = 0,
        OP_Argument__Comma,
        OP_Argument__RD,
        OP_Argument__RS1,
        OP_Argument__RS2,
        OP_Argument__RS3,
        OP_Argument__Immediate_Large,
        OP_Argument__Immediate_I,
        OP_Argument__Immediate_S,
        OP_Argument__Address,
        OP_Argument__Offset_PC_Relative_12,
        OP_Argument__Offset_PC_Relative_20,
        OP_Argument__Offset_Load,
        OP_Argument__Offset_Store,
        OP_Argument__Parenthesis_Left,
        OP_Argument__Parenthesis_Right,
        OP_Argument__Immediate_U,
        OP_Argument__Shift_Amount,
        OP_Argument__Shift_Amount_5,
        OP_Argument__Call_Expression,
        OP_Argument__COUNT,
};

#define OP_arguments_m(...) ((OP_Argument[]){ __VA_ARGS__, 0 })

// This structure holds information for a particular instruction.
//
// From GNU as, adapted.
typedef struct RISCV_Opcode RISCV_Opcode;
struct RISCV_Opcode
{
        // The name of the instruction.
        const char *name;

        // Hash of the name. NOTE: temporary, this could be dropped.
        U32 hash;

        // The requirement of xlen for the instruction, 0 if no requirement. For example, it can be 32/64 in case of
        // 32/64-bit only instruction.
        U32 length_requirement;

        // Class to which this instruction belongs.  Used to decide whether or not this instruction is legal in the
        // current -march context.
        RISCV_Instruction_Class instruction_class;

        // A 16-bit, null-terminated array describing the arguments for this instruction.
        //
        // TODO(medium): don't make this a NULL-terminated array. Add a `U8 count` field and use macros to expand.
        // This is valid: `array_count_m((Type[]){ __VA_ARGS__ })`.
        U16 *arguments;

        // The basic opcode for the instruction.  When assembling, this opcode is modified by the arguments to produce
        // the actual opcode that is used.  If pinfo is INSN_MACRO, then this is 0.
        //
        // NOTE: this field, like `mask`, are U64 in GNU as. However, no >32-bit instructions exist at the moment if I'm
        // not mistaken, so let's just use that.
        U32 match;

        // If pinfo is not INSN_MACRO, then this is a bit mask for the relevant portions of the opcode when
        // disassembling.  If the actual opcode anded with the match field equals the opcode field, then we have found
        // the correct instruction.  If pinfo is INSN_MACRO, then this field is the macro identifier.
        //
        // TODO(medium): this overloaded field in case of a macro is also quite ugly. There is the `info` field already!
        U32 mask;

        // A function to determine if a word corresponds to this instruction. Usually, this computes ((word & mask) == match).
        B32 (*match_function) (const RISCV_Opcode *opcode, U32 word);

        // For a macro, this is INSN_MACRO.  Otherwise, it is a collection of bits describing the instruction, notably
        // any relevant hazard information.
        U64 info;
};
assert_static_m(sizeof(RISCV_Opcode) <= 64, RISCV_Opcode__size_check);

// Check whether the encoded instruction bits match the provided opcode.
//
// To do so, a XOR over the opcode `match` field is performed. The operation highlights any bits that might differ from
// the mandatory encoding (e.g. opcode bits, funct3/funct7 etc..). On a perfect match, this returns zero.
// Then, we perform a bitwise AND with the opcode `mask` field, which keeps only the bits checked by the `match` field,
// skipping variable ones like immediates or registers.
static B32
match_opcode (const RISCV_Opcode *opcode, U32 instruction)
{
        B32 result = ((instruction ^ opcode->match) & opcode->mask) == 0;
        return result;
}

static B32
match_rd_nonzero (const RISCV_Opcode *opcode, U32 instruction)
{
        unused_m(opcode);
        return ((instruction >> OP_SH_RD) & OP_MASK_RD) != 0;
}

static B32
match_rs1_nonzero (const RISCV_Opcode *opcode, U32 instruction)
{
        unused_m(opcode);
        return ((instruction >> OP_SH_RS1) & OP_MASK_RS1) != 0;
}

internal const RISCV_Opcode *
RISCV_Opcode__table_find(U32 instruction_hash);

// Information about an instruction, including its format, operands
// and fixups.
// TODO(refactor): this struct is probably overloaded with unnedded stuff.
typedef struct RISCV_Instruction RISCV_Instruction;
struct RISCV_Instruction
{
  const RISCV_Opcode *opcode;

  // The long encoded instruction bits ([0] is non-zero on a long opcode).  */
  // char insn_long_opcode[RISCV_MAX_INSN_LEN];

  // The frag that contains the instruction
  Fragment *fragment;

  // The relocations associated with the instruction, if any.
  Fixup *fixup;

  // Where the instruction is located in the source.
  U32 location;

  U32 encoding;
  // The offset into fragment of the first instruction byte.
  U32 offset;

};

internal RISCV_Instruction
RISCV_Instruction__create(const RISCV_Opcode *opcode, U32 location)
{
        RISCV_Instruction result =
        {
                .opcode   = opcode,
                .encoding = opcode->match,
                .location = location,
        };
        return result;
}

internal U8
RISCV_instruction_size(U32 encoding);

internal void
RISCV_Instruction__append
(
        Arena             *arena,
        Section           *section,

        RISCV_Instruction *instruction,
        Expression        *expression,
        U16                relocation
);

internal void
RISCV_macro_build
(
        Arena           *arena,
        Section         *section,

        String8          instruction_name,
        U32              location,
        Expression      *expression,
        OP_Argument     *arguments,
        S32             *values
);

// Expand a call pseudo instruction into an `auipc + jalr` pair with the provided register for `jalr`.
internal void
RISCV_call_expand
(
        Arena           *arena,
        Section         *section,

        U8               rd,
        U8               rs1,
        Expression *expression,
        U16              relocation,
        U32              location
);

internal U8
RISCV_li_expand
(
        Section         *section,

        S64 immediate,
        U8  register_destination,
        U32 location
);

internal void
RISCV_instruction_pseudo_append
(
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,

        RISCV_Instruction  *instruction,
        Expression         *expression,
        U16                 relocation
);

#endif // RISCV_INSTRUCTION_H
