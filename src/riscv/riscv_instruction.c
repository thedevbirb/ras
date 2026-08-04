// NOTE: the empty opcode can be distinguished by the zero hash.
global const RISCV_Opcode RISCV_Opcode__table[] =
{
// Base I instructions.
{ String8__inline_m("auipc"),  OPC__I, 0, HASH_auipc, MATCH_AUIPC, MASK_AUIPC, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Immediate(OPF_I__U)), match_opcode },
{ String8__inline_m("lui"),    OPC__I, 0, HASH_lui,   MATCH_LUI,  MASK_LUI,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Immediate(OPF_I__U)), match_opcode },

// NOTE: important here to go from more specific to less specific.
{ String8__inline_m("jal"),    OPC__I, 0, HASH_jal,  MATCH_JAL,                    MASK_JAL,         OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Jal)),                                match_opcode },
{ String8__inline_m("jal"),    OPC__I, 0, HASH_jal,  MATCH_JAL|(X_RA << OP_SH_RD), MASK_JAL|MASK_RD, OP_m(OP_Offset(OPF_O__Jal)),                                                            match_opcode },
{ String8__inline_m("jalr"),   OPC__I, 0, HASH_jalr, MATCH_JALR,                   MASK_JALR,        OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },

{ String8__inline_m("lb"),     OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lb,  MATCH_LB,  MASK_LB,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lb"),     OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lb,  MATCH_LB,  MASK_LB,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lbu"),    OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lbu, MATCH_LBU, MASK_LBU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lbu"),    OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lbu, MATCH_LBU, MASK_LBU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lh"),     OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lh,  MATCH_LH,  MASK_LH,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lh"),     OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lh,  MATCH_LH,  MASK_LH,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lhu"),    OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lhu, MATCH_LHU, MASK_LHU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lhu"),    OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lhu, MATCH_LHU, MASK_LHU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lw"),     OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lw,  MATCH_LW,  MASK_LW,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lw"),     OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lw,  MATCH_LW,  MASK_LW,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("ld"),     OPC__I, INSN_DREF|INSN_8_BYTE, HASH_ld,  MATCH_LD,  MASK_LD,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("ld"),     OPC__I, INSN_DREF|INSN_8_BYTE, HASH_ld,  MATCH_LD,  MASK_LD,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ String8__inline_m("sb"),     OPC__I, INSN_DREF|INSN_1_BYTE, HASH_sb, MATCH_SB, MASK_SB, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("sh"),     OPC__I, INSN_DREF|INSN_2_BYTE, HASH_sh, MATCH_SH, MASK_SH, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("sw"),     OPC__I, INSN_DREF|INSN_4_BYTE, HASH_sw, MATCH_SW, MASK_SW, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("sd"),     OPC__I, INSN_DREF|INSN_8_BYTE, HASH_sd, MATCH_SD, MASK_SD, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ String8__inline_m("addi"),   OPC__I, 0, HASH_addi,  MATCH_ADDI,  MASK_ADDI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("addiw"),  OPC__I, 0, HASH_addiw, MATCH_ADDIW, MASK_ADDIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("slti"),   OPC__I, 0, HASH_slti,  MATCH_SLTI,  MASK_SLTI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("sltiu"),  OPC__I, 0, HASH_sltiu, MATCH_SLTIU, MASK_SLTIU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("xori"),   OPC__I, 0, HASH_xori,  MATCH_XORI,  MASK_XORI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("ori"),    OPC__I, 0, HASH_ori,   MATCH_ORI,   MASK_ORI,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("andi"),   OPC__I, 0, HASH_andi,  MATCH_ANDI,  MASK_ANDI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },

{ String8__inline_m("slli"),   OPC__I, 0, HASH_slli,  MATCH_SLLI,  MASK_SLLI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("srli"),   OPC__I, 0, HASH_srli,  MATCH_SRLI,  MASK_SRLI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("srai"),   OPC__I, 0, HASH_srai,  MATCH_SRAI,  MASK_SRAI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("slliw"),  OPC__I, 0, HASH_slliw, MATCH_SLLIW, MASK_SLLIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("srliw"),  OPC__I, 0, HASH_srliw, MATCH_SRLIW, MASK_SRLIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("sraiw"),  OPC__I, 0, HASH_sraiw, MATCH_SRAIW, MASK_SRAIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift_5)), match_opcode },

{ String8__inline_m("add"),    OPC__I, 0, HASH_add,  MATCH_ADD,  MASK_ADD,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sub"),    OPC__I, 0, HASH_sub,  MATCH_SUB,  MASK_SUB,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sll"),    OPC__I, 0, HASH_sll,  MATCH_SLL,  MASK_SLL,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("slt"),    OPC__I, 0, HASH_slt,  MATCH_SLT,  MASK_SLT,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sltu"),   OPC__I, 0, HASH_sltu, MATCH_SLTU, MASK_SLTU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("xor"),    OPC__I, 0, HASH_xor,  MATCH_XOR,  MASK_XOR,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("srl"),    OPC__I, 0, HASH_srl,  MATCH_SRL,  MASK_SRL,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sra"),    OPC__I, 0, HASH_sra,  MATCH_SRA,  MASK_SRA,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("or"),     OPC__I, 0, HASH_or,   MATCH_OR,   MASK_OR,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("and"),    OPC__I, 0, HASH_and,  MATCH_AND,  MASK_AND,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },

