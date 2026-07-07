#ifndef PARSER_DIRECTIVE_H
#define PARSER_DIRECTIVE_H

typedef enum Directive_Kind
{
        Directive_Kind__None,
        Directive_Kind__Section,
        Directive_Kind__Text,
        Directive_Kind__Data,
        Directive_Kind__Read_Only_Data,
        Directive_Kind__BSS,
        Directive_Kind__Local,
        Directive_Kind__Weak,
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
        Directive_Kind__Equiv,
        Directive_Kind__Eqv,
        Directive_Kind__Fill,
        Directive_Kind__Skip,
        Directive_Kind__Space,
        Directive_Kind__Zero,
        Directive_Kind__Common,
        Directive_Kind__Option,
        Directive_Kind__COUNT,
}
Directive_Kind;

global const String8 Directive_Kind__String8_table[Directive_Kind__COUNT] =
{
        [Directive_Kind__None]            = String8__literal(""),
        [Directive_Kind__Section]         = String8__literal(".section"),
        [Directive_Kind__Text]            = String8__literal(".text"),
        [Directive_Kind__Data]            = String8__literal(".data"),
        [Directive_Kind__Read_Only_Data]  = String8__literal(".rodata"),
        [Directive_Kind__BSS]             = String8__literal(".bss"),
        [Directive_Kind__Local]           = String8__literal(".local"),
        [Directive_Kind__Weak]            = String8__literal(".weak"),
        [Directive_Kind__Globl]           = String8__literal(".globl"),
        [Directive_Kind__Global]          = String8__literal(".global"),
        [Directive_Kind__Byte]            = String8__literal(".byte"),
        [Directive_Kind__Word_Half]       = String8__literal(".half"),
        [Directive_Kind__Word]            = String8__literal(".word"),
        [Directive_Kind__Word_Double]     = String8__literal(".dword"),
        [Directive_Kind__Ascii]           = String8__literal(".ascii"),
        [Directive_Kind__Asciz]           = String8__literal(".asciz"),
        [Directive_Kind__String]          = String8__literal(".string"),
        [Directive_Kind__Align]           = String8__literal(".align"),
        [Directive_Kind__Equality]        = String8__literal(".equ"),
        [Directive_Kind__Set]             = String8__literal(".set"),
        [Directive_Kind__Equiv]           = String8__literal(".equiv"),
        [Directive_Kind__Eqv]             = String8__literal(".eqv"),
        [Directive_Kind__Fill]            = String8__literal(".fill"),
        [Directive_Kind__Skip]            = String8__literal(".skip"),
        [Directive_Kind__Space]           = String8__literal(".space"),
        [Directive_Kind__Zero]            = String8__literal(".zero"),
        [Directive_Kind__Common]          = String8__literal(".comm"),
        [Directive_Kind__Option]          = String8__literal(".option"),
};

Directive_Kind
Directive_Kind__from_String8(String8 source);


typedef enum Set_Mode
{
        // Used in `.set/.equ`.
	Set_Mode__Override = 0,
        // Used in `.eqv`.
	Set_Mode__Strict_Forward,
        // Used in `.equiv`.
	Set_Mode__Strict,
}
Set_Mode;

internal void
binding_set
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Symbols_Table           *symbols_table,
        ELF_Symbol_Binding       binding
);

internal void
directive_set_like
(
        Arena                   *arena,
        Token_Cursor            *cursor,
        Diagnostic_List         *diagnostics,
        Expressions             *expressions,
        Symbols_Table           *symbols_table,
        Section                 *section,
        Sections_Table          *section_table,
        Set_Mode                 mode
);

#endif // PARSER_DIRECTIVE_H
