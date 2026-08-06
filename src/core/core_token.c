internal Range1_U32
Token__range(Token token)
{
        Range1_U32 result = {{ token.location, token.location + token.size }};
        return result;
}

internal B32
Token_Kind__end_of_statement(Token_Kind kind)
{
        B32 result = kind == Token_Kind__Newline
                  || kind == Token_Kind__Semicolon
                  || kind == Token_Kind__None;
        return result;
}
