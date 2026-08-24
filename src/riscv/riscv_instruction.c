
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

// Floating-point rounding modes (index == encoding in bits 14:12).
//
// Encodings 5 and 6 are RESERVED in the RISC-V spec (no defined rounding
// behavior), so they get no mnemonic.
global const String8 RISCV_rounding_mode_table[] =
{
        String8__literal("rne"),
        String8__literal("rtz"),
        String8__literal("rdn"),
        String8__literal("rup"),
        String8__literal("rmm"),
        {0}, // reserved
        {0}, // reserved
        String8__literal("dyn"),
};

// NOTE: the empty opcode can be distinguished by the zero hash.
global const RISCV_Opcode RISCV_Opcode__table[] =
{
// Base I instructions.
{ String8__inline_m("auipc"), 0, OPC__I, 0, HASH_auipc, MATCH_AUIPC, MASK_AUIPC, OP_m(OP_GPR(OPF_R__D), OP_Immediate(OPF_I__U)), match_opcode },
{ String8__inline_m("lui"),   0, OPC__I, 0, HASH_lui,   MATCH_LUI,   MASK_LUI,   OP_m(OP_GPR(OPF_R__D), OP_Immediate(OPF_I__U)), match_opcode },

// NOTE: important here to go from more specific to less specific.
{ String8__inline_m("jal"),  0, OPC__I,  0,                   HASH_jal,  MATCH_JAL,                     MASK_JAL,                   OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Jal)),                                   match_opcode },
{ String8__inline_m("jal"),  0, OPC__I,  0,                   HASH_jal,  MATCH_JAL|(X_RA << OP_SH_RD),  MASK_JAL| MASK_RD,          OP_m(OP_Offset(OPF_O__Jal)),                                                     match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  0,                   HASH_jalr, MATCH_JALR,                    MASK_JALR,                  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),               match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  INSN_ALIAS|INSN_JSR, HASH_jalr, MATCH_JALR,                    MASK_JALR|MASK_IMM,         OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  INSN_JSR,            HASH_jalr, MATCH_JALR,                    MASK_JALR,                  OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  INSN_ALIAS|INSN_JSR, HASH_jalr, MATCH_JALR|(X_RA << OP_SH_RD), MASK_JALR|MASK_RD|MASK_IMM, OP_m(OP_GPR(OPF_R__S1)),                                                         match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  INSN_ALIAS|INSN_JSR, HASH_jalr, MATCH_JALR|(X_RA << OP_SH_RD), MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR),                   match_opcode },
{ String8__inline_m("jalr"), 0, OPC__I,  INSN_ALIAS|INSN_JSR, HASH_jalr, MATCH_JALR|(X_RA << OP_SH_RD), MASK_JALR|MASK_RD,          OP_m(OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),                                 match_opcode },

{ String8__inline_m("lb"),   0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lb,  MATCH_LB,  MASK_LB,  OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("lb"),   0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lb,  MATCH_LB,  MASK_LB,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("lbu"),  0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lbu, MATCH_LBU, MASK_LBU, OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("lbu"),  0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_lbu, MATCH_LBU, MASK_LBU, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("lh"),   0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lh,  MATCH_LH,  MASK_LH,  OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("lh"),   0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lh,  MATCH_LH,  MASK_LH,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("lhu"),  0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lhu, MATCH_LHU, MASK_LHU, OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("lhu"),  0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_lhu, MATCH_LHU, MASK_LHU, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("lw"),   0, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lw,  MATCH_LW,  MASK_LW,  OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("lw"),   0, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lw,  MATCH_LW,  MASK_LW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("lwu"), 64, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lwu, MATCH_LWU, MASK_LWU, OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("lwu"), 64, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_lwu, MATCH_LWU, MASK_LWU, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("ld"),  64, OPC__I, INSN_DREF|INSN_8_BYTE, HASH_ld,  MATCH_LD,  MASK_LD,  OP_m(OP_GPR(OPF_R__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("ld"),  64, OPC__I, INSN_DREF|INSN_8_BYTE, HASH_ld,  MATCH_LD,  MASK_LD,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                                       match_opcode },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ String8__inline_m("sb"),  0, OPC__I, INSN_DREF|INSN_1_BYTE, HASH_sb, MATCH_SB, MASK_SB, OP_m(OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("sh"),  0, OPC__I, INSN_DREF|INSN_2_BYTE, HASH_sh, MATCH_SH, MASK_SH, OP_m(OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("sw"),  0, OPC__I, INSN_DREF|INSN_4_BYTE, HASH_sw, MATCH_SW, MASK_SW, OP_m(OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("sd"), 64, OPC__I, INSN_DREF|INSN_8_BYTE, HASH_sd, MATCH_SD, MASK_SD, OP_m(OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ String8__inline_m("addi"),   0, OPC__I, 0, HASH_addi,  MATCH_ADDI,  MASK_ADDI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("addiw"), 64, OPC__I, 0, HASH_addiw, MATCH_ADDIW, MASK_ADDIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("slti"),   0, OPC__I, 0, HASH_slti,  MATCH_SLTI,  MASK_SLTI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("sltiu"),  0, OPC__I, 0, HASH_sltiu, MATCH_SLTIU, MASK_SLTIU, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("xori"),   0, OPC__I, 0, HASH_xori,  MATCH_XORI,  MASK_XORI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("ori"),    0, OPC__I, 0, HASH_ori,   MATCH_ORI,   MASK_ORI,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },
{ String8__inline_m("andi"),   0, OPC__I, 0, HASH_andi,  MATCH_ANDI,  MASK_ANDI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)), match_opcode },

{ String8__inline_m("slli"),   0,  OPC__I, 0, HASH_slli,  MATCH_SLLI,  MASK_SLLI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("srli"),   0,  OPC__I, 0, HASH_srli,  MATCH_SRLI,  MASK_SRLI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("srai"),   0,  OPC__I, 0, HASH_srai,  MATCH_SRAI,  MASK_SRAI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("slliw"), 64,  OPC__I, 0, HASH_slliw, MATCH_SLLIW, MASK_SLLIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("srliw"), 64,  OPC__I, 0, HASH_srliw, MATCH_SRLIW, MASK_SRLIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("sraiw"), 64,  OPC__I, 0, HASH_sraiw, MATCH_SRAIW, MASK_SRAIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },

{ String8__inline_m("add"),   0,  OPC__I, 0,          HASH_add,  MATCH_ADD,   MASK_ADD,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("add"),   0,  OPC__I, INSN_ALIAS, HASH_add,  MATCH_ADDI,  MASK_ADDI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("sub"),   0,  OPC__I, 0,          HASH_sub,  MATCH_SUB,   MASK_SUB,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("addw"), 64,  OPC__I, 0,          HASH_addw, MATCH_ADDW,  MASK_ADDW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("addw"), 64,  OPC__I, INSN_ALIAS, HASH_addw, MATCH_ADDIW, MASK_ADDIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("subw"), 64,  OPC__I, 0,          HASH_subw, MATCH_SUBW,  MASK_SUBW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sll"),   0,  OPC__I, 0,          HASH_sll,  MATCH_SLL,   MASK_SLL,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sll"),   0,  OPC__I, INSN_ALIAS, HASH_sll,  MATCH_SLLI,  MASK_SLLI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("slt"),   0,  OPC__I, 0,          HASH_slt,  MATCH_SLT,   MASK_SLT,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("slt"),   0,  OPC__I, INSN_ALIAS, HASH_slt,  MATCH_SLTI,  MASK_SLTI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("sltu"),  0,  OPC__I, 0,          HASH_sltu, MATCH_SLTU,  MASK_SLTU,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sltu"),  0,  OPC__I, INSN_ALIAS, HASH_sltu, MATCH_SLTIU, MASK_SLTIU, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("sllw"), 64,  OPC__I, 0,          HASH_sllw, MATCH_SLLW,  MASK_SLLW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sllw"), 64,  OPC__I, INSN_ALIAS, HASH_sllw, MATCH_SLLIW, MASK_SLLIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("xor"),   0,  OPC__I, 0,          HASH_xor,  MATCH_XOR,   MASK_XOR,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("xor"),   0,  OPC__I, INSN_ALIAS, HASH_xor,  MATCH_XORI,  MASK_XORI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("srl"),   0,  OPC__I, 0,          HASH_srl,  MATCH_SRL,   MASK_SRL,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("srl"),   0,  OPC__I, INSN_ALIAS, HASH_srl,  MATCH_SRLI,  MASK_SRLI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("sra"),   0,  OPC__I, 0,          HASH_sra,  MATCH_SRA,   MASK_SRA,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sra"),   0,  OPC__I, INSN_ALIAS, HASH_sra,  MATCH_SRAI,  MASK_SRAI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("sraw"), 64,  OPC__I, 0,          HASH_sraw, MATCH_SRAW,  MASK_SRAW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sraw"), 64,  OPC__I, INSN_ALIAS, HASH_sraw, MATCH_SRAIW, MASK_SRAIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("or"),    0,  OPC__I, 0,          HASH_or,   MATCH_OR,    MASK_OR,    OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("or"),    0,  OPC__I, INSN_ALIAS, HASH_or,   MATCH_ORI,   MASK_ORI,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("and"),   0,  OPC__I, 0,          HASH_and,  MATCH_AND,   MASK_AND,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("and"),   0,  OPC__I, INSN_ALIAS, HASH_and,  MATCH_ANDI,  MASK_ANDI,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),   match_opcode },
{ String8__inline_m("srlw"), 64,  OPC__I, 0,          HASH_srlw, MATCH_SRLW,  MASK_SRLW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("srlw"), 64,  OPC__I, INSN_ALIAS, HASH_srlw, MATCH_SRLIW, MASK_SRLIW, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },

