#ifndef WRITE_INSTRUCTION_H
#define WRITE_INSTRUCTION_H

// Information about an instruction, including its format, operands
// and fixups.
// TODO(refactor): this struct is probably overloaded with unnedded stuff.
typedef struct RISCV_Instruction RISCV_Instruction;
struct RISCV_Instruction
{
  const RISCV_Opcode *opcode;

  // The long encoded instruction bits ([0] is non-zero on a long opcode).  */
  // char insn_long_opcode[RISCV_MAX_INSN_LEN];

  // The frag that contains the instruction
  Fragment *fragment;

  // The relocations associated with the instruction, if any.
  Fixup *fixup;

  // Where the instruction is located in the source.
  U32 location;

  U32 encoding;
  // The offset into fragment of the first instruction byte.
  U32 offset;

};

internal RISCV_Instruction
RISCV_Instruction__create(const RISCV_Opcode *opcode, U32 location)
{
        RISCV_Instruction result =
        {
                .opcode   = opcode,
                .encoding = opcode->match,
                .location = location,
        };
        return result;
}

internal void
RISCV_Instruction__parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Diagnostics        *diagnostics,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,
        U32                 instruction_hash,

        U16                *relocation_out,
        RISCV_Instruction  *instruction_out,
        Expression        **expression_out
);

internal U8
RISCV_instruction_size(U32 encoding);

internal void
RISCV_Instruction__append
(
        Arena             *arena,
        Section           *section,
        Options     *options,

        RISCV_Instruction *instruction,
        Expression        *expression,
        U16                relocation
);

internal void
RISCV_macro_build
(
        Arena           *arena,
        Section         *section,
        Options   *options,

        String8          instruction_name,
        U32              location,
        Expression      *expression,
        U64              arguments,
        S32             *values,
        U8               values_count
);

// Expand a call pseudo instruction into an `auipc + jalr` pair with the provided register for `jalr`.
internal void
RISCV_call_expand
(
        Arena           *arena,
        Section         *section,
        Options   *options,

        U8               rd,
        U8               rs1,
        Expression *expression,
        U16              relocation,
        U32              location
);

internal U8
RISCV_li_expand
(
        Section         *section,

        S64 immediate,
        U8  register_destination,
        U32 location
);

internal void
RISCV_instruction_pseudo_append
(
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options      *options,

        RISCV_Instruction  *instruction,
        Expression         *expression,
        U16                 relocation
);


#endif // WRITE_INSTRUCTION_H

