#ifndef CORE_TOKEN_H
#define CORE_TOKEN_H

// TODO(refactor): I wonder whether making this a U64 and having token kind as bitflags would help.
// For example, we could have masks for end of statement i.e. `Token_Kind__Newline | Token_Kind__Semicolon` and so on.
typedef enum Token_Kind
{
        Token_Kind__None = 0,
        Token_Kind__Error,

        Token_Kind__Space,
        Token_Kind__Comment,

        Token_Kind__Dot,
        Token_Kind__Comma,
        Token_Kind__Semicolon,
        Token_Kind__Colon,

        Token_Kind__At,

        Token_Kind__Parenthesis_Left,
        Token_Kind__Parenthesis_Right,

        Token_Kind__Plus,
        Token_Kind__Minus,
        Token_Kind__Star,
        Token_Kind__Slash,
        Token_Kind__Tilde,
        Token_Kind__Caret,

        Token_Kind__Newline,

        Token_Kind__Greater_2,
        Token_Kind__Greater_Equal,
        Token_Kind__Greater,

        Token_Kind__Less_2,
        Token_Kind__Less_Equal,
        Token_Kind__Less,

        Token_Kind__Equal_2,
        Token_Kind__Equal,

        Token_Kind__Equal_Bang,
        Token_Kind__Bang,

        Token_Kind__Hash,

        Token_Kind__Pipe_2,
        Token_Kind__Pipe,
        Token_Kind__Ampersand_2,
        Token_Kind__Ampersand,

        Token_Kind__Percentage,

        Token_Kind__String,

        Token_Kind__Identifier,
        Token_Kind__Number,

        Token_Kind__COUNT
}
Token_Kind;

#ifdef RAS_TOKEN_DUMP
global const char *Token_Kind_strings[Token_Kind__COUNT] =
{
        [Token_Kind__None]              = "",
        [Token_Kind__Error]             = "",

        [Token_Kind__Space]             = "<space>",
        [Token_Kind__Comment]           = "comment",

        [Token_Kind__Dot]               = ".",
        [Token_Kind__Comma]             = ",",
        [Token_Kind__Semicolon]         = ";",
        [Token_Kind__Colon]             = ":",

        [Token_Kind__Parenthesis_Left]  = "(",
        [Token_Kind__Parenthesis_Right] = ")",

        [Token_Kind__Plus]              = "+",
        [Token_Kind__Minus]             = "-",
        [Token_Kind__Star]              = "*",
        [Token_Kind__Slash]             = "/",
        [Token_Kind__Tilde]             = "~",
        [Token_Kind__Caret]             = "^",

        [Token_Kind__Newline]           = "<newline>",

        [Token_Kind__Greater_2]         = ">>",
        [Token_Kind__Greater_Equal]     = ">=",
        [Token_Kind__Greater]           = ">",

        [Token_Kind__Less_2]            = "<<",
        [Token_Kind__Less_Equal]        = "<=",
        [Token_Kind__Less]              = "<",

        [Token_Kind__Equal_2]           = "==",
        [Token_Kind__Equal]             = "=",

        [Token_Kind__Equal_Bang]        = "!=",
        [Token_Kind__Bang]              = "!",

        [Token_Kind__Hash]              = "#",

        [Token_Kind__Pipe_2]            = "||",
        [Token_Kind__Pipe]              = "|",
        [Token_Kind__Ampersand_2]       = "&&",
        [Token_Kind__Ampersand]         = "&",

        [Token_Kind__Percentage]        = "%",

        [Token_Kind__String]            = "string",

        [Token_Kind__Identifier]        = "identifier",
        [Token_Kind__Number]            = "number",
};
#endif

typedef struct Token Token;
struct Token
{
        U64         numerical_value; // No float support yet.
        U32         location;
        U32         index;

        U32         size;
        Token_Kind  kind;
};

internal Range1_U32 Token__range(Token token);

// NOTE: this is syntax-specific though.
internal B32 Token_Kind__end_of_statement(Token_Kind);

#endif // CORE_TOKEN_H
