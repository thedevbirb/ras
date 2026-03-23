#ifndef EXPRESSION_H
#define EXPRESSION_H

global char const *relocation_names[] =
{
    "hi", "lo", "pcrel_hi", "pcrel_lo", "got_pcrel_hi",
    "tprel_hi", "tprel_lo", "tprel_add",
    "tls_ie_pcrel_hi", "tls_gd_pcrel_hi", NULL,
};

typedef enum Expression_Parsed_Error_Kind Expression_Parsed_Error_Kind
{
	Expression_Parsed_Error_Kind__None,
	Expression_Parsed_Error_Kind__Token_Invalid,
	Expression_Parsed_Error_Kind__COUNT
}
Expression_Parsed_Error_Kind;

typedef enum Expression_Kind Expression_Kind
{
	Expression_Kind__None,
	Expression_Kind__Logical_Or,
	Expression_Kind__Logical_And,
	Expression_Kind__Bitwise_Or,
	Expression_Kind__Bitwise_Xor,
	Expression_Kind__Bitwise_And,
	Expression_Kind__Equality,
	Expression_Kind__Comparison,
	Expression_Kind__Shift,
	Expression_Kind__Additive,
	Expression_Kind__Multiplicative,
	Expression_Kind__Unary,
	Expression_Kind__Atom,
	Expression_Kind__COUNT,
}
Expression_Kind;

typedef struct Expression Expression;
struct Expression
{
	Token *tokens;
	U32    count;
	U32    index;

	Expression_Kind kind;
}
Expression;

#define unary_minus_precedence (1 << 15)

global const U8 token_precedence[Token_Kind__COUNT] =
{
	[Token_Kind__None]              = 0,

	[Token_Kind__Comma]             = 0,

	[Token_Kind__Left_Parenthesis]  = 0,
	[Token_Kind__Right_Parenthesis] = 0,

	[Token_Kind__Plus]              = 1 << 9,
	[Token_Kind__Minus]             = 1 << 9,
	[Token_Kind__Star]              = 1 << 10,
	[Token_Kind__Slash]             = 1 << 10,
	[Token_Kind__Tilde]             = 1 << 15,
	[Token_Kind__Caret]             = 1 << 4,

	[Token_Kind__Newline]           = 0,

	[Token_Kind__Shift_Right]       = 1 << 8,
	[Token_Kind__Greater_Equal]     = 1 << 7,
	[Token_Kind__Greater_Than]      = 1 << 7,

	[Token_Kind__Shift_Left]        = 1 << 8,
	[Token_Kind__Less_Equal]        = 1 << 7,
	[Token_Kind__Less_Than]         = 1 << 7,

	[Token_Kind__Equal]             = 1 << 6,  // '=='
	[Token_Kind__Assign]            = 0, // ' ='

	[Token_Kind__Equal_Not]         = 1 << 6,
	[Token_Kind__Bang]              = 1 << 15,

	[Token_Kind__Logical_Or]        = 1 << 1,
	[Token_Kind__Pipe]              = 1 << 3,
	[Token_Kind__Logical_And]       = 1 << 2,
	[Token_Kind__Ampersand]         = 1 << 5,

	[Token_Kind__Relocation_Prefix] = 0
	[Token_Kind__Percentage]        = 1 << 10,

	[Token_Kind__Label]             = 0,
	[Token_Kind__Directive]         = 0,

	[Token_Kind__Char_Literal]      = 0,
	[Token_Kind__String_Literal]    = 0,

	[Token_Kind__Identifier]        = 0,
	[Token_Kind__Number_Literal]    = 0,

	[Token_Kind__EOF]               = 0,
}
Token_Kind;

typedef enum Expression_Kind
{
    Expression_Kind__None,
    Expression_Kind__Integer_Literal,
    Expression_Kind__Char_Literal,
    Expression_Kind__Symbol,
    Expression_Kind__Current_Address,
    Expression_Kind__Relocation,
    Expression_Kind__Unary_Operation,
    Expression_Kind__Binary_Operation,

    Expression_Kind__COUNT,
}
Expression_Kind;