{ String8__inline_m("beq"),    OPC__I, INSN_CONDBRANCH, HASH_beq,  MATCH_BEQ,  MASK_BEQ,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bne"),    OPC__I, INSN_CONDBRANCH, HASH_bne,  MATCH_BNE,  MASK_BNE,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("blt"),    OPC__I, INSN_CONDBRANCH, HASH_blt,  MATCH_BLT,  MASK_BLT,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bge"),    OPC__I, INSN_CONDBRANCH, HASH_bge,  MATCH_BGE,  MASK_BGE,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bltu"),   OPC__I, INSN_CONDBRANCH, HASH_bltu, MATCH_BLTU, MASK_BLTU, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgeu"),   OPC__I, INSN_CONDBRANCH, HASH_bgeu, MATCH_BGEU, MASK_BGEU, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },

// Pseudo-instructions (incomplete)
{ String8__inline_m("j"),      OPC__I, INSN_ALIAS|INSN_JSR, HASH_j,   MATCH_JAL,                      MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Jal)),                                    match_opcode },
{ String8__inline_m("jr"),     OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD|MASK_IMM, OP_m(OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("jr"),     OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("jr"),     OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD,          OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)),     match_opcode },
{ String8__inline_m("ret"),    OPC__I, INSN_ALIAS|INSN_JSR, HASH_ret, MATCH_JALR|(X_RA << OP_SH_RS1), MASK_JALR|MASK_RS1,         OP_m(OP_None),                                                  match_opcode },

{ String8__inline_m("call"),   OPC__I, INSN_MACRO, HASH_call, (X_RA << OP_SH_RS1)|(X_RA << OP_SH_RD), MACRO_CALL, OP_m(OP_Call), 0 },

{ String8__inline_m("li"),     OPC__I, INSN_ALIAS, HASH_li, MATCH_ADDI, MASK_ADDI|MASK_RS1, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Immediate(OPF_I__I)),      match_opcode     },
{ String8__inline_m("li"),     OPC__I, INSN_MACRO, HASH_li, 0,          MACRO_LI,           OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Constant(OPF_C__Large)),   0                },
{ String8__inline_m("la"),     OPC__I, INSN_MACRO, HASH_la, 0,          MACRO_LA,           OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Constant(OPF_C__Address)), match_rd_nonzero },

{ String8__inline_m("nop"),    OPC__I, INSN_ALIAS, HASH_nop, MATCH_ADDI, MASK_ADDI|MASK_RD|MASK_RS1|MASK_IMM, OP_m(OP_None), match_opcode },

{ String8__inline_m("mv"),     OPC__I, INSN_ALIAS, HASH_mv, MATCH_ADDI, MASK_ADDI|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)), match_opcode },

