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

Expression_Evaluation
Resolver_expression_evaluate(Resolver *resolver, Expression_Node *node);

U32
Resolver_relax(Resolver *resolver);

#endif // RESOLVER_H

