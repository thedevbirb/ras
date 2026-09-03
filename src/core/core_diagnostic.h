#ifndef CORE_DIAGNOSTIC_H
#define CORE_DIAGNOSTIC_H

global String8 Diagnostic__previous_declaration_String8 = String8__literal("previous declaration is here");

typedef enum Diagnostic_Kind
{
        Diagnostic_Kind__Error,
        Diagnostic_Kind__Warning,
        Diagnostic_Kind__Hint,
        Diagnostic_Kind__Note,
        Diagnostic_Kind__COUNT,
}
Diagnostic_Kind;

typedef enum Diagnostic_ANSI_Color
{
        Diagnostic_ANSI_Color_Default = 39,
        Diagnostic_ANSI_Color_Red     = 31,
        Diagnostic_ANSI_Color_Green   = 32,
        Diagnostic_ANSI_Color_Yellow  = 33,
        Diagnostic_ANSI_Color_Blue    = 34,
        Diagnostic_ANSI_Color_Magenta = 35,
        Diagnostic_ANSI_Color_Cyan    = 36,
}
Diagnostic_ANSI_Color;

typedef struct Diagnostic_Style Diagnostic_Style;
struct Diagnostic_Style
{
        Diagnostic_ANSI_Color color;
        B32                   bold;
};

internal const char *diagnostic_labels[] =
{
        [Diagnostic_Kind__Error]   = "error",
        [Diagnostic_Kind__Warning] = "warning",
        [Diagnostic_Kind__Hint]    = "hint",
        [Diagnostic_Kind__Note]    = "note",
};

internal Diagnostic_Style diagnostic_styles[] =
{
        [Diagnostic_Kind__Error]   = { .color = Diagnostic_ANSI_Color_Red,     .bold = 1 },
        [Diagnostic_Kind__Warning] = { .color = Diagnostic_ANSI_Color_Yellow,  .bold = 1 },
        [Diagnostic_Kind__Hint]    = { .color = Diagnostic_ANSI_Color_Magenta, .bold = 1 },
        [Diagnostic_Kind__Note]    = { .color = Diagnostic_ANSI_Color_Cyan,    .bold = 1 },
};

global const Diagnostic_Style Diagnostic_Style__default_bold = { .color = Diagnostic_ANSI_Color_Default, .bold = 1 };

typedef struct Diagnostic_Fix Diagnostic_Fix;
struct Diagnostic_Fix
{
        Range1_U32 range;
        String8  text;
};

