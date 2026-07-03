#define A_NONE                OP_arguments_m(OP_Argument__None)

#define A_RS1                 OP_arguments_m(OP_Argument__RS1)
#define A_RS1_IMM_I           OP_arguments_m(OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Immediate_I)

#define A_RD_IMM_L            OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Immediate_Large)
#define A_RD_IMM_I            OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Immediate_I)
#define A_RD_IMM_U            OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Immediate_U)
#define A_RD_RS1              OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1)
#define A_RD_RS1_RS2          OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__RS2)
#define A_RD_RS1_IMM          OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Immediate_I)

#define A_RD_OFF_LP_RS1_RP    OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Offset_Load, OP_Argument__Parenthesis_Left, OP_Argument__RS1, OP_Argument__Parenthesis_Right)
#define A_OFF_LP_RS1_RP       OP_arguments_m(                                      OP_Argument__Offset_Load, OP_Argument__Parenthesis_Left, OP_Argument__RS1, OP_Argument__Parenthesis_Right)

#define A_RD_OFF              OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Offset_PC_Relative_20)
#define A_RS1_OFF             OP_arguments_m(OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Offset_PC_Relative_12)
#define A_RS2_OFF             OP_arguments_m(OP_Argument__RS2, OP_Argument__Comma, OP_Argument__Offset_PC_Relative_12)

#define A_RS1_RS2_OFF         OP_arguments_m(OP_Argument__RS1, OP_Argument__Comma, OP_Argument__RS2, OP_Argument__Comma, OP_Argument__Offset_PC_Relative_12)
#define A_RS2_RS1_OFF         OP_arguments_m(OP_Argument__RS2, OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Offset_PC_Relative_12)

#define A_OFF_20              OP_arguments_m(OP_Argument__Offset_PC_Relative_20)
#define A_CALL                OP_arguments_m(OP_Argument__Call_Expression)

#define RV_IC_I RISCV_Instruction_Class__I

