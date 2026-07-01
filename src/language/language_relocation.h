#ifndef LANGUAGE_RELOCATION_H
#define LANGUAGE_RELOCATION_H

typedef struct Relocation_Operator Relocation_Operator;
struct Relocation_Operator
{
	String8 text;
	Relocation_RISC_V relocation;
};

#endif // LANGUAGE_RELOCATION_H
