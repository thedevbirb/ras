#define A_NONE                OP_arguments_m(OP_Argument__None)

#define A_RS1                 OP_arguments_m(OP_Argument__RS1)
#define A_RS1_IMM_I           OP_arguments_m(OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Immediate_I)

#define A_RD_IMM_L            OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Immediate_Large)
#define A_RD_IMM_I            OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Immediate_I)
#define A_RD_IMM_U            OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Immediate_U)
#define A_RD_RS1              OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1)
#define A_RD_RS1_RS2          OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__RS2)
#define A_RD_RS1_IMM          OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Immediate_I)
#define A_RD_RS1_SHIFT        OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Shift_Amount)
#define A_RD_RS1_SHIFT5       OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__RS1, OP_Argument__Comma, OP_Argument__Shift_Amount_5)

#define A_RD_OFF_S_LP_RS1_RP  OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Offset_Store, OP_Argument__Parenthesis_Left, OP_Argument__RS1, OP_Argument__Parenthesis_Right)
#define A_RD_OFF_L_LP_RS1_RP  OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Offset_Load,  OP_Argument__Parenthesis_Left, OP_Argument__RS1, OP_Argument__Parenthesis_Right)
#define A_OFF_LP_RS1_RP       OP_arguments_m(                                      OP_Argument__Offset_Load,  OP_Argument__Parenthesis_Left, OP_Argument__RS1, OP_Argument__Parenthesis_Right)

#define A_RD_ADDRESS          OP_arguments_m(OP_Argument__RD,  OP_Argument__Comma, OP_Argument__Address)
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

// NOTE: important here to go from more specific to less specific.
{ "jal",    HASH_jal,    0, RV_IC_I, A_RD_OFF,             MATCH_JAL,                               MASK_JAL,                            match_opcode,         0                              },
{ "jal",    HASH_jal,    0, RV_IC_I, A_OFF_20,             MATCH_JAL|(X_RA << OP_SH_RD),            MASK_JAL|MASK_RD,                    match_opcode,         0                              },

{ "jalr",   HASH_jalr,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_JALR,                              MASK_JALR,                           match_opcode,         0                              },

