// Assumption: the provided arena is used ONLY for this.
internal void
Statements_initialize(Statements *statements, Arena *arena)
{
	*statements = (Statements)
	{
		.arena = arena,
		.data  = (Statement *)(Arena_push_zero_m(arena)),
		.count = 0,
	};
	return;
}

internal Statement *
Statements_push(Statements *statements, Statement statement)
{
	assert_always_m(statement.kind);

	Statement *buffer = Arena_push_struct_m(statements->arena, Statement);
	*buffer = statement;

	statements->count += 1;
	return buffer;
}
