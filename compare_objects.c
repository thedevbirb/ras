// Compares two relocatable ELF objects and reports whether they are
// equivalent.
//
// Two objects are equivalent when every section present in both objects has:
//   - matching header fields type, flags, size, link, info, alignment and
//     entry size;
//   - matching contents, byte for byte (NOBITS sections and `.note.GNU-stack`
//     are skipped);
//   - matching `.rela.*` contents, as a set of (offset, type, addend,
//     referenced-symbol) tuples. Two symbols are the same when every field
//     matches, so the comparison is insensitive to symbol indices;
//   - matching `.symtab` contents, as a multiset of
//     (name, type, binding, visibility, section-name, value, size). Order is
//     ignored, and section indices are canonicalized to section names so that
//     section ordering does not matter either.
//
// A section present in only one of the two objects is reported as a
// difference. The report is one line per section, then a summary of the
// differences. Both ELF32 and ELF64 objects are supported. The exit status is
// non-zero when any difference is found.
//
// Usage: compare_objects <object_a> <object_b>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "src/base/base_include.h"
#include "src/object/object_elf.h"

#include "compare_objects.h"

// .c files

#include "src/base/base_include.c"

//------------------------------------------------------------------------------
// ELF decoding
//------------------------------------------------------------------------------

// Resolve the NUL-terminated string in `table` starting at `offset`, clamped
// to `table_size`.
internal String8
string_from_table(U8 *data, U64 table_offset, U64 table_size, U64 offset)
{
        U64 end = offset;
        for (;;)
        {
                B32 break_should = end >= table_size || data[table_offset + end] == 0;
                if (break_should)
                {
                        break;
                }
                end += 1;
        }

        String8 result = String8__new(data + table_offset + offset, end - offset);
        return result;
}

// Read one section header entry (ELF32 or ELF64) at `offset`, resolving its
// name through the `.shstrtab` range.
internal Object_Section
section_header_read(U8 *data, U64 offset, U64 string_table_offset, U64 string_table_size, U8 class)
{
        Object_Section result = {0};
        if (class == ELF_ID_Class__32)
        {
                ELF32_Section_Header header = {0};
                String8 cursor = String8__new((U8 *)&header, sizeof(header));
                String8__serial_write_m(&cursor, (ELF32_Section_Header *)(data + offset));
                result.name       = string_from_table(data, string_table_offset, string_table_size, header.string_table_offset);
                result.type       = header.type;
                result.flags      = header.flags;
                result.offset     = header.offset;
                result.size       = header.size;
                result.link       = header.link;
                result.info       = header.info;
                result.alignment  = header.alignment;
                result.entry_size = header.entry_size;
        }
        else
        {
                ELF64_Section_Header header = {0};
                String8 cursor = String8__new((U8 *)&header, sizeof(header));
                String8__serial_write_m(&cursor, (ELF64_Section_Header *)(data + offset));
                result.name       = string_from_table(data, string_table_offset, string_table_size, header.string_table_offset);
                result.type       = header.type;
                result.flags      = header.flags;
                result.offset     = header.offset;
                result.size       = header.size;
                result.link       = header.link;
                result.info       = header.info;
                result.alignment  = header.alignment;
                result.entry_size = header.entry_size;
        }
        return result;
}

// Canonicalize a symbol's section index to a section *name*, mapping the
// reserved special indices to their conventional labels.
internal String8
symbol_section_name(Object_File *file, U16 section_index, Arena *arena)
{
        String8 result = {0};
        if (section_index == ELF_Section_Index__Undefined)
        {
                result = String8__literal("UNDEF");
        }
        else if (section_index == ELF_Section_Index__Absolute)
        {
                result = String8__literal("ABS");
        }
        else if (section_index == ELF_Section_Index__Common)
        {
                result = String8__literal("COMMON");
        }
        else if (section_index == ELF_Section_Index__XINDEX)
        {
                result = String8__literal("XINDEX");
        }
        else if (section_index < file->sections_count)
        {
                result = file->sections[section_index].name;
        }
        else
        {
                result = String8__format(arena, "shndx?%u", section_index);
        }

        return result;
}

