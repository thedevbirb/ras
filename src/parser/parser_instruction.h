#ifndef WRITE_INSTRUCTION_H
#define WRITE_INSTRUCTION_H

// Information about an instruction, including its format and operands.
typedef struct RISCV_Instruction RISCV_Instruction;
struct RISCV_Instruction
{
  const RISCV_Opcode *opcode;

  // The long encoded instruction bits ([0] is non-zero on a long opcode).  */
  // char insn_long_opcode[RISCV_MAX_INSN_LEN];

  // Where the instruction is located in the source.
  U32 location;

  U32 encoding;
};

typedef struct Instruction_Parsed Instruction_Parsed;
struct Instruction_Parsed
{
        Expression        *expression;
        RISCV_Instruction  data;
        U16                relocation;
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

internal Instruction_Parsed
RISCV_Instruction__parse
(
        Arena              *arena,
        Token_Cursor       *cursor,
        Diagnostics        *diagnostics,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,
        U32                 instruction_hash
);

internal void
RISCV_Instruction__append
(
        Arena              *arena,
        Section            *section,
        Options            *options,
        Instruction_Parsed *instruction
);

internal void
RISCV_instruction_pseudo_append
(
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        Instruction_Parsed *instruction
);

typedef struct Macro_Info Macro_Info;
struct Macro_Info
{
        String8          instruction_name;
        U32              location;
        Expression      *expression;
        U64              arguments;
        S32             *values;
        U8               values_count;
};

internal void
RISCV_macro_build
(
        Arena      *arena,
        Section    *section,
        Options    *options,
        Macro_Info *macro
);

// Expand a call pseudo instruction into an `auipc + jalr` pair with the provided register for `jalr`.
internal void
RISCV_call_expand
(
        Arena     *arena,
        Section   *section,
        Options   *options,

        U8          rd,
        U8          rs1,
        Expression *expression,
        U16         relocation,
        U32         location
);

internal U8
RISCV_li_expand
(
        Section *section,
        U8 xlen,

        S64 immediate,
        U8  register_destination,
        U32 location
);

#endif // WRITE_INSTRUCTION_H