{ String8__inline_m("beqz"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_beqz, MATCH_BEQ,  MASK_BEQ|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("blez"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_blez, MATCH_BGE,  MASK_BGE|MASK_RS1, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bgez"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgez, MATCH_BGE,  MASK_BGE|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("ble"),    OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_ble,  MATCH_BGE,  MASK_BGE,          OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bltz"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bltz, MATCH_BLT,  MASK_BLT|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bgtz"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgtz, MATCH_BLT,  MASK_BLT|MASK_RS1, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bleu"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bleu, MATCH_BGEU, MASK_BGEU,         OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgt"),    OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgt,  MATCH_BLT,  MASK_BLT,          OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgtu"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgtu, MATCH_BLTU, MASK_BLTU,         OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bnez"),   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bnez, MATCH_BNE,  MASK_BNE|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },

{ String8__inline_m("pause"),  OPC__I, 0, HASH_pause,  MATCH_PAUSE,  MASK_PAUSE,  OP_m(OP_None), match_opcode },
{ String8__inline_m("ecall"),  OPC__I, 0, HASH_ecall,  MATCH_ECALL,  MASK_ECALL,  OP_m(OP_None), match_opcode },
{ String8__inline_m("ebreak"), OPC__I, 0, HASH_ebreak, MATCH_EBREAK, MASK_EBREAK, OP_m(OP_None), match_opcode },

{ String8__inline_m(""), OPC__None, 0, 0, 0, 0, 0, 0 }
};

// TODO(low): for now this is dumb enough and works.
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

internal void
RISCV_Instruction__parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Diagnostics        *diagnostics,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        U32                 instruction_hash,

        U16                *relocation_out,
        RISCV_Instruction  *instruction_out,
        Expression        **expression_out
)
{
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
        String8 opcode_name = (String8){ .data = opcode->name, .count = opcode->count };

        Token opcode_token = cursor->current;
        token_next(cursor, diagnostics);
        Token_Cursor cursor_start = *cursor;

        Expression *expression = 0;
        B32 match = 0;

        // Iterate over opcode entries with the same name.
        for (;;)
        {
                *instruction_out = RISCV_Instruction__create(opcode, opcode_token.location);
                U64 arguments = opcode->arguments;
                U32 arguments_index = 0;
                B32 try_next = 0;

                // Iterate over opcode arguments.
                for (;;)
                {
                        U8 slot = (U8)(arguments >> (8 * arguments_index));
                        if (!slot)
                        {
                                match = !try_next && opcode->hash && (!opcode->match_function || opcode->match_function(opcode, instruction_out->encoding));
                                break;
                        }

                        switch (OP_KIND(slot))
                        {
                        case OPK__Comma:
                        {
                                // NOTE: This whole thing could extracted into a `expect_comma_and_advance`.
                                Token token_before_comma = cursor->previous;
                                if (cursor->current.kind == Token_Kind__Comma)
                                {
                                        token_next(cursor, diagnostics);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->location   = token_before_comma.location + token_before_comma.size;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
                                }
                        } break;
                        case OPK__PL:
                        {
                                if (cursor->current.kind == Token_Kind__Parenthesis_Left)
                                {
                                        token_next(cursor, diagnostics);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("'(' expected");
                                }
                        } break;
                        case OPK__PR:
                        {
                                if (cursor->current.kind == Token_Kind__Parenthesis_Right)
                                {
                                        token_next(cursor, diagnostics);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("')' expected");
                                }
                        } break;
                        case OPK__GPR:
                        {
                                String8 text = Token_Cursor__text(cursor);
                                const Register *reg = Register_List__lookup(RISCV_register_list, text, 0);
                                if (!reg)
                                {
                                       Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                       diagnostic->location   = cursor->current.location;
                                       diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Register_Invalid];
                                       diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                }

                                U8 register_number = reg ? reg->number : 0;
                                switch (OP_FIELD(slot))
                                {
                                       case OPF_R__D: { INSERT_OPERAND(RD,  *instruction_out, register_number); } break;
                                       case OPF_R__S_3:    { INSERT_OPERAND(RS3, *instruction_out, register_number); } break;
                                       case OPF_R__S_2:    { INSERT_OPERAND(RS2, *instruction_out, register_number); } break;
                                       case OPF_R__S_1:    { INSERT_OPERAND(RS1, *instruction_out, register_number); } break;
                                       default: { unreachable_m(); }
                                }
                                token_next(cursor, diagnostics);
                        } break;
                        case OPK__Constant:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_C__Address:
                                {
                                        expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
                                        expression_evaluate(expression);
                                        B32 symbol_is   = expression->evaluation == Expression_Kind__Symbol;
                                        B32 constant_is = expression->evaluation == Expression_Kind__Constant;
                                        if (!(symbol_is || constant_is))
                                        {
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message    = String8__literal("expression must be either symbol or a constant");
                                                diagnostic->location   = expression->location_range.v[0];
                                                diagnostic->ranges[0]  = expression->location_range;
                                        }

                                        if (symbol_is)
                                        {
                                                *relocation_out = Relocation_RISC_V__32_Bit;
                                        }


                                        B32 constant_fits = sign_extended_32_bit_is_m(expression->integer_value);
                                        if (constant_is && !constant_fits)
                                        {
                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                diagnostic->message    = String8__literal("offset too large for this opcode");
                                                diagnostic->location   = expression->location_range.v[0];
                                                diagnostic->ranges[0]  = expression->location_range;
                                        }
                                } break;
                                case OPF_C__Large:
                                {
                                        expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
                                        expression_evaluate(expression);
                                        if (expression->evaluation != Expression_Kind__Constant)
                                        {
                                               Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                               diagnostic->message    = String8__literal("Constant expression expected");
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                        }
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Offset:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_O__Jal:
                                {
                                        // NOTE: we use GNU as approach to add mark a branch relocation immediately.
                                        // This relocation is temporary, and could be changed, since it depends on the
                                        // value of the expression and the symbols required.
                                        //
                                        // At assembly time, we may not know how many instructions this will expand to. It is
                                        // deferred later when we know all instructions. It is a different situation compared to
                                        // a `li` or `call` instruction which, during instruction parsing, are already expanded
                                        // into a known number of instructions (`INSN_MACRO`)
                                        *relocation_out = Relocation_RISC_V__JAL;
                                        expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);


                                        // GNU as silently ignored additional symbols, but since our fixup takes a whole
                                        // expression, here even `jal label2-label1` is perfectly fine.
                                        expression_evaluate(expression);
                                } break;
                                case OPF_O__Branch:
                                {
                                        // See notes for `OPF_O__Jal`.
                                        *relocation_out = Relocation_RISC_V__Branch;
                                        expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);


                                        // NOTE: here GNU as would just consider one operand, here we can consider the whole
                                        // expression and save it as fixup symbol information.
                                } break;
                                case OPF_O__Store:
                                {
                                        U8 next = (U8)(arguments >> (8 * (arguments_index + 1)));
                                        if (next == OP_PL && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                        {
                                               // Omitted immediate, e.g. sw t1, (t0)
                                               arguments_index += 1;
                                        }
                                        else
                                        {
                                                expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, diagnostics, relocation_out, Relocation_Operator_List__stype);
                                                if (!*relocation_out)
                                                {
                                                        expression_evaluate(expression);
                                                        // TODO(RV32): normalize constant expression? See GNU as.
                                                        B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                                        if (expression->evaluation == Expression_Kind__Constant && fits)
                                                        {
                                                                // TODO(medium): GNU as does this at a later step, and by default emits a
                                                                // relocation. Consider doing the same.
                                                                U32 encoding_immediate = encode_immediate_s_m(expression->integer_value);
                                                                instruction_out->encoding |= encoding_immediate;
                                                        }
                                                        else
                                                        {
                                                                try_next = 1;
                                                        }
                                                }
                                        }
                                } break;
                                case OPF_O__Load:
                                {
                                        U8 next = (U8)(arguments >> (8 * (arguments_index + 1)));
                                        if (next == OP_PL && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                        {
                                               // Omitted immediate, e.g. lw t1, (t0)
                                               arguments_index += 1;
                                        }
                                        else
                                        {
                                                // TODO(refactor): this is mostly in common with the OPF_I__I case.
                                                expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, diagnostics, relocation_out, Relocation_Operator_List__stype);
                                                if (!*relocation_out)
                                                {
                                                        expression_evaluate(expression);
                                                        // TODO(RV32): normalize constant expression? See GNU as.
                                                        B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                                        if (expression->evaluation == Expression_Kind__Constant && fits)
                                                        {
                                                                // TODO(medium): GNU as does this at a later step, and by default emits a
                                                                // relocation. Consider doing the same.
                                                                U32 encoding_immediate = encode_immediate_i_m(expression->integer_value);
                                                                instruction_out->encoding |= encoding_immediate;
                                                        }
                                                        else
                                                        {
                                                                try_next = 1;
                                                        }
                                                }
                                        }
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Immediate:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_I__I:
                                {
                                        expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, diagnostics, relocation_out, Relocation_Operator_List__itype);
                                        if (!*relocation_out)
                                        {
                                               expression_evaluate(expression);
                                               // TODO(RV32): normalize constant expression? See GNU as.
                                               B32 fits = S64_bits_range_in(expression->integer_value, 12);
                                               if (expression->evaluation == Expression_Kind__Constant && fits)
                                               {
                                                       // TODO(medium): GNU as does this at a later step, and by default emits a
                                                       // relocation. Consider doing the same.
                                                       U32 encoding_immediate = encode_immediate_i_m(expression->integer_value);
                                                       instruction_out->encoding |= encoding_immediate;
                                               }
                                               else
                                               {
                                                       try_next = 1;
                                               }
                                        }
                                } break;
                                case OPF_I__U:
                                {
                                        expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, diagnostics, relocation_out, Relocation_Operator_List__utype);
                                        if (!*relocation_out)
                                        {
                                                expression_evaluate(expression);
                                                if (expression->evaluation == Expression_Kind__Constant)
                                                {
                                                        S64 result = expression->integer_value;
                                                        B32 fits = 0 <= result && result < (S64)(1 << 20);
                                                        if (!fits)
                                                        {
                                                                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                                diagnostic->location   = expression->location_range.v[0];
                                                                diagnostic->message    = String8__literal("constant expression value must in the range 0..1048576");
                                                                diagnostic->ranges[0]  = expression->location_range;
                                                        }


                                                        // TODO(medium): GNU as does this at a later step, and by default emits a
                                                        // relocation. Consider doing the same.
                                                        U32 encoding_immediate = encode_immediate_u_m(expression->integer_value);
                                                        instruction_out->encoding |= encoding_immediate;
                                                }
                                                else
                                                {


                                                        Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                                        diagnostic->message    = String8__literal("Non-constant expression must have an appropriate relocation operator");
                                                        diagnostic->location   = expression->location_range.v[0];
                                                        diagnostic->ranges[0]  = expression->location_range;
                                                }
                                        }
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Shift:
                        {
                                switch (OP_FIELD(slot))
                                {
                                case OPF_S__Shift:
                                {
                                        expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
                                        expression_evaluate(expression);
                                        S64 value = expression->integer_value;
                                        B32 fits = 0 <= value && value < XLEN;
                                        if (expression->evaluation != Expression_Kind__Constant || !fits)
                                        {
                                               Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                               diagnostic->message    = String8__literal("shift amount doesn't fit register size");
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                        }


                                        INSERT_OPERAND (SHAMT, *instruction_out, value);
                                } break;
                                case OPF_S__Shift_5:
                                {
                                        expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
                                        expression_evaluate(expression);
                                        S64 value = expression->integer_value;
                                        B32 fits = 0 <= value && value < (1 << 5);
                                        if (expression->evaluation != Expression_Kind__Constant || !fits)
                                        {
                                               Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                                               diagnostic->message    = String8__literal("shift amount doesn't fit register size");
                                               diagnostic->location   = expression->location_range.v[0];
                                               diagnostic->ranges[0]  = expression->location_range;
                                        }


                                        INSERT_OPERAND (SHAMT, *instruction_out, value);
                                } break;
                                default: { unreachable_m(); }
                                }
                        } break;
                        case OPK__Call:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, diagnostics);
                                *relocation_out = Relocation_RISC_V__Call_PLT;
                        } break;
                        default: { unreachable_m(); }
                        }

                        arguments_index += 1;
                }

                String8 opcode_string = String8__new(opcode->name, opcode->count);
                B32 same_name = String8__match_exact(opcode_name, opcode_string);
                if (match || opcode->hash == 0 || !same_name)
                {
                        break;
                }

                *cursor = cursor_start;
                opcode += 1;
        }

        if (!match)
        {
                Diagnostic *diagnostic = Diagnostics__push(diagnostics);
                diagnostic->location   = opcode_token.location;
                diagnostic->message    = String8__literal("unrecognized opcode");
                diagnostic->ranges[0]  = Token__range(opcode_token);
        }

        *expression_out = expression;

        return;
}