// Read one symbol table entry (ELF32 or ELF64), resolving its name and
// canonicalizing its section index.
internal Object_Symbol
symbol_read(U8 *data, U64 offset, U8 class, U64 string_table_offset, U64 string_table_size, Object_File *file, Arena *arena)
{
        Object_Symbol result = {0};
        U32 name_offset = 0;
        U16 section_index = 0;
        if (class == ELF_ID_Class__32)
        {
                ELF32_Symbol symbol = {0};
                String8 cursor = String8__new((U8 *)&symbol, sizeof(symbol));
                String8__serial_write_m(&cursor, (ELF32_Symbol *)(data + offset));
                result.type       = ELF_Symbol_type_m(symbol.type_and_binding);
                result.bind       = ELF_Symbol_bind_m(symbol.type_and_binding);
                result.visibility = symbol.visibility;
                result.value      = symbol.value;
                result.size       = symbol.size;
                name_offset       = symbol.string_table_offset;
                section_index     = symbol.section_index;
        }
        else
        {
                ELF64_Symbol symbol = {0};
                String8 cursor = String8__new((U8 *)&symbol, sizeof(symbol));
                String8__serial_write_m(&cursor, (ELF64_Symbol *)(data + offset));
                result.type       = ELF_Symbol_type_m(symbol.type_and_binding);
                result.bind       = ELF_Symbol_bind_m(symbol.type_and_binding);
                result.visibility = symbol.visibility;
                result.value      = symbol.value;
                result.size       = symbol.size;
                name_offset       = symbol.string_table_offset;
                section_index     = symbol.section_index;
        }
        result.name = string_from_table(data, string_table_offset, string_table_size, name_offset);
        result.section_name = symbol_section_name(file, section_index, arena);
        return result;
}

// Read one `.rela` entry (ELF32 or ELF64), embedding the referenced symbol by
// its fields so the comparison is insensitive to symbol indices.
internal Relocation_Item
relocation_read(U8 *data, U64 offset, U8 class, Object_Symbol *symbols, U64 symbols_count)
{
        Relocation_Item result = {0};
        U64 symbol_index = 0;
        if (class == ELF_ID_Class__32)
        {
                ELF32_Relocation_Addend relocation = {0};
                String8 cursor = String8__new((U8 *)&relocation, sizeof(relocation));
                String8__serial_write_m(&cursor, (ELF32_Relocation_Addend *)(data + offset));
                result.offset = relocation.offset;
                result.addend = relocation.addend;
                result.type   = relocation.info & 0xff;
                symbol_index  = relocation.info >> 8;
        }
        else
        {
                ELF64_Relocation_Addend relocation = {0};
                String8 cursor = String8__new((U8 *)&relocation, sizeof(relocation));
                String8__serial_write_m(&cursor, (ELF64_Relocation_Addend *)(data + offset));
                result.offset = relocation.offset;
                result.addend = relocation.addend;
                result.type   = (U32)(relocation.info & 0xffffffff);
                symbol_index  = relocation.info >> 32;
        }
        if (symbol_index < symbols_count)
        {
                result.symbol = symbols[symbol_index];
        }
        return result;
}

internal String8
object_file_read(String8 path, Arena *arena, Object_File *result)
{
        String8 error = {0};
        U8 *data = 0;
        U64 data_count = 0;
        U8 class = 0;

        S32 file_descriptor = open((char *)path.data, O_RDONLY);
        if (file_descriptor >= 0)
        {
                struct stat statistics;
                assert_always_m(fstat(file_descriptor, &statistics) == 0 && "failed to fstat input file");
                data_count = (U64)statistics.st_size;
                data = mmap(NULL, data_count + 8, PROT_READ, MAP_PRIVATE, file_descriptor, 0);
                close(file_descriptor);
        }

        // Validate the ELF header one step at a time. Each condition only
        // depends on the checks before it, so the `else if` chain runs every
        // step at most once and the parse runs only when all of them passed.
        B32 opened    = file_descriptor >= 0;
        B32 mapped    = opened && data != MAP_FAILED;
        B32 elf_sized = mapped && data_count >= 16;
        class = elf_sized ? data[4] : 0;
        B32 magic_is  = elf_sized
                     && data[0] == ELF_ID_Magic__0
                     && data[1] == ELF_ID_Magic__1
                     && data[2] == ELF_ID_Magic__2
                     && data[3] == ELF_ID_Magic__3;
        B32 class_is  = magic_is && (class == ELF_ID_Class__32 || class == ELF_ID_Class__64);
        B32 header_ok = class_is && data_count >= (class == ELF_ID_Class__32 ? (U64)sizeof(ELF32_Header) : (U64)sizeof(ELF64_Header));

        if (!opened)
        {
                error = String8__literal("could not open file");
        }
        else if (!mapped)
        {
                error = String8__literal("could not memory-map file");
        }
        else if (!elf_sized)
        {
                error = String8__literal("not an ELF");
        }
        else if (!magic_is)
        {
                error = String8__literal("not an ELF");
        }
        else if (!class_is)
        {
                error = String8__literal("unsupported ELF class");
        }
        else if (!header_ok)
        {
                error = String8__literal("not an ELF");
        }
        else
        {
                U64 section_headers_offset = 0;
                U64 section_headers_size   = 0;
                U64 sections_count         = 0;
                U64 string_table_index     = 0;
                if (class == ELF_ID_Class__32)
                {
                        ELF32_Header header = {0};
                        String8 cursor = String8__new((U8 *)&header, sizeof(header));
                        String8__serial_write_m(&cursor, (ELF32_Header *)data);
                        section_headers_offset = header.section_header_table_file_offset;
                        section_headers_size   = header.section_header_table_entry_size;
                        sections_count         = header.section_header_table_entry_count;
                        string_table_index     = header.section_header_string_table_index;
                }
                else
                {
                        ELF64_Header header = {0};
                        String8 cursor = String8__new((U8 *)&header, sizeof(header));
                        String8__serial_write_m(&cursor, (ELF64_Header *)data);
                        section_headers_offset = header.section_header_table_file_offset;
                        section_headers_size   = header.section_header_table_entry_size;
                        sections_count         = header.section_header_table_entry_count;
                        string_table_index     = header.section_header_string_table_index;
                }

                // The `.shstrtab` entry locates the string table that names every section.
                Object_Section string_table_section = section_header_read(data, section_headers_offset + string_table_index * section_headers_size, 0, 0, class);
                U64 string_table_offset = string_table_section.offset;
                U64 string_table_size   = string_table_section.size;

                Object_Section *sections = Arena__push_array_m(arena, Object_Section, sections_count);
                U64 index = 0;
                for (;;)
                {
                        if (index >= sections_count)
                        {
                                break;
                        }
                        sections[index] = section_header_read(data, section_headers_offset + index * section_headers_size, string_table_offset, string_table_size, class);
                        index += 1;
                }

                result->data           = data;
                result->data_count     = data_count;
                result->class          = class;
                result->sections       = sections;
                result->sections_count = sections_count;
        }

        return error;
}

