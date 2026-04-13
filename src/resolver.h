#ifndef RESOLVER_H
#define RESOLVER_H

typedef enum Resolver_Error_Kind
{
	Resolver_Error_Kind__None,
	Resolver_Error_Kind__Expression_Kind_Unknown,
	Resolver_Error_Kind__Expression_Evaluation_Cross,
	Resolver_Error_Kind__Label_Numeric_Backward_Not_Found,
	Resolver_Error_Kind__Label_Numeric_Forward_Not_Found,
	Resolver_Error_Kind__Label_Numeric_Section_Cross,
	Resolver_Error_Kind__Section_Relocation_Invalid,
	Resolver_Error_Kind__Relocation_Operand_Invalid,
	Resolver_Error_Kind__Relocation_Operand_Symbol_Missing,
	Resolver_Error_Kind__Relocation_Symbol_Missing,
	Resolver_Error_Kind__Relocation_Form_Invalid,
	Resolver_Error_Kind__Symbol_Cyclic,
	Resolver_Error_Kind__Operator_Between_Symbols_Invalid,
	Resolver_Error_Kind__Operator_Expression_Unresolved,
	Resolver_Error_Kind__COUNT,
}
Resolver_Error_Kind;

global const char *Resolver_Error_Kind_messages[Resolver_Error_Kind__COUNT] =
{
	[Resolver_Error_Kind__Expression_Kind_Unknown]           = "expression unknown",
	[Resolver_Error_Kind__Expression_Evaluation_Cross]       = "expression involved evaluation of two symbols from different sections",
	[Resolver_Error_Kind__Label_Numeric_Backward_Not_Found]  = "label numeric backward reference not found",
	[Resolver_Error_Kind__Label_Numeric_Forward_Not_Found]   = "label numeric backward reference not found",
	[Resolver_Error_Kind__Label_Numeric_Section_Cross]       = "label numeric reference crosses section",
	[Resolver_Error_Kind__Relocation_Operand_Invalid]        = "only addition and subtraction can be done for relocations",
	[Resolver_Error_Kind__Relocation_Operand_Symbol_Missing] = "relocation has expression without symbol",
	[Resolver_Error_Kind__Relocation_Symbol_Missing]         = "relocation operator without symbol",
	[Resolver_Error_Kind__Relocation_Form_Invalid]           = "relocation can only be of the form symbol + addend",
	[Resolver_Error_Kind__Operator_Between_Symbols_Invalid]  = "only subtraction between symbols is supported",
	[Resolver_Error_Kind__Operator_Expression_Unresolved]    = "operator applied to unresolved subexpression",
	[Resolver_Error_Kind__Symbol_Cyclic]                     = "cyclic symbol definition",
};

typedef struct Resolver_Error Resolver_Error;
struct Resolver_Error
{
	Statement *statement;
	Resolver_Error_Kind kind;

	U32 row_index;
	U32 column_index_begin;
	U32 column_index_end;
};

typedef enum Resolver_Flags
{
	// Populate relocation tables during expressions evaluations.
	Resolver_Flags__Relocations = 1 << 0,
}
Resolver_Flags;


typedef struct Resolver Resolver;
struct Resolver
{
	Arena         *arena;
	Input         *input;
	Token         *tokens;
	Statements    *statements;
	Expressions   *expressions;
	Symbols_Table *symbols_table;
	// Contains the most recent occurrence of [1:, 2:, ..., 9:]. Layout: { x: value, y: set or not }.
	Vec2_U32 *labels_numeric_statement_index;

	Object_File_Section *sections;

	Resolver_Error error;

	Statement *statement_current;
	U32 statement_index;
	B32 statements_end_reached;

	Resolver_Flags flags;

	U32 sections_offset[ELF_Section__COUNT];
	U16 section_current_index;

};

void
Resolver_error_set(Resolver *resolver, Resolver_Error_Kind kind);

void
Resolver_expression_evaluate(Resolver *resolver, Expression_Node *node);

// Relaxation refers to the algorithm used by an assembler to "relax" the amount of bytes used by the instruction within
// its code. That happens because instruction which requires jumping, or loading values of unknown sizes, can expand or
// shrink depending by the actual values provided.
//
// This function provides a fixed point algorithm that converges when instructions cannot grow further. In the previous
// part of the of the codebase, instructions have been assigned their minimum size. If an instruction expands, it means
// that every other label and instruction below shift its offset. As such, the algorithm is composed of two parts that
// are repeated in loop:
//
// 1. A offset-compute phase -- All instructions and labels for a certain section have their offset assigned.
// 2. An expansion phase     -- Considering the offset computed previously, some instructions are expanded.
//
// If some instructions are expanded in step 2, then offsets in step 1 will be different, potentially changing the
// outcome of step 2 again. As such, the whole process is repeated until step 2 no longer expands instructions.
U32
Resolver_relax(Resolver *resolver);

void
Resolver_relocation_emit(Resolver *resolver, U32 symbol_index, S64 addend, Relocation_Operator operator);

#endif // RESOLVER_H

