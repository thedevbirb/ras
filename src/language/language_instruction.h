#ifndef LANGUAGE_INSTRUCTION_H
#define LANGUAGE_INSTRUCTION_H

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

	Instruction_Kind__CSRRW,
	Instruction_Kind__CSRRS,
	Instruction_Kind__CSRRC,
	Instruction_Kind__CSRRWI,
	Instruction_Kind__CSRRSI,
	Instruction_Kind__CSRRCI,

	// M extension
	Instruction_Kind__MUL,
	Instruction_Kind__MULH,
	Instruction_Kind__MULHSU,
	Instruction_Kind__MULHU,
	Instruction_Kind__DIV,
	Instruction_Kind__DIVU,
	Instruction_Kind__REM,
	Instruction_Kind__REMU,

	Instruction_Kind__MULW,
	Instruction_Kind__DIVW,
	Instruction_Kind__DIVUW,
	Instruction_Kind__REMW,
	Instruction_Kind__REMUW,

	// A extension
	Instruction_Kind__LR_W,
	Instruction_Kind__SC_W,
	Instruction_Kind__AMOSWAP_W,
	Instruction_Kind__AMOADD_W,
	Instruction_Kind__AMOXOR_W,
	Instruction_Kind__AMOAND_W,
	Instruction_Kind__AMOOR_W,
	Instruction_Kind__AMOMIN_W,
	Instruction_Kind__AMOMAX_W,
	Instruction_Kind__AMOMINU_W,
	Instruction_Kind__AMOMAXU_W,

	Instruction_Kind__LR_D,
	Instruction_Kind__SC_D,
	Instruction_Kind__AMOSWAP_D,
	Instruction_Kind__AMOADD_D,
	Instruction_Kind__AMOXOR_D,
	Instruction_Kind__AMOAND_D,
	Instruction_Kind__AMOOR_D,
	Instruction_Kind__AMOMIN_D,
	Instruction_Kind__AMOMAX_D,
	Instruction_Kind__AMOMINU_D,
	Instruction_Kind__AMOMAXU_D,

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

	[Instruction_Kind__CSRRW] = "csrrw",
	[Instruction_Kind__CSRRS] = "csrrs",
	[Instruction_Kind__CSRRC] = "csrrc",
	[Instruction_Kind__CSRRWI] = "csrrwi",
	[Instruction_Kind__CSRRSI] = "csrrsi",
	[Instruction_Kind__CSRRCI] = "csrrci",

	[Instruction_Kind__MUL] = "mul",
	[Instruction_Kind__MULH] = "mulh",
	[Instruction_Kind__MULHSU] = "mulhsu",
	[Instruction_Kind__MULHU] = "mulhu",
	[Instruction_Kind__DIV] = "div",
	[Instruction_Kind__DIVU] = "divu",
	[Instruction_Kind__REM] = "rem",
	[Instruction_Kind__REMU] = "remu",

	[Instruction_Kind__MULW] = "mulw",
	[Instruction_Kind__DIVW] = "divw",
	[Instruction_Kind__DIVUW] = "divuw",
	[Instruction_Kind__REMW] = "remw",
	[Instruction_Kind__REMUW] = "remuw",

	[Instruction_Kind__LR_W] = "lr.w",
	[Instruction_Kind__SC_W] = "sc.w",
	[Instruction_Kind__AMOSWAP_W] = "amoswap.w",
	[Instruction_Kind__AMOADD_W] = "amoadd.w",
	[Instruction_Kind__AMOXOR_W] = "amoxor.w",
	[Instruction_Kind__AMOAND_W] = "amoand.w",
	[Instruction_Kind__AMOOR_W] = "amoor.w",
	[Instruction_Kind__AMOMIN_W] = "amomin.w",
	[Instruction_Kind__AMOMAX_W] = "amomax.w",
	[Instruction_Kind__AMOMINU_W] = "amominu.w",
	[Instruction_Kind__AMOMAXU_W] = "amomaxu.w",

	[Instruction_Kind__LR_D] = "lr.d",
	[Instruction_Kind__SC_D] = "sc.d",
	[Instruction_Kind__AMOSWAP_D] = "amoswap.d",
	[Instruction_Kind__AMOADD_D] = "amoadd.d",
	[Instruction_Kind__AMOXOR_D] = "amoxor.d",
	[Instruction_Kind__AMOAND_D] = "amoand.d",
	[Instruction_Kind__AMOOR_D] = "amoor.d",
	[Instruction_Kind__AMOMIN_D] = "amomin.d",
	[Instruction_Kind__AMOMAX_D] = "amomax.d",
	[Instruction_Kind__AMOMINU_D] = "amominu.d",
	[Instruction_Kind__AMOMAXU_D] = "amomaxu.d",
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
	[Pseudo_Instruction_Kind__None] = "",

	[Pseudo_Instruction_Kind__NOP]   = "nop",
	[Pseudo_Instruction_Kind__LI]    = "li",
	[Pseudo_Instruction_Kind__MV]    = "mv",
	[Pseudo_Instruction_Kind__NOT]   = "not",
	[Pseudo_Instruction_Kind__NEG]   = "neg",
	[Pseudo_Instruction_Kind__NEGW]  = "negw",
	[Pseudo_Instruction_Kind__SEXT_W]= "sext.w",

	[Pseudo_Instruction_Kind__SEQZ]  = "seqz",
	[Pseudo_Instruction_Kind__SNEZ]  = "snez",
	[Pseudo_Instruction_Kind__SLTZ]  = "sltz",
	[Pseudo_Instruction_Kind__SGTZ]  = "sgtz",

	[Pseudo_Instruction_Kind__BEQZ]  = "beqz",
	[Pseudo_Instruction_Kind__BNEZ]  = "bnez",
	[Pseudo_Instruction_Kind__BLEZ]  = "blez",
	[Pseudo_Instruction_Kind__BGEZ]  = "bgez",
	[Pseudo_Instruction_Kind__BLTZ]  = "bltz",
	[Pseudo_Instruction_Kind__BGTZ]  = "bgtz",

	[Pseudo_Instruction_Kind__J]     = "j",
	[Pseudo_Instruction_Kind__JR]    = "jr",
	[Pseudo_Instruction_Kind__RET]   = "ret",
	[Pseudo_Instruction_Kind__CALL]  = "call",
	[Pseudo_Instruction_Kind__TAIL]  = "tail",

	[Pseudo_Instruction_Kind__LA]    = "la",
};

#endif // LANGUAGE_INSTRUCTION_H