typedef struct Expression_Node Expression_Node;
struct Expression_Node
{
    Expression_Node *left;
    Expression_Node *right;
    Expression_Kind  kind;
    Expression_Error error
    union
    {
	    U64   integer_unsigned;
	    S64   integer_signed;
	    F64   double;
	    Range range_input;
    } value;
};


#define expression_recursion_max 8

static Expression_Node
parse_null_denotation(Expression *expression, Token token, Arena *arena)
{
	Expression_Node left = {0};
	switch (token.kind)
	{
		case Token_Kind__Number_Literal:
		{
			left = expression_node_integer_literal(token.integer_value);
		} break;

		case Token_Kind__Char_Literal:
			{
				return expression_node_char_literal(token.integer_value);
			}

		case Token_Kind__Identifier:
			{
				return expression_node_symbol(token.string_value);
			}

		case Token_Kind__Dot:
			{
				return expression_node_current_address();
			}

		case Token_Kind__Minus:
		case Token_Kind__Tilde:
		case Token_Kind__Bang:
			{
				Expression_Node *operand = parse_expression(parser, Binding_Power__Unary);
				return expression_node_unary_operation(token.kind, operand);
			}

		case Token_Kind__Relocation_Prefix:
			{
				parser_expect(parser, Token_Kind__Left_Parenthesis);
				Expression_Node *inner_expression = parse_expression(parser, Binding_Power__None);
				parser_expect(parser, Token_Kind__Right_Parenthesis);
				return expression_node_relocation(token.string_value, inner_expression);
			}

		case Token_Kind__Left_Parenthesis:
			{
				Expression_Node *inner_expression = parse_expression(parser, Binding_Power__None);
				parser_expect(parser, Token_Kind__Right_Parenthesis);
				return inner_expression;
			}

		default:
			{
				fprintf(stderr, "error: unexpected token kind %d at start of expression\n",
						token.kind);
				exit(1);
			}
	}
}

internal Expression_Node
Expression_parse(Expression *expression, U32 precedence, Arena *arena)
{
	Token token = expression.tokens[expression.index];
	Expression_Node *left = parse_null_denotation(expression, token, arena);

	for (;;)
	{
		B32 break_should = parsed.expression.index >= parsed.expression.count || parsed.error;
		if (break_should)
		{
			break;
		}

		Token token = expression.tokens[expression.index];
		switch (token.kind)
		{
		case Token_Kind__None: { assert_always_m(0 && "unreachable, lexer bug"); } break;
		case Token_Kind__Comma: { error = 1; } break;

		case Token_Kind__Left_Parenthesis:
		{
			parsed.expression.index += 1;
			expression = Expression_Parsed(expression);
			// ?
		} break;

		case Token_Kind__Number_Literal:
		{
			parsed.node.integer_unsigned = Number_from(token); //?
			parsed.expression.index += 1;
		} break;
		case Token_Kind__Plus:
		{

		}
		default:
		{
		} break;
		}
	}

	return parsed;
}


//expression calls logical_or on the entire remaining token list starting from the current position. There is no
//filtering or subsetting — every rule receives the same token stream and a cursor (an index) indicating where to start
//reading. So if your token list is: [%, hi, (, my_array, +, 8, ), +, 3, *, 2] and the cursor is at position 0, then
//expression calls logical_or with that same list and cursor at 0. logical_or calls logical_and with the same list,
//cursor still at 0. This continues all the way down to atom, which is the first rule that actually advances the cursor
//by consuming tokens. As each rule returns, it passes back two things: the node it parsed, and the new cursor position.
//So when atom finishes consuming %hi(my_array + 8), it returns its node and a cursor now pointing at position 7 (the
//+). multiplicative receives that, checks the token at position 7, sees + which is not *///%, and returns the same node
//and cursor unchanged. additive receives that, checks position 7, sees +, matches it, advances the cursor to 8, and
//calls multiplicative again starting at position 8 to parse the right operand. The grammar rules never slice or copy
//the token list. They all operate on the same list, just passing around the current read position.


// https://matklad.github.io/2025/12/23/zig-newtype-index-pattern.html
//
// hints on how to built a tree also.

