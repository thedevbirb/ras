#ifndef LANGUAGE_INSTRUCTION_H
#define LANGUAGE_INSTRUCTION_H

#define INSTRUCTION_SIZE 4
#define IMMEDIATE_NOMINAL_J_SIZE_BIT   21
#define IMMEDIATE_NOMINAL_U_SIZE_BIT   21
#define IMMEDIATE_NOMINAL_B_SIZE_BIT   13
#define IMMEDIATE_NOMINAL_I_S_SIZE_BIT 12

typedef enum Instruction_Format
{
	Instruction_Format__None = 0,
	Instruction_Format__R,
	Instruction_Format__I,
	Instruction_Format__S,
	Instruction_Format__B,
	Instruction_Format__U,
	Instruction_Format__J,
        Instruction_Format__Expandable,
	Instruction_Format__COUNT,
}
Instruction_Format;

typedef enum Instruction_Kind
{
	Instruction_Kind__None = 0,

	// RV64I base
	Instruction_Kind__LUI,
	Instruction_Kind__AUIPC,

	Instruction_Kind__JAL,
	Instruction_Kind__JALR,

	Instruction_Kind__BEQ,
	Instruction_Kind__BNE,
	Instruction_Kind__BLT,
	Instruction_Kind__BGE,
	Instruction_Kind__BLTU,
	Instruction_Kind__BGEU,

	Instruction_Kind__LB,
	Instruction_Kind__LH,
	Instruction_Kind__LW,
	Instruction_Kind__LD,
	Instruction_Kind__LBU,
	Instruction_Kind__LHU,
	Instruction_Kind__LWU,

	Instruction_Kind__SB,
	Instruction_Kind__SH,
	Instruction_Kind__SW,
	Instruction_Kind__SD,

	Instruction_Kind__ADDI,
	Instruction_Kind__SLTI,
	Instruction_Kind__SLTIU,
	Instruction_Kind__XORI,
	Instruction_Kind__ORI,
	Instruction_Kind__ANDI,

	Instruction_Kind__SLLI,
	Instruction_Kind__SRLI,
	Instruction_Kind__SRAI,

	Instruction_Kind__ADD,
	Instruction_Kind__SUB,
	Instruction_Kind__SLL,
	Instruction_Kind__SLT,
	Instruction_Kind__SLTU,
	Instruction_Kind__XOR,
	Instruction_Kind__SRL,
	Instruction_Kind__SRA,
	Instruction_Kind__OR,
	Instruction_Kind__AND,

	// RV64-specific
	Instruction_Kind__ADDIW,
	Instruction_Kind__SLLIW,
	Instruction_Kind__SRLIW,
	Instruction_Kind__SRAIW,

	Instruction_Kind__ADDW,
	Instruction_Kind__SUBW,
	Instruction_Kind__SLLW,
	Instruction_Kind__SRLW,
	Instruction_Kind__SRAW,

	// Pseudo-instructions
	Instruction_Kind__NOP,
	Instruction_Kind__RET,
	Instruction_Kind__MV,
	Instruction_Kind__NOT,
	Instruction_Kind__NEG,
	Instruction_Kind__NEGW,
	Instruction_Kind__SEXT_W,
	Instruction_Kind__SEQZ,
	Instruction_Kind__SNEZ,
	Instruction_Kind__SLTZ,
	Instruction_Kind__SGTZ,
	Instruction_Kind__BEQZ,
	Instruction_Kind__BNEZ,
	Instruction_Kind__BLEZ,
	Instruction_Kind__BGEZ,
	Instruction_Kind__BLTZ,
	Instruction_Kind__BGTZ,
	Instruction_Kind__BGT,
	Instruction_Kind__BLE,
	Instruction_Kind__BGTU,
	Instruction_Kind__BLEU,
	Instruction_Kind__J,
	Instruction_Kind__CALL,
	Instruction_Kind__TAIL,
	Instruction_Kind__JR,
	Instruction_Kind__LI,
	Instruction_Kind__LA,

	// SYSTEM
	Instruction_Kind__ECALL,
	Instruction_Kind__EBREAK,
	Instruction_Kind__PAUSE,
	Instruction_Kind__FENCE,
	Instruction_Kind__FENCE_TSO,

	// Instruction_Kind__CSRRW,
	// Instruction_Kind__CSRRS,
	// Instruction_Kind__CSRRC,
	// Instruction_Kind__CSRRWI,
	// Instruction_Kind__CSRRSI,
	// Instruction_Kind__CSRRCI,
	//
	// // M extension
	// Instruction_Kind__MUL,
	// Instruction_Kind__MULH,
	// Instruction_Kind__MULHSU,
	// Instruction_Kind__MULHU,
	// Instruction_Kind__DIV,
	// Instruction_Kind__DIVU,
	// Instruction_Kind__REM,
	// Instruction_Kind__REMU,
	//
	// Instruction_Kind__MULW,
	// Instruction_Kind__DIVW,
	// Instruction_Kind__DIVUW,
	// Instruction_Kind__REMW,
	// Instruction_Kind__REMUW,
	//
	// // A extension
	// Instruction_Kind__LR_W,
	// Instruction_Kind__SC_W,
	// Instruction_Kind__AMOSWAP_W,
	// Instruction_Kind__AMOADD_W,
	// Instruction_Kind__AMOXOR_W,
	// Instruction_Kind__AMOAND_W,
	// Instruction_Kind__AMOOR_W,
	// Instruction_Kind__AMOMIN_W,
	// Instruction_Kind__AMOMAX_W,
	// Instruction_Kind__AMOMINU_W,
	// Instruction_Kind__AMOMAXU_W,
	//
	// Instruction_Kind__LR_D,
	// Instruction_Kind__SC_D,
	// Instruction_Kind__AMOSWAP_D,
	// Instruction_Kind__AMOADD_D,
	// Instruction_Kind__AMOXOR_D,
	// Instruction_Kind__AMOAND_D,
	// Instruction_Kind__AMOOR_D,
	// Instruction_Kind__AMOMIN_D,
	// Instruction_Kind__AMOMAX_D,
	// Instruction_Kind__AMOMINU_D,
	// Instruction_Kind__AMOMAXU_D,

	Instruction_Kind__COUNT,
}
Instruction_Kind;

