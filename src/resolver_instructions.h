#ifndef RESOLVER_INSTRUCTIONS_H
#define RESOLVER_INSTRUCTIONS_H

#define OPCODE_LUI    0x37
#define OPCODE_AUIPC  0x17
#define OPCODE_JAL    0x6F
#define OPCODE_JALR   0x67
#define OPCODE_BRANCH 0x63
#define OPCODE_LOAD   0x03
#define OPCODE_STORE  0x23
#define OPCODE_I_TYPE 0x13
#define OPCODE_R_TYPE 0x33
#define OPCODE_FENCE  0x0F
#define OPCODE_ECALL  0x73
#define FUNCT3_BEQ    0x00
#define FUNCT3_BNE    0x01
#define FUNCT3_BLT    0x04
#define FUNCT3_BGE    0x05
#define FUNCT3_BLTU   0x06
#define FUNCT3_BGEU   0x07
#define FUNCT3_LB     0x00
#define FUNCT3_LH     0x01
#define FUNCT3_LW     0x02
#define FUNCT3_LBU    0x04
#define FUNCT3_LHU    0x05
#define FUNCT3_SB     0x00
#define FUNCT3_SH     0x01
#define FUNCT3_SW     0x02
#define FUNCT3_ADDI   0x00
#define FUNCT3_SLTI   0x02
#define FUNCT3_SLTIU  0x03
#define FUNCT3_XORI   0x04
#define FUNCT3_ORI    0x06
#define FUNCT3_ANDI   0x07
#define FUNCT3_SLLI   0x01
#define FUNCT3_SRLI   0x05
#define FUNCT3_SRAI   0x05
#define FUNCT3_ADD    0x00
#define FUNCT3_SUB    0x00
#define FUNCT3_SLL    0x01
#define FUNCT3_SLT    0x02
#define FUNCT3_SLTU   0x03
#define FUNCT3_XOR    0x04
#define FUNCT3_SRL    0x05
#define FUNCT3_SRA    0x05
#define FUNCT3_OR     0x06
#define FUNCT3_AND    0x07
#define FUNCT7_SLLI   0x00
#define FUNCT7_SRLI   0x00
#define FUNCT7_SRAI   0x20
#define FUNCT7_ADD    0x00
#define FUNCT7_SUB    0x20
#define FUNCT7_SLL    0x00
#define FUNCT7_SLT    0x00
#define FUNCT7_SLTU   0x00
#define FUNCT7_XOR    0x00
#define FUNCT7_SRL    0x00
#define FUNCT7_SRA    0x20
#define FUNCT7_OR     0x00
#define FUNCT7_AND    0x00

// R-type encoding (32 bits):
// [31:25] funct7 | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_r_encode_m(rd, rs1, rs2, opcode, funct3, funct7) \
    (((U32)(opcode)       & 0x7F) <<  0) | /* bits  6:0  */    \
    (((U32)(rd)           & 0x1F) <<  7) | /* bits 11:7  */    \
    (((U32)(funct3)       & 0x07) << 12) | /* bits 14:12 */    \
    (((U32)(rs1)          & 0x1F) << 15) | /* bits 19:15 */    \
    (((U32)(rs2)          & 0x1F) << 20) | /* bits 24:20 */    \
    (((U32)(funct7)       & 0x7F) << 25)   /* bits 31:25 */

internal void
Resolver_instruction_I_encode(Resolver *resolver, Instruction_Kind instruction_kind);

internal void
Resolver_instruction_I_load_encode(Resolver *resolver, Instruction_Kind instruction_kind);

internal void
Resolver_instruction_R_encode(Resolver *resolver, Instruction_Kind instruction_kind);

internal void
Resolver_instruction_S_encode(Resolver *resolver, Instruction_Kind instruction_kind);

internal void
Resolver_instruction_B_encode(Resolver *resolver, Instruction_Kind instruction_kind);

internal void
Resolver_instruction_U_encode(Resolver *resolver, Instruction_Kind instruction_kind);

internal void
Resolver_instruction_J_encode(Resolver *resolver, Instruction_Kind instruction_kind);

// nop -> addi x0, x0, 0
internal void
Resolver_instruction_nop_encode(Resolver *resolver);

// mv rd, rs -> addi rd, rs, 0
internal void
Resolver_instruction_mv_encode(Resolver *resolver);

// not rd, rs -> xori rd, rs, -1
internal void
Resolver_instruction_not_encode(Resolver *resolver);

