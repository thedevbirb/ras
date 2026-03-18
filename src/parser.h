#ifndef PARSER_H
#define PARSER_H

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