typedef enum DG
{
	DG__None = 0,

	// Lexer
	DG__Character_Literal_Unterminated = 1000,
	DG__Character_Literal_Multiline,
	DG__Character_Unexpected,
	DG__Escape_Sequence_Invalid,
	DG__Numeric_Binary_Literal_Invalid,
	DG__Numeric_Hex_Literal_Invalid,
	DG__Numeric_Octal_Literal_Invalid,
	DG__String_Literal_Unterminated,
	DG__String_Multiline_Unsupported,

	// Statement & Label
	DG__Label_Duplicate = 2000,
	DG__Label_Numeric_Expected_Colon,
	DG__Label_Backward_Not_Found,
	DG__Line_End_Junk,
	DG__Line_Invalid,

	// Expression
	DG__Binary_Operator_Unsupported_Non_Constant = 3000,
	DG__Unary_Operator_Unsupported_Non_Constant,
	DG__Constant_Expression_Expected,
	DG__Constant_Expression_Expected_Lower,
	DG__Constant_Value_Range,
	DG__Expression_Not_Resolved_Finalized,
	DG__Expression_Null_Denotation,
	DG__Expression_Recursive_Symbolic,
	DG__Expression_Symbol_Or_Constant,
	DG__Parenthesis_Left_Unclosed,
	DG__Parenthesis_Right_Unmatched,
	DG__Subtract_Cannot_Resolve,

	// Instruction
	DG__Offset_Too_Large = 4000,
	DG__Opcode_Format_Unrecognized,
	DG__Shift_Doesnt_Fit,
	DG__CSR_Number_Invalid,
	DG__CSR_Immediate_Invalid,
	DG__AMO_Offset_Nonzero,

	// Relocation
	DG__Jump_Offset_Invalid = 5000,
	DG__PC_Relative_High_Offset_Invalid,
	DG__Relocation_8_16_Bit_Cant_Represent,
	DG__Relocation_Addend_32_Bits,
	DG__Relocation_Non_Constant,
	DG__Relocation_Operator_Invalid_Instruction,
	DG__TLS_Relocation_Constant,

	// Symbol
	DG__Declaration_Previous = 6000,
	DG__Symbol_Redefined,
	DG__Symbol_Value_Truncated,

	// Directive
	DG__Architecture_Parse = 7000,
	DG__Attribute_After_Assembly,
	DG__Attribute_Unknown,
	DG__Attribute_Value_0_1,
	DG__Base64_Character_Invalid,
	DG__Base64_Length_Multiple_4,
	DG__Comma_Expected,
	DG__Data_Directive_Disrupts_Alignment,
	DG__Directive_Escape_Sequence_Invalid,
	DG__Directive_Unknown,
	DG__Directive_Unsupported,
	DG__Identifier_Expected,
	DG__Number_Or_String_Expected,
	DG__Option_Unknown,
	DG__Size_Already_Set,
	DG__Size_Expression_Exceeds_32,
	DG__Size_Expression_Not_Constant,
	DG__Size_Expression_Not_Positive,
	DG__String_File_Expected,
	DG__String_Literal_Expected,
	DG__Type_Syntax_Expected,
	DG__Value_Too_Large_Truncated,

	// Section
	DG__Entry_Size_Invalid_Ignored = 8000,
	DG__Flags_Redefinition_Ignored,
	DG__Section_Flags_Invalid,
	DG__Section_Name_Empty,
	DG__Section_Size_Not_Multiple,
	DG__Section_Type_Invalid,
	DG__Section_Type_Syntax_Invalid,
	DG__Type_Redefinition_Ignored,
        DG__Section_Pop_Unmatched,
        DG__Section_Previous_Undefined,

	// Alignment
	DG__Alignment_Boundary_Not_Constant = 9000,
	DG__Alignment_Boundary_Not_Positive,
	DG__Alignment_Boundary_Not_Power_Of_Two,
	DG__Alignment_Larger_2_32,
	DG__Alignment_Not_Power_Of_Two,
	DG__Alignment_Padding_Multiple,
	DG__Alignment_Pattern_Too_Large,
	DG__Alignment_Max_Non_Positive,

	// Fill
	DG__Fill_Disrupts_Alignment = 10000,
	DG__Fill_Negative,
	DG__Fill_Not_Constant,
	DG__Fill_Size_1_8,
	DG__Fill_Size_Capping,

	DG__COUNT,
}
DG;


// A singly-linked double-ended list (queue) of diagnostics. I've chosen a queue because I just want to append
// them as we go, with only a very light container struct to manage them.

typedef struct Diagnostic Diagnostic;
struct Diagnostic
{
        Diagnostic     *previous;
        Diagnostic     *next;
        String8         message;
        // Logical location where the diagnostic has been emitted.
        U32             location;
        // TODO(low): flag support. This field is a placeholder for now, but its usage means
        // that we're interested in seeing the source code where the error occurred, because there isn't any to show.
        B32             location_disabled;
        // TODO(low): value support. Most diagnostic should have an unique, numeric identifier to disambiguate.
        DG              code;
        // Squiggly ('~') ranges to put under the line of the provided logical location.
        Range1_U32      ranges[4];
        // Additional lines with text fixes.
        Diagnostic_Fix  fixes[4];
        Diagnostic_Kind kind;
};

#define Diagnostics__errors_max_default 20

// TODO(feature): may need to support:
//
// 1. Configuration, e.g. transform warning in errors.
// 2. Maybe counter of errors to abort on too many of them?
typedef struct Diagnostics Diagnostics;
struct Diagnostics
{
        Arena      *arena;
        // A valid, empty diagnostic to return.
        //
        // TODO(low): use this to implement the pattern of at most one diagnostic per line:
        // Call `Diagnostics__limit` to set a maximum amount of diagnostics to emit. Then manually recall
        // `Diagnostics__limit_reset` to reset it.
        Diagnostic *dummy;
        Diagnostic *first;
        Diagnostic *last;
        U32         count;
        U32         count_max;
        U32         errors_count;
        U32         errors_max;

        // E.g. to limit the amount of diagnostics per line
        U8          limit_max;
        U8          limit_count;
};