{ String8__inline_m("sext.w"), 64, OPC__I, INSN_ALIAS, HASH_sext_w, MATCH_ADDIW, MASK_ADDIW|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("seqz"), 0, OPC__I, INSN_ALIAS, HASH_seqz, MATCH_SLTIU|(1 << 20),   MASK_SLTIU|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                    match_opcode },
{ String8__inline_m("snez"), 0, OPC__I, INSN_ALIAS, HASH_snez, MATCH_SLTU,              MASK_SLTU|MASK_RS1,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S2)),                    match_opcode },
{ String8__inline_m("sltz"), 0, OPC__I, INSN_ALIAS, HASH_sltz, MATCH_SLT,               MASK_SLT|MASK_RS2,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                    match_opcode },
{ String8__inline_m("sgtz"), 0, OPC__I, INSN_ALIAS, HASH_sgtz, MATCH_SLT,               MASK_SLT|MASK_RS1,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S2)),                    match_opcode },
{ String8__inline_m("sgt"),  0, OPC__I, INSN_ALIAS, HASH_sgt,  MATCH_SLT,               MASK_SLT,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S2), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("sgtu"), 0, OPC__I, INSN_ALIAS, HASH_sgtu, MATCH_SLTU,              MASK_SLTU,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S2), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("beq"),  0,   OPC__I, INSN_CONDBRANCH, HASH_beq,  MATCH_BEQ,  MASK_BEQ,  OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bne"),  0,   OPC__I, INSN_CONDBRANCH, HASH_bne,  MATCH_BNE,  MASK_BNE,  OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("blt"),  0,   OPC__I, INSN_CONDBRANCH, HASH_blt,  MATCH_BLT,  MASK_BLT,  OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bge"),  0,   OPC__I, INSN_CONDBRANCH, HASH_bge,  MATCH_BGE,  MASK_BGE,  OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bltu"), 0,   OPC__I, INSN_CONDBRANCH, HASH_bltu, MATCH_BLTU, MASK_BLTU, OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgeu"), 0,   OPC__I, INSN_CONDBRANCH, HASH_bgeu, MATCH_BGEU, MASK_BGEU, OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)), match_opcode },