// NOTE: the empty opcode can be distinguished by the zero hash.
global const RISCV_Opcode RISCV_Opcode__table[] =
{
// Base I instructions.
{ "auipc",  HASH_auipc,  0, RV_IC_I, A_RD_IMM_U,           MATCH_AUIPC,                             MASK_AUIPC,                          match_opcode,         0                              },
{ "lui",    HASH_lui,    0, RV_IC_I, A_RD_IMM_U,           MATCH_LUI,                               MASK_LUI,                            match_opcode,         0                              },

{ "jal",    HASH_jal,    0, RV_IC_I, A_OFF_20,             MATCH_JAL|(X_RA << OP_SH_RD),            MASK_JAL|MASK_RD,                    match_opcode,         0                              },
{ "jal",    HASH_jal,    0, RV_IC_I, A_RD_OFF,             MATCH_JAL,                               MASK_JAL,                            match_opcode,         0                              },

{ "jalr",   HASH_jalr,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_JALR,                              MASK_JALR,                           match_opcode,         0                              },

{ "lb",     HASH_lb,     0, RV_IC_I, A_RD_OFF_LP_RS1_RP,   MATCH_LB,                                MASK_LB,                             match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lb",     HASH_lb,     0, RV_IC_I, A_RD_RS1,             MATCH_LB,                                MASK_LB,                             match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lbu",    HASH_lbu,    0, RV_IC_I, A_RD_OFF_LP_RS1_RP,   MATCH_LBU,                               MASK_LBU,                            match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lbu",    HASH_lbu,    0, RV_IC_I, A_RD_RS1,             MATCH_LBU,                               MASK_LBU,                            match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lh",     HASH_lh,     0, RV_IC_I, A_RD_OFF_LP_RS1_RP,   MATCH_LH,                                MASK_LH,                             match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lh",     HASH_lh,     0, RV_IC_I, A_RD_RS1,             MATCH_LH,                                MASK_LH,                             match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lhu",    HASH_lhu,    0, RV_IC_I, A_RD_OFF_LP_RS1_RP,   MATCH_LHU,                               MASK_LHU,                            match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lhu",    HASH_lhu,    0, RV_IC_I, A_RD_RS1,             MATCH_LHU,                               MASK_LHU,                            match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lw",     HASH_lw,     0, RV_IC_I, A_RD_OFF_LP_RS1_RP,   MATCH_LW,                                MASK_LW,                             match_opcode,         INSN_DREF|INSN_4_BYTE          },
{ "lw",     HASH_lw,     0, RV_IC_I, A_RD_RS1,             MATCH_LW,                                MASK_LW,                             match_opcode,         INSN_DREF|INSN_4_BYTE          },
// TODO: add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ "addi",   HASH_addi,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ADDI,                              MASK_ADDI,                           match_opcode,         0                              },
{ "addiw",  HASH_addiw,  0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ADDIW,                             MASK_ADDIW,                          match_opcode,         0                              },
{ "slti",   HASH_slti,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_SLTI,                              MASK_SLTI,                           match_opcode,         0                              },
{ "sltiu",  HASH_sltiu,  0, RV_IC_I, A_RD_RS1_IMM,         MATCH_SLTIU,                             MASK_SLTIU,                          match_opcode,         0                              },
{ "xori",   HASH_xori,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_XORI,                              MASK_XORI,                           match_opcode,         0                              },
{ "ori",    HASH_ori,    0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ORI,                               MASK_ORI,                            match_opcode,         0                              },
{ "andi",   HASH_andi,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ANDI,                              MASK_ANDI,                           match_opcode,         0                              },


{ "add",    HASH_add,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_ADD,                               MASK_ADD,                            match_opcode,         0                              },
{ "sub",    HASH_sub,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_SUB,                               MASK_SUB,                            match_opcode,         0                              },
{ "sll",    HASH_sll,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_SLL,                               MASK_SLL,                            match_opcode,         0                              },
{ "slt",    HASH_slt,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_SLT,                               MASK_SLT,                            match_opcode,         0                              },
{ "sltu",   HASH_sltu,   0, RV_IC_I, A_RD_RS1_RS2,         MATCH_SLTU,                              MASK_SLTU,                           match_opcode,         0                              },
{ "xor",    HASH_xor,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_XOR,                               MASK_XOR,                            match_opcode,         0                              },
{ "srl",    HASH_srl,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_SRL,                               MASK_SRL,                            match_opcode,         0                              },
{ "sra",    HASH_sra,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_SRA,                               MASK_SRA,                            match_opcode,         0                              },
{ "or",     HASH_or,     0, RV_IC_I, A_RD_RS1_RS2,         MATCH_OR,                                MASK_OR,                             match_opcode,         0                              },
{ "and",    HASH_and,    0, RV_IC_I, A_RD_RS1_RS2,         MATCH_AND,                               MASK_AND,                            match_opcode,         0                              },

{ "beq",    HASH_beq,    0, RV_IC_I, A_RS1_RS2_OFF,        MATCH_BEQ,                               MASK_BEQ,                            match_opcode,         INSN_CONDBRANCH                },
{ "bne",    HASH_bne,    0, RV_IC_I, A_RS1_RS2_OFF,        MATCH_BNE,                               MASK_BNE,                            match_opcode,         INSN_CONDBRANCH                },
{ "blt",    HASH_blt,    0, RV_IC_I, A_RS1_RS2_OFF,        MATCH_BLT,                               MASK_BLT,                            match_opcode,         INSN_CONDBRANCH                },
{ "bge",    HASH_bge,    0, RV_IC_I, A_RS1_RS2_OFF,        MATCH_BGE,                               MASK_BGE,                            match_opcode,         INSN_CONDBRANCH                },
{ "bltu",   HASH_bltu,   0, RV_IC_I, A_RS1_RS2_OFF,        MATCH_BLTU,                              MASK_BLTU,                           match_opcode,         INSN_CONDBRANCH                },
{ "bgeu",   HASH_bgeu,   0, RV_IC_I, A_RS1_RS2_OFF,        MATCH_BGEU,                              MASK_BGEU,                           match_opcode,         INSN_CONDBRANCH                },

// Pseudo-instructions (incomplete)
{ "jr",     HASH_jr,     0, RV_IC_I, A_RS1,                MATCH_JALR,                              MASK_JALR|MASK_RD|MASK_IMM,          match_opcode,         INSN_ALIAS|INSN_BRANCH         },
{ "jr",     HASH_jr,     0, RV_IC_I, A_OFF_LP_RS1_RP,      MATCH_JALR,                              MASK_JALR|MASK_RD,                   match_opcode,         INSN_ALIAS|INSN_BRANCH         },
{ "jr",     HASH_jr,     0, RV_IC_I, A_RS1_IMM_I,          MATCH_JALR,                              MASK_JALR|MASK_RD,                   match_opcode,         INSN_ALIAS|INSN_BRANCH         },

{ "call",   HASH_call,   0, RV_IC_I, A_CALL,               (X_RA << OP_SH_RS1)|(X_RA << OP_SH_RD),  M_CALL,                              0,                    INSN_MACRO                     },
{ "li",     HASH_li,     0, RV_IC_I, A_RD_IMM_I,           MATCH_ADDI,                              MASK_ADDI|MASK_RS1,                  match_opcode,         INSN_ALIAS                     },
{ "li",     HASH_li,     0, RV_IC_I, A_RD_IMM_L,           0,                                       M_LI,                                0,                    INSN_MACRO                     },

{ "nop",    HASH_nop,    0, RV_IC_I, A_NONE,               MATCH_ADDI,                              MASK_ADDI|MASK_RD|MASK_RS1|MASK_IMM, match_opcode,         INSN_ALIAS                     },
{ "mv",     HASH_mv,     0, RV_IC_I, A_RD_RS1,             MATCH_ADDI,                              MASK_ADDI|MASK_IMM,                  match_opcode,         INSN_ALIAS                     },

{ "beqz",   HASH_beqz,   0, RV_IC_I, A_RS1_OFF,            MATCH_BEQ,                               MASK_BEQ|MASK_RS2,                   match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "blez",   HASH_blez,   0, RV_IC_I, A_RS2_OFF,            MATCH_BGE,                               MASK_BGE|MASK_RS1,                   match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bgez",   HASH_bgez,   0, RV_IC_I, A_RS1_OFF,            MATCH_BGE,                               MASK_BGE|MASK_RS2,                   match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "ble",    HASH_ble,    0, RV_IC_I, A_RS2_RS1_OFF,        MATCH_BGE,                               MASK_BGE,                            match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bltz",   HASH_bltz,   0, RV_IC_I, A_RS1_OFF,            MATCH_BLT,                               MASK_BLT|MASK_RS2,                   match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bgtz",   HASH_bgtz,   0, RV_IC_I, A_RS2_OFF,            MATCH_BLT,                               MASK_BLT|MASK_RS1,                   match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bleu",   HASH_bleu,   0, RV_IC_I, A_RS2_RS1_OFF,        MATCH_BGEU,                              MASK_BGEU,                           match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bgt",    HASH_bgt,    0, RV_IC_I, A_RS2_RS1_OFF,        MATCH_BLT,                               MASK_BLT,                            match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bgtu",   HASH_bgtu,   0, RV_IC_I, A_RS2_RS1_OFF,        MATCH_BLTU,                              MASK_BLTU,                           match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },
{ "bnez",   HASH_bnez,   0, RV_IC_I, A_RS1_OFF,            MATCH_BNE,                               MASK_BNE|MASK_RS2,                   match_opcode,         INSN_ALIAS|INSN_CONDBRANCH     },

{ "pause",  HASH_pause,  0, RV_IC_I, A_NONE,               MATCH_PAUSE,                             MASK_PAUSE,                          match_opcode,         0                              },
{ "ecall",  HASH_ecall,  0, RV_IC_I, A_NONE,               MATCH_ECALL,                             MASK_ECALL,                          match_opcode,         0                              },
{ "ebreak", HASH_ebreak, 0, RV_IC_I, A_NONE,               MATCH_EBREAK,                            MASK_EBREAK,                         match_opcode,         0                              },

{ "",       0,           0, 0,       A_NONE,               0,                                       0,                                   0,                    0                              }
};

// TODO: undef the helper macros defined above.

// TODO: for now this is dumb enough and works.
//
// Returns empty opcode if not found.
internal const RISCV_Opcode *
RISCV_Opcode__table_find(U32 instruction_hash)
{
	U32 count = array_count_m(RISCV_Opcode__table);
	U32 index = 0;

	B32 match = 0;

	const RISCV_Opcode *result = 0;
	for (;;)
	{
		B32 break_should = match || index >= count;
		if (break_should)
		{
			break;
		}

		result = &RISCV_Opcode__table[index];
		match = result->hash == instruction_hash ? 1 : 0;

		index += 1;
	}

	assert_always_m(result);
	assert_always_m(match || result->hash == 0);

	return result;
}

internal U8
RISCV_instruction_size(U32 encoding)
{
	B32 bit_32_encoding = (encoding & 0x1f) != 0x1f;
	assert_always_m(bit_32_encoding && "only 32-bit instruction supported");
	U8 length = 4;
	return length;
}
