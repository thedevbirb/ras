#ifndef PARSER_H
#define PARSER_H

// // 7 Bit representation of opcodes
// #define OP_RALU  = 0b0110011
// #define OP_IALU  = 0b0010011
// #define OP_LOAD  = 0b0000011
// #define OP_STORE = 0b0100011
// // TODO: keep filling
//
// // 16-bit register bitfield containing 0, rs2, rs1, rd.
// typedef Register_Bitfield U16;
// // 16-bit function bitfield containing 000000, funct7, funct3
// typedef Function_Bitfield U16;
//
// typedef Register U8;
//
// #define Register_Bitfield_rs2_mask 0b0111110000000000
// #define Register_Bitfield_rs1_mask 0b0000001111100000
// #define Register_Bitfield_rd_mask  0b0000000000011111
//
// internal Register
// Register_Bitfield_rs2(Register_Bitfield rb)
// {
// 	Register result = rb & Register_Bitfield_rs2_mask;
// 	return result;
// }
//
// internal Register
// Register_Bitfield_rs1(Register_Bitfield rb)
// {
// 	Register result = rb & Register_Bitfield_rs1_mask;
// 	return result;
// }
//
// internal Register
// Register_Bitfield_rd(Register_Bitfield rb)
// {
// 	Register result = rb & Register_Bitfield_rd_mask;
// 	return result;
// }

// typedef enum Element_Kind
// {
//
// }
// Element_Kind;
//
// typedef enum Opcode Opcode
// {
// 	Opcode__Add,
// 	Opcode__Sub,
// 	// etc.
// 	Opcode__COUNT,
// }
// Opcode;
//
// typedef enum Directive_Kind Directive_Kind
// {
//     Directive_Kind__text,
//     Directive_Kind__data,
//     Directive_Kind__globl,
//     Directive_Kind__word,
//     Directive_Kind__ascii,
//     Directive_Kind__asciz,
//     Directive_Kind__COUNT,
// }
//
// // hard!
//
// typedef struct Directive Directive
// {
// 	Directive_Kind kind;
// 	union
// 	{
// 		String8 label; // 16 bytes;
// 		S32 value;
// 		U32 value2;
// 	};
// }
//
// typedef struct Instruction Instruction;
// struct Instruction
// {
// 	Opcode opcode,
// 	U8     register_source_2;
// 	U8     register_source_1;
// 	U8     register_destination;
// 	S32    immediate;
// };
//
// typedef Label Label;
// struct Label
// {
//
// };
//
// typedef struct Metadata Metadata;
// struct Metadata
// {
// 	U32 row_index;
// 	U32 column_index;
// 	U32 size; // byte size of the string representation.
// };
//
// typedef struct Element Element;
// struct Element
// {
// 	Element_Kind kind;
// 	union
// 	{
// 		Instruction instruction;
// 		String8	    label;
//
//
// 	};
// 	U32 token_index;
// };

#endif // PARSER_H

