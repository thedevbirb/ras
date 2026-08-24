#ifndef COMPARE_OBJECTS_H
#define COMPARE_OBJECTS_H

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

// Result of merging two sorted record arrays as multisets.
typedef struct Multiset_Diff Multiset_Diff;
struct Multiset_Diff
{
        // Distinct keys present only in one multiset (or in excess), sorted.
        void *only_a;
        void *only_b;
        U64   only_a_count;
        U64   only_b_count;
        U64   common;
        U64   only_a_total;
        U64   only_b_total;
};

// Counts accumulated while printing the streamed report.
typedef struct Report_Result Report_Result;
struct Report_Result
{
        U64 identical_count;
        U64 diff_count;
        B32 text_first_has;
        U64 text_first;
};

// Compare the two objects and print the report to stdout. Returns non-zero
// when any difference is found.
internal S32 compare_objects_run(String8 path_first, String8 path_second);

#endif // COMPARE_OBJECTS_H