global const char *Instruction_Kind_strings[Instruction_Kind__COUNT] =
{
	[Instruction_Kind__None] = "",

	[Instruction_Kind__LUI] = "lui",
	[Instruction_Kind__AUIPC] = "auipc",

	[Instruction_Kind__JAL] = "jal",
	[Instruction_Kind__JALR] = "jalr",

	[Instruction_Kind__BEQ] = "beq",
	[Instruction_Kind__BNE] = "bne",
	[Instruction_Kind__BLT] = "blt",
	[Instruction_Kind__BGE] = "bge",
	[Instruction_Kind__BLTU] = "bltu",
	[Instruction_Kind__BGEU] = "bgeu",

	[Instruction_Kind__LB] = "lb",
	[Instruction_Kind__LH] = "lh",
	[Instruction_Kind__LW] = "lw",
	[Instruction_Kind__LD] = "ld",
	[Instruction_Kind__LBU] = "lbu",
	[Instruction_Kind__LHU] = "lhu",
	[Instruction_Kind__LWU] = "lwu",

	[Instruction_Kind__SB] = "sb",
	[Instruction_Kind__SH] = "sh",
	[Instruction_Kind__SW] = "sw",
	[Instruction_Kind__SD] = "sd",

	[Instruction_Kind__ADDI] = "addi",
	[Instruction_Kind__SLTI] = "slti",
	[Instruction_Kind__SLTIU] = "sltiu",
	[Instruction_Kind__XORI] = "xori",
	[Instruction_Kind__ORI] = "ori",
	[Instruction_Kind__ANDI] = "andi",

	[Instruction_Kind__SLLI] = "slli",
	[Instruction_Kind__SRLI] = "srli",
	[Instruction_Kind__SRAI] = "srai",

	[Instruction_Kind__ADD] = "add",
	[Instruction_Kind__SUB] = "sub",
	[Instruction_Kind__SLL] = "sll",
	[Instruction_Kind__SLT] = "slt",
	[Instruction_Kind__SLTU] = "sltu",
	[Instruction_Kind__XOR] = "xor",
	[Instruction_Kind__SRL] = "srl",
	[Instruction_Kind__SRA] = "sra",
	[Instruction_Kind__OR] = "or",
	[Instruction_Kind__AND] = "and",

	[Instruction_Kind__ADDIW] = "addiw",
	[Instruction_Kind__SLLIW] = "slliw",
	[Instruction_Kind__SRLIW] = "srliw",
	[Instruction_Kind__SRAIW] = "sraiw",

	[Instruction_Kind__ADDW] = "addw",
	[Instruction_Kind__SUBW] = "subw",
	[Instruction_Kind__SLLW] = "sllw",
	[Instruction_Kind__SRLW] = "srlw",
	[Instruction_Kind__SRAW] = "sraw",

	[Instruction_Kind__ECALL] = "ecall",
	[Instruction_Kind__EBREAK] = "ebreak",
	[Instruction_Kind__PAUSE] = "pause",
	[Instruction_Kind__FENCE] = "fence",
	[Instruction_Kind__FENCE_TSO] = "fence.tso",

	// [Instruction_Kind__CSRRW] = "csrrw",
	// [Instruction_Kind__CSRRS] = "csrrs",
	// [Instruction_Kind__CSRRC] = "csrrc",
	// [Instruction_Kind__CSRRWI] = "csrrwi",
	// [Instruction_Kind__CSRRSI] = "csrrsi",
	// [Instruction_Kind__CSRRCI] = "csrrci",
	//
	// [Instruction_Kind__MUL] = "mul",
	// [Instruction_Kind__MULH] = "mulh",
	// [Instruction_Kind__MULHSU] = "mulhsu",
	// [Instruction_Kind__MULHU] = "mulhu",
	// [Instruction_Kind__DIV] = "div",
	// [Instruction_Kind__DIVU] = "divu",
	// [Instruction_Kind__REM] = "rem",
	// [Instruction_Kind__REMU] = "remu",
	//
	// [Instruction_Kind__MULW] = "mulw",
	// [Instruction_Kind__DIVW] = "divw",
	// [Instruction_Kind__DIVUW] = "divuw",
	// [Instruction_Kind__REMW] = "remw",
	// [Instruction_Kind__REMUW] = "remuw",
	//
	// [Instruction_Kind__LR_W] = "lr.w",
	// [Instruction_Kind__SC_W] = "sc.w",
	// [Instruction_Kind__AMOSWAP_W] = "amoswap.w",
	// [Instruction_Kind__AMOADD_W] = "amoadd.w",
	// [Instruction_Kind__AMOXOR_W] = "amoxor.w",
	// [Instruction_Kind__AMOAND_W] = "amoand.w",
	// [Instruction_Kind__AMOOR_W] = "amoor.w",
	// [Instruction_Kind__AMOMIN_W] = "amomin.w",
	// [Instruction_Kind__AMOMAX_W] = "amomax.w",
	// [Instruction_Kind__AMOMINU_W] = "amominu.w",
	// [Instruction_Kind__AMOMAXU_W] = "amomaxu.w",
	//
	// [Instruction_Kind__LR_D] = "lr.d",
	// [Instruction_Kind__SC_D] = "sc.d",
	// [Instruction_Kind__AMOSWAP_D] = "amoswap.d",
	// [Instruction_Kind__AMOADD_D] = "amoadd.d",
	// [Instruction_Kind__AMOXOR_D] = "amoxor.d",
	// [Instruction_Kind__AMOAND_D] = "amoand.d",
	// [Instruction_Kind__AMOOR_D] = "amoor.d",
	// [Instruction_Kind__AMOMIN_D] = "amomin.d",
	// [Instruction_Kind__AMOMAX_D] = "amomax.d",
	// [Instruction_Kind__AMOMINU_D] = "amominu.d",
	// [Instruction_Kind__AMOMAXU_D] = "amomaxu.d",
};