// TODO: I have to return an Expression_Tree and moving on in the parsing stage, become some symbols will be unresolved
// and I may not be able to evalute it immediately, so that needs to be done at a separate stage.


#endif // EXPRESSION_H


// %hi('A' + my_symbol) || ~. && !1 | 0xF ^ 0b1 & (2 == 3) != 4 <= 5 >= 6 < 7 > 8 << 9 >> 10 + 11 - 12 * 13 / 14 % 15
//
// ~. && (!1 | 0xF) ^ 0b1 & (2 == 3) != 4 <= 5 >= 6 < 7 > 8 << 9 >> ((10 + 11) - (12 * 13)) / 14 % 15
//
// (symbol * (5 + 6))
//
// Algorithm:
//
// Allocate a list of expressions based with capacity the number of tokens.
//
// [, , , , , ..., ]
//
// Scan for the current level of precedence:
//
// 1. Unary -> [~., !1]
//
// Everything that can be evaluated immediately is done as such and saved. What can't be evaluated immediately,
// you save a placeholder of a deferred computation, demanding resulution at that token.
//
// New idea
//
// First, again the goal is to _parse_ the expression, we cannot evaluate it already because of symbols. So the result
// is something that we know how to evaluate once we can resolve the symbols. If we cannot resolve the symbol, its
// computation is deferred. It's a bit unclear how that looks like in a cascading scenario, probably placeholders until
// done?
//
// Consider the expression:
//
// -1 * -(3 * (my_sym + 2)) * (1 + 2)
//
// [my_sim + 2, * 3, -1
//
// Made by the tokens:
//
// [-, 1, *, -, (, 3, *, (, my_sym, +, 2, ), ), *, (, 1, +, 2, )]
//
// The goal of the algorithm is to reorder the expression in something easy to evalute from left to right.
//
// Output:
//
// [my_sim + 2, 1 + 2, 3 * index_0, - index_2, -1 * index_4, index_4 * index_1]
// 0          1      2            3          4             4
//
// The evaluation process, assuming we know my_sym equals 4, looks as follow. We iterate over the such output and we
// replace values:
//
// [6         , 3    , 18         , -18      , 18          , 48               ]
//
// And in the last position we have our result.
//
// Let's see how we can create this list. We first allocate a list with the same size of the tokens, so we know it fits.
// Then we iter over each token with a cursor:
//
struct Cursor
{
	U32 output_index;
	U32 input_index;
	U32 left_parenthesis_last_index;
};
// The goal of the cursor is to remove parenthesis. So it would scan the tokens to look for left parenthesis tracking
// their index, and as soon as a right index is found, the following happens:
//
// 1. All the tokens inside the parenthesis are "copied" in the output array, at output_index.
// 2.

// -------------------------------

/*
 * RISC-V GAS Expression Pratt Parser
 *
 * A top-down operator precedence parser that builds an abstract syntax
 * tree from a token stream. Covers the full GAS operator precedence
 * chain, unary operators, integer literals, symbol references,
 * relocations, and parenthesized sub-expressions.
 *
 * Written in Casey Muratori's C style: no abbreviations, explicit
 * control flow, flat structures, clear naming.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


/* ----------------------------------------------------------------
 * Token
 * ---------------------------------------------------------------- */

typedef enum
{
    Token_Kind__None = 0,
    Token_Kind__Comma,
    Token_Kind__Left_Parenthesis,
    Token_Kind__Right_Parenthesis,
    Token_Kind__Plus,
    Token_Kind__Minus,
    Token_Kind__Star,
    Token_Kind__Slash,
    Token_Kind__Pipe,
    Token_Kind__Ampersand,
    Token_Kind__Tilde,
    Token_Kind__Caret,
    Token_Kind__Modulo,
    Token_Kind__Relocation_Prefix,
    Token_Kind__Bang,
    Token_Kind__Newline,
    Token_Kind__Greater_Than,
    Token_Kind__Less_Than,
    Token_Kind__Greater_Equal,
    Token_Kind__Less_Equal,
    Token_Kind__Shift_Left,
    Token_Kind__Shift_Right,
    Token_Kind__Equal,
    Token_Kind__Not_Equal,
    Token_Kind__Logical_Or,
    Token_Kind__Logical_And,
    Token_Kind__Assign,
    Token_Kind__Label,
    Token_Kind__Directive,
    Token_Kind__Char_Literal,
    Token_Kind__String_Literal,
    Token_Kind__Identifier,
    Token_Kind__Number_Literal,
    Token_Kind__Dot,
    Token_Kind__End_Of_File,
    Token_Kind__COUNT,
}
Token_Kind;