internal Object_Section *
object_file_section(Object_File *file, String8 name)
{
        Object_Section *result = 0;
        U64 index = 0;
        for (;;)
        {
                if (index >= file->sections_count)
                {
                        break;
                }
                Object_Section *section = &file->sections[index];
                if (String8__match_exact(section->name, name))
                {
                        result = section;
                }
                index += 1;
        }

        return result;
}

internal Object_Symbol *
object_file_symbols(Object_File *file, Arena *arena, U64 *count_out)
{
        Object_Section *symtab = object_file_section(file, String8__literal(".symtab"));
        Object_Section *strtab = object_file_section(file, String8__literal(".strtab"));

        U64 count = 0;
        if (symtab && strtab && symtab->entry_size != 0)
        {
                count = symtab->size / symtab->entry_size;
        }

        Object_Symbol *result = Arena__push_array_m(arena, Object_Symbol, count);
        U64 index = 0;
        for (;;)
        {
                if (index >= count)
                {
                        break;
                }
                result[index] = symbol_read(file->data, symtab->offset + index * symtab->entry_size, file->class, strtab->offset, strtab->size, file, arena);
                index += 1;
        }

        *count_out = count;
        return result;
}

//------------------------------------------------------------------------------
// Sorting and multiset machinery
//------------------------------------------------------------------------------

// Total order over the (name, type, bind, visibility, section, value, size)
// tuple. The `const void *` signature is the one `qsort` requires.
internal S32
symbol_compare(const void *a, const void *b)
{
        Object_Symbol const *sa = a;
        Object_Symbol const *sb = b;
        S32 result = String8__compare(sa->name, sb->name);
        if (result == 0)
        {
                result = (S32)sa->type - (S32)sb->type;
        }
        if (result == 0)
        {
                result = (S32)sa->bind - (S32)sb->bind;
        }
        if (result == 0)
        {
                result = (S32)sa->visibility - (S32)sb->visibility;
        }
        if (result == 0)
        {
                result = String8__compare(sa->section_name, sb->section_name);
        }
        if (result == 0)
        {
                result = (S32)(sa->value > sb->value) - (S32)(sa->value < sb->value);
        }
        if (result == 0)
        {
                result = (S32)(sa->size > sb->size) - (S32)(sa->size < sb->size);
        }
        return result;
}

internal S32
relocation_item_compare(const void *a, const void *b)
{
        Relocation_Item const *ra = a;
        Relocation_Item const *rb = b;
        S32 result = (S32)(ra->offset > rb->offset) - (S32)(ra->offset < rb->offset);
        if (result == 0)
        {
                result = (S32)(ra->type > rb->type) - (S32)(ra->type < rb->type);
        }
        if (result == 0)
        {
                result = (S32)(ra->addend > rb->addend) - (S32)(ra->addend < rb->addend);
        }
        if (result == 0)
        {
                result = symbol_compare(&ra->symbol, &rb->symbol);
        }
        return result;
}