typedef enum Pseudo_Instruction_Kind
{
	Pseudo_Instruction_Kind__None = 0,

	Pseudo_Instruction_Kind__NOP,
	Pseudo_Instruction_Kind__LI,
	Pseudo_Instruction_Kind__MV,
	Pseudo_Instruction_Kind__NOT,
	Pseudo_Instruction_Kind__NEG,
	Pseudo_Instruction_Kind__NEGW,
	Pseudo_Instruction_Kind__SEXT_W,

	Pseudo_Instruction_Kind__SEQZ,
	Pseudo_Instruction_Kind__SNEZ,
	Pseudo_Instruction_Kind__SLTZ,
	Pseudo_Instruction_Kind__SGTZ,

	Pseudo_Instruction_Kind__BEQZ,
	Pseudo_Instruction_Kind__BNEZ,
	Pseudo_Instruction_Kind__BLEZ,
	Pseudo_Instruction_Kind__BGEZ,
	Pseudo_Instruction_Kind__BLTZ,
	Pseudo_Instruction_Kind__BGTZ,

	Pseudo_Instruction_Kind__J,
	Pseudo_Instruction_Kind__JR,
	Pseudo_Instruction_Kind__RET,
	Pseudo_Instruction_Kind__CALL,
	Pseudo_Instruction_Kind__TAIL,

	Pseudo_Instruction_Kind__LA,

	Pseudo_Instruction_Kind__COUNT,
}
Pseudo_Instruction_Kind;

global const char *Pseudo_Instruction_Kind_strings[Pseudo_Instruction_Kind__COUNT] =
{
	[Pseudo_Instruction_Kind__None]   = "",

	[Pseudo_Instruction_Kind__NOP]    = "nop",
	[Pseudo_Instruction_Kind__LI]     = "li",
	[Pseudo_Instruction_Kind__MV]     = "mv",
	[Pseudo_Instruction_Kind__NOT]    = "not",
	[Pseudo_Instruction_Kind__NEG]    = "neg",
	[Pseudo_Instruction_Kind__NEGW]   = "negw",
	[Pseudo_Instruction_Kind__SEXT_W] = "sext.w",

	[Pseudo_Instruction_Kind__SEQZ]   = "seqz",
	[Pseudo_Instruction_Kind__SNEZ]   = "snez",
	[Pseudo_Instruction_Kind__SLTZ]   = "sltz",
	[Pseudo_Instruction_Kind__SGTZ]   = "sgtz",

	[Pseudo_Instruction_Kind__BEQZ]   = "beqz",
	[Pseudo_Instruction_Kind__BNEZ]   = "bnez",
	[Pseudo_Instruction_Kind__BLEZ]   = "blez",
	[Pseudo_Instruction_Kind__BGEZ]   = "bgez",
	[Pseudo_Instruction_Kind__BLTZ]   = "bltz",
	[Pseudo_Instruction_Kind__BGTZ]   = "bgtz",

	[Pseudo_Instruction_Kind__J]      = "j",
	[Pseudo_Instruction_Kind__JR]     = "jr",
	[Pseudo_Instruction_Kind__RET]    = "ret",
	[Pseudo_Instruction_Kind__CALL]   = "call",
	[Pseudo_Instruction_Kind__TAIL]   = "tail",

	[Pseudo_Instruction_Kind__LA]     = "la",
};

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