typedef struct
{
    Token_Kind kind;
    long       integer_value;
    char       string_value[128];
}
Token;




/* ----------------------------------------------------------------
 * AST Node
 * ---------------------------------------------------------------- */

typedef enum
{
    Expression_Kind__Integer_Literal,
    Expression_Kind__Char_Literal,
    Expression_Kind__Symbol,
    Expression_Kind__Current_Address,
    Expression_Kind__Relocation,
    Expression_Kind__Unary_Operation,
    Expression_Kind__Binary_Operation,
}
Expression_Kind;

typedef struct Expression_Node Expression_Node;
struct Expression_Node
{
    Expression_Kind  kind;
    long             integer_value;
    char             string_value[128];
    Token_Kind       operator_kind;
    Expression_Node *left;
    Expression_Node *right;
};

static Expression_Node *
expression_node_allocate(Expression_Kind kind)
{
    Expression_Node *node = calloc(1, sizeof(Expression_Node));
    node->kind = kind;
    return node;
}

static Expression_Node *
expression_node_integer_literal(long value)
{
    Expression_Node *node = expression_node_allocate(Expression_Kind__Integer_Literal);
    node->integer_value = value;
    return node;
}

static Expression_Node *
expression_node_char_literal(long value)
{
    Expression_Node *node = expression_node_allocate(Expression_Kind__Char_Literal);
    node->integer_value = value;
    return node;
}

static Expression_Node *
expression_node_symbol(char const *name)
{
    Expression_Node *node = expression_node_allocate(Expression_Kind__Symbol);
    strncpy(node->string_value, name, 127);
    return node;
}

static Expression_Node *
expression_node_current_address(void)
{
    return expression_node_allocate(Expression_Kind__Current_Address);
}

static Expression_Node *
expression_node_relocation(char const *relocation_name, Expression_Node *inner_expression)
{
    Expression_Node *node = expression_node_allocate(Expression_Kind__Relocation);
    strncpy(node->string_value, relocation_name, 127);
    node->left = inner_expression;
    return node;
}

static Expression_Node *
expression_node_unary_operation(Token_Kind operator_kind, Expression_Node *operand)
{
    Expression_Node *node = expression_node_allocate(Expression_Kind__Unary_Operation);
    node->operator_kind = operator_kind;
    node->left = operand;
    return node;
}

static Expression_Node *
expression_node_binary_operation(Token_Kind operator_kind,
                                 Expression_Node *left_operand,
                                 Expression_Node *right_operand)
{
    Expression_Node *node = expression_node_allocate(Expression_Kind__Binary_Operation);
    node->operator_kind = operator_kind;
    node->left  = left_operand;
    node->right = right_operand;
    return node;
}


/* ----------------------------------------------------------------
 * Parser
 * ---------------------------------------------------------------- */

typedef struct
{
    Lexer lexer;
    Token current_token;
    bool  has_current_token;
}
Parser;

static void
parser_initialize(Parser *parser, char const *source)
{
    lexer_initialize(&parser->lexer, source);
    parser->has_current_token = false;
}

static Token
parser_peek(Parser *parser)
{
    if (!parser->has_current_token)
    {
        parser->current_token     = lexer_read_next_token(&parser->lexer);
        parser->has_current_token = true;
    }
    return parser->current_token;
}

static Token
parser_advance(Parser *parser)
{
    Token token = parser_peek(parser);
    parser->has_current_token = false;
    return token;
}

static Token
parser_expect(Parser *parser, Token_Kind expected_kind)
{
    Token token = parser_advance(parser);
    if (token.kind != expected_kind)
    {
        fprintf(stderr, "error: expected token kind %d, got %d\n",
                expected_kind, token.kind);
        exit(1);
    }
    return token;
}


