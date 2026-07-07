#ifndef LEXER_H
#define LEXER_H

global const U8 escape_valid_table[256];

typedef enum Lexing_Error_Kind
{
        Lexer_Error_Kind__None,
        Lexer_Error_Kind__String_Multiline_Unsupported,
        Lexer_Error_Kind__String_Literal_Unterminated,
        Lexer_Error_Kind__Escape_Sequence_Invalid,
        Lexer_Error_Kind__Character_Literal_Multiline_Unsupported,
        Lexer_Error_Kind__Character_Literal_Empty,
        Lexer_Error_Kind__Character_Literal_Multiple,
        Lexer_Error_Kind__Character_Literal_Escape_Invalid,
        Lexer_Error_Kind__Character_Literal_Unterminated,
        Lexer_Error_Kind__Numeric_Literal_Invalid,
        Lexer_Error_Kind__Numeric_Hex_Literal_Invalid,
        Lexer_Error_Kind__Numeric_Octal_Literal_Invalid,
        Lexer_Error_Kind__Numeric_Binary_Literal_Invalid,
        Lexer_Error_Kind__Character_Unexpected,
        Lexer_Error_Kind__Escape_Sequence_Unterminated,
        Lexer_Error_Kind__Label_Directive_Invalid,
        Lexer_Error_Kind__COUNT,
}
Lexer_Error_Kind;

global const String8 lexer_error_kind_messages[Lexer_Error_Kind__COUNT] =
{
        [Lexer_Error_Kind__None]                                    = String8__literal(""),
        [Lexer_Error_Kind__String_Multiline_Unsupported]            = String8__literal("multiline strings are not supported"),
        [Lexer_Error_Kind__String_Literal_Unterminated]             = String8__literal("string literal unterminated"),
        [Lexer_Error_Kind__Escape_Sequence_Invalid]                 = String8__literal("escape sequence invalid"),
        [Lexer_Error_Kind__Character_Literal_Multiline_Unsupported] = String8__literal("multiline character literals are not supported"),
        [Lexer_Error_Kind__Character_Literal_Empty]                 = String8__literal("empty character literal"),
        [Lexer_Error_Kind__Character_Literal_Multiple]              = String8__literal("character literal contains multiple characters"),
        [Lexer_Error_Kind__Character_Literal_Escape_Invalid]        = String8__literal("character literal contains invalid escape"),
        [Lexer_Error_Kind__Character_Literal_Unterminated]          = String8__literal("character literal untermindated"),
        [Lexer_Error_Kind__Numeric_Literal_Invalid]                 = String8__literal("numerical literal is invalid"),
        [Lexer_Error_Kind__Numeric_Hex_Literal_Invalid]             = String8__literal("numerical hex literal is invalid"),
        [Lexer_Error_Kind__Numeric_Octal_Literal_Invalid]           = String8__literal("numerical octal literal is invalid"),
        [Lexer_Error_Kind__Numeric_Binary_Literal_Invalid]          = String8__literal("numerical binary literal is invalid"),
        [Lexer_Error_Kind__Escape_Sequence_Unterminated]            = String8__literal("escape sequence unterminated"),
        [Lexer_Error_Kind__Label_Directive_Invalid]                 = String8__literal("invalid label or directive"),
        [Lexer_Error_Kind__Character_Unexpected]                    = String8__literal("unexpected character"),
};

typedef struct Token Token;
struct Token
{
        U64         numerical_value; // No float support yet.
        U32         location;
        U32         index;

        U32         size;
        Token_Kind  kind;
};

internal Range1_U32
Token__range(Token token)
{
        Range1_U32 result = {{ token.location, token.location + token.size }};
        return result;
}

typedef struct Token_Cursor Token_Cursor;
struct Token_Cursor
{
        Source *source;
        // The last token read with `token_next`.
        Token current;
        // The previous token read with `token_next`.
        Token previous;
        U32   source_index;
};

internal String8
Token_Cursor__text(Token_Cursor *cursor)
{
        String8 result =
        {
                .data  = &cursor->source->data[cursor->current.index],
                .count =  cursor->current.size
        };
        return result;
}

// Remove quotes.
internal String8
String8__skip_chop(String8 token_string)
{

        String8 result = {0};
        result = String8__skip(token_string, 1);
        result = String8__chop(result, 1);
        return result;
}

#endif // LEXER_H