// NOTE: On 64-bit, the shift amount grows to 6 bits (since registers are 64 bits wide), so shamt uses bits [25:20] and
// funct7 shrinks to a 6-bit funct6 in bits [31:26].
#define FUNCT6_SLLI                    0x00
#define FUNCT6_SRLI                    0x00
#define FUNCT6_SRAI                    0x10
// #define FUNCT7_SLLI                 0x00
// #define FUNCT7_SRLI                 0x00
// #define FUNCT7_SRAI                 0x20

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


// R-type encoding (32 bits):
// [31:25] funct7 | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_r_encode_m(rd, rs1, rs2, opcode, funct3, funct7)                                           \
	 (((U32)(opcode)       & 0x7F) <<  0) | /* bits  6:0                     */                            \
   	 (((U32)(rd)           & 0x1F) <<  7) | /* bits 11:7                     */                            \
   	 (((U32)(funct3)       & 0x07) << 12) | /* bits 14:12                    */                            \
   	 (((U32)(rs1)          & 0x1F) << 15) | /* bits 19:15                    */                            \
   	 (((U32)(rs2)          & 0x1F) << 20) | /* bits 24:20                    */                            \
   	 (((U32)(funct7)       & 0x7F) << 25)   /* bits 31:25                    */

// I-type encoding (32 bits):
// [31:20] imm[11:0] | [19:15] rs1 | [14:12] funct3 | [11:7] rd | [6:0] opcode
#define instruction_i_encode_m(rd, rs1, imm, opcode, funct3)                                                   \
	(((U32)(opcode)       &  0x7F) <<  0) | /* bits  6:0                     */                            \
    	(((U32)(rd)           &  0x1F) <<  7) | /* bits 11:7                     */                            \
    	(((U32)(funct3)       &  0x07) << 12) | /* bits 14:12                    */                            \
    	(((U32)(rs1)          &  0x1F) << 15) | /* bits 19:15                    */                            \
    	(((U32)(imm)          & 0xFFF) << 20)   /* bits 31:20                    */

#define instruction_i_shift_encode_m(rd, rs1, shamt, opcode, funct3, funct6)                                   \
	instruction_i_encode_m(rd, rs1, ((funct6) << 6) | ((shamt) & 0x3F), opcode, funct3)

#define instruction_i_shift_wide_encode_m(rd, rs1, shamt, opcode, funct3, funct7)                              \
	instruction_i_encode_m(rd, rs1, (funct7 << 5) | (shamt & 0x1F), opcode, funct3)

// S-type encoding (32 bits):
// [31:25] imm[11:5] | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:7] imm[4:0] | [6:0] opcode
#define instruction_s_encode_m(rs2, rs1, imm, opcode, funct3)                                                  \
	(((U32)(opcode)       &  0x7F) <<  0) | /* bits  6:0                     */                            \
    	(((U32)(imm)          &  0x1F) <<  7) | /* bits 11:7                     */                            \
    	(((U32)(funct3)       &  0x07) << 12) | /* bits 14:12                    */                            \
    	(((U32)(rs1)          &  0x1F) << 15) | /* bits 19:15                    */                            \
    	(((U32)(rs2)          &  0x1F) << 20) | /* bits 24:20                    */                            \
    	(((U32)(imm)          & 0xFE0) << 25)   /* bits 31:25                    */

// B-type encoding (32 bits):
// [31] imm[12] | [30:25] imm[10:5] | [24:20] rs2 | [19:15] rs1 | [14:12] funct3 | [11:8] imm[4:1] | [7] imm[11] | [6:0] opcode
#define instruction_b_encode_m(rs2, rs1, imm, opcode, funct3)                                                  \
	(((U32)(opcode)              &  0x7F) <<  0) | /* bits  6:0              */                            \
    	(((U32)((imm) >> 11)         &  0x01) <<  7) | /* bit   7     imm[11]    */                            \
    	(((U32)((imm) >>  1)         &  0x0F) <<  8) | /* bits 11:8   imm[4:1]   */                            \
    	(((U32)(funct3)              &  0x07) << 12) | /* bits 14:12             */                            \
    	(((U32)(rs1)                 &  0x1F) << 15) | /* bits 19:15             */                            \
    	(((U32)(rs2)                 &  0x1F) << 20) | /* bits 24:20             */                            \
    	(((U32)((imm) >>  5)         &  0x3F) << 25) | /* bits 30:25  imm[10:5]  */                            \
    	(((U32)((imm) >> 12)         &  0x01) << 31)   /* bit  31     imm[12]    */