// Number of entries equal to `symbols[start]` at the head of the run.
internal U64
symbol_runs_count(Object_Symbol *symbols, U64 count, U64 start)
{
        U64 run = 0;
        U64 index = start;
        for (;;)
        {
                B32 break_should = index >= count || symbol_compare(&symbols[index], &symbols[start]) != 0;
                if (break_should)
                {
                        break;
                }
                run += 1;
                index += 1;
        }
        return run;
}

// Build the sorted, de-duplicated set of relocation tuples for `section`,
// resolving each referenced symbol's *value* through `symbols`.
internal U64
relocation_items_build(Object_File *file, Object_Section *section, Object_Symbol *symbols, U64 symbols_count, Arena *arena, Relocation_Item **items_out)
{
        U64 entry_count = section->entry_size != 0 ? section->size / section->entry_size : 0;
        Relocation_Item *items = Arena__push_array_m(arena, Relocation_Item, entry_count);

        U64 index = 0;
        for (;;)
        {
                if (index >= entry_count)
                {
                        break;
                }
                items[index] = relocation_read(file->data, section->offset + index * section->entry_size, file->class, symbols, symbols_count);
                index += 1;
        }

        qsort(items, entry_count, sizeof(Relocation_Item), relocation_item_compare);

        // Collapse duplicates, since the comparison unit is a set.
        U64 unique_count = 0;
        index = 0;
        for (;;)
        {
                if (index >= entry_count)
                {
                        break;
                }
                B32 duplicate_is = unique_count > 0 && relocation_item_compare(&items[unique_count - 1], &items[index]) == 0;
                if (!duplicate_is)
                {
                        items[unique_count] = items[index];
                        unique_count += 1;
                }
                index += 1;
        }

        *items_out = items;
        return unique_count;
}

// Set difference of two sorted, de-duplicated relocation arrays. `sample` is
// the first gnu-only element if any, else the first ras-only element.
internal void
relocation_sets_difference
(
        Relocation_Item *gnu,
        U64              gnu_count,
        Relocation_Item *ras,
        U64              ras_count,
        U64             *common_out,
        U64             *gnu_only_out,
        U64             *ras_only_out,
        Relocation_Item *sample_out
)
{
        U64 gnu_index = 0;
        U64 ras_index = 0;
        U64 common    = 0;
        U64 gnu_only  = 0;
        U64 ras_only  = 0;
        Relocation_Item sample = {0};
        B32 sample_has = 0;

        for (;;)
        {
                B32 break_should = gnu_index >= gnu_count || ras_index >= ras_count;
                if (break_should)
                {
                        break;
                }

                S32 order = relocation_item_compare(&gnu[gnu_index], &ras[ras_index]);
                if (order < 0)
                {
                        if (!sample_has)
                        {
                                sample     = gnu[gnu_index];
                                sample_has = 1;
                        }
                        gnu_only += 1;
                        gnu_index += 1;
                }
                else if (order > 0)
                {
                        if (!sample_has)
                        {
                                sample     = ras[ras_index];
                                sample_has = 1;
                        }
                        ras_only += 1;
                        ras_index += 1;
                }
                else
                {
                        common += 1;
                        gnu_index += 1;
                        ras_index += 1;
                }
        }

        for (;;)
        {
                if (gnu_index >= gnu_count)
                {
                        break;
                }
                if (!sample_has)
                {
                        sample     = gnu[gnu_index];
                        sample_has = 1;
                }
                gnu_only += 1;
                gnu_index += 1;
        }

        for (;;)
        {
                if (ras_index >= ras_count)
                {
                        break;
                }
                if (!sample_has)
                {
                        sample     = ras[ras_index];
                        sample_has = 1;
                }
                ras_only += 1;
                ras_index += 1;
        }

        *common_out   = common;
        *gnu_only_out = gnu_only;
        *ras_only_out = ras_only;
        *sample_out   = sample;
        return;
}

//------------------------------------------------------------------------------
// Output helpers
//------------------------------------------------------------------------------

// Print `value` as minimal lowercase hex, with `0x0` for zero (C's `%#llx`
// would print `0`).
internal void
hex_print(FILE *out, U64 value)
{
        if (value == 0)
        {
                fprintf(out, "0x0");
        }
        else
        {
                fprintf(out, "%#llx", value);
        }
        return;
}