{ String8__inline_m("j"),   0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_j,   MATCH_JAL,                      MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Jal)),                                    match_opcode },
{ String8__inline_m("jr"),  0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD|MASK_IMM, OP_m(OP_GPR(OPF_R__S1)),                                       match_opcode },
{ String8__inline_m("jr"),  0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD,          OP_m(OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("jr"),  0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_jr,  MATCH_JALR,                     MASK_JALR|MASK_RD,          OP_m(OP_GPR(OPF_R__S1), OP_Immediate(OPF_I__I)),     match_opcode },
{ String8__inline_m("ret"), 0,    OPC__I, INSN_ALIAS|INSN_JSR, HASH_ret, MATCH_JALR|(X_RA << OP_SH_RS1), MASK_JALR|MASK_RS1,         OP_m(OP_None),                                                  match_opcode },

{ String8__inline_m("call"), 0,   OPC__I, INSN_MACRO, HASH_call, (X_RA << OP_SH_RS1)|(X_RA << OP_SH_RD), MACRO_CALL, OP_m(OP_Call), 0 },
{ String8__inline_m("tail"), 0,   OPC__I, INSN_MACRO, HASH_tail, (X_T1 << OP_SH_RS1),                    MACRO_CALL, OP_m(OP_Call), 0 },

{ String8__inline_m("li"),  0,    OPC__I, INSN_ALIAS, HASH_li,  MATCH_ADDI, MASK_ADDI|MASK_RS1, OP_m(OP_GPR(OPF_R__D), OP_Immediate(OPF_I__I)),      match_opcode     },
{ String8__inline_m("li"),  0,    OPC__I, INSN_MACRO, HASH_li,  0,          MACRO_LI,           OP_m(OP_GPR(OPF_R__D), OP_Constant(OPF_C__Large)),   0                },
{ String8__inline_m("la"),  0,    OPC__I, INSN_MACRO, HASH_la,  0,          MACRO_LA,           OP_m(OP_GPR(OPF_R__D), OP_Constant(OPF_C__Address)), match_rd_nonzero },
{ String8__inline_m("lla"), 0,    OPC__I, INSN_MACRO, HASH_lla,  0,          MACRO_LLA,          OP_m(OP_GPR(OPF_R__D), OP_Constant(OPF_C__Address)), match_rd_nonzero },
{ String8__inline_m("lga"), 0,    OPC__I, INSN_MACRO, HASH_lga,  0,          MACRO_LGA,          OP_m(OP_GPR(OPF_R__D), OP_Constant(OPF_C__Address)), match_rd_nonzero },

{ String8__inline_m("nop"), 0,    OPC__I, INSN_ALIAS, HASH_nop, MATCH_ADDI, MASK_ADDI|MASK_RD|MASK_RS1|MASK_IMM, OP_m(OP_None), match_opcode },

{ String8__inline_m("not"), 0,    OPC__I, INSN_ALIAS, HASH_not, MATCH_XORI|MASK_IMM, MASK_XORI|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("mv"),   0,   OPC__I, INSN_ALIAS, HASH_mv,   MATCH_ADDI, MASK_ADDI|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("move"), 0,   OPC__I, INSN_ALIAS, HASH_move, MATCH_ADDI, MASK_ADDI|MASK_IMM, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("neg"),   0,  OPC__I, INSN_ALIAS, HASH_neg,  MATCH_SUB,  MASK_SUB|MASK_RS1,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("negw"), 64,  OPC__I, INSN_ALIAS, HASH_negw, MATCH_SUBW, MASK_SUBW|MASK_RS1, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S2)), match_opcode },

