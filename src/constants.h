#ifndef CONSTANTS_H
#define CONSTANTS_H

#define expression_recursion_max 8

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
	Token_Kind__Label_Numeric,                     // e.g. 1:
	Token_Kind__Label_Numeric_Reference_Forward,   // e.g. 1f
	Token_Kind__Label_Numeric_Reference_Backward, // e.g. 1b
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

global const U8 ELF64_Section_from_Directive_Kind[Directive_Kind__COUNT] =
{
	[Directive_Kind__None]           = 0,
	[Directive_Kind__Section]        = 0,
	[Directive_Kind__Text]           = ELF64_Section__Text,
	[Directive_Kind__Data]           = ELF64_Section__Data,
	[Directive_Kind__Read_Only_Data] = ELF64_Section__Read_Only_Data,
	[Directive_Kind__BSS]            = ELF64_Section__BSS,
	[Directive_Kind__Globl]          = 0,
	[Directive_Kind__Byte]           = 0,
	[Directive_Kind__Word_Half]      = 0,
	[Directive_Kind__Word]           = 0,
	[Directive_Kind__Word_Double]    = 0,
	[Directive_Kind__Ascii]          = 0,
	[Directive_Kind__Asciz]          = 0,
	[Directive_Kind__Equality]       = 0,
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
	Expression_Flags__Deferred  = 1 << 0,
	Expression_Flags__Immediate = 1 << 1,
}
Expression_Flags;

typedef U8 Instruction_Format;
#define Instruction_Format__None       0 << 0
#define Instruction_Format__R          1 << 0
#define Instruction_Format__I          1 << 1
#define Instruction_Format__S          1 << 2
#define Instruction_Format__B          1 << 3
#define Instruction_Format__U          1 << 4
#define Instruction_Format__J	       1 << 5
// TODO: I don't know whether this is the right place.
#define Instruction_Format__Expandable 1 << 6

typedef enum Instruction_Kind
{
	Instruction_Kind__None = 0,

	// RV64I base
	Instruction_Kind__LUI,
	Instruction_Kind__AUIPC,

	Instruction_Kind__JAL,
	Instruction_Kind__JALR,

	Instruction_Kind__BEQ,
	Instruction_Kind__BNE,
	Instruction_Kind__BLT,
	Instruction_Kind__BGE,
	Instruction_Kind__BLTU,
	Instruction_Kind__BGEU,

	Instruction_Kind__LB,
	Instruction_Kind__LH,
	Instruction_Kind__LW,
	Instruction_Kind__LD,
	Instruction_Kind__LBU,
	Instruction_Kind__LHU,
	Instruction_Kind__LWU,

	Instruction_Kind__SB,
	Instruction_Kind__SH,
	Instruction_Kind__SW,
	Instruction_Kind__SD,

	Instruction_Kind__ADDI,
	Instruction_Kind__SLTI,
	Instruction_Kind__SLTIU,
	Instruction_Kind__XORI,
	Instruction_Kind__ORI,
	Instruction_Kind__ANDI,

	Instruction_Kind__SLLI,
	Instruction_Kind__SRLI,
	Instruction_Kind__SRAI,

	Instruction_Kind__ADD,
	Instruction_Kind__SUB,
	Instruction_Kind__SLL,
	Instruction_Kind__SLT,
	Instruction_Kind__SLTU,
	Instruction_Kind__XOR,
	Instruction_Kind__SRL,
	Instruction_Kind__SRA,
	Instruction_Kind__OR,
	Instruction_Kind__AND,

	// RV64-specific
	Instruction_Kind__ADDIW,
	Instruction_Kind__SLLIW,
	Instruction_Kind__SRLIW,
	Instruction_Kind__SRAIW,

	Instruction_Kind__ADDW,
	Instruction_Kind__SUBW,
	Instruction_Kind__SLLW,
	Instruction_Kind__SRLW,
	Instruction_Kind__SRAW,

	// Pseudo-instructions
	Instruction_Kind__NOP,
	Instruction_Kind__RET,
	Instruction_Kind__MV,
	Instruction_Kind__NOT,
	Instruction_Kind__NEG,
	Instruction_Kind__NEGW,
	Instruction_Kind__SEXT_W,
	Instruction_Kind__SEQZ,
	Instruction_Kind__SNEZ,
	Instruction_Kind__SLTZ,
	Instruction_Kind__SGTZ,
	Instruction_Kind__BEQZ,
	Instruction_Kind__BNEZ,
	Instruction_Kind__BLEZ,
	Instruction_Kind__BGEZ,
	Instruction_Kind__BLTZ,
	Instruction_Kind__BGTZ,
	Instruction_Kind__BGT,
	Instruction_Kind__BLE,
	Instruction_Kind__BGTU,
	Instruction_Kind__BLEU,
	Instruction_Kind__J,
	Instruction_Kind__CALL,
	Instruction_Kind__TAIL,
	Instruction_Kind__JR,
	Instruction_Kind__LI,
	Instruction_Kind__LA,

	// SYSTEM
	Instruction_Kind__ECALL,
	Instruction_Kind__EBREAK,
	Instruction_Kind__FENCE,
	Instruction_Kind__FENCE_I,

	Instruction_Kind__CSRRW,
	Instruction_Kind__CSRRS,
	Instruction_Kind__CSRRC,
	Instruction_Kind__CSRRWI,
	Instruction_Kind__CSRRSI,
	Instruction_Kind__CSRRCI,

	// M extension
	Instruction_Kind__MUL,
	Instruction_Kind__MULH,
	Instruction_Kind__MULHSU,
	Instruction_Kind__MULHU,
	Instruction_Kind__DIV,
	Instruction_Kind__DIVU,
	Instruction_Kind__REM,
	Instruction_Kind__REMU,

	Instruction_Kind__MULW,
	Instruction_Kind__DIVW,
	Instruction_Kind__DIVUW,
	Instruction_Kind__REMW,
	Instruction_Kind__REMUW,

	// A extension
	Instruction_Kind__LR_W,
	Instruction_Kind__SC_W,
	Instruction_Kind__AMOSWAP_W,
	Instruction_Kind__AMOADD_W,
	Instruction_Kind__AMOXOR_W,
	Instruction_Kind__AMOAND_W,
	Instruction_Kind__AMOOR_W,
	Instruction_Kind__AMOMIN_W,
	Instruction_Kind__AMOMAX_W,
	Instruction_Kind__AMOMINU_W,
	Instruction_Kind__AMOMAXU_W,

	Instruction_Kind__LR_D,
	Instruction_Kind__SC_D,
	Instruction_Kind__AMOSWAP_D,
	Instruction_Kind__AMOADD_D,
	Instruction_Kind__AMOXOR_D,
	Instruction_Kind__AMOAND_D,
	Instruction_Kind__AMOOR_D,
	Instruction_Kind__AMOMIN_D,
	Instruction_Kind__AMOMAX_D,
	Instruction_Kind__AMOMINU_D,
	Instruction_Kind__AMOMAXU_D,

	Instruction_Kind__COUNT,
}
Instruction_Kind;