// Format `value` the way `hex_print` does, into arena memory.
internal String8
hex_string(Arena *arena, U64 value)
{
        String8 result;
        if (value == 0)
        {
                result = String8__literal("0x0");
        }
        else
        {
                result = String8__format(arena, "%#llx", value);
        }
        return result;
}

// First index in `[0, count)` where the byte arrays differ, else
// `default_value`.
internal U64
bytes_first_difference(U8 *a, U8 *b, U64 count, U64 default_value)
{
        U64 result = default_value;
        B32 found  = 0;
        U64 index  = 0;
        for (;;)
        {
                B32 break_should = found || index >= count;
                if (break_should)
                {
                        break;
                }
                if (a[index] != b[index])
                {
                        result = index;
                        found  = 1;
                }
                index += 1;
        }
        return result;
}

// Print `label gnu=0x.. ras=0x..` for a differing section header field,
// separated from previously printed fields by `; `.
internal void
section_field_diff_print(FILE *out, String8 label, U64 gnu_value, U64 ras_value, B32 *first_diff)
{
        if (gnu_value != ras_value)
        {
                if (*first_diff)
                {
                        *first_diff = 0;
                }
                else
                {
                        fprintf(out, "; ");
                }
                fprintf(out, "%.*s gnu=", String8__varg(label));
                hex_print(out, gnu_value);
                fprintf(out, " ras=");
                hex_print(out, ras_value);
        }
        return;
}

//------------------------------------------------------------------------------
// Reports
//------------------------------------------------------------------------------

internal void
sections_headers_compare(Object_File *ras, Object_File *gnu, Arena *arena, String8_Node_List *diffs)
{
        fprintf(stdout, "=== SECTION HEADERS ===\n");

        U64 index = 0;
        for (;;)
        {
                if (index >= gnu->sections_count)
                {
                        break;
                }

                Object_Section *g = &gnu->sections[index];
                if (g->name.count != 0)
                {
                        Object_Section *r = object_file_section(ras, g->name);
                        if (r == 0)
                        {
                                fprintf(stdout, "  MISSING  %.*s\n", String8__varg(g->name));
                                String8_Node *node = Arena__push_struct_m(arena, String8_Node);
                                node->string = String8__format(arena, "%.*s: missing in ras", String8__varg(g->name));
                                String8_Node_List__push(diffs, node);
                        }
                        else
                        {
                                // Compare every field except name and offset;
                                // those are expected to differ.
                                B32 type_diff      = g->type        != r->type;
                                B32 flags_diff     = g->flags       != r->flags;
                                B32 size_diff      = g->size        != r->size;
                                B32 link_diff      = g->link        != r->link;
                                B32 info_diff      = g->info        != r->info;
                                B32 alignment_diff = g->alignment   != r->alignment;
                                B32 entry_diff     = g->entry_size  != r->entry_size;
                                B32 any_diff       = type_diff || flags_diff || size_diff || link_diff || info_diff || alignment_diff || entry_diff;

                                fprintf(stdout, "  %s  %.*s", any_diff ? "DIFF" : "OK  ", String8__varg(g->name));
                                if (any_diff)
                                {
                                        fprintf(stdout, "  [");
                                        B32 first_diff = 1;
                                        section_field_diff_print(stdout, String8__literal("type"),      g->type,       r->type,       &first_diff);
                                        section_field_diff_print(stdout, String8__literal("flags"),     g->flags,      r->flags,      &first_diff);
                                        section_field_diff_print(stdout, String8__literal("size"),      g->size,       r->size,       &first_diff);
                                        section_field_diff_print(stdout, String8__literal("link"),      g->link,       r->link,       &first_diff);
                                        section_field_diff_print(stdout, String8__literal("info"),      g->info,       r->info,       &first_diff);
                                        section_field_diff_print(stdout, String8__literal("addralign"), g->alignment,  r->alignment,  &first_diff);
                                        section_field_diff_print(stdout, String8__literal("entsize"),   g->entry_size, r->entry_size, &first_diff);
                                        fprintf(stdout, "]");
                                }
                                fprintf(stdout, "\n");
                        }
                }

                index += 1;
        }
        return;
}