internal void
RISCV_Instruction__append
(
        Arena             *arena,
        Section           *section,

        RISCV_Instruction *instruction,
        Expression        *expression,
        U16                relocation
)
{
        Fixup *fixup                 = 0;
        // NOTE: although jumps are assumed to be in range, if the compressed extension is enabled
        // then this might get reduced to a compressed 2-byte instruction.
        B32    jump_unconditional_is = relocation == Relocation_RISC_V__JAL;
        B32    jump_is               = relocation == Relocation_RISC_V__Branch || jump_unconditional_is;
        // NOTE: fixups, which are deferred patches, can be created only for fixed size instructions
        // (non-jump_is) because they need a precise location to be applied. Jump instructions,
        // like branches, break this invariant. However, some kind of fixup AND relocation will be needed, so for those
        // instruction we emit a tentative fixup attached to the relaxation information.
        U32    encoding              = instruction->encoding;
        U8     encoding_size         = RISCV_instruction_size(encoding);
        U32    location              = instruction->location;

        if (relocation)
        {
                fixup                  = Arena__push_struct_m(arena, Fixup);
                fixup->expression      = expression;
                fixup->relocation_type = relocation;
                DLL_push_back_m(section->fixups.first, section->fixups.last, fixup);
        }

        if (jump_is)
        {
                Relax_Info relax_info =
                {
                        .jump =
                        {
                                .expression              = expression,
                                .fixup                   = fixup,
                                .compressed_is           = encoding_size == 2,
                                .unconditional_is        = jump_unconditional_is,
                                .instructions_total_size = encoding_size
                        }
                };

                Fragment *sealed = Fragments__variable
                (
                        &section->fragments,
                        location,
                        relax_info,
                        Relax_State__Jump,
                        (U8 *)&encoding,
                        encoding_size
                );

                sealed->relax_info.jump.fixup->fragment = sealed;
                sealed->relax_info.jump.fixup->offset   = sealed->data_size;
        }
        else
        {
                Section__add_instruction_fixed
                (
                        section,
                        fixup,
                        encoding,
                        encoding_size,
                        instruction->location
                );
        }

        return;
}