// neg rd, rs -> sub rd, x0, rs
internal void
Resolver_instruction_neg_encode(Resolver *resolver);

// negw rd, rs -> subw rd, x0, rs (RV64)
internal void
Resolver_instruction_negw_encode(Resolver *resolver);

// sext.w rd, rs -> addiw rd, rs, 0 (RV64)
internal void
Resolver_instruction_sext_w_encode(Resolver *resolver);

// seqz rd, rs -> sltiu rd, rs, 1
internal void
Resolver_instruction_seqz_encode(Resolver *resolver);

// snez rd, rs -> sltu rd, x0, rs
internal void
Resolver_instruction_snez_encode(Resolver *resolver);

// sltz rd, rs -> slt rd, rs, x0
internal void
Resolver_instruction_sltz_encode(Resolver *resolver);

// sgtz rd, rs -> slt rd, x0, rs
internal void
Resolver_instruction_sgtz_encode(Resolver *resolver);

// beqz rs, offset -> beq rs, x0, offset
internal void
Resolver_instruction_beqz_encode(Resolver *resolver);

// bnez rs, offset -> bne rs, x0, offset
internal void
Resolver_instruction_bnez_encode(Resolver *resolver);

// blez rs, offset -> bge x0, rs, offset
internal void
Resolver_instruction_blez_encode(Resolver *resolver);

// bgez rs, offset -> bge rs, x0, offset
internal void
Resolver_instruction_bgez_encode(Resolver *resolver);

// bltz rs, offset -> blt rs, x0, offset
internal void
Resolver_instruction_bltz_encode(Resolver *resolver);

// bgtz rs, offset -> blt x0, rs, offset
internal void
Resolver_instruction_bgtz_encode(Resolver *resolver);

// bgt rs, rt, offset -> blt rt, rs, offset
internal void
Resolver_instruction_bgt_encode(Resolver *resolver);

// ble rs, rt, offset -> bge rt, rs, offset
internal void
Resolver_instruction_ble_encode(Resolver *resolver);

// bgtu rs, rt, offset -> bltu rt, rs, offset
internal void
Resolver_instruction_bgtu_encode(Resolver *resolver);

// bleu rs, rt, offset -> bgeu rt, rs, offset
internal void
Resolver_instruction_bleu_encode(Resolver *resolver);

// j offset -> jal x0, offset
internal void
Resolver_instruction_j_encode(Resolver *resolver);

// jr rs -> jalr x0, rs, 0
internal void
Resolver_instruction_jr_encode(Resolver *resolver);

// jalr rs -> jalr ra, rs, 0 (single-operand form)
internal void
Resolver_instruction_jalr_pseudo_encode(Resolver *resolver);

// ret -> jalr x0, ra, 0
internal void
Resolver_instruction_ret_encode(Resolver *resolver);

// li rd, imm -> lui rd, %hi(imm) + addi rd, rd, %lo(imm)
// NOTE: For small immediates that fit in 12 bits, a single addi suffices.
//       The expansion decision may be deferred to a later pass.
internal void
Resolver_instruction_li_encode(Resolver *resolver);

// la rd, symbol -> auipc rd, %pcrel_hi(symbol) + addi rd, rd, %pcrel_lo(symbol)
// NOTE: Expansion is deferred to a later pass.
internal void
Resolver_instruction_la_encode(Resolver *resolver);

// call offset -> auipc ra, offsetHi + jalr ra, ra, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Resolver_instruction_call_encode(Resolver *resolver);

// tail offset -> auipc t1, offsetHi + jalr x0, t1, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Resolver_instruction_tail_encode(Resolver *resolver);

internal void
Resolver_instruction_ecall_encode(Resolver *resolver);

internal void
Resolver_instruction_ebreak_encode(Resolver *resolver);

internal void
Resolver_instruction_pause_encode(Resolver *resolver);

internal void
Resolver_instruction_fence_tso_encode(Resolver *resolver);

// encodes a fence ordering operand: a string composed of the characters i, o, r, w
// in that order. Returns a 4-bit mask: i=bit3, o=bit2, r=bit1, w=bit0.
internal U8
Resolver_expect_fence_operand(Resolver *resolver);

// fence pred, succ   -> e.g. fence iorw, iorw
// fence              -> shorthand for fence iorw, iorw
internal void
Resolver_instruction_fence_encode(Resolver *resolver);

#endif // RESOLVER_INSTRUCTIONS_H