internal U64
sections_content_compare(Object_File *ras, Object_File *gnu, Arena *arena, String8_Node_List *diffs)
{
        fprintf(stdout, "=== CONTENT ===\n");

        U64 identical_count = 0;
        U64 index = 0;
        for (;;)
        {
                if (index >= gnu->sections_count)
                {
                        break;
                }

                Object_Section *g = &gnu->sections[index];
                if (g->name.count != 0)
                {
                        Object_Section *r = object_file_section(ras, g->name);
                        B32 skip = g->type == ELF_Section_Header_Type__No_Data
                                || String8__match_exact(g->name, String8__literal(".note.GNU-stack"));
                        if (r && !skip)
                        {
                                U8 *gnu_bytes = gnu->data + g->offset;
                                U8 *ras_bytes = ras->data + r->offset;
                                B32 equal = g->size == r->size && memory_match(gnu_bytes, ras_bytes, g->size) == 0;
                                if (equal)
                                {
                                        fprintf(stdout, "  OK    %.*s (%lluB)\n", String8__varg(g->name), g->size);
                                        identical_count += 1;
                                }
                                else
                                {
                                        U64 compare_count = min_m(g->size, r->size);
                                        U64 first = bytes_first_difference(gnu_bytes, ras_bytes, compare_count, compare_count);
                                        fprintf(stdout, "  DIFF  %.*s gnu=%lluB ras=%lluB first=",
                                                String8__varg(g->name), g->size, r->size);
                                        hex_print(stdout, first);
                                        fprintf(stdout, "\n");
                                        String8_Node *node = Arena__push_struct_m(arena, String8_Node);
                                        node->string = String8__format(arena, "%.*s: content (first byte %.*s, gnu %lluB ras %lluB)",
                                                String8__varg(g->name), String8__varg(hex_string(arena, first)),
                                                g->size, r->size);
                                        String8_Node_List__push(diffs, node);
                                }
                        }
                }

                index += 1;
        }

        // Sections present only in ras.
        index = 0;
        for (;;)
        {
                if (index >= ras->sections_count)
                {
                        break;
                }
                Object_Section *r = &ras->sections[index];
                if (r->name.count != 0)
                {
                        B32 in_gnu = object_file_section(gnu, r->name) != 0;
                        if (!in_gnu)
                        {
                                String8_Node *node = Arena__push_struct_m(arena, String8_Node);
                                node->string = String8__format(arena, "%.*s: extra in ras", String8__varg(r->name));
                                String8_Node_List__push(diffs, node);
                        }
                }
                index += 1;
        }

        return identical_count;
}

internal void
relocations_compare
(
        Object_File      *ras,
        Object_File      *gnu,
        Object_Symbol    *ras_symbols,
        U64               ras_symbols_count,
        Object_Symbol    *gnu_symbols,
        U64               gnu_symbols_count,
        Arena            *arena,
        String8_Node_List *diffs
)
{
        fprintf(stdout, "=== RELOCATIONS (content, by offset/type/addend/symbol-value) ===\n");

        U64 index = 0;
        for (;;)
        {
                if (index >= gnu->sections_count)
                {
                        break;
                }

                Object_Section *g = &gnu->sections[index];
                B32 is_rela = String8__match_prefix(g->name, String8__literal(".rela"));
                if (is_rela)
                {
                        Object_Section *r = object_file_section(ras, g->name);
                        if (r)
                        {
                                Relocation_Item *gnu_items = 0;
                                U64 gnu_items_count = relocation_items_build(gnu, g, gnu_symbols, gnu_symbols_count, arena, &gnu_items);
                                Relocation_Item *ras_items = 0;
                                U64 ras_items_count = relocation_items_build(ras, r, ras_symbols, ras_symbols_count, arena, &ras_items);

                                U64 common    = 0;
                                U64 gnu_only  = 0;
                                U64 ras_only  = 0;
                                Relocation_Item sample = {0};
                                relocation_sets_difference(gnu_items, gnu_items_count, ras_items, ras_items_count, &common, &gnu_only, &ras_only, &sample);

                                B32 any_diff = gnu_only != 0 || ras_only != 0;
                                fprintf(stdout, "  %s %.*s: GNU=%llu ras=%llu common=%llu gnu-only=%llu ras-only=%llu\n",
                                        any_diff ? "DIFF" : "OK  ", String8__varg(g->name),
                                        gnu_items_count, ras_items_count, common, gnu_only, ras_only);
                                if (any_diff)
                                {
                                        String8_Node *node_diff = Arena__push_struct_m(arena, String8_Node);
                                        node_diff->string = String8__format(arena, "%.*s: reloc content differs (sample (%llu, %u, %lld, '%.*s'))",
                                                String8__varg(g->name),
                                                sample.offset, sample.type, sample.addend,
                                                String8__varg(sample.symbol.name));
                                        String8_Node_List__push(diffs, node_diff);
                                }
                        }
                }

                index += 1;
        }
        return;
}

