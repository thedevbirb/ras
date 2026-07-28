#ifndef CORE_SECTION_H
#define CORE_SECTION_H

global const String8 section_name_text      = String8__literal(".text");
global const String8 section_name_data      = String8__literal(".data");
global const String8 section_name_bss       = String8__literal(".bss");
global const String8 section_name_undefined = String8__literal("*UNDEFINED*");
global const String8 section_name_absolute  = String8__literal("*ABSOLUTE*");
global const String8 section_name_common    = String8__literal("*COMMON*");

// Forward declaration for pointer use.
typedef struct Expression Expression;

// A data structure modelling an object file section, in memory.
typedef struct Section Section;
struct Section
{
        Section             *previous;
        Section             *next;

        Fragments            fragments;
        String8              name;
        // TODO(low): probably useless.
        U32                  location;

        // Output fields.

        // This can be reliably set only close to object file writing, unless the section is either undefined, absolute,
        // or common. As such, use this for section comparison only against those. Otherwise, compare section pointers.
        U32                  index;
        ELF64_Section_Header elf;
};

typedef struct Sections_Trie Sections_Trie;
struct Sections_Trie
{
        Section         section;
        Sections_Trie  *children[4];
};

typedef struct Sections_Table Sections_Table;
struct Sections_Table
{
        Arena                    *arena;
        Sections_Trie            *root;

        // The underlying doubly-linked list collection. It contains only non-virtual sections, so it excludes the
        // undefined, absolute and common section.
        Section                  *first;
        Section                  *last;

        Section                  *current;
        Section                  *undefined;
        Section                  *absolute;
        // TODO(common): unsupported;
        Section                  *common;
        U32                       count;
};

internal Sections_Table *
Sections_Table__default(void);

internal Section *
Sections_Table__get(Sections_Table *sections_table, String8 name);

internal Section *
Sections_Table__get_or_default(Sections_Table *sections_table, String8 name, U32 location);

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
