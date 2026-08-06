#ifndef RISCV_ATTRIBUTES_H
#define RISCV_ATTRIBUTES_H

// Reference: https://riscv-non-isa.github.io/riscv-elf-psabi-doc/#_list_of_attributes

typedef enum RISCV_Tag
{
	RISCV_Tag__None              =  0,

	RISCV_Tag__Stack_Alignment   =  4,
	RISCV_Tag__Architecture      =  5,
	RISCV_Tag__Unaligned_Acess   =  6,
	RISCV_Tag__Atomic_ABI        = 14,

	RISCV_Tag__X3_Register_Usage = 16,
#define RISCV_Tag__COUNT 5
}
RISCV_Tag;

typedef struct RISCV_Attribute RISCV_Attribute;
struct RISCV_Attribute
{
        RISCV_Tag tag;

        // Can be either a string or a number. If it's a string, `value_u` is the length of such string.
        // The layout is identical to a `String8`.
        U64   value_u;
        U8   *value_s;
};

typedef struct RISCV_Attributes
struct RISCV_Attributes
{
        U64 count;
        RISCV_Attributes data[RISCV_Tag__COUNT];
};

#endif // RISCV_ATTRIBUTES_H