internal void
symbols_compare(Object_File *ras, Object_File *gnu, Arena *arena, String8_Node_List *diffs)
{
        fprintf(stdout, "=== SYMBOL TABLE (contents as multiset, order ignored) ===\n");

        U64 gnu_count = 0;
        U64 ras_count = 0;
        Object_Symbol *gnu_symbols = object_file_symbols(gnu, arena, &gnu_count);
        Object_Symbol *ras_symbols = object_file_symbols(ras, arena, &ras_count);

        // Sorted copies, since the value-by-index lookup for relocations still
        // needs the original file order.
        Object_Symbol *gnu_sorted = Arena__push_array_m(arena, Object_Symbol, gnu_count);
        Object_Symbol *ras_sorted = Arena__push_array_m(arena, Object_Symbol, ras_count);
        memory_copy(gnu_sorted, gnu_symbols, gnu_count * sizeof(Object_Symbol));
        memory_copy(ras_sorted, ras_symbols, ras_count * sizeof(Object_Symbol));
        qsort(gnu_sorted, gnu_count, sizeof(Object_Symbol), symbol_compare);
        qsort(ras_sorted, ras_count, sizeof(Object_Symbol), symbol_compare);

        // Merge both sorted arrays, counting the symbols present in both files
        // and collecting the distinct keys present in only one.
        Object_Symbol *gnu_only = Arena__push_array_m(arena, Object_Symbol, gnu_count + ras_count);
        Object_Symbol *ras_only = Arena__push_array_m(arena, Object_Symbol, gnu_count + ras_count);
        U64 common         = 0;
        U64 gnu_only_total = 0;
        U64 ras_only_total = 0;
        U64 gnu_only_count = 0;
        U64 ras_only_count = 0;
        U64 gnu_index = 0;
        U64 ras_index = 0;
        for (;;)
        {
                B32 break_should = gnu_index >= gnu_count || ras_index >= ras_count;
                if (break_should)
                {
                        break;
                }

                S32 order = symbol_compare(&gnu_sorted[gnu_index], &ras_sorted[ras_index]);
                if (order < 0)
                {
                        U64 run = symbol_runs_count(gnu_sorted, gnu_count, gnu_index);
                        gnu_only[gnu_only_count] = gnu_sorted[gnu_index];
                        gnu_only_count += 1;
                        gnu_only_total += run;
                        gnu_index += run;
                }
                else if (order > 0)
                {
                        U64 run = symbol_runs_count(ras_sorted, ras_count, ras_index);
                        ras_only[ras_only_count] = ras_sorted[ras_index];
                        ras_only_count += 1;
                        ras_only_total += run;
                        ras_index += run;
                }
                else
                {
                        U64 gnu_run = symbol_runs_count(gnu_sorted, gnu_count, gnu_index);
                        U64 ras_run = symbol_runs_count(ras_sorted, ras_count, ras_index);
                        common += min_m(gnu_run, ras_run);
                        if (gnu_run > ras_run)
                        {
                                gnu_only[gnu_only_count] = gnu_sorted[gnu_index];
                                gnu_only_count += 1;
                                gnu_only_total += gnu_run - ras_run;
                        }
                        else if (ras_run > gnu_run)
                        {
                                ras_only[ras_only_count] = ras_sorted[ras_index];
                                ras_only_count += 1;
                                ras_only_total += ras_run - gnu_run;
                        }
                        gnu_index += gnu_run;
                        ras_index += ras_run;
                }
        }

        // Remaining runs belong to a single file.
        for (;;)
        {
                if (gnu_index >= gnu_count)
                {
                        break;
                }
                U64 run = symbol_runs_count(gnu_sorted, gnu_count, gnu_index);
                gnu_only[gnu_only_count] = gnu_sorted[gnu_index];
                gnu_only_count += 1;
                gnu_only_total += run;
                gnu_index += run;
        }

        for (;;)
        {
                if (ras_index >= ras_count)
                {
                        break;
                }
                U64 run = symbol_runs_count(ras_sorted, ras_count, ras_index);
                ras_only[ras_only_count] = ras_sorted[ras_index];
                ras_only_count += 1;
                ras_only_total += run;
                ras_index += run;
        }

        if (gnu_only_total == 0 && ras_only_total == 0)
        {
                fprintf(stdout, "  OK    %llu symbols, all match\n", gnu_count);
        }
        else
        {
                fprintf(stdout, "  DIFF  GNU=%llu ras=%llu common=%llu gnu-only=%llu ras-only=%llu\n",
                        gnu_count, ras_count, common, gnu_only_total, ras_only_total);

                U64 index = 0;
                for (;;)
                {
                        if (index >= gnu_only_count)
                        {
                                break;
                        }
                        String8_Node *node = Arena__push_struct_m(arena, String8_Node);
                        node->string = String8__format(arena, ".symtab: gnu-only ('%.*s', %u, %u, %u, '%.*s', %llu, %llu)",
                                String8__varg(gnu_only[index].name), gnu_only[index].type, gnu_only[index].bind,
                                gnu_only[index].visibility, String8__varg(gnu_only[index].section_name),
                                gnu_only[index].value, gnu_only[index].size);
                        String8_Node_List__push(diffs, node);
                        index += 1;
                }

                index = 0;
                for (;;)
                {
                        if (index >= ras_only_count)
                        {
                                break;
                        }
                        String8_Node *node = Arena__push_struct_m(arena, String8_Node);
                        node->string = String8__format(arena, ".symtab: ras-only ('%.*s', %u, %u, %u, '%.*s', %llu, %llu)",
                                String8__varg(ras_only[index].name), ras_only[index].type, ras_only[index].bind,
                                ras_only[index].visibility, String8__varg(ras_only[index].section_name),
                                ras_only[index].value, ras_only[index].size);
                        String8_Node_List__push(diffs, node);
                        index += 1;
                }
        }
        return;
}

