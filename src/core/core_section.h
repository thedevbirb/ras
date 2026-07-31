#ifndef CORE_SECTION_H
#define CORE_SECTION_H

global String8 section_name_text      = String8__literal(".text");
global String8 section_name_data      = String8__literal(".data");
global String8 section_name_bss       = String8__literal(".bss");
global String8 section_name_undefined = String8__literal("");
global String8 section_name_absolute  = String8__literal("*ABSOLUTE*");
global String8 section_name_common    = String8__literal("*COMMON*");

// Forward declaration for pointer use.
typedef struct Expression Expression;
typedef struct Symbol_Ref Symbol_Ref;

// A data structure modelling an object file section, in memory.
typedef struct Section Section;
struct Section
{
        Section             *previous;
        Section             *next;
        Symbol_Ref          *symbol;
        Fixups               fixups;
        Fragments            fragments;

        // https://gabi.xinuos.com/v42/elf/03-sheader.html#special-sections
        B32 special;

        // Output fields.

        // This can be reliably set only close to object file writing, unless the section is either  absolute, or
        // common. As such, use this for section comparison only against those. Otherwise, compare section pointers.
        U32                  index;
        ELF64_Section_Header elf;
};

internal void
Section__add_jump_instruction
(
        Section           *section,
        U32                encoding,
        U8                 encoding_size,
        U32                location,
        U8                 worst_case_size,
        U8                 best_case_size,
        Expression        *expression,
        U8                 jump_instructions_size
);

internal void
Section__add_instruction_fixed
(
        Section *section,
        Fixup   *fixup,

        U32      encoding,
        U8       encoding_size,
        U32      location
);

internal void
Section__finish(Section *section);

#endif // CORE_SECTION_H
