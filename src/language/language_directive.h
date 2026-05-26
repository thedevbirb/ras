#ifndef LANGUAGE_DIRECTIVE_H
#define LANGUAGE_DIRECTIVE_H

typedef enum Directive_Kind
{
	Directive_Kind__None,
	Directive_Kind__Section,
	Directive_Kind__Text,
	Directive_Kind__Data,
	Directive_Kind__Read_Only_Data,
	Directive_Kind__BSS,
	Directive_Kind__Local,
	Directive_Kind__Globl,
	Directive_Kind__Global,
	Directive_Kind__Byte,
	Directive_Kind__Word_Half,
	Directive_Kind__Word,
	Directive_Kind__Word_Double,
	Directive_Kind__Ascii,
	Directive_Kind__Asciz,
	Directive_Kind__String,
	Directive_Kind__Align,
	Directive_Kind__Equality,
	Directive_Kind__Set,
	Directive_Kind__Skip,
	Directive_Kind__Zero,
	Directive_Kind__Common,
	Directive_Kind__Option,
	Directive_Kind__COUNT,
}
Directive_Kind;

global const String8 Directive_Kind__String8_table[Directive_Kind__COUNT] =
{
	[Directive_Kind__None]            = { .data = (U8 *)"",         .count = 0 },
	[Directive_Kind__Section]         = { .data = (U8 *)".section", .count = 8 },
	[Directive_Kind__Text]            = { .data = (U8 *)".text",    .count = 5 },
	[Directive_Kind__Data]            = { .data = (U8 *)".data",    .count = 5 },
	[Directive_Kind__Read_Only_Data]  = { .data = (U8 *)".rodata",  .count = 7 },
	[Directive_Kind__BSS]             = { .data = (U8 *)".bss",     .count = 4 },
	[Directive_Kind__Local]           = { .data = (U8 *)".local",   .count = 6 },
	[Directive_Kind__Globl]           = { .data = (U8 *)".globl",   .count = 6 },
	[Directive_Kind__Global]          = { .data = (U8 *)".global",  .count = 7 },
	[Directive_Kind__Byte]            = { .data = (U8 *)".byte",    .count = 5 },
	[Directive_Kind__Word_Half]       = { .data = (U8 *)".half",    .count = 5 },
	[Directive_Kind__Word]            = { .data = (U8 *)".word",    .count = 5 },
	[Directive_Kind__Word_Double]     = { .data = (U8 *)".dword",   .count = 6 },
	[Directive_Kind__Ascii]           = { .data = (U8 *)".ascii",   .count = 6 },
	[Directive_Kind__Asciz]           = { .data = (U8 *)".asciz",   .count = 6 },
	[Directive_Kind__String]          = { .data = (U8 *)".string",  .count = 7 },
	[Directive_Kind__Align]           = { .data = (U8 *)".align",   .count = 6 },
	[Directive_Kind__Equality]        = { .data = (U8 *)".equ",     .count = 4 },
	[Directive_Kind__Set]             = { .data = (U8 *)".set",     .count = 4 },
	[Directive_Kind__Skip]            = { .data = (U8 *)".skip",    .count = 5 },
	[Directive_Kind__Zero]            = { .data = (U8 *)".zero",    .count = 5 },
	[Directive_Kind__Common]          = { .data = (U8 *)".comm",    .count = 5 },
	[Directive_Kind__Option]          = { .data = (U8 *)".option",  .count = 7 },
};

global const char *Directive_Kind_strings[Directive_Kind__COUNT] =
{
	[Directive_Kind__None]            = "",
	[Directive_Kind__Section]         = ".section",
	[Directive_Kind__Text]            = ".text",
	[Directive_Kind__Data]            = ".data",
	[Directive_Kind__Read_Only_Data]  = ".rodata",
	[Directive_Kind__BSS]             = ".bss",
	[Directive_Kind__Local]           = ".local",
	[Directive_Kind__Globl]           = ".globl",
	[Directive_Kind__Global]          = ".global",
	[Directive_Kind__Byte]            = ".byte",
	[Directive_Kind__Word_Half]       = ".half",
	[Directive_Kind__Word]            = ".word",
	[Directive_Kind__Word_Double]     = ".dword",
	[Directive_Kind__Ascii]           = ".ascii",
	[Directive_Kind__Asciz]           = ".asciz",
	[Directive_Kind__String]          = ".string",
	[Directive_Kind__Align]           = ".align",
	[Directive_Kind__Equality]        = ".equ",
	[Directive_Kind__Set]             = ".set",
	[Directive_Kind__Skip]            = ".skip",
	[Directive_Kind__Zero]            = ".zero",
	[Directive_Kind__Common]          = ".comm",
	[Directive_Kind__Option]          = ".option",
};

Directive_Kind
Directive_Kind__from_String8(String8 source);

#endif // LANGUAGE_DIRECTIVE_H