#define instruction_u_encode_m(rd, imm, opcode)                                                                \
	(((U32)(opcode)              &  0x7F) <<  0) | /* bits  6:0              */                            \
	(((U32)(rd)                  &  0x1F) <<  7) | /* bits 11:7              */                            \
	(((U32)(imm)             & 0xFFFFF) << 12)     /* bits 31:12  imm[31:12] */

#define instruction_j_encode_m(rd, imm, opcode)                                                                \
	(((U32)(opcode)              &  0x7F) <<  0) | /* bits  6:0              */                            \
	(((U32)(rd)                  &  0x1F) <<  7) | /* bits 11:7              */                            \
	(((U32)((imm) >> 12)         &  0xFF) << 12) | /* bits 19:12 imm[19:12]  */                            \
	(((U32)((imm) >> 11)         &  0x01) << 20) | /* bit  20     imm[11]    */                            \
	(((U32)((imm) >>  1)         &  0x3FF) << 21) | /* bits 30:21 imm[10:1]  */                            \
	(((U32)((imm) >> 20)         &  0x01) << 31)   /* bit  31     imm[20]    */

// typedef U8 Instruction_Flags;
// enum
// {
//     Instruction_Flags__None                = 0,
//     // Operand transformations for pseudo-instruction encoding:
//     //   Swap_1: swap rs1 and rs2 (e.g., bgt -> blt with swapped ops)
//     //   Swap_2: rs1 -> x0, rs2 -> rs1 (e.g., bgtz -> blt x0, rs, label)
//     Instruction_Flags__Swap_1              = 1 << 0,
//     Instruction_Flags__Swap_2              = 1 << 1,
//     // The assembler attaches R_RISCV_RELAX to this instruction when it
//     // carries a symbol relocation under .option relax. Signals to the
//     // linker that this instruction participates in a relaxable pattern.
//     Instruction_Flags__Relax_Hint          = 1 << 2,
//     // Whether the instruction may be grow or shrink due to relaxation made by the linker or assembler.
//     Instruction_Flags__Expandable          = 1 << 3,
//     // The instruction's immediate can be written with a relocation operator
//     // (%lo, %hi, %pcrel_lo, %pcrel_hi, %tprel_lo, %tprel_hi, etc.).
//     Instruction_Flags__Relocation_Operator = 1 << 4,
// };
//
// typedef struct Instruction_Encoding Instruction_Encoding;
// struct Instruction_Encoding
// {
//     U8 opcode;
//     U8 funct3;
//     U8 funct7;     // also funct6 for RV64 shifts
//     U8 flags;
// };
//
// global const Instruction_Encoding Instruction_Encoding_table[Instruction_Kind__COUNT] =
// {
//     // RV64I — U-type
//     // lui/auipc take %hi/%pcrel_hi; both participate in relaxable address pairs.
//     [Instruction_Kind__LUI]    = { OPCODE_LUI,      0,            0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__AUIPC]  = { OPCODE_AUIPC,    0,            0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//
//     // RV64I — J-type
//     // jal has R_RISCV_JAL for symbolic targets but no %-operator syntax.
//     // It's the target of branch expansion (branches expand to jal form),
//     // and `call`/`tail` pseudos can relax to jal — but jal itself doesn't
//     // further expand or shrink.
//     [Instruction_Kind__JAL]    = { OPCODE_JAL,      0,            0,             0                                                                      },
//
//     // RV64I — I-type (JALR)
//     // jalr pairs with auipc in relaxable call/tail sequences; takes %pcrel_lo.
//     [Instruction_Kind__JALR]   = { OPCODE_JALR,     FUNCT3_JALR,  0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//
//     // RV64I — B-type
//     // Branches have R_RISCV_BRANCH for resolvable backward in-range targets,
//     // but the assembler expands them to jal-based long form when the target
//     // isn't provably reachable. No %-operator syntax.
//     // Linker does not shrink branches -> no Instruction_Flags__Relax_Hint hint.
//     [Instruction_Kind__BEQ]    = { OPCODE_BRANCH,   FUNCT3_BEQ,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BNE]    = { OPCODE_BRANCH,   FUNCT3_BNE,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BLT]    = { OPCODE_BRANCH,   FUNCT3_BLT,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BGE]    = { OPCODE_BRANCH,   FUNCT3_BGE,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BLTU]   = { OPCODE_BRANCH,   FUNCT3_BLTU,  0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BGEU]   = { OPCODE_BRANCH,   FUNCT3_BGEU,  0,             Instruction_Flags__Expandable                                          },
//
//     // RV64I — Loads
//     // Load immediates take %lo/%pcrel_lo/%tprel_lo; pair with lui/auipc for
//     // address materialization, relaxable.
//     [Instruction_Kind__LB]     = { OPCODE_LOAD,     FUNCT3_LB,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__LH]     = { OPCODE_LOAD,     FUNCT3_LH,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__LW]     = { OPCODE_LOAD,     FUNCT3_LW,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__LD]     = { OPCODE_LOAD,     FUNCT3_LD,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__LBU]    = { OPCODE_LOAD,     FUNCT3_LBU,   0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__LHU]    = { OPCODE_LOAD,     FUNCT3_LHU,   0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__LWU]    = { OPCODE_LOAD,     FUNCT3_LWU,   0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//
//     // RV64I — Stores
//     // Same as loads but with R_RISCV_LO12_S / R_RISCV_PCREL_LO12_S.
//     [Instruction_Kind__SB]     = { OPCODE_STORE,    FUNCT3_SB,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__SH]     = { OPCODE_STORE,    FUNCT3_SH,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__SW]     = { OPCODE_STORE,    FUNCT3_SW,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__SD]     = { OPCODE_STORE,    FUNCT3_SD,    0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//
//     // RV64I — I-type ALU
//     // addi is the canonical low-12 target for %lo/%pcrel_lo/%tprel_lo.
//     // Other I-type ALU ops (slti, xori, ori, andi, sltiu) accept the same
//     // operators syntactically in GAS, though semantically unusual.
//     [Instruction_Kind__ADDI]   = { OPCODE_I_TYPE,   FUNCT3_ADDI,  0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__SLTI]   = { OPCODE_I_TYPE,   FUNCT3_SLTI,  0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__SLTIU]  = { OPCODE_I_TYPE,   FUNCT3_SLTIU, 0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__XORI]   = { OPCODE_I_TYPE,   FUNCT3_XORI,  0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__ORI]    = { OPCODE_I_TYPE,   FUNCT3_ORI,   0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//     [Instruction_Kind__ANDI]   = { OPCODE_I_TYPE,   FUNCT3_ANDI,  0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//
//     // RV64I — Shift immediates (shamt, no symbol ever)
//     [Instruction_Kind__SLLI]   = { OPCODE_I_TYPE,   FUNCT3_SLLI,  FUNCT6_SLLI,   0                                                                      },
//     [Instruction_Kind__SRLI]   = { OPCODE_I_TYPE,   FUNCT3_SRLI,  FUNCT6_SRLI,   0                                                                      },
//     [Instruction_Kind__SRAI]   = { OPCODE_I_TYPE,   FUNCT3_SRAI,  FUNCT6_SRAI,   0                                                                      },
//
//     // RV64I — R-type (no immediate field, no relocation possible)
//     [Instruction_Kind__ADD]    = { OPCODE_R_TYPE,   FUNCT3_ADD,   FUNCT7_ADD,    0                                                                      },
//     [Instruction_Kind__SUB]    = { OPCODE_R_TYPE,   FUNCT3_SUB,   FUNCT7_SUB,    0                                                                      },
//     [Instruction_Kind__SLL]    = { OPCODE_R_TYPE,   FUNCT3_SLL,   FUNCT7_SLL,    0                                                                      },
//     [Instruction_Kind__SLT]    = { OPCODE_R_TYPE,   FUNCT3_SLT,   FUNCT7_SLT,    0                                                                      },
//     [Instruction_Kind__SLTU]   = { OPCODE_R_TYPE,   FUNCT3_SLTU,  FUNCT7_SLTU,   0                                                                      },
//     [Instruction_Kind__XOR]    = { OPCODE_R_TYPE,   FUNCT3_XOR,   FUNCT7_XOR,    0                                                                      },
//     [Instruction_Kind__SRL]    = { OPCODE_R_TYPE,   FUNCT3_SRL,   FUNCT7_SRL,    0                                                                      },
//     [Instruction_Kind__SRA]    = { OPCODE_R_TYPE,   FUNCT3_SRA,   FUNCT7_SRA,    0                                                                      },
//     [Instruction_Kind__OR]     = { OPCODE_R_TYPE,   FUNCT3_OR,    FUNCT7_OR,     0                                                                      },
//     [Instruction_Kind__AND]    = { OPCODE_R_TYPE,   FUNCT3_AND,   FUNCT7_AND,    0                                                                      },
//
//     // RV64-W — I-type W
//     // addiw accepts %-operators syntactically (GAS attaches the relocation
//     // mechanically), even though the resulting 32-bit arithmetic is rarely
//     // useful for address computation. Mirrors GAS behavior.
//     [Instruction_Kind__ADDIW]  = { OPCODE_I_TYPE_W, FUNCT3_ADDIW, 0,             Instruction_Flags__Relax_Hint | Instruction_Flags__Relocation_Operator },
//
//     // RV64-W — Shift immediates W (shamt, no symbol)
//     [Instruction_Kind__SLLIW]  = { OPCODE_I_TYPE_W, FUNCT3_SLLIW, FUNCT7_SLLIW,  0                                                                      },
//     [Instruction_Kind__SRLIW]  = { OPCODE_I_TYPE_W, FUNCT3_SRLIW, FUNCT7_SRLIW,  0                                                                      },
//     [Instruction_Kind__SRAIW]  = { OPCODE_I_TYPE_W, FUNCT3_SRAIW, FUNCT7_SRAIW,  0                                                                      },
//
//     // RV64-W — R-type W (no immediate)
//     [Instruction_Kind__ADDW]   = { OPCODE_R_TYPE_W, FUNCT3_ADDW,  FUNCT7_ADDW,   0                                                                      },
//     [Instruction_Kind__SUBW]   = { OPCODE_R_TYPE_W, FUNCT3_SUBW,  FUNCT7_SUBW,   0                                                                      },
//     [Instruction_Kind__SLLW]   = { OPCODE_R_TYPE_W, FUNCT3_SLLW,  FUNCT7_SLLW,   0                                                                      },
//     [Instruction_Kind__SRLW]   = { OPCODE_R_TYPE_W, FUNCT3_SRLW,  FUNCT7_SRLW,   0                                                                      },
//     [Instruction_Kind__SRAW]   = { OPCODE_R_TYPE_W, FUNCT3_SRAW,  FUNCT7_SRAW,   0                                                                      },
//
//     // Pseudo — I-type mapped (register-register moves, not symbol-accepting)
//     [Instruction_Kind__MV]     = { OPCODE_I_TYPE,   FUNCT3_ADDI,  0,             0                                                                      },
//     [Instruction_Kind__NOT]    = { OPCODE_I_TYPE,   FUNCT3_XORI,  0,             0                                                                      },
//     [Instruction_Kind__SEXT_W] = { OPCODE_I_TYPE_W, FUNCT3_ADDIW, 0,             0                                                                      },
//     [Instruction_Kind__SEQZ]   = { OPCODE_I_TYPE,   FUNCT3_SLTIU, 0,             0                                                                      },
//     [Instruction_Kind__JR]     = { OPCODE_JALR,     FUNCT3_JALR,  0,             0                                                                      },
//
//     // Pseudo — R-type mapped (no immediate)
//     [Instruction_Kind__NEG]    = { OPCODE_R_TYPE,   FUNCT3_SUB,   FUNCT7_SUB,    Instruction_Flags__Swap_1                                              },
//     [Instruction_Kind__NEGW]   = { OPCODE_R_TYPE_W, FUNCT3_SUBW,  FUNCT7_SUBW,   Instruction_Flags__Swap_1                                              },
//     [Instruction_Kind__SNEZ]   = { OPCODE_R_TYPE,   FUNCT3_SLTU,  FUNCT7_SLTU,   Instruction_Flags__Swap_1                                              },
//     [Instruction_Kind__SGTZ]   = { OPCODE_R_TYPE,   FUNCT3_SLT,   FUNCT7_SLT,    Instruction_Flags__Swap_1                                              },
//     [Instruction_Kind__SLTZ]   = { OPCODE_R_TYPE,   FUNCT3_SLT,   FUNCT7_SLT,    0                                                                      },
//
//     // Pseudo — B-type mapped (expand to jal form when out of branch range,
//     // same as their real-instruction counterparts)
//     [Instruction_Kind__BEQZ]   = { OPCODE_BRANCH,   FUNCT3_BEQ,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BNEZ]   = { OPCODE_BRANCH,   FUNCT3_BNE,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BLTZ]   = { OPCODE_BRANCH,   FUNCT3_BLT,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BGEZ]   = { OPCODE_BRANCH,   FUNCT3_BGE,   0,             Instruction_Flags__Expandable                                          },
//     [Instruction_Kind__BLEZ]   = { OPCODE_BRANCH,   FUNCT3_BGE,   0,             Instruction_Flags__Expandable | Instruction_Flags__Swap_2              },
//     [Instruction_Kind__BGTZ]   = { OPCODE_BRANCH,   FUNCT3_BLT,   0,             Instruction_Flags__Expandable | Instruction_Flags__Swap_2              },
//     [Instruction_Kind__BGT]    = { OPCODE_BRANCH,   FUNCT3_BLT,   0,             Instruction_Flags__Expandable | Instruction_Flags__Swap_1              },
//     [Instruction_Kind__BLE]    = { OPCODE_BRANCH,   FUNCT3_BGE,   0,             Instruction_Flags__Expandable | Instruction_Flags__Swap_1              },
//     [Instruction_Kind__BGTU]   = { OPCODE_BRANCH,   FUNCT3_BLTU,  0,             Instruction_Flags__Expandable | Instruction_Flags__Swap_1              },
//     [Instruction_Kind__BLEU]   = { OPCODE_BRANCH,   FUNCT3_BGEU,  0,             Instruction_Flags__Expandable | Instruction_Flags__Swap_1              },
//
//     // Pseudo — J-type mapped
//     // j is jal x0, label — same properties as jal itself.
//     [Instruction_Kind__J]      = { OPCODE_JAL,      0,            0,             0                                                                      },
// };