//------------------------------------------------------------------------------
// Driver
//------------------------------------------------------------------------------

// Compare the two parsed objects and print the report. Returns non-zero when
// any difference is found.
internal S32
compare_objects_execute(Object_File *ras, Object_File *gnu, Arena *arena)
{
        String8_Node_List diffs = {0};

        sections_headers_compare(ras, gnu, arena, &diffs);

        // Symbols are needed to interpret relocations and for the symtab comparison.
        U64 ras_symbols_count = 0;
        U64 gnu_symbols_count = 0;
        Object_Symbol *ras_symbols = object_file_symbols(ras, arena, &ras_symbols_count);
        Object_Symbol *gnu_symbols = object_file_symbols(gnu, arena, &gnu_symbols_count);

        U64 identical_count = sections_content_compare(ras, gnu, arena, &diffs);

        relocations_compare(ras, gnu, ras_symbols, ras_symbols_count, gnu_symbols, gnu_symbols_count, arena, &diffs);

        symbols_compare(ras, gnu, arena, &diffs);

        // First differing byte of `.text`, if any.
        B32 text_first_has = 0;
        U64 text_first     = 0;
        Object_Section *text_gnu = object_file_section(gnu, String8__literal(".text"));
        Object_Section *text_ras = object_file_section(ras, String8__literal(".text"));
        if (text_gnu && text_ras)
        {
                U64 compare_count = min_m(text_gnu->size, text_ras->size);
                U64 first = bytes_first_difference(gnu->data + text_gnu->offset, ras->data + text_ras->offset, compare_count, compare_count);
                if (first < compare_count)
                {
                        text_first_has = 1;
                        text_first     = first;
                }
        }

        fprintf(stdout, "=== SUMMARY ===\n");
        fprintf(stdout, "identical sections: %llu\n", identical_count);
        fprintf(stdout, "differing: %llu\n", diffs.count);

        for each_node_m(diffs.first, node)
        {
                fprintf(stdout, "  - %.*s\n", String8__varg(node->string));
        }

        fprintf(stdout, "text first diff: ");
        if (text_first_has)
        {
                hex_print(stdout, text_first);
        }
        else
        {
                fprintf(stdout, "none (identical)");
        }
        fprintf(stdout, "\n");

        S32 result = diffs.count != 0;
        return result;
}

internal S32
compare_objects_run(String8 path_first, String8 path_second)
{
        Arena *arena = Arena__allocate_m();

        S32 result = 1;
        Object_File ras = {0};
        Object_File gnu = {0};
        String8 error_ras = object_file_read(path_first, arena, &ras);
        if (error_ras.count)
        {
                fprintf(stderr, "error: ras object: %.*s\n", String8__varg(error_ras));
        }
        else
        {
                String8 error_gnu = object_file_read(path_second, arena, &gnu);
                if (error_gnu.count)
                {
                        fprintf(stderr, "error: gnu object: %.*s\n", String8__varg(error_gnu));
                }
                else
                {
                        result = compare_objects_execute(&ras, &gnu, arena);
                }
        }
        return result;
}

S32
main(S32 argument_count, char **argument_vector)
{
        S32 result = 1;
        if (argument_count == 3)
        {
                String8 path_first  = String8__from_cstring(argument_vector[1]);
                String8 path_second = String8__from_cstring(argument_vector[2]);
                result = compare_objects_run(path_first, path_second);
        }
        else
        {
                fprintf(stderr, "usage: compare_objects <object_a> <object_b>\n");
        }
        return result;
}
