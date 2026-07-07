internal Range1_U32
Token__range(Token token)
{
        Range1_U32 result = {{ token.location, token.location + token.size }};
        return result;
}