#define ENCODING_NOP    0x00000013
#define ENCODING_RET    0x00008067
#define ENCODING_ECALL  0x00000073
#define ENCODING_EBREAK 0x00100073
#define ENCODING_PAUSE  0x0100000F
#define ENCODING_TSO    0x8330000F


/* RV fields.  */

#define OP_MASK_OP		0x7f
#define OP_SH_OP		0
#define OP_MASK_RS2		0x1f
#define OP_SH_RS2		20
#define OP_MASK_RS1		0x1f
#define OP_SH_RS1		15
#define OP_MASK_RS3		0x1fU
#define OP_SH_RS3		27
#define OP_MASK_RD		0x1f
#define OP_SH_RD		7
#define OP_MASK_SHAMT		0x3f
#define OP_SH_SHAMT		20
#define OP_MASK_SHAMTW		0x1f
#define OP_SH_SHAMTW		20
#define OP_MASK_RM		0x7
#define OP_SH_RM		12
#define OP_MASK_PRED		0xf
#define OP_SH_PRED		24
#define OP_MASK_SUCC		0xf
#define OP_SH_SUCC		20
#define OP_MASK_AQ		0x1
#define OP_SH_AQ		26
#define OP_MASK_RL		0x1
#define OP_SH_RL		25

#define OP_MASK_CSR		0xfffU
#define OP_SH_CSR		20