global const char *Instruction_Kind_strings[Instruction_Kind__COUNT] =
{
	[Instruction_Kind__None] = "",

	[Instruction_Kind__LUI] = "lui",
	[Instruction_Kind__AUIPC] = "auipc",

	[Instruction_Kind__JAL] = "jal",
	[Instruction_Kind__JALR] = "jalr",

	[Instruction_Kind__BEQ] = "beq",
	[Instruction_Kind__BNE] = "bne",
	[Instruction_Kind__BLT] = "blt",
	[Instruction_Kind__BGE] = "bge",
	[Instruction_Kind__BLTU] = "bltu",
	[Instruction_Kind__BGEU] = "bgeu",

	[Instruction_Kind__LB] = "lb",
	[Instruction_Kind__LH] = "lh",
	[Instruction_Kind__LW] = "lw",
	[Instruction_Kind__LD] = "ld",
	[Instruction_Kind__LBU] = "lbu",
	[Instruction_Kind__LHU] = "lhu",
	[Instruction_Kind__LWU] = "lwu",

	[Instruction_Kind__SB] = "sb",
	[Instruction_Kind__SH] = "sh",
	[Instruction_Kind__SW] = "sw",
	[Instruction_Kind__SD] = "sd",

	[Instruction_Kind__ADDI] = "addi",
	[Instruction_Kind__SLTI] = "slti",
	[Instruction_Kind__SLTIU] = "sltiu",
	[Instruction_Kind__XORI] = "xori",
	[Instruction_Kind__ORI] = "ori",
	[Instruction_Kind__ANDI] = "andi",

	[Instruction_Kind__SLLI] = "slli",
	[Instruction_Kind__SRLI] = "srli",
	[Instruction_Kind__SRAI] = "srai",

	[Instruction_Kind__ADD] = "add",
	[Instruction_Kind__SUB] = "sub",
	[Instruction_Kind__SLL] = "sll",
	[Instruction_Kind__SLT] = "slt",
	[Instruction_Kind__SLTU] = "sltu",
	[Instruction_Kind__XOR] = "xor",
	[Instruction_Kind__SRL] = "srl",
	[Instruction_Kind__SRA] = "sra",
	[Instruction_Kind__OR] = "or",
	[Instruction_Kind__AND] = "and",

	[Instruction_Kind__ADDIW] = "addiw",
	[Instruction_Kind__SLLIW] = "slliw",
	[Instruction_Kind__SRLIW] = "srliw",
	[Instruction_Kind__SRAIW] = "sraiw",

	[Instruction_Kind__ADDW] = "addw",
	[Instruction_Kind__SUBW] = "subw",
	[Instruction_Kind__SLLW] = "sllw",
	[Instruction_Kind__SRLW] = "srlw",
	[Instruction_Kind__SRAW] = "sraw",

	[Instruction_Kind__ECALL] = "ecall",
	[Instruction_Kind__EBREAK] = "ebreak",
	[Instruction_Kind__FENCE] = "fence",
	[Instruction_Kind__FENCE_I] = "fence.i",

	[Instruction_Kind__CSRRW] = "csrrw",
	[Instruction_Kind__CSRRS] = "csrrs",
	[Instruction_Kind__CSRRC] = "csrrc",
	[Instruction_Kind__CSRRWI] = "csrrwi",
	[Instruction_Kind__CSRRSI] = "csrrsi",
	[Instruction_Kind__CSRRCI] = "csrrci",

	[Instruction_Kind__MUL] = "mul",
	[Instruction_Kind__MULH] = "mulh",
	[Instruction_Kind__MULHSU] = "mulhsu",
	[Instruction_Kind__MULHU] = "mulhu",
	[Instruction_Kind__DIV] = "div",
	[Instruction_Kind__DIVU] = "divu",
	[Instruction_Kind__REM] = "rem",
	[Instruction_Kind__REMU] = "remu",

	[Instruction_Kind__MULW] = "mulw",
	[Instruction_Kind__DIVW] = "divw",
	[Instruction_Kind__DIVUW] = "divuw",
	[Instruction_Kind__REMW] = "remw",
	[Instruction_Kind__REMUW] = "remuw",

	[Instruction_Kind__LR_W] = "lr.w",
	[Instruction_Kind__SC_W] = "sc.w",
	[Instruction_Kind__AMOSWAP_W] = "amoswap.w",
	[Instruction_Kind__AMOADD_W] = "amoadd.w",
	[Instruction_Kind__AMOXOR_W] = "amoxor.w",
	[Instruction_Kind__AMOAND_W] = "amoand.w",
	[Instruction_Kind__AMOOR_W] = "amoor.w",
	[Instruction_Kind__AMOMIN_W] = "amomin.w",
	[Instruction_Kind__AMOMAX_W] = "amomax.w",
	[Instruction_Kind__AMOMINU_W] = "amominu.w",
	[Instruction_Kind__AMOMAXU_W] = "amomaxu.w",

	[Instruction_Kind__LR_D] = "lr.d",
	[Instruction_Kind__SC_D] = "sc.d",
	[Instruction_Kind__AMOSWAP_D] = "amoswap.d",
	[Instruction_Kind__AMOADD_D] = "amoadd.d",
	[Instruction_Kind__AMOXOR_D] = "amoxor.d",
	[Instruction_Kind__AMOAND_D] = "amoand.d",
	[Instruction_Kind__AMOOR_D] = "amoor.d",
	[Instruction_Kind__AMOMIN_D] = "amomin.d",
	[Instruction_Kind__AMOMAX_D] = "amomax.d",
	[Instruction_Kind__AMOMINU_D] = "amominu.d",
	[Instruction_Kind__AMOMAXU_D] = "amomaxu.d",
};