/* ----------------------------------------------------------------
 * Binding Power
 * ---------------------------------------------------------------- */

typedef enum
{
    Binding_Power__None           =   0,
    Binding_Power__Logical_Or     =   2,
    Binding_Power__Logical_And    =   4,
    Binding_Power__Bitwise_Or     =   6,
    Binding_Power__Bitwise_Xor    =   8,
    Binding_Power__Bitwise_And    =  10,
    Binding_Power__Equality       =  12,
    Binding_Power__Comparison     =  14,
    Binding_Power__Shift          =  16,
    Binding_Power__Additive       =  18,
    Binding_Power__Multiplicative =  20,
    Binding_Power__Unary          = 100,
}
Binding_Power;

static int
binding_power_for_token_kind(Token_Kind kind)
{
    switch (kind)
    {
        case Token_Kind__Logical_Or:    return Binding_Power__Logical_Or;
        case Token_Kind__Logical_And:   return Binding_Power__Logical_And;
        case Token_Kind__Pipe:          return Binding_Power__Bitwise_Or;
        case Token_Kind__Caret:         return Binding_Power__Bitwise_Xor;
        case Token_Kind__Ampersand:     return Binding_Power__Bitwise_And;
        case Token_Kind__Equal:
        case Token_Kind__Not_Equal:     return Binding_Power__Equality;
        case Token_Kind__Less_Than:
        case Token_Kind__Greater_Than:
        case Token_Kind__Less_Equal:
        case Token_Kind__Greater_Equal: return Binding_Power__Comparison;
        case Token_Kind__Shift_Left:
        case Token_Kind__Shift_Right:   return Binding_Power__Shift;
        case Token_Kind__Plus:
        case Token_Kind__Minus:         return Binding_Power__Additive;
        case Token_Kind__Star:
        case Token_Kind__Slash:
        case Token_Kind__Modulo:        return Binding_Power__Multiplicative;
        default:                        return Binding_Power__None;
    }
}


/* ----------------------------------------------------------------
 * Expression Parsing (Pratt Parser)
 * ---------------------------------------------------------------- */

static Expression_Node *parse_expression(Parser *parser, int minimum_binding_power);

static Expression_Node *
parse_null_denotation(Parser *parser, Token token)
{
    switch (token.kind)
    {
        case Token_Kind__Number_Literal:
        {
            return expression_node_integer_literal(token.integer_value);
        }

        case Token_Kind__Char_Literal:
        {
            return expression_node_char_literal(token.integer_value);
        }

        case Token_Kind__Identifier:
        {
            return expression_node_symbol(token.string_value);
        }

        case Token_Kind__Dot:
        {
            return expression_node_current_address();
        }

        case Token_Kind__Minus:
        case Token_Kind__Tilde:
        case Token_Kind__Bang:
        {
            Expression_Node *operand = parse_expression(parser, Binding_Power__Unary);
            return expression_node_unary_operation(token.kind, operand);
        }

        case Token_Kind__Relocation_Prefix:
        {
            parser_expect(parser, Token_Kind__Left_Parenthesis);
            Expression_Node *inner_expression = parse_expression(parser, Binding_Power__None);
            parser_expect(parser, Token_Kind__Right_Parenthesis);
            return expression_node_relocation(token.string_value, inner_expression);
        }

        case Token_Kind__Left_Parenthesis:
        {
            Expression_Node *inner_expression = parse_expression(parser, Binding_Power__None);
            parser_expect(parser, Token_Kind__Right_Parenthesis);
            return inner_expression;
        }

        default:
        {
            fprintf(stderr, "error: unexpected token kind %d at start of expression\n",
                    token.kind);
            exit(1);
        }
    }
}

static Expression_Node *
parse_left_denotation(Parser *parser, Token_Kind operator_kind, Expression_Node *left_operand)
{
    int right_binding_power = binding_power_for_token_kind(operator_kind);
    Expression_Node *right_operand = parse_expression(parser, right_binding_power);
    return expression_node_binary_operation(operator_kind, left_operand, right_operand);
}