internal void
RISCV_macro_build
(
        Arena       *arena,
        Section     *section,

        String8      instruction_name,
        U32          location,
        Expression  *expression,
        U64          arguments,
        S32         *values,
        U8           values_count
)
{
        U32 instruction_hash = FNV_hash_U32(instruction_name);
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
        assert_always_m(opcode && opcode->hash);

        RISCV_Instruction instruction = RISCV_Instruction__create(opcode, location);

        U16 relocation = 0;
        U32 arguments_index = 0;

        for (;;)
        {
                U8 slot = (U8)(arguments >> (8 * arguments_index));
                B32 break_should = !slot || arguments_index >= values_count;
                if (break_should)
                {
                        break;
                }

                S32 value = values[arguments_index];
                switch (OP_KIND(slot))
                {
                        default: { unreachable_m(); } break;
                        case OPK__Relocation: { relocation = (U16)value; } break;
                        case OPK__GPR:
                        {
                                switch (OP_FIELD(slot))
                                {
                                        default: { unreachable_m(); } break;
                                        case OPF_R__D:   { INSERT_OPERAND(RD,  instruction, value); } break;
                                        case OPF_R__S_3: { INSERT_OPERAND(RS3, instruction, value); } break;
                                        case OPF_R__S_2: { INSERT_OPERAND(RS2, instruction, value); } break;
                                        case OPF_R__S_1: { INSERT_OPERAND(RS1, instruction, value); } break;
                                }
                        } break;
                }

                arguments_index += 1;
        }

        assert_always_m(relocation ? expression != 0 : 1);

        RISCV_Instruction__append
        (
                arena,
                section,
                &instruction,
                expression,
                relocation
        );
}

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
)
{
        U64 arguments_auipc = OP_m(OP_GPR(OPF_R__D), OP_Relocation);
        S32 values_auipc[2] = {rs1, relocation};
        U64 arguments_jalr  = OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S_1));
        S32 values_jalr[2]  = {rd, rs1};


        // Ensure both instructions land in the same fragment.
        Fragments__ensure(&section->fragments, 8);
        RISCV_macro_build
        (
                arena,
                section,

                String8__literal("auipc"),
                location,
                expression,
                arguments_auipc,
                values_auipc,
                array_count_m(values_auipc)
        );
        RISCV_macro_build
        (
                arena,
                section,


                String8__literal("jalr"),
                location,
                0,
                arguments_jalr,
                values_jalr,
                array_count_m(values_jalr)
        );
        // NOTE: I trust GNU as that is better to seal the fragment now.
        Fragment__wane(section->fragments.last);
        Fragments__push_empty_fragment(&section->fragments, location);
}

