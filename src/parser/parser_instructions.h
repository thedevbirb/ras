#ifndef PARSER_INSTRUCTIONS_H
#define PARSER_INSTRUCTIONS_H

internal void
Parser_instruction_I_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_I_load_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_R_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_R_pseudo_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_S_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_B_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_B_pseudo_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_U_parse(Parser *parser, Instruction_Kind instruction_kind);

internal void
Parser_instruction_J_parse(Parser *parser, Instruction_Kind instruction_kind);

// nop -> addi x0, x0, 0
internal void
Parser_instruction_nop_parse(Parser *parser);

// mv rd, rs -> addi rd, rs, 0
internal void
Parser_instruction_mv_parse(Parser *parser);

// not rd, rs -> xori rd, rs, -1
internal void
Parser_instruction_not_parse(Parser *parser);

// sext.w rd, rs -> addiw rd, rs, 0 (RV64)
internal void
Parser_instruction_sext_w_parse(Parser *parser);

// j offset -> jal x0, offset
internal void
Parser_instruction_j_parse(Parser *parser);

// jr rs -> jalr x0, rs, 0
internal void
Parser_instruction_jr_parse(Parser *parser);

// jalr rs -> jalr ra, rs, 0 (single-operand form)
internal void
Parser_instruction_jalr_pseudo_parse(Parser *parser);

// ret -> jalr x0, ra, 0
internal void
Parser_instruction_ret_parse(Parser *parser);

// li rd, imm -> lui rd, %hi(imm) + addi rd, rd, %lo(imm)
// NOTE: For small immediates that fit in 12 bits, a single addi suffices.
//       The expansion decision may be deferred to a later pass.
internal void
Parser_instruction_li_parse(Parser *parser);

// la rd, symbol -> auipc rd, %pcrel_hi(symbol) + addi rd, rd, %pcrel_lo(symbol)
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_la_parse(Parser *parser);

// call offset -> auipc ra, offsetHi + jalr ra, ra, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_call_parse(Parser *parser);

// tail offset -> auipc t1, offsetHi + jalr x0, t1, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Parser_instruction_tail_parse(Parser *parser);

internal void
Parser_instruction_ecall_parse(Parser *parser);

internal void
Parser_instruction_ebreak_parse(Parser *parser);

internal void
Parser_instruction_pause_parse(Parser *parser);

internal void
Parser_instruction_fence_tso_parse(Parser *parser);

// Parses a fence ordering operand: a string composed of the characters i, o, r, w
// in that order. Returns a 4-bit mask: i=bit3, o=bit2, r=bit1, w=bit0.
internal U8
Parser_expect_fence_operand(Parser *parser);

// fence pred, succ   -> e.g. fence iorw, iorw
// fence              -> shorthand for fence iorw, iorw
internal void
Parser_instruction_fence_parse(Parser *parser);

#endif // PARSER_INSTRUCTIONS_H