static Expression_Node *
parse_expression(Parser *parser, int minimum_binding_power)
{
    Token first_token = parser_advance(parser);
    Expression_Node *left = parse_null_denotation(parser, first_token);

    for (;;)
    {
        Token_Kind next_kind           = parser_peek(parser).kind;
        int        next_binding_power  = binding_power_for_token_kind(next_kind);

        if (next_binding_power <= minimum_binding_power)
        {
            break;
        }

        parser_advance(parser);
        left = parse_left_denotation(parser, next_kind, left);
    }

    return left;
}


/* ----------------------------------------------------------------
 * AST Printer
 * ---------------------------------------------------------------- */

static char const *
operator_kind_to_string(Token_Kind kind)
{
    switch (kind)
    {
        case Token_Kind__Plus:          return "+";
        case Token_Kind__Minus:         return "-";
        case Token_Kind__Star:          return "*";
        case Token_Kind__Slash:         return "/";
        case Token_Kind__Modulo:        return "%";
        case Token_Kind__Tilde:         return "~";
        case Token_Kind__Bang:          return "!";
        case Token_Kind__Ampersand:     return "&";
        case Token_Kind__Pipe:          return "|";
        case Token_Kind__Caret:         return "^";
        case Token_Kind__Shift_Left:    return "<<";
        case Token_Kind__Shift_Right:   return ">>";
        case Token_Kind__Less_Than:     return "<";
        case Token_Kind__Greater_Than:  return ">";
        case Token_Kind__Less_Equal:    return "<=";
        case Token_Kind__Greater_Equal: return ">=";
        case Token_Kind__Equal:         return "==";
        case Token_Kind__Not_Equal:     return "!=";
        case Token_Kind__Logical_And:   return "&&";
        case Token_Kind__Logical_Or:    return "||";
        default:                        return "?";
    }
}

static void
expression_node_print(Expression_Node *node, int depth)
{
    for (int index = 0; index < depth; index++)
    {
        printf("  ");
    }

    switch (node->kind)
    {
        case Expression_Kind__Integer_Literal:
        {
            printf("Integer_Literal(%ld)\n", node->integer_value);
        } break;

        case Expression_Kind__Char_Literal:
        {
            printf("Char_Literal(%ld)\n", node->integer_value);
        } break;

        case Expression_Kind__Symbol:
        {
            printf("Symbol(%s)\n", node->string_value);
        } break;

        case Expression_Kind__Current_Address:
        {
            printf("Current_Address(.)\n");
        } break;

        case Expression_Kind__Relocation:
        {
            printf("Relocation(%%%s)\n", node->string_value);
            expression_node_print(node->left, depth + 1);
        } break;

        case Expression_Kind__Unary_Operation:
        {
            printf("Unary_Operation(%s)\n", operator_kind_to_string(node->operator_kind));
            expression_node_print(node->left, depth + 1);
        } break;

        case Expression_Kind__Binary_Operation:
        {
            printf("Binary_Operation(%s)\n", operator_kind_to_string(node->operator_kind));
            expression_node_print(node->left, depth + 1);
            expression_node_print(node->right, depth + 1);
        } break;
    }
}


/* ----------------------------------------------------------------
 * Free the AST
 * ---------------------------------------------------------------- */

static void
expression_node_free(Expression_Node *node)
{
    if (!node) return;
    expression_node_free(node->left);
    expression_node_free(node->right);
    free(node);
}


/* ----------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------- */

int
main(void)
{
    char const *test_expressions[] =
    {
        "%hi(my_array + 8) + 3 * 2",
        "~(flags & 0xFF)",
        ". - start",
        "%pcrel_lo(.Lref) + 4",
        "1 + 2 * 3 + 4",
        "a << 3 | b >> 1",
        "-x + !y * ~z",
        "(a + b) * (c - d)",
        NULL,
    };

    for (char const **test = test_expressions; *test; test++)
    {
        printf("--- %s ---\n", *test);

        Parser parser;
        parser_initialize(&parser, *test);

        Expression_Node *expression_tree = parse_expression(&parser, Binding_Power__None);
        expression_node_print(expression_tree, 0);
        expression_node_free(expression_tree);

        printf("\n");
    }

    return 0;
}
