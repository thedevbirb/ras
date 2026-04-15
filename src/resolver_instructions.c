internal void
Resolver_instruction_I_encode(Resolver *resolver, Instruction_Kind instruction_kind)
{
}

internal void
Resolver_instruction_I_load_encode(Resolver *resolver, Instruction_Kind instruction_kind)
{
}

internal void
Resolver_instruction_R_encode(Statement *statement, U8 opcode, U8 funct3, U8 funct7)
{

}

internal void
Resolver_instruction_S_encode(Resolver *resolver, Instruction_Kind instruction_kind)
{
}

internal void
Resolver_instruction_B_encode(Resolver *resolver, Instruction_Kind instruction_kind)
{
}

internal void
Resolver_instruction_U_encode(Resolver *resolver, Instruction_Kind instruction_kind)
{
}

internal void
Resolver_instruction_J_encode(Resolver *resolver, Instruction_Kind instruction_kind)
{
}

// nop -> addi x0, x0, 0
internal void
Resolver_instruction_nop_encode(Resolver *resolver)
{
}

// mv rd, rs -> addi rd, rs, 0
internal void
Resolver_instruction_mv_encode(Resolver *resolver)
{
}

// not rd, rs -> xori rd, rs, -1
internal void
Resolver_instruction_not_encode(Resolver *resolver)
{
}

// neg rd, rs -> sub rd, x0, rs
internal void
Resolver_instruction_neg_encode(Resolver *resolver)
{
}

// negw rd, rs -> subw rd, x0, rs (RV64)
internal void
Resolver_instruction_negw_encode(Resolver *resolver)
{
}

// sext.w rd, rs -> addiw rd, rs, 0 (RV64)
internal void
Resolver_instruction_sext_w_encode(Resolver *resolver)
{
}

// seqz rd, rs -> sltiu rd, rs, 1
internal void
Resolver_instruction_seqz_encode(Resolver *resolver)
{
}

// snez rd, rs -> sltu rd, x0, rs
internal void
Resolver_instruction_snez_encode(Resolver *resolver)
{
}

// sltz rd, rs -> slt rd, rs, x0
internal void
Resolver_instruction_sltz_encode(Resolver *resolver)
{
}

// sgtz rd, rs -> slt rd, x0, rs
internal void
Resolver_instruction_sgtz_encode(Resolver *resolver)
{
}

// beqz rs, offset -> beq rs, x0, offset
internal void
Resolver_instruction_beqz_encode(Resolver *resolver)
{
}

// bnez rs, offset -> bne rs, x0, offset
internal void
Resolver_instruction_bnez_encode(Resolver *resolver)
{
}

// blez rs, offset -> bge x0, rs, offset
internal void
Resolver_instruction_blez_encode(Resolver *resolver)
{
}

// bgez rs, offset -> bge rs, x0, offset
internal void
Resolver_instruction_bgez_encode(Resolver *resolver)
{
}

// bltz rs, offset -> blt rs, x0, offset
internal void
Resolver_instruction_bltz_encode(Resolver *resolver)
{
}

// bgtz rs, offset -> blt x0, rs, offset
internal void
Resolver_instruction_bgtz_encode(Resolver *resolver)
{
}

// bgt rs, rt, offset -> blt rt, rs, offset
internal void
Resolver_instruction_bgt_encode(Resolver *resolver)
{
}

// ble rs, rt, offset -> bge rt, rs, offset
internal void
Resolver_instruction_ble_encode(Resolver *resolver)
{
}

// bgtu rs, rt, offset -> bltu rt, rs, offset
internal void
Resolver_instruction_bgtu_encode(Resolver *resolver)
{
}

// bleu rs, rt, offset -> bgeu rt, rs, offset
internal void
Resolver_instruction_bleu_encode(Resolver *resolver)
{
}

// j offset -> jal x0, offset
internal void
Resolver_instruction_j_encode(Resolver *resolver)
{
}

// jr rs -> jalr x0, rs, 0
internal void
Resolver_instruction_jr_encode(Resolver *resolver)
{
}

// jalr rs -> jalr ra, rs, 0 (single-operand form)
internal void
Resolver_instruction_jalr_pseudo_encode(Resolver *resolver)
{
}

// ret -> jalr x0, ra, 0
internal void
Resolver_instruction_ret_encode(Resolver *resolver)
{
}

// li rd, imm -> lui rd, %hi(imm) + addi rd, rd, %lo(imm)
// NOTE: For small immediates that fit in 12 bits, a single addi suffices.
//       The expansion decision may be deferred to a later pass.
internal void
Resolver_instruction_li_encode(Resolver *resolver)
{
}

// la rd, symbol -> auipc rd, %pcrel_hi(symbol) + addi rd, rd, %pcrel_lo(symbol)
// NOTE: Expansion is deferred to a later pass.
internal void
Resolver_instruction_la_encode(Resolver *resolver)
{
}

// call offset -> auipc ra, offsetHi + jalr ra, ra, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Resolver_instruction_call_encode(Resolver *resolver)
{
}

// tail offset -> auipc t1, offsetHi + jalr x0, t1, offsetLo
// NOTE: Expansion is deferred to a later pass.
internal void
Resolver_instruction_tail_encode(Resolver *resolver)
{
}

internal void
Resolver_instruction_ecall_encode(Resolver *resolver)
{
}

internal void
Resolver_instruction_ebreak_encode(Resolver *resolver)
{
}

internal void
Resolver_instruction_pause_encode(Resolver *resolver)
{
}

internal void
Resolver_instruction_fence_tso_encode(Resolver *resolver)
{
}

// encodes a fence ordering operand: a string composed of the characters i, o, r, w
// in that order. Returns a 4-bit mask: i=bit3, o=bit2, r=bit1, w=bit0.
internal U8
Resolver_expect_fence_operand(Resolver *resolver)
{
}

// fence pred, succ   -> e.g. fence iorw, iorw
// fence              -> shorthand for fence iorw, iorw
internal void
Resolver_instruction_fence_encode(Resolver *resolver)
{
}