{ String8__inline_m("beqz"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_beqz, MATCH_BEQ,  MASK_BEQ|MASK_RS2, OP_m(OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("blez"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_blez, MATCH_BGE,  MASK_BGE|MASK_RS1, OP_m(OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bgez"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgez, MATCH_BGE,  MASK_BGE|MASK_RS2, OP_m(OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("ble"),  0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_ble,  MATCH_BGE,  MASK_BGE,          OP_m(OP_GPR(OPF_R__S2), OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bltz"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bltz, MATCH_BLT,  MASK_BLT|MASK_RS2, OP_m(OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bgtz"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgtz, MATCH_BLT,  MASK_BLT|MASK_RS1, OP_m(OP_GPR(OPF_R__S2), OP_Offset(OPF_O__Branch)),                               match_opcode },
{ String8__inline_m("bleu"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bleu, MATCH_BGEU, MASK_BGEU,         OP_m(OP_GPR(OPF_R__S2), OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgt"),  0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgt,  MATCH_BLT,  MASK_BLT,          OP_m(OP_GPR(OPF_R__S2), OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bgtu"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bgtu, MATCH_BLTU, MASK_BLTU,         OP_m(OP_GPR(OPF_R__S2), OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)), match_opcode },
{ String8__inline_m("bnez"), 0,   OPC__I, INSN_ALIAS|INSN_CONDBRANCH, HASH_bnez, MATCH_BNE,  MASK_BNE|MASK_RS2, OP_m(OP_GPR(OPF_R__S1), OP_Offset(OPF_O__Branch)),                               match_opcode },

{ String8__inline_m("pause"),  0, OPC__I, 0,          HASH_pause,  MATCH_PAUSE,  MASK_PAUSE,  OP_m(OP_None), match_opcode },
{ String8__inline_m("ecall"),  0, OPC__I, 0,          HASH_ecall,  MATCH_ECALL,  MASK_ECALL,  OP_m(OP_None), match_opcode },
{ String8__inline_m("ebreak"), 0, OPC__I, 0,          HASH_ebreak, MATCH_EBREAK, MASK_EBREAK, OP_m(OP_None), match_opcode },
{ String8__inline_m("scall"),  0, OPC__I, INSN_ALIAS, HASH_scall,  MATCH_SCALL,  MASK_SCALL,  OP_m(OP_None), match_opcode },
{ String8__inline_m("sbreak"), 0, OPC__I, INSN_ALIAS, HASH_sbreak, MATCH_SBREAK, MASK_SBREAK, OP_m(OP_None), match_opcode },

// Privileged / system instructions.
{ String8__inline_m("uret"),       0, OPC__I, 0,          HASH_uret, MATCH_URET, MASK_URET, OP_m(OP_None), match_opcode },
{ String8__inline_m("sret"),       0, OPC__I, 0,          HASH_sret, MATCH_SRET, MASK_SRET, OP_m(OP_None), match_opcode },
{ String8__inline_m("hret"),       0, OPC__I, 0,          HASH_hret, MATCH_HRET, MASK_HRET, OP_m(OP_None), match_opcode },
{ String8__inline_m("mret"),       0, OPC__I, 0,          HASH_mret, MATCH_MRET, MASK_MRET, OP_m(OP_None), match_opcode },
{ String8__inline_m("dret"),       0, OPC__I, 0,          HASH_dret, MATCH_DRET, MASK_DRET, OP_m(OP_None), match_opcode },
{ String8__inline_m("wfi"),        0, OPC__I, 0,          HASH_wfi,  MATCH_WFI,  MASK_WFI,  OP_m(OP_None), match_opcode },

{ String8__inline_m("sfence.vma"), 0, OPC__I, 0,          HASH_sfence_vma, MATCH_SFENCE_VMA, MASK_SFENCE_VMA,                   OP_m(OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),  match_opcode },
{ String8__inline_m("sfence.vma"), 0, OPC__I, INSN_ALIAS, HASH_sfence_vma, MATCH_SFENCE_VMA, MASK_SFENCE_VMA|MASK_RS2,          OP_m(OP_GPR(OPF_R__S1)),                     match_opcode },
{ String8__inline_m("sfence.vma"), 0, OPC__I, INSN_ALIAS, HASH_sfence_vma, MATCH_SFENCE_VMA, MASK_SFENCE_VMA|MASK_RS1|MASK_RS2, OP_m(OP_None),                               match_opcode },

{ String8__inline_m("fence"),     0, OPC__I, 0,          HASH_fence,     MATCH_FENCE,                     MASK_FENCE|MASK_RD|MASK_RS1|(MASK_IMM & ~MASK_PRED & ~MASK_SUCC), OP_m(OP_Predecessor, OP_Successor), match_opcode },
{ String8__inline_m("fence"),     0, OPC__I, INSN_ALIAS, HASH_fence,     MATCH_FENCE|MASK_PRED|MASK_SUCC, MASK_FENCE|MASK_RD|MASK_RS1|MASK_IMM,                             OP_m(OP_None),                                match_opcode },
{ String8__inline_m("fence.tso"), 0, OPC__I, 0,          HASH_fence_tso, MATCH_FENCE_TSO,                 MASK_FENCE_TSO|MASK_RD|MASK_RS1,                                  OP_m(OP_None),                                match_opcode },

// M/ZMMUL extension instructions.
{ String8__inline_m("mul"),    0, OPC__ZMMUL,  0, HASH_mul,    MATCH_MUL,    MASK_MUL,    OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("mulh"),   0, OPC__ZMMUL,  0, HASH_mulh,   MATCH_MULH,   MASK_MULH,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("mulhsu"), 0, OPC__ZMMUL,  0, HASH_mulhsu, MATCH_MULHSU, MASK_MULHSU, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("mulhu"),  0, OPC__ZMMUL,  0, HASH_mulhu,  MATCH_MULHU,  MASK_MULHU,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("div"),    0, OPC__M,      0, HASH_div,    MATCH_DIV,    MASK_DIV,    OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("divu"),   0, OPC__M,      0, HASH_divu,   MATCH_DIVU,   MASK_DIVU,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("rem"),    0, OPC__M,      0, HASH_rem,    MATCH_REM,    MASK_REM,    OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("remu"),   0, OPC__M,      0, HASH_remu,   MATCH_REMU,   MASK_REMU,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("mulw"),  64, OPC__ZMMUL,  0, HASH_mulw,   MATCH_MULW,   MASK_MULW,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("divw"),  64, OPC__M,      0, HASH_divw,   MATCH_DIVW,   MASK_DIVW,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("divuw"), 64, OPC__M,      0, HASH_divuw,  MATCH_DIVUW,  MASK_DIVUW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("remw"),  64, OPC__M,      0, HASH_remw,   MATCH_REMW,   MASK_REMW,   OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("remuw"), 64, OPC__M,      0, HASH_remuw,  MATCH_REMUW,  MASK_REMUW,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },

// F extension (single-precision floating-point).
// NOTE: for instructions with an optional rounding-mode operand, the 4-operand (rm) form comes
// first, since this parser does not verify that all input was consumed (unlike GNU as).
{ String8__inline_m("flw"), 0, OPC__F, INSN_DREF|INSN_4_BYTE, HASH_flw, MATCH_FLW, MASK_FLW, OP_m(OP_FPR(OPF_FPR__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },
{ String8__inline_m("fsw"), 0, OPC__F, INSN_DREF|INSN_4_BYTE, HASH_fsw, MATCH_FSW, MASK_FSW, OP_m(OP_FPR(OPF_FPR__S2), OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },

{ String8__inline_m("fmv.x.w"), 0, OPC__F, INSN_ALIAS, HASH_fmv_x_w, MATCH_FMV_X_S, MASK_FMV_X_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fmv.w.x"), 0, OPC__F, INSN_ALIAS, HASH_fmv_w_x, MATCH_FMV_S_X, MASK_FMV_S_X, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fmv.x.s"), 0, OPC__F, INSN_ALIAS, HASH_fmv_x_s, MATCH_FMV_X_S, MASK_FMV_X_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fmv.s.x"), 0, OPC__F, INSN_ALIAS, HASH_fmv_s_x, MATCH_FMV_S_X, MASK_FMV_S_X, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("fmv.s"),  0, OPC__F, INSN_ALIAS, HASH_fmv_s,  MATCH_FSGNJ_S,  MASK_FSGNJ_S,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S12)), match_opcode },
{ String8__inline_m("fneg.s"), 0, OPC__F, INSN_ALIAS, HASH_fneg_s, MATCH_FSGNJN_S, MASK_FSGNJN_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S12)), match_opcode },
{ String8__inline_m("fabs.s"), 0, OPC__F, INSN_ALIAS, HASH_fabs_s, MATCH_FSGNJX_S, MASK_FSGNJX_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S12)), match_opcode },

{ String8__inline_m("fsgnj.s"),  0, OPC__F, 0, HASH_fsgnj_s,  MATCH_FSGNJ_S,  MASK_FSGNJ_S,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fsgnjn.s"), 0, OPC__F, 0, HASH_fsgnjn_s, MATCH_FSGNJN_S, MASK_FSGNJN_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fsgnjx.s"), 0, OPC__F, 0, HASH_fsgnjx_s, MATCH_FSGNJX_S, MASK_FSGNJX_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },

{ String8__inline_m("fadd.s"), 0, OPC__F, 0, HASH_fadd_s, MATCH_FADD_S, MASK_FADD_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fadd.s"), 0, OPC__F, 0, HASH_fadd_s, MATCH_FADD_S|MASK_RM, MASK_FADD_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fsub.s"), 0, OPC__F, 0, HASH_fsub_s, MATCH_FSUB_S, MASK_FSUB_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fsub.s"), 0, OPC__F, 0, HASH_fsub_s, MATCH_FSUB_S|MASK_RM, MASK_FSUB_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fmul.s"), 0, OPC__F, 0, HASH_fmul_s, MATCH_FMUL_S, MASK_FMUL_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fmul.s"), 0, OPC__F, 0, HASH_fmul_s, MATCH_FMUL_S|MASK_RM, MASK_FMUL_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fdiv.s"), 0, OPC__F, 0, HASH_fdiv_s, MATCH_FDIV_S, MASK_FDIV_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fdiv.s"), 0, OPC__F, 0, HASH_fdiv_s, MATCH_FDIV_S|MASK_RM, MASK_FDIV_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },

{ String8__inline_m("fsqrt.s"), 0, OPC__F, 0, HASH_fsqrt_s, MATCH_FSQRT_S, MASK_FSQRT_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fsqrt.s"), 0, OPC__F, 0, HASH_fsqrt_s, MATCH_FSQRT_S|MASK_RM, MASK_FSQRT_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1)), match_opcode },

{ String8__inline_m("fmin.s"), 0, OPC__F, 0, HASH_fmin_s, MATCH_FMIN_S, MASK_FMIN_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fmax.s"), 0, OPC__F, 0, HASH_fmax_s, MATCH_FMAX_S, MASK_FMAX_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },

{ String8__inline_m("fmadd.s"),  0, OPC__F, 0, HASH_fmadd_s,  MATCH_FMADD_S,  MASK_FMADD_S,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fmadd.s"),  0, OPC__F, 0, HASH_fmadd_s,  MATCH_FMADD_S|MASK_RM,  MASK_FMADD_S|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },
{ String8__inline_m("fnmadd.s"), 0, OPC__F, 0, HASH_fnmadd_s, MATCH_FNMADD_S, MASK_FNMADD_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fnmadd.s"), 0, OPC__F, 0, HASH_fnmadd_s, MATCH_FNMADD_S|MASK_RM, MASK_FNMADD_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },
{ String8__inline_m("fmsub.s"),  0, OPC__F, 0, HASH_fmsub_s,  MATCH_FMSUB_S,  MASK_FMSUB_S,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fmsub.s"),  0, OPC__F, 0, HASH_fmsub_s,  MATCH_FMSUB_S|MASK_RM,  MASK_FMSUB_S|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },
{ String8__inline_m("fnmsub.s"), 0, OPC__F, 0, HASH_fnmsub_s, MATCH_FNMSUB_S, MASK_FNMSUB_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fnmsub.s"), 0, OPC__F, 0, HASH_fnmsub_s, MATCH_FNMSUB_S|MASK_RM, MASK_FNMSUB_S|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },

{ String8__inline_m("fcvt.w.s"),  0, OPC__F, 0, HASH_fcvt_w_s,  MATCH_FCVT_W_S,  MASK_FCVT_W_S,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.w.s"),  0, OPC__F, 0, HASH_fcvt_w_s,  MATCH_FCVT_W_S|MASK_RM,  MASK_FCVT_W_S|MASK_RM,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.wu.s"), 0, OPC__F, 0, HASH_fcvt_wu_s, MATCH_FCVT_WU_S, MASK_FCVT_WU_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.wu.s"), 0, OPC__F, 0, HASH_fcvt_wu_s, MATCH_FCVT_WU_S|MASK_RM, MASK_FCVT_WU_S|MASK_RM, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.l.s"), 64, OPC__F, 0, HASH_fcvt_l_s,  MATCH_FCVT_L_S,  MASK_FCVT_L_S,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.l.s"), 64, OPC__F, 0, HASH_fcvt_l_s,  MATCH_FCVT_L_S|MASK_RM,  MASK_FCVT_L_S|MASK_RM,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.lu.s"), 64, OPC__F, 0, HASH_fcvt_lu_s, MATCH_FCVT_LU_S, MASK_FCVT_LU_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.lu.s"), 64, OPC__F, 0, HASH_fcvt_lu_s, MATCH_FCVT_LU_S|MASK_RM, MASK_FCVT_LU_S|MASK_RM, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },

{ String8__inline_m("fcvt.s.w"),   0, OPC__F, 0, HASH_fcvt_s_w,  MATCH_FCVT_S_W,          MASK_FCVT_S_W,          OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.s.w"),   0, OPC__F, 0, HASH_fcvt_s_w,  MATCH_FCVT_S_W|MASK_RM,  MASK_FCVT_S_W|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fcvt.s.wu"),  0, OPC__F, 0, HASH_fcvt_s_wu, MATCH_FCVT_S_WU,         MASK_FCVT_S_WU,         OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.s.wu"),  0, OPC__F, 0, HASH_fcvt_s_wu, MATCH_FCVT_S_WU|MASK_RM, MASK_FCVT_S_WU|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fcvt.s.l"),  64, OPC__F, 0, HASH_fcvt_s_l,  MATCH_FCVT_S_L,          MASK_FCVT_S_L,          OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.s.l"),  64, OPC__F, 0, HASH_fcvt_s_l,  MATCH_FCVT_S_L|MASK_RM,  MASK_FCVT_S_L|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fcvt.s.lu"), 64, OPC__F, 0, HASH_fcvt_s_lu, MATCH_FCVT_S_LU,         MASK_FCVT_S_LU,         OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.s.lu"), 64, OPC__F, 0, HASH_fcvt_s_lu, MATCH_FCVT_S_LU|MASK_RM, MASK_FCVT_S_LU|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("fclass.s"), 0, OPC__F, 0, HASH_fclass_s, MATCH_FCLASS_S, MASK_FCLASS_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("feq.s"), 0, OPC__F, 0, HASH_feq_s, MATCH_FEQ_S, MASK_FEQ_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("flt.s"), 0, OPC__F, 0, HASH_flt_s, MATCH_FLT_S, MASK_FLT_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fle.s"), 0, OPC__F, 0, HASH_fle_s, MATCH_FLE_S, MASK_FLE_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fgt.s"), 0, OPC__F, INSN_ALIAS, HASH_fgt_s, MATCH_FLT_S, MASK_FLT_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fge.s"), 0, OPC__F, INSN_ALIAS, HASH_fge_s, MATCH_FLE_S, MASK_FLE_S, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S1)), match_opcode },

