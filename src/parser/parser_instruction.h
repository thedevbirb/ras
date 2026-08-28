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

// Represents a single instruction part of a macro expansion (e.g. an `auipc` part of a `call` expansion).
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

typedef struct Instruction_Parsed Instruction_Parsed;
struct Instruction_Parsed
{
        Expression        *expression;
        RISCV_Instruction  data;
        U16                relocation;
        U8                 macro_generated_is;
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
        Arena              *arena,
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        Instruction_Parsed *instruction
);

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
        B32 compressed,

        S64 immediate,
        U8  register_destination,
        U32 location
);

// Expand `la`/`lla`/`lga` into an `auipc + addi` pair using %pcrel_hi/%pcrel_lo. Used for local addresses (`lla`)
// and for `la`/`lga` when not generating position-independent code.
internal void
RISCV_la_pcrel_expand
(
        Arena              *arena,
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        U8                  rd,
        Expression         *expression,
        U32                 location
);

// Expand `la`/`lga` into an `auipc + ld/lw` pair using %got_pcrel_hi/%pcrel_lo, loading the address of a (possibly
// global) symbol through the GOT. Used for global addresses (`la` under PIC, and `lga` always).
internal void
RISCV_la_got_expand
(
        Arena              *arena,
        Section            *section,
        Expressions        *expressions,
        Symbols_Table      *symbols_table,
        Options            *options,

        U8                  rd,
        Expression         *expression,
        U32                 location
);

#endif // WRITE_INSTRUCTION_H

