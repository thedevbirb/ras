
// FENCE: predecessor/successor sets.
global const String8 RISCV_predecessor_successor_table[] =
{
         {0},
         String8__literal("w"),
         String8__literal("r"),
         String8__literal("rw"),
         String8__literal("o"),
         String8__literal("ow"),
         String8__literal("or"),
         String8__literal("orw"),

         String8__literal("i"),
         String8__literal("iw"),
         String8__literal("ir"),
         String8__literal("irw"),
         String8__literal("io"),
         String8__literal("iow"),
         String8__literal("ior"),
         String8__literal("iorw"),
};

// NOTE: the empty opcode can be distinguished by the zero hash.
global const RISCV_Opcode RISCV_Opcode__table[] =
{
// Base I instructions.
{ String8__inline_m("auipc"), 0, OPC__I,   0, HASH_auipc, MATCH_AUIPC, MASK_AUIPC, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Immediate(OPF_I__U)), match_opcode },
{ String8__inline_m("lui"),   0, OPC__I, 0, HASH_lui,   MATCH_LUI,   MASK_LUI,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Immediate(OPF_I__U)), match_opcode },

// NOTE: important here to go from more specific to less specific.
{ String8__inline_m("jal"),  0, OPC__I, 0, HASH_jal,  MATCH_JAL,                    MASK_JAL,         OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Jal)),                                match_opcode },
{ String8__inline_m("jal"),  0, OPC__I, 0, HASH_jal,  MATCH_JAL|(X_RA << OP_SH_RD), MASK_JAL|MASK_RD, OP_m(OP_Offset(OPF_O__Jal)),                                                            match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  0, HASH_jalr, MATCH_JALR,                   MASK_JALR,        OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },

{ String8__inline_m("lb"),  0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lb,  MATCH_LB,  MASK_LB,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lb"),  0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lb,  MATCH_LB,  MASK_LB,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lbu"), 0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lbu, MATCH_LBU, MASK_LBU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lbu"), 0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lbu, MATCH_LBU, MASK_LBU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lh"),  0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lh,  MATCH_LH,  MASK_LH,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lh"),  0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lh,  MATCH_LH,  MASK_LH,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lhu"), 0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lhu, MATCH_LHU, MASK_LHU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lhu"), 0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lhu, MATCH_LHU, MASK_LHU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("lw"),  0, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lw,  MATCH_LW,  MASK_LW,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("lw"),  0, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lw,  MATCH_LW,  MASK_LW,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("ld"), 64, OPC__I, INSN_DREF|INSN_8_BYTE, HASH_ld,  MATCH_LD,  MASK_LD,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("ld"), 64, OPC__I, INSN_DREF|INSN_8_BYTE, HASH_ld,  MATCH_LD,  MASK_LD,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)),                                       match_opcode },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ String8__inline_m("sb"),  0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_sb, MATCH_SB, MASK_SB, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("sh"),  0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_sh, MATCH_SH, MASK_SH, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("sw"),  0, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_sw, MATCH_SW, MASK_SW, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("sd"), 64, OPC__I, INSN_DREF|INSN_8_BYTE, HASH_sd, MATCH_SD, MASK_SD, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ String8__inline_m("addi"),   0, OPC__I, 0, HASH_addi,  MATCH_ADDI,  MASK_ADDI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("addiw"), 64, OPC__I, 0, HASH_addiw, MATCH_ADDIW, MASK_ADDIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("slti"),   0, OPC__I, 0, HASH_slti,  MATCH_SLTI,  MASK_SLTI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("sltiu"),  0, OPC__I, 0, HASH_sltiu, MATCH_SLTIU, MASK_SLTIU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("xori"),   0, OPC__I, 0, HASH_xori,  MATCH_XORI,  MASK_XORI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("ori"),    0, OPC__I, 0, HASH_ori,   MATCH_ORI,   MASK_ORI,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("andi"),   0, OPC__I, 0, HASH_andi,  MATCH_ANDI,  MASK_ANDI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)), match_opcode },

{ String8__inline_m("slli"),   0,  OPC__I, 0, HASH_slli,  MATCH_SLLI,  MASK_SLLI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("srli"),   0,  OPC__I, 0, HASH_srli,  MATCH_SRLI,  MASK_SRLI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("srai"),   0,  OPC__I, 0, HASH_srai,  MATCH_SRAI,  MASK_SRAI,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("slliw"), 64,  OPC__I, 0, HASH_slliw, MATCH_SLLIW, MASK_SLLIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("srliw"), 64,  OPC__I, 0, HASH_srliw, MATCH_SRLIW, MASK_SRLIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("sraiw"), 64,  OPC__I, 0, HASH_sraiw, MATCH_SRAIW, MASK_SRAIW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Shift(OPF_S__Shift_5)), match_opcode },

