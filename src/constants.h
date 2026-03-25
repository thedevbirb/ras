#ifndef CONSTANTS_H
#define CONSTANTS_H

//////////////////////////////////////////
// Tokens

typedef enum Token_Kind
{
	Token_Kind__None = 0,

	Token_Kind__Dot,
	Token_Kind__Comma,

	Token_Kind__Left_Parenthesis,
	Token_Kind__Right_Parenthesis,

	Token_Kind__Plus,
	Token_Kind__Minus,
	Token_Kind__Star,
	Token_Kind__Slash,
	Token_Kind__Tilde,
	Token_Kind__Caret,

	Token_Kind__Newline,

	Token_Kind__Shift_Right,
	Token_Kind__Greater_Equal,
	Token_Kind__Greater_Than,

	Token_Kind__Shift_Left,
	Token_Kind__Less_Equal,
	Token_Kind__Less_Than,

	Token_Kind__Equal,  // '=='
	Token_Kind__Assign, // '='

	Token_Kind__Equal_Not,
	Token_Kind__Bang,

	Token_Kind__Logical_Or,
	Token_Kind__Pipe,
	Token_Kind__Logical_And,
	Token_Kind__Ampersand,

	Token_Kind__Relocation_Prefix, // %
	Token_Kind__Percentage,

	Token_Kind__Label,
	Token_Kind__Directive,

	Token_Kind__Char_Literal,
	Token_Kind__String_Literal,

	Token_Kind__Identifier,
	Token_Kind__Number_Literal,

	Token_Kind__EOF,

	Token_Kind__COUNT
}
Token_Kind;


//////////////////////////////////////////
// Directives

typedef enum Directive_Kind
{
	Directive_Kind__None,
	Directive_Kind__Section,
	Directive_Kind__Text,
	Directive_Kind__Data,
	Directive_Kind__Read_Only_Data,
	Directive_Kind__BSS,
	Directive_Kind__Globl,
	Directive_Kind__Word,
	Directive_Kind__Ascii,
	Directive_Kind__Asciz,
	Directive_Kind__Align,
	Directive_Kind__Equality,
	Directive_Kind__COUNT,
}
Directive_Kind;

global const char *Directive_Kind_strings[Directive_Kind__COUNT] =
{
	[Directive_Kind__None]            = "",
	[Directive_Kind__Section]         = ".section",
	[Directive_Kind__Text]            = ".text",
	[Directive_Kind__Data]            = ".data",
	[Directive_Kind__Read_Only_Data]  = ".rodata",
	[Directive_Kind__BSS]             = ".bss",
	[Directive_Kind__Globl]           = ".globl",
	[Directive_Kind__Word]            = ".word",
	[Directive_Kind__Ascii]           = ".ascii",
	[Directive_Kind__Align]           = ".align",
	[Directive_Kind__Equality]        = ".equ",
	[Directive_Kind__Asciz]           = ".asciz",
};

// TODO: probably can change with just memcmp
internal Directive_Kind
Directive_Kind__from_String8(String8 string)
{
	Directive_Kind kind = Directive_Kind__None;

	U32 token_index = 0;
	B32 found = 0;
	for (;;)
	{
		B32 break_should = found || token_index >= Directive_Kind__COUNT;
		if (break_should)
		{
			break;
		}

		const char *target = Directive_Kind_strings[token_index];

		U32 index_match = 0;
		B32 mismatch = 0;
		for (;;)
		{
			B32 break_should = mismatch || index_match >= string.count || target[index_match] == '\0';
			if (break_should)
			{
				break;
			}

			mismatch = string.data[index_match] != target[index_match];
			index_match += 1;
		}

		found = !mismatch && index_match == string.count && target[index_match] == '\0';
		if (found)
		{
			kind = token_index;
		}
		else
		{
			token_index += 1;
		}
	}

	return kind;
}

////////////////////////////////////////
// ELF

typedef enum ELF64_Section_Header_Type
{
	ELF64_Section_Header_Type__None             = 0,
	ELF64_Section_Header_Type__Program_Bits     = 1,
	ELF64_Section_Header_Type__Symbols_Table    = 2,
	ELF64_Section_Header_Type__String_Table     = 3,
	ELF64_Section_Header_Type__Relocations      = 4,
	ELF64_Section_Header_Type__Note             = 7,
	ELF64_Section_Header_Type__No_Bits          = 8,
	ELF64_Section_Header_Type__RISCV_Attributes = 0x70000003
}
ELF64_Section_Header_Type;