typedef enum Pseudo_Instruction_Kind
{
	Pseudo_Instruction_Kind__None = 0,

	Pseudo_Instruction_Kind__NOP,
	Pseudo_Instruction_Kind__LI,
	Pseudo_Instruction_Kind__MV,
	Pseudo_Instruction_Kind__NOT,
	Pseudo_Instruction_Kind__NEG,
	Pseudo_Instruction_Kind__NEGW,
	Pseudo_Instruction_Kind__SEXT_W,

	Pseudo_Instruction_Kind__SEQZ,
	Pseudo_Instruction_Kind__SNEZ,
	Pseudo_Instruction_Kind__SLTZ,
	Pseudo_Instruction_Kind__SGTZ,

	Pseudo_Instruction_Kind__BEQZ,
	Pseudo_Instruction_Kind__BNEZ,
	Pseudo_Instruction_Kind__BLEZ,
	Pseudo_Instruction_Kind__BGEZ,
	Pseudo_Instruction_Kind__BLTZ,
	Pseudo_Instruction_Kind__BGTZ,

	Pseudo_Instruction_Kind__J,
	Pseudo_Instruction_Kind__JR,
	Pseudo_Instruction_Kind__RET,
	Pseudo_Instruction_Kind__CALL,
	Pseudo_Instruction_Kind__TAIL,

	Pseudo_Instruction_Kind__LA,

	Pseudo_Instruction_Kind__COUNT,
}
Pseudo_Instruction_Kind;