// Encodes all the instructions required during a LI pseudo-instruction. Pass `section = NULL` to count only; pass a
// valid section pointer (with `rd` set) to additionally emit the encoded instructions.
//
// The algorithm proceeds by range analysis:
//
//   - If the value fits in a 12-bit signed range, a single ADDI suffices.
//   - If it fits in a 32-bit signed range, it takes LUI alone (if the low 12 bits are zero) or LUI + ADDIW otherwise.
//     ADDIW (not ADDI) is used because the result is meant to be a 32-bit sign-extended value.
//
// Otherwise, we peel the low 12 bits off as a sign-extended tail (to be spliced back with an ADDI later),
// arithmetic-shift the remainder right by 12, and recurse on the upper portion. Each recursive level contributes one
// SLLI (to shift the upper part back into place) plus one ADDI (to splice in the peeled 12 bits, if non-zero).
//
// Note: after the initial LUI + ADDIW builds the topmost 32-bit chunk, every subsequent low-bit insertion uses plain
// ADDI, not ADDIW. ADDIW would discard the upper 32 bits we just shifted in.
//
// Example: li x1, 0x12345111333555
//
// Peeling (top-down analysis):
//
//   value = 0x12345111333555
//     peel low 12 bits = 0x555, shift right by 12
//   value = 0x12345111333
//     peel low 12 bits = 0x333, shift right by 12
//   value = 0x12345111
//     fits in 32-bit signed -> LUI 0x12345, ADDIW 0x111
//
// Emission (bottom-up assembly, 6 instructions):
//
//   lui   ra, 0x12345    ; ra = 0x0000000012345000
//   addiw ra, ra, 0x111  ; ra = 0x0000000012345111   <- base case
//   slli  ra, ra, 12     ; ra = 0x0000012345111000
//   addi  ra, ra, 0x333  ; ra = 0x0000012345111333   <- splice 0x333
//   slli  ra, ra, 12     ; ra = 0x0012345111333000
//   addi  ra, ra, 0x555  ; ra = 0x0012345111333555   <- splice 0x555
//
// The symmetry is the key insight: each level of peeling on the way down (shift right by 12, record a tail) becomes one
// SLLI + ADDI pair on the way back up (shift left by 12, replay the tail). The base case at the bottom of the recursion
// is the LUI (+ optional ADDIW) that seeds the topmost 32-bit chunk.
//
// Other minor optimizations are in place. In particular, the algorithm will also take into account additional trailing
// zeros after shifting right by 12, so that numbers with many trailing zero don't need more instructions than needed.
internal U8
RISCV_li_expand
(
        Section         *section,

        S64 immediate,
        U8  register_destination,
        U32 location
)
{
        U8  instructions_count = 0;
        S64 immediate_low_12   = 0;
        U32 index              = 0;

        // Peeled chunks: for each level we store the shift amount AND the
        // low-12-bit tail. Shifts are at least 12, but can be larger because
        // trailing zero bits of the upper residual are absorbed into the next
        // SLLI (folding runs of zeros for free). Worst case on RV64 is 3
        // peels = 8 total instructions (LUI + ADDIW + 3 x (SLLI + ADDI)).
        struct { U8 shift; S64 tail; } peels[4];
        U32 peels_count = 0;

        for (;;)
        {
                B32 range_12     = S64_bits_range_in(immediate, 12);
                B32 range_32     = S64_bits_range_in(immediate, 32);
                B32 break_should = range_12 || range_32;

                if (range_12)
                {
                        instructions_count += 1;
                        if (section)
                        {
                                // Single ADDIW from x0
                                U32 addiw_encoding      = instruction_i_encode_m(register_destination, 0, immediate, OPCODE_I_TYPE_W, FUNCT3_ADDIW);
                                U8  addiw_encoding_size = RISCV_instruction_size(addiw_encoding);
                                Section__add_instruction_fixed(section, 0, addiw_encoding, addiw_encoding_size, location);
                        }
                }
                else if (range_32)
                {
                        immediate_low_12 = (immediate << 52) >> 52;
                        B32 lui_suffices = immediate_low_12 == 0;
                        instructions_count += lui_suffices ? 1 : 2;
                        if (section)
                        {
                                // LUI, plus ADDIW if the low 12 bits are non-zero. The LUI
                                // immediate is `immediate` with its low 12 bits cleared;
                                // ADDIW splices them back in (sign-extended to 64 bits).
                                // instruction_u_encode_m expects the 20-bit U-field (the value already shifted right by 12).
                                S64 lui_immediate     = (S64)((U32)(immediate - immediate_low_12) >> 12);
                                U32 lui_encoding      = instruction_u_encode_m(register_destination, lui_immediate, OPCODE_LUI);
                                U8  lui_encoding_size = RISCV_instruction_size(lui_encoding);
                                Section__add_instruction_fixed(section, 0, lui_encoding, lui_encoding_size, location);
                                if (!lui_suffices)
                                {
                                        U32 addiw_encoding = instruction_i_encode_m(register_destination, register_destination, immediate_low_12,
                                                OPCODE_I_TYPE_W, FUNCT3_ADDIW);
                                        U8  addiw_encoding_size = RISCV_instruction_size(addiw_encoding);
                                        Section__add_instruction_fixed(section, 0, addiw_encoding, addiw_encoding_size, location);
                                }
                        }
                }
                else
                {
                        immediate_low_12 = (immediate << 52) >> 52;
                        // Here, we override immediate to repeat the algorithm the next iterations on a smaller number
                        // composed by the 54 highest bits. However, as we see below there might be more trailing zeros!
                        immediate        = (immediate - immediate_low_12) >> 12;

                        // Absorb trailing zero bits of the upper residual into this
                        // peel's SLLI. Each absorbed bit means the residual we recurse
                        // on is denser, potentially bottoming out in fewer iterations
                        // (e.g. a huge value like 0x8000000000000000 collapses to just
                        // ADDI + SLLI after this).
                        U8 trailing = count_trailing_zeros((U64)immediate);
                        U8 shift    = (12 + trailing);
                        immediate  >>= trailing;

                        // SLLI is always needed to shift the upper part into place;
                        // ADDI is only needed when the peeled tail is non-zero.
                        B32 addi_needed = (immediate_low_12 != 0);
                        instructions_count += 1 + (addi_needed ? 1 : 0);

                        if (section)
                        {
                                // Record (shift, tail) for later replay. No emission yet:
                                // the SLLI + (optional) ADDI can't be emitted until the
                                // upper residual has been materialized by the base case.
                                assert_always_m(peels_count < 4 && "LI expansion exceeded worst case");
                                peels[peels_count].shift = shift;
                                peels[peels_count].tail  = immediate_low_12;
                                peels_count += 1;
                        }
                }

                if (break_should)
                {
                        break;
                }
                index += 1;
                assert_always_m(index < 8 && "infinite loop");
        }

        // Replay phase: emit SLLI + optional ADDI for each peeled level in
        // reverse order. `register_destination` already holds the base-case residual; each
        // iteration shifts it left by the recorded amount (12 + absorbed
        // trailing zeros) and splices the next tail back in (when non-zero).
        // Plain ADDI (not ADDIW) is used because we're building a 64-bit
        // value; ADDIW would discard the upper bits just shifted into place
        // by SLLI.
        if (section)
        {
                S32 peel_index = peels_count - 1;
                for (;;)
                {
                        B32 break_should = peel_index < 0;
                        if (break_should)
                        {
                                break;
                        }

                        U8  shift = peels[peel_index].shift;
                        S64 tail  = peels[peel_index].tail;

                        U32 slli_encoding      = instruction_i_encode_m(register_destination, register_destination, shift, OPCODE_I_TYPE, FUNCT3_SLLI);
                        U8  slli_encoding_size = RISCV_instruction_size(slli_encoding);
                        Section__add_instruction_fixed(section, 0, slli_encoding, slli_encoding_size, location);

                        if (tail != 0)
                        {
                                U32 addi_encoding      = instruction_i_encode_m(register_destination, register_destination, tail, OPCODE_I_TYPE, FUNCT3_ADDI);
                                U8  addi_encoding_size = RISCV_instruction_size(addi_encoding);
                                Section__add_instruction_fixed(section, 0, addi_encoding, addi_encoding_size, location);
                        }

                        peel_index -= 1;
                }
        }

        assert_always_m(instructions_count > 0);
        return instructions_count;
}

