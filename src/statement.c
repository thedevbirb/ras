internal void
Statements_initialize(Statements *statements, Arena *arena)
{
	*statements = (Statements)
	{
		.arena = arena,
		.data  = (Statement *)(Arena_push_struct_m(arena, Statement)),
		.count = 1,
	};
	return;
}

internal Statement *
Statements_push(Statements *statements, Statement statement)
{
	// TODO: make a list of invariants that must be met when pushing a statement.
	assert_always_m(statement.kind);
	assert_always_m((statement.instruction_kind && statement.directive_kind) == 0);

	Statement *buffer = Arena_push_struct_m(statements->arena, Statement);
	*buffer = statement;

	statements->count += 1;
	return buffer;
}