global const char *Pseudo_Instruction_Kind_strings[Pseudo_Instruction_Kind__COUNT] =
{
	[Pseudo_Instruction_Kind__None] = "",

	[Pseudo_Instruction_Kind__NOP]   = "nop",
	[Pseudo_Instruction_Kind__LI]    = "li",
	[Pseudo_Instruction_Kind__MV]    = "mv",
	[Pseudo_Instruction_Kind__NOT]   = "not",
	[Pseudo_Instruction_Kind__NEG]   = "neg",
	[Pseudo_Instruction_Kind__NEGW]  = "negw",
	[Pseudo_Instruction_Kind__SEXT_W]= "sext.w",

	[Pseudo_Instruction_Kind__SEQZ]  = "seqz",
	[Pseudo_Instruction_Kind__SNEZ]  = "snez",
	[Pseudo_Instruction_Kind__SLTZ]  = "sltz",
	[Pseudo_Instruction_Kind__SGTZ]  = "sgtz",

	[Pseudo_Instruction_Kind__BEQZ]  = "beqz",
	[Pseudo_Instruction_Kind__BNEZ]  = "bnez",
	[Pseudo_Instruction_Kind__BLEZ]  = "blez",
	[Pseudo_Instruction_Kind__BGEZ]  = "bgez",
	[Pseudo_Instruction_Kind__BLTZ]  = "bltz",
	[Pseudo_Instruction_Kind__BGTZ]  = "bgtz",

	[Pseudo_Instruction_Kind__J]     = "j",
	[Pseudo_Instruction_Kind__JR]    = "jr",
	[Pseudo_Instruction_Kind__RET]   = "ret",
	[Pseudo_Instruction_Kind__CALL]  = "call",
	[Pseudo_Instruction_Kind__TAIL]  = "tail",

	[Pseudo_Instruction_Kind__LA]    = "la",
};

typedef struct Register Register;
struct Register
{
    const char *name;
    U8 number; // Bad padding but that's it.
};

static const Register register_map[] =
{
	{"x0",  0},  {"x1",  1},  {"x2",  2},  {"x3",  3},
	{"x4",  4},  {"x5",  5},  {"x6",  6},  {"x7",  7},
	{"x8",  8},  {"x9",  9},  {"x10", 10}, {"x11", 11},
	{"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15},
	{"x16", 16}, {"x17", 17}, {"x18", 18}, {"x19", 19},
	{"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23},
	{"x24", 24}, {"x25", 25}, {"x26", 26}, {"x27", 27},
	{"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31},

	{"zero", 0},
	{"ra",   1},
	{"sp",   2},
	{"gp",   3},
	{"tp",   4},
	{"fp",   8},  // fp is alias for s0.

	{"t0",  5},  {"t1",  6},  {"t2",  7},
	{"t3", 28},  {"t4", 29},  {"t5", 30}, {"t6", 31},

	{"s0",  8},  {"s1",  9},
	{"s2", 18},  {"s3", 19},  {"s4", 20}, {"s5", 21},
	{"s6", 22},  {"s7", 23},  {"s8", 24}, {"s9", 25},
	{"s10", 26}, {"s11", 27},

	{"a0", 10},  {"a1", 11},  {"a2", 12}, {"a3", 13},
	{"a4", 14},  {"a5", 15},  {"a6", 16}, {"a7", 17},

	// Extra buffer to avoid faults when doing length-based checks.
	{"", 0xFF}
};

#define register_map_size (sizeof(register_map) / sizeof(register_map[0])) - 1
#define register_invalid 0xFF

// Returns 0xFF on not found.
U8
register_lookup(String8 string)
{
	U8 register_value = register_invalid;
	S8 index    = 0;
	B32 found   = 0;
	S32 result  = -1;
	for (;;)
	{
		result = os_memory_match(string.data, (unsigned char *)register_map[index].name, string.count);
		found = result == 0;
		B32 break_should = index >= (S8)register_map_size || found;
		if (break_should)
		{
			break;
		}
		index += 1;

	}
	register_value = found ? index : register_invalid;

	return register_value;
}

#endif // CONSTANTS_H