// D extension (double-precision floating-point).
{ String8__inline_m("fld"), 0, OPC__D, INSN_DREF|INSN_8_BYTE, HASH_fld, MATCH_FLD, MASK_FLD, OP_m(OP_FPR(OPF_FPR__D), OP_Offset(OPF_O__Load), OP_PL, OP_GPR(OPF_R__S1), OP_PR),   match_opcode },
{ String8__inline_m("fsd"), 0, OPC__D, INSN_DREF|INSN_8_BYTE, HASH_fsd, MATCH_FSD, MASK_FSD, OP_m(OP_FPR(OPF_FPR__S2), OP_Offset(OPF_O__Store), OP_PL, OP_GPR(OPF_R__S1), OP_PR), match_opcode },

{ String8__inline_m("fmv.x.d"), 64, OPC__D, 0, HASH_fmv_x_d, MATCH_FMV_X_D, MASK_FMV_X_D, OP_m(OP_GPR(OPF_R__D),   OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fmv.d.x"), 64, OPC__D, 0, HASH_fmv_d_x, MATCH_FMV_D_X, MASK_FMV_D_X, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)),   match_opcode },

{ String8__inline_m("fmv.d"),  0, OPC__D, INSN_ALIAS, HASH_fmv_d,  MATCH_FSGNJ_D,  MASK_FSGNJ_D,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S12)), match_opcode },
{ String8__inline_m("fneg.d"), 0, OPC__D, INSN_ALIAS, HASH_fneg_d, MATCH_FSGNJN_D, MASK_FSGNJN_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S12)), match_opcode },
{ String8__inline_m("fabs.d"), 0, OPC__D, INSN_ALIAS, HASH_fabs_d, MATCH_FSGNJX_D, MASK_FSGNJX_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S12)), match_opcode },