{ String8__inline_m("add"),   0,  OPC__I, 0, HASH_add,  MATCH_ADD,  MASK_ADD,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sub"),   0,  OPC__I, 0, HASH_sub,  MATCH_SUB,  MASK_SUB,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("addw"), 64,  OPC__I, 0, HASH_addw, MATCH_ADDW, MASK_ADDW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("subw"), 64,  OPC__I, 0, HASH_subw, MATCH_SUBW, MASK_SUBW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sll"),   0,  OPC__I, 0, HASH_sll,  MATCH_SLL,  MASK_SLL,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("slt"),   0,  OPC__I, 0, HASH_slt,  MATCH_SLT,  MASK_SLT,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sltu"),  0,  OPC__I, 0, HASH_sltu, MATCH_SLTU, MASK_SLTU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sllw"), 64,  OPC__I, 0, HASH_sllw, MATCH_SLLW, MASK_SLLW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("xor"),   0,  OPC__I, 0, HASH_xor,  MATCH_XOR,  MASK_XOR,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("srl"),   0,  OPC__I, 0, HASH_srl,  MATCH_SRL,  MASK_SRL,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sra"),   0,  OPC__I, 0, HASH_sra,  MATCH_SRA,  MASK_SRA,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("sraw"), 64,  OPC__I, 0, HASH_sraw, MATCH_SRAW, MASK_SRAW, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("or"),    0,  OPC__I, 0, HASH_or,   MATCH_OR,   MASK_OR,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("and"),   0,  OPC__I, 0, HASH_and,  MATCH_AND,  MASK_AND,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },

{ String8__inline_m("beq"),  0,   OPC__I, INSN_CONDBRANCH, HASH_beq,  MATCH_BEQ,  MASK_BEQ,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bne"),  0,   OPC__I, INSN_CONDBRANCH, HASH_bne,  MATCH_BNE,  MASK_BNE,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("blt"),  0,   OPC__I, INSN_CONDBRANCH, HASH_blt,  MATCH_BLT,  MASK_BLT,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bge"),  0,   OPC__I, INSN_CONDBRANCH, HASH_bge,  MATCH_BGE,  MASK_BGE,  OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bltu"), 0,   OPC__I, INSN_CONDBRANCH, HASH_bltu, MATCH_BLTU, MASK_BLTU, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgeu"), 0,   OPC__I, INSN_CONDBRANCH, HASH_bgeu, MATCH_BGEU, MASK_BGEU, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },

{ String8__inline_m("j"),   0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_j,   MATCH_JAL,                      MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Jal)),                                    match_opcode },
{ String8__inline_m("jr"),  0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD|MASK_IMM, OP_m(OP_GPR(OPF_R__S_1)),                                       match_opcode },
{ String8__inline_m("jr"),  0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S_1), OP_PR), match_opcode },
{ String8__inline_m("jr"),  0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD,          OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Immediate(OPF_I__I)),     match_opcode },
{ String8__inline_m("ret"), 0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_ret, MATCH_JALR|(X_RA << OP_SH_RS1), MASK_JALR|MASK_RS1,         OP_m(OP_None),                                                  match_opcode },

{ String8__inline_m("call"), 0,   OPC__I, INSN_MACRO, HASH_call, (X_RA << OP_SH_RS1)|(X_RA << OP_SH_RD), MACRO_CALL, OP_m(OP_Call), 0 },

{ String8__inline_m("li"),  0,    OPC__I, INSN_ALIAS, HASH_li,  MATCH_ADDI, MASK_ADDI|MASK_RS1, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Immediate(OPF_I__I)),      match_opcode     },
{ String8__inline_m("li"),  0,    OPC__I, INSN_MACRO, HASH_li,  0,          MACRO_LI,           OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Constant(OPF_C__Large)),   0                },
{ String8__inline_m("la"),  0,    OPC__I, INSN_MACRO, HASH_la,  0,          MACRO_LA,           OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Constant(OPF_C__Address)), match_rd_nonzero },
{ String8__inline_m("lla"), 0,    OPC__I, INSN_MACRO, HASH_lla,  0,          MACRO_LLA,          OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_Constant(OPF_C__Address)), match_rd_nonzero },

{ String8__inline_m("nop"), 0,    OPC__I, INSN_ALIAS, HASH_nop, MATCH_ADDI, MASK_ADDI|MASK_RD|MASK_RS1|MASK_IMM, OP_m(OP_None), match_opcode },

{ String8__inline_m("not"), 0,    OPC__I, INSN_ALIAS, HASH_not, MATCH_XORI|MASK_IMM, MASK_XORI|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)), match_opcode },

{ String8__inline_m("mv"),  0,    OPC__I, INSN_ALIAS, HASH_mv, MATCH_ADDI, MASK_ADDI|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1)), match_opcode },

{ String8__inline_m("neg"),   0,  OPC__I, INSN_ALIAS, HASH_neg,  MATCH_SUB,  MASK_SUB|MASK_RS1,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("negw"), 64,  OPC__I, INSN_ALIAS, HASH_negw, MATCH_SUBW, MASK_SUBW|MASK_RS1, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },

{ String8__inline_m("beqz"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_beqz, MATCH_BEQ,  MASK_BEQ|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("blez"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_blez, MATCH_BGE,  MASK_BGE|MASK_RS1, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bgez"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgez, MATCH_BGE,  MASK_BGE|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("ble"),  0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_ble,  MATCH_BGE,  MASK_BGE,          OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bltz"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bltz, MATCH_BLT,  MASK_BLT|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bgtz"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgtz, MATCH_BLT,  MASK_BLT|MASK_RS1, OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bleu"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bleu, MATCH_BGEU, MASK_BGEU,         OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgt"),  0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgt,  MATCH_BLT,  MASK_BLT,          OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgtu"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgtu, MATCH_BLTU, MASK_BLTU,         OP_m(OP_GPR(OPF_R__S_2), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bnez"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bnez, MATCH_BNE,  MASK_BNE|MASK_RS2, OP_m(OP_GPR(OPF_R__S_1), OP_Comma, OP_Offset(OPF_O__Branch)),                               match_opcode },

{ String8__inline_m("pause"),  0, OPC__I, 0, HASH_pause,  MATCH_PAUSE,  MASK_PAUSE,  OP_m(OP_None), match_opcode },
{ String8__inline_m("ecall"),  0, OPC__I, 0, HASH_ecall,  MATCH_ECALL,  MASK_ECALL,  OP_m(OP_None), match_opcode },
{ String8__inline_m("ebreak"), 0, OPC__I, 0, HASH_ebreak, MATCH_EBREAK, MASK_EBREAK, OP_m(OP_None), match_opcode },

// First most specific, then generic.
{ String8__inline_m("fence"),     0, OPC__I, 0,          HASH_fence,     MATCH_FENCE,                     MASK_FENCE|MASK_RD|MASK_RS1|(MASK_IMM & ~MASK_PRED & ~MASK_SUCC), OP_m(OP_Predecessor, OP_Comma, OP_Successor), match_opcode },
{ String8__inline_m("fence"),     0, OPC__I, INSN_ALIAS, HASH_fence,     MATCH_FENCE|MASK_PRED|MASK_SUCC, MASK_FENCE|MASK_RD|MASK_RS1|MASK_IMM,                             OP_m(OP_None),                                match_opcode },
{ String8__inline_m("fence.tso"), 0, OPC__I, 0,          HASH_fence_tso, MATCH_FENCE_TSO,                 MASK_FENCE_TSO|MASK_RD|MASK_RS1,                                  OP_m(OP_None),                                match_opcode },

// M/ZMMUL extension instructions.
{ String8__inline_m("mul"),    0, OPC__ZMMUL, 0, HASH_mul,    MATCH_MUL,    MASK_MUL,    OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("mulh"),   0, OPC__ZMMUL,  0, HASH_mulh,   MATCH_MULH,   MASK_MULH,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("mulhsu"), 0, OPC__ZMMUL,    0, HASH_mulhsu, MATCH_MULHSU, MASK_MULHSU, OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("mulhu"),  0, OPC__ZMMUL,   0, HASH_mulhu,  MATCH_MULHU,  MASK_MULHU,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("div"),    0, OPC__M,     0, HASH_div,    MATCH_DIV,    MASK_DIV,    OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("divu"),   0, OPC__M,      0, HASH_divu,   MATCH_DIVU,   MASK_DIVU,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("rem"),    0, OPC__M,     0, HASH_rem,    MATCH_REM,    MASK_REM,    OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("remu"),   0, OPC__M,      0, HASH_remu,   MATCH_REMU,   MASK_REMU,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("mulw"),  64, OPC__ZMMUL, 0, HASH_mulw,   MATCH_MULW,   MASK_MULW,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("divw"),  64, OPC__M,     0, HASH_divw,   MATCH_DIVW,   MASK_DIVW,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("divuw"), 64, OPC__M,     0, HASH_divuw,  MATCH_DIVUW,  MASK_DIVUW,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("remw"),  64, OPC__M,     0, HASH_remw,   MATCH_REMW,   MASK_REMW,   OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },
{ String8__inline_m("remuw"), 64, OPC__M,     0, HASH_remuw,  MATCH_REMUW,  MASK_REMUW,  OP_m(OP_GPR(OPF_R__D), OP_Comma, OP_GPR(OPF_R__S_1), OP_Comma, OP_GPR(OPF_R__S_2)), match_opcode },

{ String8__inline_m(""), 0, OPC__None, 0, 0, 0, 0, 0, 0 }
};

// TODO(low): for now this is dumb enough and works. However, it would be nicer to create a fixed-size hashmap at
// compile-time.
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
        U8 size = 0;

        if (0) {}
        else if ((encoding & 0x3)  !=  0x3) { size = 2; }
        else if ((encoding & 0x1f) != 0x1f) { size = 4; }
        else if ((encoding & 0x3f) == 0x1f) { size = 6; }
        else if ((encoding & 0x7f) == 0x3f) { size = 8; }
        else { unreachable_m() }

        return size;
}

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

// internal const RISCV_Opcode *
// RISCV_Opcode__table_find(U32 instruction_hash);