#define OP_MASK_FUNCT3		0x7
#define OP_SH_FUNCT3		12
#define OP_MASK_FUNCT7		0x7fU
#define OP_SH_FUNCT7		25
#define OP_MASK_FUNCT2		0x3
#define OP_SH_FUNCT2		25

#define MATCH_ADDI 0x13
#define MASK_ADDI  0x707f

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

typedef enum RISCV_Instruction_Class
{
	RISCV_Instruction_Class__None,
	RISCV_Instruction_Class__I,

	RISCV_Instruction_Class__COUNT,

}
RISCV_Instruction_Class;

#define OP_arguments_m(...) ((U16[]){ __VA_ARGS__, 0 })

typedef U16 OP_Argument;
enum
{
	OP_Argument__None = 0,
	OP_Argument__Comma,
	OP_Argument__RD,
	OP_Argument__RS1,
	OP_Argument__RS2,
	OP_Argument__RS3,
	OP_Argument__Immediate_I,
	OP_Argument__Immediate_S,
	OP_Argument__Offset,
	OP_Argument__COUNT,
};

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
	U32 mask;

	// A function to determine if a word corresponds to this instruction. Usually, this computes ((word & mask) == match).
	B32 (*match_function) (const RISCV_Opcode *opcode, U32 word);

	// For a macro, this is INSN_MACRO.  Otherwise, it is a collection of bits describing the instruction, notably
	// any relevant hazard information.
	U64 info;
};

