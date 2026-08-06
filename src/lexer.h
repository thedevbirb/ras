#ifndef LEXER_H
#define LEXER_H

global const U8 escape_valid_table[256] =
{
        ['a']  = 1,  // bell
        ['b']  = 1,  // backspace
        ['t']  = 1,  // tab
        ['n']  = 1,  // newline
        ['v']  = 1,  // vertical tab
        ['f']  = 1,  // form feed
        ['r']  = 1,  // carriage return
        ['e']  = 1,  // escape
        ['\\'] = 1,  // backslash
        ['\''] = 1,  // single quote
        ['"']  = 1,  // double quote
        ['0']  = 1,  // null or octal begin
        ['1']  = 1,  // octal begin
        ['2']  = 1,  // octal begin
        ['3']  = 1,  // octal begin
        ['x']  = 1,  // hex begin
};

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

internal String8 Token_Cursor__text(Token_Cursor *);

internal Token lex_at(const Source *, U32 index_current, Diagnostics *);

internal Token token_peek(Token_Cursor const *, Diagnostics *);
internal void  token_next(Token_Cursor *,       Diagnostics *);

// Read raw bytes from the token cursor as the next `Token_Kind__Identifier`, advancing it. The result is available at
// `Token_Cursor.current`.
//
// Rationale: life is made of exceptions. Consider this precise case: `.section .note.GNU-stack`. Clearly, this section
// name cannot be lexed entirely as `Token_Kind__Identifier`, due to the minus sign. Yet, it is emitted by compilers and
// accepted by assemblers. The `.section` makes this exception and reads raw bytes unil one belonging to an ending set
// is found. In other places or directives instead, the usual notion an identifier applies.
internal void
Token_Cursor__read_raw_identifier_until(Token_Cursor *cursor, String8 ending_bytes_set, B32 skip_whitespace);

#endif // LEXER_H