internal void
RISCV_instruction_pseudo_append
(
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,

        RISCV_Instruction  *instruction,
        Expression         *expression,
        U16                relocation
)
{
        U8 rd  = (instruction->encoding >> OP_SH_RD)  & OP_MASK_RD;
        U8 rs1 = (instruction->encoding >> OP_SH_RS1) & OP_MASK_RS1;
        U8 rs2 = (instruction->encoding >> OP_SH_RS2) & OP_MASK_RS2;
        unused_m(rs2);

        U32 pseudo_type = instruction->opcode->mask;

        switch (pseudo_type)
        {
        default: { unreachable_m(); } break;
        case MACRO_CALL:
        {
                RISCV_call_expand
                (
                        symbols_table->arena,
                        section,
                        rd,
                        rs1,
                        expression,
                        relocation,
                        instruction->location
                );
        } break;
        case MACRO_LA:
        {
                if (expression->evaluation == Expression_Kind__Constant)
                {
                        RISCV_li_expand
                        (
                                section,
                                expression->integer_value,
                                rd,
                                instruction->location
                        );
                }
                else
                {
                        // TODO(low): no support yet for Position-Indipendent-Code (PIC) or GOT etc.

                        // We just expand to a `auipc + addi` combination.
                        // How it works:
                        //
                        // Suppose we have a symbol with 32-bit address `a`. We have to split its value
                        // into two instructions. The %pcrel_hi relocation operator computes `(a - pc) >> 12` (returns
                        // the upper 20 bits) while `auipc rd, immediate` computes `pc + (immediate << 12)` and saves it
                        // into `rd`, so that yields (once computed by linker) the value
                        //      `pc + ((a - pc) >> 12) << 12` == `pc + hi20(a - pc) << 12
                        // into `rd`. Lastly, the program counter is increased.
                        //
                        // Now, we have to add the remaining lower 12-bits of `(a - pc)` i.e. `lo12(a - pc)`, so that we
                        // erase `pc` from `rd` and get the final address `a`.
                        // We could use an `addi` paired with `%pcrel_lo`. However, now `pc` has been increased by 4
                        // bytes or whatever the instruction size is, so it would be off.
                        //
                        // To mitigate this in a standardized way, the RISC-V ELF psABI mandates the following steps:
                        //
                        // 1. A symbol (label) inside `%pcrel_lo` must point to the matching `%pcrel_hi` relocation;
                        // 2. The linker will discover the value of `a` by looking at the matching relocation, and
                        //    complete the computation by adding the sign-extended, lower 12-bits of `(a - pc)`.
                        //
                        // In essence, the `%pcrel_lo` relocation is just an artificial way to point to the matching
                        // `%pcrel_hi` because the value `(label - pc_of_addi) >> 20` is never used (it would equal -4
                        // in most cases, by the way).
                        //
                        // This is why we have to create a local label like ".L0 " is created above the `auipc`
                        // instruction.

                        U64 arguments_auipc = OP_m(OP_GPR(OPF_R__D), OP_Relocation);
                        S32 values_auipc[]  = {rd, Relocation_RISC_V__PC_Relative_High_20};
                        U64 arguments_addi  = OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S_1), OP_Relocation);
                        S32 values_addi[]   = {rd, rd, Relocation_RISC_V__PC_Relative_Low_12_I_Type};

                        Symbol_Ref *internal_label          = Symbols_Table__create_internal(symbols_table, section);
                                    internal_label->flags  |= Symbol_Flags__Relocation;
                        Expression *expression_addi         = Expressions_push_empty(expressions, symbols_table->arena);
                                    expression_addi->symbol = internal_label;
                                    expression_addi->kind   = Expression_Kind__Symbol;

                        // Ensure the instructions are in the same fragment
                        Fragments__ensure(&section->fragments, 8);
                        RISCV_macro_build
                        (
                                symbols_table->arena,
                                section,


                                String8__literal("auipc"),
                                instruction->location,
                                expression,
                                arguments_auipc,
                                values_auipc,
                                array_count_m(values_auipc)
                        );
                        // NOTE: GNU as creates also a second expression with an fake label for addi, why?
                        RISCV_macro_build
                        (
                                symbols_table->arena,
                                section,


                                String8__literal("addi"),
                                instruction->location,
                                expression_addi,
                                arguments_addi,
                                values_addi,
                                array_count_m(values_addi)
                        );
                        // TODO(medium, check-gas): wane and new here?
                }
        } break;
        case MACRO_LI:
        {
                RISCV_li_expand(section, expression->integer_value, rd, instruction->location);
        } break;
        }
}