internal Diagnostics *
Diagnostics__new(Arena *arena);

// `Arena` for lazily computing the line start indexes.
internal void
Diagnostic__print(Diagnostic *diagnostic, Source *source, Arena *arena);

typedef struct DG_Info DG_Info;
struct DG_Info
{
        Diagnostic_Kind severity;
        String8 message;
};

global const DG_Info DG_Info__table[] =
{
	// Lexer
	[DG__Character_Literal_Unterminated]   = { Diagnostic_Kind__Error,  String8__literal("character literal untermindated") },
	[DG__Character_Literal_Multiline]      = { Diagnostic_Kind__Error,  String8__literal("multiline character literals are not supported") },
	[DG__Character_Unexpected]             = { Diagnostic_Kind__Error,  String8__literal("unexpected character") },
	[DG__Escape_Sequence_Invalid]          = { Diagnostic_Kind__Error,  String8__literal("escape sequence invalid") },
	[DG__Numeric_Binary_Literal_Invalid]   = { Diagnostic_Kind__Error,  String8__literal("numerical binary literal is invalid") },
	[DG__Numeric_Hex_Literal_Invalid]      = { Diagnostic_Kind__Error,  String8__literal("numerical hex literal is invalid") },
	[DG__Numeric_Octal_Literal_Invalid]    = { Diagnostic_Kind__Error,  String8__literal("numerical octal literal is invalid") },
	[DG__String_Literal_Unterminated]      = { Diagnostic_Kind__Error,  String8__literal("string literal unterminated") },
	[DG__String_Multiline_Unsupported]     = { Diagnostic_Kind__Error,  String8__literal("multiline strings are not supported") },

	// Statement & Label
	[DG__Label_Duplicate]                  = { Diagnostic_Kind__Error,  String8__literal("duplicate label found") },
	[DG__Label_Numeric_Expected_Colon]     = { Diagnostic_Kind__Error,  String8__literal("expected ':' for numeric label declaration") },
	[DG__Label_Backward_Not_Found]         = { Diagnostic_Kind__Error,  String8__literal("backward label reference not found") },
	[DG__Line_End_Junk]                    = { Diagnostic_Kind__Error,  String8__literal("junk found at the end of line") },
	[DG__Line_Invalid]                     = { Diagnostic_Kind__Error,  String8__literal("line can only start with a directive), label or instruction") },

	// Expression
	[DG__Binary_Operator_Unsupported_Non_Constant]  = { Diagnostic_Kind__Error, String8__literal("unsupported binary operator on non-constant symbols") },
	[DG__Unary_Operator_Unsupported_Non_Constant]   = { Diagnostic_Kind__Error, String8__literal("unsupported unary operator on non-constant symbols") },
	[DG__Constant_Expression_Expected]              = { Diagnostic_Kind__Error, String8__literal("Constant expression expected") },
	[DG__Constant_Expression_Expected_Lower]        = { Diagnostic_Kind__Error, String8__literal("constant expression expected") },
	[DG__Constant_Value_Range]                      = { Diagnostic_Kind__Error, String8__literal("constant expression value must in the range 0..1048576") },
	[DG__Expression_Not_Resolved_Finalized]         = { Diagnostic_Kind__Error, String8__literal("expression cannot be fully resolved and finalized") },
	[DG__Expression_Null_Denotation]                = { Diagnostic_Kind__Error, String8__literal("expected number, symbol or unary operator") },
	[DG__Expression_Recursive_Symbolic]             = { Diagnostic_Kind__Error, String8__literal("recursive symbolic expression found") },
	[DG__Expression_Symbol_Or_Constant]             = { Diagnostic_Kind__Error, String8__literal("expression must be either symbol or a constant") },
	[DG__Parenthesis_Left_Unclosed]                 = { Diagnostic_Kind__Error, String8__literal("unclosed '('") },
	[DG__Parenthesis_Right_Unmatched]               = { Diagnostic_Kind__Error, String8__literal("unexpected ')' to close non-matching '('") },
	[DG__Subtract_Cannot_Resolve]                   = { Diagnostic_Kind__Error, String8__literal("") },

	// Instruction
	[DG__Offset_Too_Large]                 = { Diagnostic_Kind__Error,  String8__literal("offset too large for this opcode") },
	[DG__Opcode_Format_Unrecognized]       = { Diagnostic_Kind__Error,  String8__literal("unrecognized opcode format") },
	[DG__Shift_Doesnt_Fit]                 = { Diagnostic_Kind__Error,  String8__literal("shift amount doesn't fit register size") },
	[DG__CSR_Number_Invalid]               = { Diagnostic_Kind__Error,  String8__literal("CSR address must be a constant expression in the range 0..4095") },
	[DG__CSR_Immediate_Invalid]            = { Diagnostic_Kind__Error,  String8__literal("CSR immediate must be a constant expression in the range 0..31") },
	[DG__AMO_Offset_Nonzero]              = { Diagnostic_Kind__Error,  String8__literal("atomic instructions require an omitted or zero offset") },

	// Relocation
	[DG__Jump_Offset_Invalid]                     = { Diagnostic_Kind__Error, String8__literal("") },
	[DG__PC_Relative_High_Offset_Invalid]         = { Diagnostic_Kind__Error, String8__literal("") },
	[DG__Relocation_8_16_Bit_Cant_Represent]      = { Diagnostic_Kind__Error, String8__literal("cannot represent an 8-bit or 16-bit relocation on RISC-V/ELF object file") },
	[DG__Relocation_Addend_32_Bits]               = { Diagnostic_Kind__Error, String8__literal("relocation addend doesn't fit in 32 bits") },
	[DG__Relocation_Non_Constant]                 = { Diagnostic_Kind__Error, String8__literal("Non-constant expression must have an appropriate relocation operator") },
	[DG__Relocation_Operator_Invalid_Instruction] = { Diagnostic_Kind__Error, String8__literal("invalid relocation operator for instruction") },
	[DG__TLS_Relocation_Constant]                 = { Diagnostic_Kind__Error, String8__literal("TLS relocation against a constant") },

	// Symbol
	[DG__Declaration_Previous]             = { Diagnostic_Kind__Note,   String8__literal("previous declaration is here") },
	[DG__Symbol_Redefined]                 = { Diagnostic_Kind__Error,  String8__literal("symbol cannot be redefined") },
	[DG__Symbol_Value_Truncated]           = { Diagnostic_Kind__Warning, String8__literal("symbol value exceeds 32 bits, will be truncated") },

	// Directive
	[DG__Architecture_Parse]                 = { Diagnostic_Kind__Error,   String8__literal("") },
	[DG__Attribute_After_Assembly]           = { Diagnostic_Kind__Error,   String8__literal("cannot set this attribute after assembly started") },
	[DG__Attribute_Unknown]                  = { Diagnostic_Kind__Error,   String8__literal("unknown attribute") },
	[DG__Attribute_Value_0_1]                = { Diagnostic_Kind__Error,   String8__literal("invalid value for attributes, must be either 0 or 1") },
	[DG__Base64_Character_Invalid]           = { Diagnostic_Kind__Error,   String8__literal("invalid base64 character") },
	[DG__Base64_Length_Multiple_4]           = { Diagnostic_Kind__Error,   String8__literal("base64 string length should a multiple of 4") },
	[DG__Comma_Expected]                     = { Diagnostic_Kind__Error,   String8__literal("comma expected") },
	[DG__Data_Directive_Disrupts_Alignment]  = { Diagnostic_Kind__Error,   String8__literal("") },
	[DG__Directive_Escape_Sequence_Invalid]  = { Diagnostic_Kind__Error,   String8__literal("invalid escape sequence") },
	[DG__Directive_Unknown]                  = { Diagnostic_Kind__Error,   String8__literal("unknown directive found") },
	[DG__Directive_Unsupported]              = { Diagnostic_Kind__Warning, String8__literal("directive unsupported, skipping") },
	[DG__Identifier_Expected]                = { Diagnostic_Kind__Error,   String8__literal("expected identifier") },
	[DG__Number_Or_String_Expected]          = { Diagnostic_Kind__Error,   String8__literal("expected number or string, depending on attribute") },
	[DG__Option_Unknown]                     = { Diagnostic_Kind__Error,   String8__literal("unknown option") },
	[DG__Size_Already_Set]                   = { Diagnostic_Kind__Warning, String8__literal("size already set, not changing it") },
	[DG__Size_Expression_Exceeds_32]         = { Diagnostic_Kind__Error,   String8__literal("size expression exceeds 32 bits") },
	[DG__Size_Expression_Not_Constant]       = { Diagnostic_Kind__Error,   String8__literal("size expression expected to have constant evaluation") },
	[DG__Size_Expression_Not_Positive]       = { Diagnostic_Kind__Error,   String8__literal("size expression expected to have positive evaluation") },
	[DG__String_File_Expected]               = { Diagnostic_Kind__Error,   String8__literal("expected string file") },
	[DG__String_Literal_Expected]            = { Diagnostic_Kind__Error,   String8__literal("string literal expected") },
	[DG__Type_Syntax_Expected]               = { Diagnostic_Kind__Error,   String8__literal("`.type <name>,@<type>` syntax expected") },
	[DG__Value_Too_Large_Truncated]          = { Diagnostic_Kind__Warning, String8__literal("value too large, truncated") },

	// Section
	[DG__Entry_Size_Invalid_Ignored]       = { Diagnostic_Kind__Warning, String8__literal("invalid section entry size, ignored") },
	[DG__Flags_Redefinition_Ignored]       = { Diagnostic_Kind__Warning, String8__literal("ignoring redefinition of flags for special section") },
	[DG__Section_Flags_Invalid]            = { Diagnostic_Kind__Error,   String8__literal("invalid section flags, expected: " ELF_Section_Header_Flags__cstring) },
	[DG__Section_Name_Empty]               = { Diagnostic_Kind__Error,   String8__literal("section directive has empty name") },
	[DG__Section_Size_Not_Multiple]        = { Diagnostic_Kind__Warning, String8__literal("") },
	[DG__Section_Type_Invalid]             = { Diagnostic_Kind__Error,   String8__literal("invalid section type") },
	[DG__Section_Type_Syntax_Invalid]      = { Diagnostic_Kind__Error,   String8__literal("invalid section type syntax, expected @type") },
	[DG__Type_Redefinition_Ignored]        = { Diagnostic_Kind__Warning, String8__literal("ignoring redefinition of type for special section") },
        [DG__Section_Pop_Unmatched]            = { Diagnostic_Kind__Error,   String8__literal(".popsection without corresponding .pushsection") },
        [DG__Section_Previous_Undefined]       = { Diagnostic_Kind__Error,   String8__literal(".previous without corresponding .section") },

	// Alignment
	[DG__Alignment_Boundary_Not_Constant]     = { Diagnostic_Kind__Error,   String8__literal("alignment_boundary expression expected to have constant evaluation") },
	[DG__Alignment_Boundary_Not_Positive]     = { Diagnostic_Kind__Error,   String8__literal("alignment_boundary expression expected to have positive evaluation") },
	[DG__Alignment_Boundary_Not_Power_Of_Two] = { Diagnostic_Kind__Error,   String8__literal("alignment_boundary is not a power of two") },
	[DG__Alignment_Larger_2_32]               = { Diagnostic_Kind__Error,   String8__literal("alignment larger than 2^32 bytes") },
	[DG__Alignment_Not_Power_Of_Two]          = { Diagnostic_Kind__Warning, String8__literal("alignment boundary not a power of two") },
	[DG__Alignment_Padding_Multiple]          = { Diagnostic_Kind__Error,   String8__literal("") },
	[DG__Alignment_Pattern_Too_Large]         = { Diagnostic_Kind__Error,   String8__literal("alignment pattern larger than pattern size") },
	[DG__Alignment_Max_Non_Positive]          = { Diagnostic_Kind__Warning, String8__literal("non-positive max alignment write size, ensuring it is zero") },

	// Fill
	[DG__Fill_Disrupts_Alignment] = { Diagnostic_Kind__Error,   String8__literal("") },
	[DG__Fill_Negative]           = { Diagnostic_Kind__Error,   String8__literal("filling directive resolves to negative value") },
	[DG__Fill_Not_Constant]       = { Diagnostic_Kind__Error,   String8__literal("filling directive doesn't resolve to constant expression") },
	[DG__Fill_Size_1_8]           = { Diagnostic_Kind__Error,   String8__literal("expected constant size expression between 1 and 8 included") },
	[DG__Fill_Size_Capping]       = { Diagnostic_Kind__Warning, String8__literal("capping fill size to 2^31 bytes") },
};

internal Diagnostic *
Diagnostics__push(Diagnostics *diagnostics, DG dg);

#endif // CORE_DIAGNOSTIC_H
