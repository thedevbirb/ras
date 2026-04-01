#ifndef RESOLVER_H
#define RESOLVER_H

typedef enum Resolver_Error_Kind
{
	Resolver_Error_Kind__None,
	Resolver_Error_Kind__Expression_Kind_Unknown,
	Resolver_Error_Kind__COUNT,
}
Resolver_Error_Kind;

typedef struct Resolver_Error Resolver_Error;
struct Resolver_Error
{
	Resolver_Error_Kind kind;
};

typedef struct Resolver Resolver;
struct Resolver
{
	Arena         *arena;
	Input         *input;
	Token         *tokens;
	Statements    *statements;
	Expressions   *expressions;
	Symbols_Table *symbols_table;

	Resolver_Error error;

	U32 sections_offset[ELF64_Section__COUNT];
	U16 section_current_index;
};

void
Resolver_error_set(Resolver *resolver, Resolver_Error_Kind kind);

typedef struct Expression_Evaluation Expression_Evaluation;
struct Expression_Evaluation
{
	U64 value;
	B32 unresolved;
};

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

#endif // RESOLVER_H

