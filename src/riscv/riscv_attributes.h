#ifndef RISCV_ATTRIBUTES_H
#define RISCV_ATTRIBUTES_H

// Reference: https://riscv-non-isa.github.io/riscv-elf-psabi-doc/#_list_of_attributes

typedef enum RISCV_Tag_Index
{
	RISCV_Tag_Index__Stack_Alignment,
	RISCV_Tag_Index__Architecture,
	RISCV_Tag_Index__Unaligned_Access,
	// RISCV_Tag_Index__Atomic_ABI,
	// RISCV_Tag_Index__X3_Register_Usage,
	RISCV_Tag_Index__COUNT,
}
RISCV_Tag_Index;

#define RISCV_Tag__None 0
typedef U32 RISCV_Tag;
enum
{
	RISCV_Tag__Stack_Alignment   =  4,
	RISCV_Tag__Architecture      =  5,
	RISCV_Tag__Unaligned_Access  =  6,
	// RISCV_Tag__Atomic_ABI        = 14,
	// RISCV_Tag__X3_Register_Usage = 16,
};

internal B32 RISCV_Tag__is_ntbs(RISCV_Tag tag);

typedef struct RISCV_Tag_Entry RISCV_Tag_Entry;
struct RISCV_Tag_Entry
{
        String8 tag_name;
        RISCV_Tag tag;
};

global const RISCV_Tag_Entry RISCV_Tag__table[] =
{
        { String8__literal("stack_align"),      RISCV_Tag__Stack_Alignment   },
        { String8__literal("arch"),             RISCV_Tag__Architecture      },
        { String8__literal("unaligned_access"), RISCV_Tag__Unaligned_Access  },
        // { String8__literal("atomic_abi"),       RISCV_Tag__Atomic_ABI        },
        // { String8__literal("x3_reg_usage"),     RISCV_Tag__X3_Register_Usage },
};

internal RISCV_Tag
RISCV_Tag__find(String8 tag_name)
{
        const RISCV_Tag_Entry *entry = 0;
        RISCV_Tag result = RISCV_Tag__None;
        U8 index = 0;
        for (;;)
        {
                B32 break_should = result || index >= array_count_m(RISCV_Tag__table);
                if (break_should)
                {
                        break;
                }

                entry = &RISCV_Tag__table[index];
                B32 match = String8__match_exact(tag_name, entry->tag_name);
                result = match ? entry->tag : 0;
                index += 1;
        }

        return result;
}

typedef struct RISCV_Attribute RISCV_Attribute;
struct RISCV_Attribute
{
        RISCV_Tag tag;

        // Can be either a string or a number. If it's a string, `value_u` is the length of such string.
        // The layout is identical to a `String8`.
        U64   value_u;
        U8   *value_s;
};

#define RISCV_Attributes_List__max array_count_m(RISCV_Tag__table)

typedef struct RISCV_Attributes_List RISCV_Attributes_List;
struct RISCV_Attributes_List
{
        U64 count;
        RISCV_Attribute data[RISCV_Attributes_List__max];
};

typedef struct RISCV_Attributes RISCV_Attributes;
struct RISCV_Attributes
{
        String8 architecture;
        U32     stack_alignment;
        B32     unaligned_access;
        // u8      atomic_abi_version;
        // U8      x3_register_usage;
};

internal void
RISCV_Attributes_List__add(RISCV_Attributes_List *list, RISCV_Tag tag, U64 value_u, U8 *value_s);

#endif // RISCV_ATTRIBUTES_H