{ "lb",     HASH_lb,     0, RV_IC_I, A_RD_OFF_L_LP_RS1_RP, MATCH_LB,                                MASK_LB,                             match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lb",     HASH_lb,     0, RV_IC_I, A_RD_RS1,             MATCH_LB,                                MASK_LB,                             match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lbu",    HASH_lbu,    0, RV_IC_I, A_RD_OFF_L_LP_RS1_RP, MATCH_LBU,                               MASK_LBU,                            match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lbu",    HASH_lbu,    0, RV_IC_I, A_RD_RS1,             MATCH_LBU,                               MASK_LBU,                            match_opcode,         INSN_DREF|INSN_1_BYTE          },
{ "lh",     HASH_lh,     0, RV_IC_I, A_RD_OFF_L_LP_RS1_RP, MATCH_LH,                                MASK_LH,                             match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lh",     HASH_lh,     0, RV_IC_I, A_RD_RS1,             MATCH_LH,                                MASK_LH,                             match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lhu",    HASH_lhu,    0, RV_IC_I, A_RD_OFF_L_LP_RS1_RP, MATCH_LHU,                               MASK_LHU,                            match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lhu",    HASH_lhu,    0, RV_IC_I, A_RD_RS1,             MATCH_LHU,                               MASK_LHU,                            match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "lw",     HASH_lw,     0, RV_IC_I, A_RD_OFF_L_LP_RS1_RP, MATCH_LW,                                MASK_LW,                             match_opcode,         INSN_DREF|INSN_4_BYTE          },
{ "lw",     HASH_lw,     0, RV_IC_I, A_RD_RS1,             MATCH_LW,                                MASK_LW,                             match_opcode,         INSN_DREF|INSN_4_BYTE          },
// TODO(low): add symbol version of this. GNU as treats the version A_RD_RS1 where RS1 is part of an expression and RS1 is a register symbol.

{ "sw",     HASH_sw,     0, RV_IC_I, A_RD_OFF_S_LP_RS1_RP, MATCH_SW,                                MASK_SW,                             match_opcode,         INSN_DREF|INSN_4_BYTE          },
{ "sh",     HASH_sh,     0, RV_IC_I, A_RD_OFF_S_LP_RS1_RP, MATCH_SH,                                MASK_SH,                             match_opcode,         INSN_DREF|INSN_2_BYTE          },
{ "sb",     HASH_sb,     0, RV_IC_I, A_RD_OFF_S_LP_RS1_RP, MATCH_SB,                                MASK_SB,                             match_opcode,         INSN_DREF|INSN_1_BYTE          },

{ "addi",   HASH_addi,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ADDI,                              MASK_ADDI,                           match_opcode,         0                              },
{ "addiw",  HASH_addiw,  0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ADDIW,                             MASK_ADDIW,                          match_opcode,         0                              },
{ "slti",   HASH_slti,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_SLTI,                              MASK_SLTI,                           match_opcode,         0                              },
{ "sltiu",  HASH_sltiu,  0, RV_IC_I, A_RD_RS1_IMM,         MATCH_SLTIU,                             MASK_SLTIU,                          match_opcode,         0                              },
{ "xori",   HASH_xori,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_XORI,                              MASK_XORI,                           match_opcode,         0                              },
{ "ori",    HASH_ori,    0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ORI,                               MASK_ORI,                            match_opcode,         0                              },
{ "andi",   HASH_andi,   0, RV_IC_I, A_RD_RS1_IMM,         MATCH_ANDI,                              MASK_ANDI,                           match_opcode,         0                              },

{ "slli",   HASH_slli,   0, RV_IC_I, A_RD_RS1_SHIFT,       MATCH_SLLI,                              MASK_SLLI,                           match_opcode,         0                              },
{ "srli",   HASH_srli,   0, RV_IC_I, A_RD_RS1_SHIFT,       MATCH_SRLI,                              MASK_SRLI,                           match_opcode,         0                              },
{ "srai",   HASH_srai,   0, RV_IC_I, A_RD_RS1_SHIFT,       MATCH_SRAI,                              MASK_SRAI,                           match_opcode,         0                              },
{ "slliw",  HASH_slliw,  0, RV_IC_I, A_RD_RS1_SHIFT5,      MATCH_SLLIW,                             MASK_SLLIW,                          match_opcode,         0                              },
{ "srliw",  HASH_slli,   0, RV_IC_I, A_RD_RS1_SHIFT5,      MATCH_SRLIW,                             MASK_SRLIW,                          match_opcode,         0                              },
{ "sraiw",  HASH_sraiw,  0, RV_IC_I, A_RD_RS1_SHIFT5,      MATCH_SRAIW,                             MASK_SRAIW,                          match_opcode,         0                              },

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
{ "j",      HASH_j,      0, RV_IC_I, A_OFF_20,             MATCH_JAL,                               MASK_JALR|MASK_RD,                   match_opcode,         INSN_ALIAS|INSN_JSR            },
{ "jr",     HASH_jr,     0, RV_IC_I, A_RS1,                MATCH_JALR,                              MASK_JALR|MASK_RD|MASK_IMM,          match_opcode,         INSN_ALIAS|INSN_JSR            },
{ "jr",     HASH_jr,     0, RV_IC_I, A_OFF_LP_RS1_RP,      MATCH_JALR,                              MASK_JALR|MASK_RD,                   match_opcode,         INSN_ALIAS|INSN_JSR            },
{ "jr",     HASH_jr,     0, RV_IC_I, A_RS1_IMM_I,          MATCH_JALR,                              MASK_JALR|MASK_RD,                   match_opcode,         INSN_ALIAS|INSN_JSR            },
{ "ret",    HASH_ret,    0, RV_IC_I, A_NONE,               MATCH_JALR|(X_RA << OP_SH_RD),           MASK_JALR|MASK_RD,                   match_opcode,         INSN_ALIAS|INSN_JSR            },

{ "call",   HASH_call,   0, RV_IC_I, A_CALL,               (X_RA << OP_SH_RS1)|(X_RA << OP_SH_RD),  M_CALL,                              0,                    INSN_MACRO                     },
{ "li",     HASH_li,     0, RV_IC_I, A_RD_IMM_I,           MATCH_ADDI,                              MASK_ADDI|MASK_RS1,                  match_opcode,         INSN_ALIAS                     },
{ "li",     HASH_li,     0, RV_IC_I, A_RD_IMM_L,           0,                                       M_LI,                                0,                    INSN_MACRO                     },

{ "la",     HASH_la,     0, RV_IC_I, A_RD_ADDRESS,         0,                                       M_LA,                                match_rd_nonzero,     INSN_MACRO                     },

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

// TODO(low): undef the helper macros defined above.

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
        Diagnostic_List    *diagnostics,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Sections_Table     *sections_table,
        U32                 instruction_hash,

        U16                *relocation_out,
        RISCV_Instruction  *instruction_out,
        Expression        **expression_out
)
{
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
        String8 opcode_name = String8__from_cstring(opcode->name);

        Token opcode_token = cursor->current;
        token_next(cursor, diagnostics, arena);
        Token_Cursor cursor_start = *cursor;

        Expression *expression = 0;
        B32 match = 0;

        // Iterate over opcode entries with the same name.
        for (;;)
        {
                *instruction_out = RISCV_Instruction__create(opcode, opcode_token.location);
                OP_Argument *arguments = opcode->arguments;
                B32 try_next = 0;

                // Iterate over opcode arguments.
                for (;;)
                {
                        OP_Argument argument = *arguments;
                        if (!argument)
                        {
                                match = !try_next && opcode->hash && (!opcode->match_function || opcode->match_function(opcode, instruction_out->encoding));
                                break;
                        }

                        switch (argument)
                        {
                        case OP_Argument__Comma:
                        {
                                // NOTE: This whole thing could extracted into a `expect_comma_and_advance`.
                                Token token_before_comma = cursor->previous;
                                if (cursor->current.kind == Token_Kind__Comma)
                                {
                                        token_next(cursor, diagnostics, arena);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = token_before_comma.location + token_before_comma.size;
                                        diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Comma_Expected];
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Parenthesis_Left:
                        {
                                if (cursor->current.kind == Token_Kind__Parenthesis_Left)
                                {
                                        token_next(cursor, diagnostics, arena);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("'(' expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Parenthesis_Right:
                        {
                                if (cursor->current.kind == Token_Kind__Parenthesis_Right)
                                {
                                        token_next(cursor, diagnostics, arena);
                                }
                                else
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->location   = cursor->current.location;
                                        diagnostic->message    = String8__literal("')' expected");
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__RD:  {} // fallthrough
                        case OP_Argument__RS3: {} // fallthrough
                        case OP_Argument__RS2: {} // fallthrough
                        case OP_Argument__RS1:
                        {
                                String8 text = Token_Cursor__text(cursor);
                                const Register *reg = Register_List__lookup(RISCV_register_list, text, 0);
                                if (!reg)
                                {
                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                       diagnostic->location   = cursor->current.location;
                                       diagnostic->message    = Parser_Error_Kind_messages[Parser_Error_Kind__Register_Invalid];
                                       diagnostic->ranges[0]  = (Range1_U32){{ cursor->current.location, cursor->current.location + cursor->current.size }};
                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                U8 register_number = reg ? reg->number : Register__invalid_number;
                                switch (argument)
                                {
                                       case OP_Argument__RD:  { INSERT_OPERAND(RD,  *instruction_out, register_number); } break;
                                       case OP_Argument__RS3: { INSERT_OPERAND(RS3, *instruction_out, register_number); } break;
                                       case OP_Argument__RS2: { INSERT_OPERAND(RS2, *instruction_out, register_number); } break;
                                       case OP_Argument__RS1: { INSERT_OPERAND(RS1, *instruction_out, register_number); } break;
                                }
                                token_next(cursor, diagnostics, arena);
                        } break;
                        case OP_Argument__Address:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                                B32 symbol_is   = expression->kind == Expression_Kind__Symbol;
                                B32 constant_is = expression->kind == Expression_Kind__Constant;
                                if (!(symbol_is || constant_is))
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("expression must be either symbol or a constant");
                                        diagnostic->location   = expression->location_range.v[0];
                                        diagnostic->ranges[0]  = expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

                                if (symbol_is)
                                {
                                        *relocation_out = Relocation_RISC_V__32_Bit;
                                }

                                B32 constant_fits = sign_extended_32_bit_is_m(expression->integer_value);
                                if (constant_is && !constant_fits)
                                {
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("offset too large for this opcode");
                                        diagnostic->location   = expression->location_range.v[0];
                                        diagnostic->ranges[0]  = expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Offset_PC_Relative_20:
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
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);

                                // For branches we can't support a fixup. While GNU as silently ignores additional
                                // symbols, here we either warn or error.
                                expression_evaluate(expression);
                                if (expression->symbol_operand)
                                {
                                        // TODO(low): this diagnostic could be better, I should probably support re-lexing
                                        // from a specific location to get the exact token.
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("PC relative offset expression contains operand symbol which will be skipped");
                                        diagnostic->location   = expression->location_range.v[0];
                                        diagnostic->ranges[0]  = expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Offset_PC_Relative_12:
                        {
                                // See notes for `OP_Argument__Offset_PC_Relative_20`.
                                *relocation_out = Relocation_RISC_V__Branch;
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);

                                // For branches we can't support a fixup. While GNU as silently ignores additional
                                // symbols, here we either warn or error.
                                expression_evaluate(expression);
                                if (expression->symbol_operand)
                                {
                                        // TODO(low): this diagnostic could be better, I should probably support re-lexing
                                        // from a specific location to get the exact token.
                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                        diagnostic->message    = String8__literal("PC relative offset expression contains operand symbol which will be skipped");
                                        diagnostic->location   = expression->location_range.v[0];
                                        diagnostic->ranges[0]  = expression->location_range;
                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Offset_Store:
                        {
                                OP_Argument *next = arguments + 1;
                                assert_always_m(next && "invalid operand list");

                                if (*next == OP_Argument__Parenthesis_Left && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                {
                                       // Omitted immediate, e.g. sw t1, (t0)
                                       arguments += 1;
                                }
                                else
                                {
                                        expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, sections_table, diagnostics, relocation_out, Relocation_Operator_List__stype);
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
                        case OP_Argument__Offset_Load:
                        {
                                OP_Argument *next = arguments + 1;
                                assert_always_m(next && "invalid operand list");

                                if (*next == OP_Argument__Parenthesis_Left && cursor->current.kind == Token_Kind__Parenthesis_Left)
                                {
                                       // Omitted immediate, e.g. lw t1, (t0)
                                       arguments += 1;
                                }
                                else
                                {
                                        // TODO(refactor): this is mostly in common with OP_Argument__Immediate_I case.
                                        expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, sections_table, diagnostics, relocation_out, Relocation_Operator_List__stype);
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
                        case OP_Argument__Immediate_I:
                        {
                                expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, sections_table, diagnostics, relocation_out, Relocation_Operator_List__itype);
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
                        case OP_Argument__Immediate_U:
                        {
                                expression = expression_parse_with_relocation(arena, cursor, expressions, symbols_table, sections_table, diagnostics, relocation_out, Relocation_Operator_List__utype);
                                if (!*relocation_out)
                                {
                                        expression_evaluate(expression);
                                        if (expression->evaluation == Expression_Kind__Constant)
                                        {
                                                S64 result = expression->integer_value;
                                                B32 fits = 0 <= result && result < (S64)(1 << 20);
                                                if (!fits)
                                                {
                                                        Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                        diagnostic->location   = expression->location_range.v[0];
                                                        diagnostic->message    = String8__literal("constant expression value must in the range 0..1048576");
                                                        diagnostic->ranges[0]  = expression->location_range;
                                                        SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                                }

                                                // TODO(medium): GNU as does this at a later step, and by default emits a
                                                // relocation. Consider doing the same.
                                                U32 encoding_immediate = encode_immediate_u_m(expression->integer_value);
                                                instruction_out->encoding |= encoding_immediate;
                                        }
                                        else
                                        {

                                                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                                diagnostic->message    = String8__literal("Non-constant expression must have an appropriate relocation operator");
                                                diagnostic->location   = expression->location_range.v[0];
                                                diagnostic->ranges[0]  = expression->location_range;
                                                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                        }
                                }
                        } break;
                        case OP_Argument__Immediate_Large:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                                expression_evaluate(expression);
                                if (expression->evaluation != Expression_Kind__Constant)
                                {
                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                       diagnostic->message    = String8__literal("Constant expression expected");
                                       diagnostic->location   = expression->location_range.v[0];
                                       diagnostic->ranges[0]  = expression->location_range;
                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }
                        } break;
                        case OP_Argument__Shift_Amount:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                                expression_evaluate(expression);
                                S64 value = expression->integer_value;
                                B32 fits = 0 <= value && value < XLEN;
                                if (expression->evaluation != Expression_Kind__Constant || !fits)
                                {
                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                       diagnostic->message    = String8__literal("shift amount doesn't fit register size");
                                       diagnostic->location   = expression->location_range.v[0];
                                       diagnostic->ranges[0]  = expression->location_range;
                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

	                        INSERT_OPERAND (SHAMT, *instruction_out, value);
                        } break;
                        case OP_Argument__Shift_Amount_5:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                                expression_evaluate(expression);
                                S64 value = expression->integer_value;
                                B32 fits = 0 <= value && value < (1 << 5);
                                if (expression->evaluation != Expression_Kind__Constant || !fits)
                                {
                                       Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                                       diagnostic->message    = String8__literal("shift amount doesn't fit register size");
                                       diagnostic->location   = expression->location_range.v[0];
                                       diagnostic->ranges[0]  = expression->location_range;
                                       SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
                                }

	                        INSERT_OPERAND (SHAMT, *instruction_out, value);
                        } break;
                        case OP_Argument__Call_Expression:
                        {
                                expression = expression_parse(arena, cursor, expressions, symbols_table, sections_table, diagnostics);
                                *relocation_out = Relocation_RISC_V__Call_PLT;
                        } break;
                        default: { unreachable_m(); }
                        }

                        arguments += 1;
                }

                B32 same_name = String8__match_exact(opcode_name, String8__from_cstring(opcode->name));
                if (match || opcode->hash == 0 || !same_name)
                {
                        break;
                }

                *cursor = cursor_start;
                opcode += 1;
        }

        if (!match)
        {
                Diagnostic *diagnostic = Arena__push_struct_m(arena, Diagnostic);
                diagnostic->location   = opcode_token.location;
                diagnostic->message    = String8__literal("unrecognized opcode");
                diagnostic->ranges[0]  = Token__range(opcode_token);
                SLL_queue_push_m(diagnostics->first, diagnostics->last, diagnostic);
        }

        *expression_out = expression;

        return;
}

internal void
RISCV_Instruction__append
(
        Section           *section,
        Fixups            *fixups,

        RISCV_Instruction *instruction,
        Expression        *expression,
        U16                relocation
)
{
        Fixup *fixup         = 0;
        B32    jump_is       = relocation == Relocation_RISC_V__JAL;
        // NOTE: although jumps are assumed to be in range, if the compressed extension is enabled
        // then this might get reduced to a compressed 2-byte instruction.
        B32    relaxable     = relocation == Relocation_RISC_V__Branch || jump_is;
        // NOTE: fixups, which are deferred patches, can be created only for fixed size instructions
        // (non-relaxable) because they need a precise location to be applied. Relaxable instructions,
        // like branches, break this invariant.
        B32    fixable       = relocation && !relaxable;
        U32    encoding      = instruction->encoding;
        U8     encoding_size = RISCV_instruction_size(encoding);
        U32    location      = instruction->location;


        if (fixable)
        {
                fixup = Fixups__push(fixups);
                fixup->expression      = expression;
                fixup->relocation_type = relocation;
        }

        if (relaxable)
        {
                Relax_Info relax_info =
                {
                        .jump =
                        {
                                .expression              = expression,
                                .compressed_is           = encoding_size == 2,
                                .unconditional_is        = jump_is,
                                .instructions_total_size = encoding_size
                        }
                };

                Fragments__variable
                (
                        &section->fragments,
                        location,
                        relax_info,
                        Relax_State__Jump,
                        (U8 *)&encoding,
                        encoding_size
                );
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

        Section     *section,
        Fixups      *fixups,

        String8      instruction_name,
        U32          location,
        Expression  *expression,
        // TODO(medium): avoid null-terminated arrays.
        OP_Argument *arguments,
        S32         *values
)
{
        U32 instruction_hash = FNV_hash_U32(instruction_name);
        const RISCV_Opcode *opcode = RISCV_Opcode__table_find(instruction_hash);
        assert_always_m(opcode && opcode->hash);

        RISCV_Instruction instruction = RISCV_Instruction__create(opcode, location);

        U16 relocation = 0;

        for (;;)
        {
                OP_Argument argument = *arguments;
                if (argument == 0)
                {
                        break;
                }

                S32 value = *values;

                switch (argument)
                {
                        default: { unreachable_m(); } break;

                        case OP_Argument__RD:  { INSERT_OPERAND(RD,  instruction, value); } break;
                        case OP_Argument__RS3: { INSERT_OPERAND(RS3, instruction, value); } break;
                        case OP_Argument__RS2: { INSERT_OPERAND(RS2, instruction, value); } break;
                        case OP_Argument__RS1: { INSERT_OPERAND(RS1, instruction, value); } break;

                        // TODO(medium): I know I've done this to follow GNU as, but this is horrible. Create special
                        // OP_Argument__Relocation and just do that.
                        case OP_Argument__Immediate_I: {} // fallthrough
                        case OP_Argument__Immediate_U: { relocation = value; } break;
                }

                arguments += 1;
                values    += 1;
        }

        assert_always_m(relocation ? expression != 0 : 1);

        RISCV_Instruction__append
        (
                section,
                fixups,
                &instruction,
                expression,
                relocation
        );
}

// Expand a call pseudo instruction into an `auipc + jalr` pair with the provided register for `jalr`.
internal void
RISCV_call_expand
(
        Section         *section,
        Fixups          *fixups,

        U8               rd,
        U8               rs1,
        Expression *expression,
        U16              relocation,
        U32              location
)
{
        OP_Argument *arguments_auipc = OP_arguments_m(OP_Argument__RD, OP_Argument__Immediate_U);
        S32 values_auipc[2]          = {rs1, relocation};
        OP_Argument *arguments_jalr  = OP_arguments_m(OP_Argument__RD, OP_Argument__RS1);
        S32 values_jalr[2]           = {rd, rs1};

        // Ensure both instructions land in the same fragment.
        Fragments__ensure(&section->fragments, 8);
        RISCV_macro_build
        (
                section,
                fixups,

                String8__literal("auipc"),
                location,
                expression,
                arguments_auipc,
                values_auipc
        );
        RISCV_macro_build
        (
                section,
                fixups,

                String8__literal("jalr"),
                location,
                0,
                arguments_jalr,
                values_jalr
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
                                // Single ADDI from x0.
                                U32 addi_encoding      = instruction_i_encode_m(register_destination, 0, immediate, OPCODE_I_TYPE, FUNCT3_ADDI);
                                U8  addi_encoding_size = RISCV_instruction_size(addi_encoding);
                                Section__add_instruction_fixed(section, 0, addi_encoding, addi_encoding_size, location);
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
                                S64 lui_immediate     = immediate - immediate_low_12;
                                U32 lui_encoding      = instruction_u_encode_m(register_destination, lui_immediate, OPCODE_LUI);
                                U8  lui_encoding_size = RISCV_instruction_size(lui_encoding);
                                Section__add_instruction_fixed(section, 0, lui_encoding, lui_encoding_size, location);
                                if (!lui_suffices)
                                {
                                        U32 addiw_encoding = instruction_i_encode_m(register_destination, register_destination, immediate_low_12,
                                                OPCODE_I_TYPE, FUNCT3_ADDIW);
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
        Arena                   *arena,
        Section                 *section,
        Fixups                  *fixups,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,

        RISCV_Instruction       *instruction,
        Expression         *expression,
        U16                      relocation
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
        case M_CALL:
        {
                RISCV_call_expand
                (
                        section,
                        fixups,
                        rd,
                        rs1,
                        expression,
                        relocation,
                        instruction->location
                );
        } break;
        case M_LA:
        {
                if (expression->kind == Expression_Kind__Constant)
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

                        OP_Argument *arguments_auipc = OP_arguments_m(OP_Argument__RD, OP_Argument__Immediate_U);
                        S32 values_auipc[]           = {rd, Relocation_RISC_V__PC_Relative_High_20};
                        OP_Argument *arguments_addi  = OP_arguments_m(OP_Argument__RD, OP_Argument__RS1, OP_Argument__Immediate_I);
                        S32 values_addi[]            = {rd, rd, Relocation_RISC_V__PC_Relative_Low_12_I_Type};

                        Symbol_Ref *internal_label          = Symbols_Table__internal_label(symbols_table, section);
                        Expression *expression_addi         = Expressions_push_empty(expressions, arena);
                                    expression_addi->symbol = internal_label;
                                    expression_addi->kind   = Expression_Kind__Symbol;

                        // Ensure the instructions are in the same fragment
                        Fragments__ensure(&section->fragments, 8);
                        RISCV_macro_build
                        (
                                 section,
                                 fixups,

                                 String8__literal("auipc"),
                                 instruction->location,
                                 expression,
                                 arguments_auipc,
                                 values_auipc
                        );
                        // NOTE: GNU as creates also a second expression with an fake label for addi, why?
                        RISCV_macro_build
                        (
                                 section,
                                 fixups,

                                 String8__literal("addi"),
                                 instruction->location,
                                 expression_addi,
                                 arguments_addi,
                                 values_addi
                        );
                        // TODO(medium, check-gas): wane and new here?
                }
        } break;
        case M_LI:
        {
                RISCV_li_expand
                (
                        section,
                        expression->integer_value,
                        rd,
                        instruction->location
                );
        } break;
        }
}