{ String8__inline_m("fsgnj.d"),  0, OPC__D, 0, HASH_fsgnj_d,  MATCH_FSGNJ_D,  MASK_FSGNJ_D,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fsgnjn.d"), 0, OPC__D, 0, HASH_fsgnjn_d, MATCH_FSGNJN_D, MASK_FSGNJN_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fsgnjx.d"), 0, OPC__D, 0, HASH_fsgnjx_d, MATCH_FSGNJX_D, MASK_FSGNJX_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },

{ String8__inline_m("fadd.d"), 0, OPC__D, 0, HASH_fadd_d, MATCH_FADD_D,         MASK_FADD_D,         OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fadd.d"), 0, OPC__D, 0, HASH_fadd_d, MATCH_FADD_D|MASK_RM, MASK_FADD_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)),                   match_opcode },
{ String8__inline_m("fsub.d"), 0, OPC__D, 0, HASH_fsub_d, MATCH_FSUB_D,         MASK_FSUB_D,         OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fsub.d"), 0, OPC__D, 0, HASH_fsub_d, MATCH_FSUB_D|MASK_RM, MASK_FSUB_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)),                   match_opcode },
{ String8__inline_m("fmul.d"), 0, OPC__D, 0, HASH_fmul_d, MATCH_FMUL_D,         MASK_FMUL_D,         OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fmul.d"), 0, OPC__D, 0, HASH_fmul_d, MATCH_FMUL_D|MASK_RM, MASK_FMUL_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)),                   match_opcode },
{ String8__inline_m("fdiv.d"), 0, OPC__D, 0, HASH_fdiv_d, MATCH_FDIV_D,         MASK_FDIV_D,         OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fdiv.d"), 0, OPC__D, 0, HASH_fdiv_d, MATCH_FDIV_D|MASK_RM, MASK_FDIV_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)),                   match_opcode },

{ String8__inline_m("fsqrt.d"), 0, OPC__D, 0, HASH_fsqrt_d, MATCH_FSQRT_D,         MASK_FSQRT_D,         OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fsqrt.d"), 0, OPC__D, 0, HASH_fsqrt_d, MATCH_FSQRT_D|MASK_RM, MASK_FSQRT_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1)),                   match_opcode },

{ String8__inline_m("fmin.d"), 0, OPC__D, 0, HASH_fmin_d, MATCH_FMIN_D, MASK_FMIN_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fmax.d"), 0, OPC__D, 0, HASH_fmax_d, MATCH_FMAX_D, MASK_FMAX_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },

{ String8__inline_m("fmadd.d"),  0, OPC__D, 0, HASH_fmadd_d,  MATCH_FMADD_D,  MASK_FMADD_D,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fmadd.d"),  0, OPC__D, 0, HASH_fmadd_d,  MATCH_FMADD_D|MASK_RM,  MASK_FMADD_D|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },
{ String8__inline_m("fnmadd.d"), 0, OPC__D, 0, HASH_fnmadd_d, MATCH_FNMADD_D, MASK_FNMADD_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fnmadd.d"), 0, OPC__D, 0, HASH_fnmadd_d, MATCH_FNMADD_D|MASK_RM, MASK_FNMADD_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },
{ String8__inline_m("fmsub.d"),  0, OPC__D, 0, HASH_fmsub_d,  MATCH_FMSUB_D,  MASK_FMSUB_D,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fmsub.d"),  0, OPC__D, 0, HASH_fmsub_d,  MATCH_FMSUB_D|MASK_RM,  MASK_FMSUB_D|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },
{ String8__inline_m("fnmsub.d"), 0, OPC__D, 0, HASH_fnmsub_d, MATCH_FNMSUB_D, MASK_FNMSUB_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fnmsub.d"), 0, OPC__D, 0, HASH_fnmsub_d, MATCH_FNMSUB_D|MASK_RM, MASK_FNMSUB_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S3)), match_opcode },

{ String8__inline_m("fcvt.w.d"),   0, OPC__D, 0, HASH_fcvt_w_d,  MATCH_FCVT_W_D,  MASK_FCVT_W_D,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.w.d"),   0, OPC__D, 0, HASH_fcvt_w_d,  MATCH_FCVT_W_D|MASK_RM,  MASK_FCVT_W_D|MASK_RM,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.wu.d"),  0, OPC__D, 0, HASH_fcvt_wu_d, MATCH_FCVT_WU_D, MASK_FCVT_WU_D, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.wu.d"),  0, OPC__D, 0, HASH_fcvt_wu_d, MATCH_FCVT_WU_D|MASK_RM, MASK_FCVT_WU_D|MASK_RM, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.l.d"),  64, OPC__D, 0, HASH_fcvt_l_d,  MATCH_FCVT_L_D,  MASK_FCVT_L_D,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.l.d"),  64, OPC__D, 0, HASH_fcvt_l_d,  MATCH_FCVT_L_D|MASK_RM,  MASK_FCVT_L_D|MASK_RM,  OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.lu.d"), 64, OPC__D, 0, HASH_fcvt_lu_d, MATCH_FCVT_LU_D, MASK_FCVT_LU_D, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.lu.d"), 64, OPC__D, 0, HASH_fcvt_lu_d, MATCH_FCVT_LU_D|MASK_RM, MASK_FCVT_LU_D|MASK_RM, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)), match_opcode },

{ String8__inline_m("fcvt.d.w"),   0, OPC__D, 0, HASH_fcvt_d_w,  MATCH_FCVT_D_W,  MASK_FCVT_D_W,  OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fcvt.d.wu"),  0, OPC__D, 0, HASH_fcvt_d_wu, MATCH_FCVT_D_WU, MASK_FCVT_D_WU, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fcvt.d.l"),  64, OPC__D, 0, HASH_fcvt_d_l,  MATCH_FCVT_D_L,  MASK_FCVT_D_L,  OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.d.l"),  64, OPC__D, 0, HASH_fcvt_d_l,  MATCH_FCVT_D_L|MASK_RM,  MASK_FCVT_D_L|MASK_RM,  OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },
{ String8__inline_m("fcvt.d.lu"), 64, OPC__D, 0, HASH_fcvt_d_lu, MATCH_FCVT_D_LU, MASK_FCVT_D_LU, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.d.lu"), 64, OPC__D, 0, HASH_fcvt_d_lu, MATCH_FCVT_D_LU|MASK_RM, MASK_FCVT_D_LU|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_GPR(OPF_R__S1)), match_opcode },

{ String8__inline_m("fcvt.d.s"), 0, OPC__D, 0, HASH_fcvt_d_s, MATCH_FCVT_D_S, MASK_FCVT_D_S, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fcvt.s.d"), 0, OPC__D, 0, HASH_fcvt_s_d, MATCH_FCVT_S_D, MASK_FCVT_S_D, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1), OP_Rounding_Mode), match_opcode },
{ String8__inline_m("fcvt.s.d"), 0, OPC__D, 0, HASH_fcvt_s_d, MATCH_FCVT_S_D|MASK_RM, MASK_FCVT_S_D|MASK_RM, OP_m(OP_FPR(OPF_FPR__D), OP_FPR(OPF_FPR__S1)), match_opcode },

{ String8__inline_m("fclass.d"), 0, OPC__D, 0,          HASH_fclass_d, MATCH_FCLASS_D, MASK_FCLASS_D, OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1)),                      match_opcode },
{ String8__inline_m("feq.d"),    0, OPC__D, 0,          HASH_feq_d,    MATCH_FEQ_D,    MASK_FEQ_D,    OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("flt.d"),    0, OPC__D, 0,          HASH_flt_d,    MATCH_FLT_D,    MASK_FLT_D,    OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fle.d"),    0, OPC__D, 0,          HASH_fle_d,    MATCH_FLE_D,    MASK_FLE_D,    OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S1), OP_FPR(OPF_FPR__S2)), match_opcode },
{ String8__inline_m("fgt.d"),    0, OPC__D, INSN_ALIAS, HASH_fgt_d,    MATCH_FLT_D,    MASK_FLT_D,    OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S1)), match_opcode },
{ String8__inline_m("fge.d"),    0, OPC__D, INSN_ALIAS, HASH_fge_d,    MATCH_FLE_D,    MASK_FLE_D,    OP_m(OP_GPR(OPF_R__D), OP_FPR(OPF_FPR__S2), OP_FPR(OPF_FPR__S1)), match_opcode },

// Zifencei extension.
{ String8__inline_m("fence.i"), 0, OPC__ZIFENCEI, 0, HASH_fence_i, MATCH_FENCE_I, MASK_FENCE|MASK_RD|MASK_RS1|MASK_IMM, OP_m(OP_None), match_opcode },

// Zicntr extension (pseudo-ops).
{ String8__inline_m("rdcycle"),    0, OPC__ZICNTR, INSN_ALIAS, HASH_rdcycle,    MATCH_RDCYCLE,   MASK_RDCYCLE,   OP_m(OP_GPR(OPF_R__D)), match_opcode },
{ String8__inline_m("rdtime"),     0, OPC__ZICNTR, INSN_ALIAS, HASH_rdtime,     MATCH_RDTIME,    MASK_RDTIME,    OP_m(OP_GPR(OPF_R__D)), match_opcode },
{ String8__inline_m("rdinstret"),  0, OPC__ZICNTR, INSN_ALIAS, HASH_rdinstret,  MATCH_RDINSTRET, MASK_RDINSTRET, OP_m(OP_GPR(OPF_R__D)), match_opcode },

// Zicond extension.
{ String8__inline_m("czero.eqz"), 0, OPC__ZICOND, 0, HASH_czero_eqz, MATCH_CZERO_EQZ, MASK_CZERO_EQZ, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("czero.nez"), 0, OPC__ZICOND, 0, HASH_czero_nez, MATCH_CZERO_NEZ, MASK_CZERO_NEZ, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },

// Zba extension (address generation).
{ String8__inline_m("sh1add"),     0,  OPC__ZBA, 0,          HASH_sh1add,    MATCH_SH1ADD,    MASK_SH1ADD,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("sh2add"),     0,  OPC__ZBA, 0,          HASH_sh2add,    MATCH_SH2ADD,    MASK_SH2ADD,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("sh3add"),     0,  OPC__ZBA, 0,          HASH_sh3add,    MATCH_SH3ADD,    MASK_SH3ADD,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("sh1add.uw"), 64,  OPC__ZBA, 0,          HASH_sh1add_uw, MATCH_SH1ADD_UW, MASK_SH1ADD_UW,       OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("sh2add.uw"), 64,  OPC__ZBA, 0,          HASH_sh2add_uw, MATCH_SH2ADD_UW, MASK_SH2ADD_UW,       OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("sh3add.uw"), 64,  OPC__ZBA, 0,          HASH_sh3add_uw, MATCH_SH3ADD_UW, MASK_SH3ADD_UW,       OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("add.uw"),    64,  OPC__ZBA, 0,          HASH_add_uw,    MATCH_ADD_UW,    MASK_ADD_UW,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),      match_opcode },
{ String8__inline_m("slli.uw"),   64,  OPC__ZBA, 0,          HASH_slli_uw,   MATCH_SLLI_UW,   MASK_SLLI_UW,         OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)), match_opcode },
{ String8__inline_m("zext.w"),    64,  OPC__ZBA, INSN_ALIAS, HASH_zext_w,    MATCH_ADD_UW,    MASK_ADD_UW|MASK_RS2, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                         match_opcode },

// Zbc extension (carry-less multiplication).
{ String8__inline_m("clmul"),  0, OPC__ZBC, 0, HASH_clmul,  MATCH_CLMUL,  MASK_CLMUL,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("clmulh"), 0, OPC__ZBC, 0, HASH_clmulh, MATCH_CLMULH, MASK_CLMULH, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },
{ String8__inline_m("clmulr"), 0, OPC__ZBC, 0, HASH_clmulr, MATCH_CLMULR, MASK_CLMULR, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)), match_opcode },

// Zbs extension (single-bit operations).
{ String8__inline_m("bclr"),  0, OPC__ZBS, 0,          HASH_bclr,  MATCH_BCLR,  MASK_BCLR,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("bclr"),  0, OPC__ZBS, INSN_ALIAS, HASH_bclr,  MATCH_BCLRI, MASK_BCLRI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("bclri"), 0, OPC__ZBS, 0,          HASH_bclri, MATCH_BCLRI, MASK_BCLRI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("bext"),  0, OPC__ZBS, 0,          HASH_bext,  MATCH_BEXT,  MASK_BEXT,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("bext"),  0, OPC__ZBS, INSN_ALIAS, HASH_bext,  MATCH_BEXTI, MASK_BEXTI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("bexti"), 0, OPC__ZBS, 0,          HASH_bexti, MATCH_BEXTI, MASK_BEXTI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("binv"),  0, OPC__ZBS, 0,          HASH_binv,  MATCH_BINV,  MASK_BINV,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("binv"),  0, OPC__ZBS, INSN_ALIAS, HASH_binv,  MATCH_BINVI, MASK_BINVI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("binvi"), 0, OPC__ZBS, 0,          HASH_binvi, MATCH_BINVI, MASK_BINVI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("bset"),  0, OPC__ZBS, 0,          HASH_bset,  MATCH_BSET,  MASK_BSET,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("bset"),  0, OPC__ZBS, INSN_ALIAS, HASH_bset,  MATCH_BSETI, MASK_BSETI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("bseti"), 0, OPC__ZBS, 0,          HASH_bseti, MATCH_BSETI, MASK_BSETI, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },

// Zbb extension (basic bit manipulation).
{ String8__inline_m("clz"),     0, OPC__ZBB, 0,          HASH_clz,    MATCH_CLZ,    MASK_CLZ,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("ctz"),     0, OPC__ZBB, 0,          HASH_ctz,    MATCH_CTZ,    MASK_CTZ,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("cpop"),    0, OPC__ZBB, 0,          HASH_cpop,   MATCH_CPOP,   MASK_CPOP,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("clzw"),   64, OPC__ZBB, 0,          HASH_clzw,   MATCH_CLZW,   MASK_CLZW,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("ctzw"),   64, OPC__ZBB, 0,          HASH_ctzw,   MATCH_CTZW,   MASK_CTZW,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("cpopw"),  64, OPC__ZBB, 0,          HASH_cpopw,  MATCH_CPOPW,  MASK_CPOPW,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("min"),     0, OPC__ZBB, 0,          HASH_min,    MATCH_MIN,    MASK_MIN,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("minu"),    0, OPC__ZBB, 0,          HASH_minu,   MATCH_MINU,   MASK_MINU,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("max"),     0, OPC__ZBB, 0,          HASH_max,    MATCH_MAX,    MASK_MAX,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("maxu"),    0, OPC__ZBB, 0,          HASH_maxu,   MATCH_MAXU,   MASK_MAXU,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("sext.b"),  0, OPC__ZBB, 0,          HASH_sext_b, MATCH_SEXT_B, MASK_SEXT_B,         OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("sext.h"),  0, OPC__ZBB, 0,          HASH_sext_h, MATCH_SEXT_H, MASK_SEXT_H,         OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("zext.h"), 32, OPC__ZBB, 0,          HASH_zext_h, MATCH_PACK,   MASK_PACK|MASK_RS2,  OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("zext.h"), 64, OPC__ZBB, 0,          HASH_zext_h, MATCH_PACKW,  MASK_PACKW|MASK_RS2, OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("andn"),    0, OPC__ZBB, 0,          HASH_andn,   MATCH_ANDN,   MASK_ANDN,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("orn"),     0, OPC__ZBB, 0,          HASH_orn,    MATCH_ORN,    MASK_ORN,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("xnor"),    0, OPC__ZBB, 0,          HASH_xnor,   MATCH_XNOR,   MASK_XNOR,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("rol"),     0, OPC__ZBB, 0,          HASH_rol,    MATCH_ROL,    MASK_ROL,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("ror"),     0, OPC__ZBB, 0,          HASH_ror,    MATCH_ROR,    MASK_ROR,            OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("ror"),     0, OPC__ZBB, INSN_ALIAS, HASH_ror,    MATCH_RORI,   MASK_RORI,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("rori"),    0, OPC__ZBB, 0,          HASH_rori,   MATCH_RORI,   MASK_RORI,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift)),   match_opcode },
{ String8__inline_m("rolw"),   64, OPC__ZBB, 0,          HASH_rolw,   MATCH_ROLW,   MASK_ROLW,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("rorw"),   64, OPC__ZBB, 0,          HASH_rorw,   MATCH_RORW,   MASK_RORW,           OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_GPR(OPF_R__S2)),        match_opcode },
{ String8__inline_m("rorw"),   64, OPC__ZBB, INSN_ALIAS, HASH_rorw,   MATCH_RORIW,  MASK_RORIW,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("roriw"),  64, OPC__ZBB, 0,          HASH_roriw,  MATCH_RORIW,  MASK_RORIW,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1), OP_Shift(OPF_S__Shift_5)), match_opcode },
{ String8__inline_m("rev8"),   32, OPC__ZBB, 0,          HASH_rev8,   MATCH_REV8_RV32, MASK_REV8,        OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("rev8"),   64, OPC__ZBB, 0,          HASH_rev8,   MATCH_REV8,      MASK_REV8,        OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },
{ String8__inline_m("orc.b"),   0, OPC__ZBB, 0,          HASH_orc_b,  MATCH_ORC_B,  MASK_ORC_B,          OP_m(OP_GPR(OPF_R__D), OP_GPR(OPF_R__S1)),                           match_opcode },

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

// Normalize a constant for the current XLEN. On RV32, values with bit 31 set and all higher bits
// clear must be sign-extended from 32 bits.
// This lets e.g. `addi x1, x0, 0xfffff800` (which normalizes to -2048) pass the 12-bit range
// check. On RV64 the normalization is a no-op.
internal S64
RISCV_normalize_constant_expression(S64 value, U8 xlen)
{
        S64 result = value;
        B32 extend = xlen == 32 && zero_extended_32_bit_is_m(value);
        if (extend)
        {
                // From left to right: truncate, interpret as signed, optionally sign-extend.
                result = (S64)(S32)(U32)(value);
        }

        return result;
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
