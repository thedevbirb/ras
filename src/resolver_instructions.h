#ifndef RESOLVER_INSTRUCTIONS_H
#define RESOLVER_INSTRUCTIONS_H

#define OPCODE_LUI      0x37
#define OPCODE_AUIPC    0x17
#define OPCODE_JAL      0x6F
#define OPCODE_JALR     0x67
#define OPCODE_BRANCH   0x63
#define OPCODE_LOAD     0x03
#define OPCODE_STORE    0x23
#define OPCODE_I_TYPE   0x13
#define OPCODE_I_TYPE_W 0x1B
#define OPCODE_R_TYPE   0x33
#define OPCODE_R_TYPE_W 0x3B
#define OPCODE_FENCE    0x0F
#define OPCODE_ECALL    0x73

#define FUNCT3_JALR   0x00
#define FUNCT3_BEQ    0x00
#define FUNCT3_BNE    0x01
#define FUNCT3_BLT    0x04
#define FUNCT3_BGE    0x05
#define FUNCT3_BLTU   0x06
#define FUNCT3_BGEU   0x07
#define FUNCT3_LB     0x00
#define FUNCT3_LH     0x01
#define FUNCT3_LW     0x02
#define FUNCT3_LD     0x03
#define FUNCT3_LBU    0x04
#define FUNCT3_LHU    0x05
#define FUNCT3_LWU    0x06
#define FUNCT3_SB     0x00
#define FUNCT3_SH     0x01
#define FUNCT3_SW     0x02
#define FUNCT3_SD     0x03
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
// 64-bit
#define FUNCT3_ADDIW  0x00
#define FUNCT3_SLLIW  0x01
#define FUNCT3_SRLIW  0x05
#define FUNCT3_SRAIW  0x05
#define FUNCT3_ADDW   0x00
#define FUNCT3_SUBW   0x00
#define FUNCT3_SLLW   0x01
#define FUNCT3_SRLW   0x05
#define FUNCT3_SRAW   0x05

// NOTE: On 64-bit, the shift amount grows to 6 bits (since registers are 64 bits wide), so shamt uses bits [25:20] and
// funct7 shrinks to a 6-bit funct6 in bits [31:26].
#define FUNCT6_SLLI   0x00
#define FUNCT6_SRLI   0x00
#define FUNCT6_SRAI   0x10
// #define FUNCT7_SLLI   0x00
// #define FUNCT7_SRLI   0x00
// #define FUNCT7_SRAI   0x20

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
// 64-bit
#define FUNCT7_ADDW   0x00
#define FUNCT7_SUBW   0x20
#define FUNCT7_SLLW   0x00
#define FUNCT7_SRLW   0x00
#define FUNCT7_SRAW   0x20
#define FUNCT7_SLLIW  0x00
#define FUNCT7_SRLIW  0x00
#define FUNCT7_SRAIW  0x20


// R-type encoding (32 bits):
// [31:25] funct7 | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_r_encode_m(rd, rs1, rs2, opcode, funct3, funct7)      \
	 (((U32)(opcode)       & 0x7F) <<  0) | /* bits  6:0  */          \
   	 (((U32)(rd)           & 0x1F) <<  7) | /* bits 11:7  */          \
   	 (((U32)(funct3)       & 0x07) << 12) | /* bits 14:12 */          \
   	 (((U32)(rs1)          & 0x1F) << 15) | /* bits 19:15 */          \
   	 (((U32)(rs2)          & 0x1F) << 20) | /* bits 24:20 */          \
   	 (((U32)(funct7)       & 0x7F) << 25)   /* bits 31:25 */

// I-type encoding (32 bits):
// [31:20] imm[11:0] | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_i_encode_m(rd, rs1, imm, opcode, funct3)     \
	(((U32)(opcode)       &  0x7F) <<  0) | /* bits  6:0  */ \
    	(((U32)(rd)           &  0x1F) <<  7) | /* bits 11:7  */ \
    	(((U32)(funct3)       &  0x07) << 12) | /* bits 14:12 */ \
    	(((U32)(rs1)          &  0x1F) << 15) | /* bits 19:15 */ \
    	(((U32)(imm)          & 0xFFF) << 20)   /* bits 31:20 */

#define instruction_i_shift_encode_m(rd, rs1, shamt, opcode, funct3, funct6)                \
	instruction_i_encode_m(rd, rs1, ((funct6) << 6) | ((shamt) & 0x3F), opcode, funct3)

#define instruction_i_shift_wide_encode_m(rd, rs1, shamt, opcode, funct3, funct7)           \
	instruction_i_encode_m(rd, rs1, (funct7 << 5) | (shamt & 0x1F), opcode, funct3)

// S-type encoding (32 bits):
// [31:25] imm[11:5] | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] imm[4:0] | [6:0] opcode
#define instruction_s_encode_m(rs2, rs1, imm, opcode, funct3)     \
	(((U32)(opcode)       &  0x7F) <<  0) | /* bits  6:0  */  \
    	(((U32)(imm)          &  0x1F) <<  7) | /* bits 11:7  */  \
    	(((U32)(funct3)       &  0x07) << 12) | /* bits 14:12 */  \
    	(((U32)(rs1)          &  0x1F) << 15) | /* bits 19:15 */  \
    	(((U32)(rs2)          &  0x1F) << 20) | /* bits 24:20 */  \
    	(((U32)(imm)          & 0xFE0) << 25)   /* bits 31:25 */

// B-type encoding (32 bits):
// [31] imm[12] | [30:25] imm[10:5] | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:8] imm[4:1] | [7] imm[11] | [6:0] opcode
#define instruction_b_encode_m(rs2, rs1, imm, opcode, funct3)                     \
	(((U32)(opcode)              &  0x7F) <<  0) | /* bits  6:0            */ \
    	(((U32)((imm) >> 11)         &  0x01) <<  7) | /* bit   7     imm[11]  */ \
    	(((U32)((imm) >>  1)         &  0x0F) <<  8) | /* bits 11:8   imm[4:1] */ \
    	(((U32)(funct3)              &  0x07) << 12) | /* bits 14:12           */ \
    	(((U32)(rs1)                 &  0x1F) << 15) | /* bits 19:15           */ \
    	(((U32)(rs2)                 &  0x1F) << 20) | /* bits 24:20           */ \
    	(((U32)((imm) >>  5)         &  0x3F) << 25) | /* bits 30:25  imm[10:5]*/ \
    	(((U32)((imm) >> 12)         &  0x01) << 31)   /* bit  31     imm[12]  */

#define instruction_u_encode_m(rd, imm, opcode)                                      \
	(((U32)(opcode)              &  0x7F) <<  0) | /* bits  6:0              */  \
	(((U32)(rd)                  &  0x1F) <<  7) | /* bits 11:7              */  \
	(((U32)(imm)             & 0xFFFFF) << 12)     /* bits 31:12  imm[31:12] */

#endif // RESOLVER_INSTRUCTIONS_H

