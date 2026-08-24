#ifndef COMPARE_OBJECTS_H
#define COMPARE_OBJECTS_H

// Compares two relocatable ELF objects (ELF32 or ELF64) and reports whether
// they are equivalent. The equivalence definition is described at the top of
// compare_objects.c.

// A decoded ELF section header: name plus the fields the comparison uses.
typedef struct Object_Section Object_Section;
struct Object_Section
{
        String8 name;
        U32     type;
        U64     flags;
        U64     offset;
        U64     size;
        U32     link;
        U32     info;
        U64     alignment;
        U64     entry_size;
};

// A decoded `.symtab` entry, with its section index canonicalized to a name.
typedef struct Object_Symbol Object_Symbol;
struct Object_Symbol
{
        String8 name;
        String8 section_name;
        U8      type;
        U8      bind;
        U8      visibility;
        U64     value;
        U64     size;
};

// A `.rela` entry, compared by the referenced symbol's *fields*: two symbols
// are the same when every field matches, regardless of their indices.
typedef struct Relocation_Item Relocation_Item;
struct Relocation_Item
{
        U64 offset;
        U32 type;
        S64 addend;
        // Referenced symbol; `{0}` when the index is out of range.
        Object_Symbol symbol;
};

// A parsed ELF object. `sections` are in file order; all `String8` members
// point into `data`.
typedef struct Object_File Object_File;
struct Object_File
{
        U8             *data;
        U64             data_count;
        U8              class;
        Object_Section *sections;
        U64             sections_count;
};

// Compare the two objects and print the report to stdout. Returns non-zero
// when any difference is found.
internal S32 compare_objects_run(String8 path_first, String8 path_second);

#endif // COMPARE_OBJECTS_H