#ifndef STATEMENT_H
#define STATEMENT_H

// TODO: this is a bit redundant with Instruction_Kind and Directive_Kind. Ideally we would not have this tag.
typedef U8 Statement_Kind;
enum
{
	Statement_Kind__None,
	Statement_Kind__Instruction,
	Statement_Kind__Directive,
	Statement_Kind__Label,
	Statement_Kind__Label_Numeric,
};

typedef enum Statement_Flags
{
	Statement_Flags__Relax_Disabled                  = 1 << 0,
	Statement_Flags__Size_Variable                   = 1 << 1,
	Statement_Flags__JAL_Register_Destination_Unset  = 1 << 2,
	Statement_Flags__CALL_Register_Destination_Unset = 1 << 3,
}
Statement_Flags;

// TODO: revisit padding.
//
// An in-memory representation of parsed assembly statement, which can be either an instruction, a directive, or a label
// definition.
typedef struct Statement Statement;
struct Statement
{
	// The list of parsed expressions that occurred in this statement.
	// If its an instruction, once evaluated it would yield its immediate.
	U32 *expressions_indexes;

	// The symbol defined by the statement, if any.
	Symbols_Table_Entry *s_symbol;

	// If this is a label definition, this represents the slot of the Symbols_Table where this symbol is saved.
	// U32 label_symbol_slot;
	// If this is a numeric label definition, it is its number.
	U8  label_numeric_value;

	U32 expressions_count;
	// The list of tokens that make the statement.
	U32 token_index_begin;
	// Included
	U32 token_index_end;

	Instruction_Kind    instruction_kind;
	Directive_Kind      directive_kind;

	Relocation_Operator relocation_operator;

	U8  instruction_format; // R, I, S, B, ...
	U8  register_destination;
	U8  register_source_1;
	U8  register_source_2;

	// Relaxation-related fields. These fields can change during relaxation process.

	// The offset in the object file section where this statement is written to.
	U32 section_offset;
	// The size of the statement, as it would be written in the object file section. Note that a directive will have
	// size zero.
	U32 size;

	// End of relaxation-related fields.

	// The index of the object file section this statement belongs to.
	U8  section_index;
	Statement_Kind kind;

	Statement_Flags flags;
};


typedef struct Statements Statements;
struct Statements
{
	Arena *arena;
	Statement *data;
	U32 count;
};

// NOTE: the first element will be zero-initialized, so it can act as a sentinel value.
internal void
Statements_initialize(Statements *statements, Arena *arena);


// Return the pointer to the arena-allocated statement
//
// The arena cannot be used for other purposes while pushing elements into it, it assumes a contiguous range of
// statements for safe iteration.
internal Statement *
Statements_push(Statements *statements, Statement statement);

#endif // STATEMENT_H