static B32
match_opcode (const RISCV_Opcode *opcode, U32 instruction)
{
	B32 result = ((instruction ^ opcode->match) & opcode->mask) == 0;
	return result;
}

global const RISCV_Opcode RISCV_Opcode__table[] =
{
	{ "addi", HASH_addi, 0, 0, OP_arguments_m(OP_Argument__RD, OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Immediate_I), MATCH_ADDI, MASK_ADDI, match_opcode, 0 }
};



internal const RISCV_Opcode *
RISCV_Opcode__table_find(U32 instruction_hash)
{
	U32 count = array_count_m(RISCV_Opcode__table);
	U32 index = 0;

	const RISCV_Opcode *result = 0;
	for (;;)
	{
		B32 break_should = result || index >= count;
		if (break_should)
		{
			break;
		}

		const RISCV_Opcode *opcode = &RISCV_Opcode__table[index];
		result = opcode->hash == instruction_hash ? opcode : 0;

		index += 1;
	}

	return result;
}

// Information about an instruction, including its format, operands
// and fixups.
typedef struct RISCV_Instruction RISCV_Instruction;
struct RISCV_Instruction
{
  const RISCV_Opcode *opcode;

  // The long encoded instruction bits ([0] is non-zero on a long opcode).  */
  // char insn_long_opcode[RISCV_MAX_INSN_LEN];

  // The frag that contains the instruction
  Fragment *fragment;

  U32 encoding;
  // The offset into FRAG of the first instruction byte.
  U32 offset;

  // The relocations associated with the instruction, if any.
  Fixup *fixup;
};

internal RISCV_Instruction
RISCV_Instruction__create(const RISCV_Opcode *opcode)
{
	RISCV_Instruction result = {0};
	result.opcode   = opcode;
	result.encoding = opcode->match;
	return result;
}


#endif // LANGUAGE_INSTRUCTION_H