typedef enum ELF64_Section
{
	ELF64_Section__None = 0,
	ELF64_Section__Text,
	ELF64_Section__Data,
	ELF64_Section__Read_Only_Data,
	ELF64_Section__BSS,
	ELF64_Section__Relocations_Text,
	ELF64_Section__Relocations_Data,
	ELF64_Section__Symbols_Table,
	ELF64_Section__String_Table,
	ELF64_Section__Section_Names,
	ELF64_Section__RISCV_Attributes,
	// ELF64_Section__Note_GNU_Stack,
	ELF64_Section__COUNT,
}
ELF64_Section;

global const U8 ELF64_Section_from_Directive_Kind[ELF64_Section__COUNT] =
{
	[Directive_Kind__None]           = 0,
	[Directive_Kind__Section]        = 0,
	[Directive_Kind__Text]           = ELF64_Section__Text,
	[Directive_Kind__Data]           = ELF64_Section__Data,
	[Directive_Kind__Read_Only_Data] = ELF64_Section__Read_Only_Data,
	[Directive_Kind__BSS]            = ELF64_Section__BSS,
	[Directive_Kind__Globl]          = 0,
	[Directive_Kind__Word]           = 0,
	[Directive_Kind__Ascii]          = 0,
	[Directive_Kind__Asciz]          = 0,
	[Directive_Kind__Align]          = 0,
};

global const char *ELF64_Section_strings[ELF64_Section__COUNT] =
{
	[ELF64_Section__None]             = "",
	[ELF64_Section__Text]             = ".text",
	[ELF64_Section__Data]             = ".data",
	[ELF64_Section__Read_Only_Data]   = ".rodata",
	[ELF64_Section__BSS]              = ".bss",
	[ELF64_Section__Relocations_Text] = ".rela.text",
	[ELF64_Section__Relocations_Data] = ".rela.data",
	[ELF64_Section__Symbols_Table]    = ".symtab",
	[ELF64_Section__String_Table]     = ".strtab",
	[ELF64_Section__Section_Names]    = ".shstrtab",
	[ELF64_Section__RISCV_Attributes] = ".riscv.attributes",
	// [ELF64_Section__Note_GNU_Stack = ".note.GNU-stack",
};

ELF64_Section_Header_Type ELF64_Section_Header_Type_from_ELF64_Section[ELF64_Section__COUNT] =
{
	[ELF64_Section__None]              = ELF64_Section_Header_Type__None,
	[ELF64_Section__Text]              = ELF64_Section_Header_Type__Program_Bits,
	[ELF64_Section__Data]              = ELF64_Section_Header_Type__Program_Bits,
	[ELF64_Section__Read_Only_Data]    = ELF64_Section_Header_Type__Program_Bits,
	[ELF64_Section__BSS]               = ELF64_Section_Header_Type__No_Bits,
	[ELF64_Section__Relocations_Text]  = ELF64_Section_Header_Type__Relocations,
	[ELF64_Section__Relocations_Data]  = ELF64_Section_Header_Type__Relocations,
	[ELF64_Section__Symbols_Table]     = ELF64_Section_Header_Type__Symbols_Table,
	[ELF64_Section__String_Table]      = ELF64_Section_Header_Type__String_Table,
	[ELF64_Section__Section_Names]     = ELF64_Section_Header_Type__String_Table,
	[ELF64_Section__RISCV_Attributes]  = ELF64_Section_Header_Type__RISCV_Attributes,
};

// Default value for section alignments.
global const U8 ELF64_Section_alignments[ELF64_Section__COUNT] =
{
	[ELF64_Section__None]              = 0,
	[ELF64_Section__Text]              = 4,
	[ELF64_Section__Data]              = 8,
	[ELF64_Section__Read_Only_Data]    = 8,
	[ELF64_Section__BSS]               = 8,
	[ELF64_Section__Relocations_Text]  = 8,
	[ELF64_Section__Relocations_Data]  = 8,
	[ELF64_Section__Symbols_Table]     = 8,
	[ELF64_Section__String_Table]      = 1,
	[ELF64_Section__Section_Names]     = 1,
	[ELF64_Section__RISCV_Attributes]  = 1
};

//////////////////////////////////////////////
// Expression

typedef enum Expression_Flags
{
	Expression_Flags__Deferred  = 0,
	Expression_Flags__Immediate = 1,
}
Expression_Flags;


#endif // CONSTANTS_H

