#include "riscv_extensions.h"

// The canonical order of single-letter extensions.
//
// Follows GNU as canonical order: single-letter extensions sort first, then prefixed ones, so the emitted architecture
// regardless of the `-march` input order.  A letter that appears here can be written consecutively in `-march` without
// a separator, e.g. "rv64imfdc".
global const String8 RISCV_extensions_single_order = String8__literal("eigmafdqlcbkjtpvnh");

// HOW TO ADD AN EXTENSION
// -----------------------
// 1. Add an OPC__* class in riscv_instruction.h and assign it to the matching
//    instructions in riscv_instruction.c.
// 2. Single-letter extension: insert the letter into
//    RISCV_extensions_single_order, keeping the canonical RISC-V order.
// 3. Prefixed extension: insert the name (with its default version) into
//    RISCV_Extension__defaults; that table is what resolves versions and
//    recognizes the name in `-march`.
// 4. If the extension implies others, add a rule to RISCV_extensions_implicit.
//    Implicit names do NOT need to be listed in the defaults table: they are
//    resolved from it when added.

global const RISCV_Extension RISCV_Extension__defaults[] =
{
        { .name = String8__literal("e"),        1, 9 },
        { .name = String8__literal("i"),        2, 1 },
        { .name = String8__literal("m"),        2, 0 },
        { .name = String8__literal("a"),        2, 1 },
        { .name = String8__literal("f"),        2, 2 },
        { .name = String8__literal("d"),        2, 2 },
        { .name = String8__literal("c"),        2, 0 },
        { .name = String8__literal("b"),        1, 0 },
        { .name = String8__literal("zba"),      1, 0 },
        { .name = String8__literal("zbb"),      1, 0 },
        { .name = String8__literal("zbc"),      1, 0 },
        { .name = String8__literal("zbs"),      1, 0 },
        { .name = String8__literal("zca"),      1, 0 },
        { .name = String8__literal("zcd"),      1, 0 },
        { .name = String8__literal("zcf"),      1, 0 },
        { .name = String8__literal("zicntr"),   2, 0 },
        { .name = String8__literal("zicond"),   1, 0 },
        { .name = String8__literal("zicsr"),    2, 0 },
        { .name = String8__literal("zifencei"), 2, 0 },
        { .name = String8__literal("zmmul"),    1, 0 },
        { .name = String8__literal("zaamo"),    1, 0 },
        { .name = String8__literal("zalrsc"),   1, 0 },
};
assert_static_m(array_count_m(RISCV_Extension__defaults) <= RISCV_Extensions__max, RISCV_Extension__defaults_size_check);

global const RISCV_Implicit_Extension RISCV_extensions_implicit[] =
{
        { .extension = String8__literal("e"),      .implicit = String8__literal("i"),               .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("c"),      .implicit = String8__literal("zca"),             .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("c"),      .implicit = String8__literal("zcd"),             .requires = String8__literal("d"), .xlen =  0  },
        { .extension = String8__literal("c"),      .implicit = String8__literal("zcf"),             .requires = String8__literal("f"), .xlen = 32 },
        { .extension = String8__literal("zicntr"), .implicit = String8__literal("zicsr"),           .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("m"),      .implicit = String8__literal("zmmul"),           .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("a"),      .implicit = String8__literal("zaamo,zalrsc"),    .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("d"),      .implicit = String8__literal("f"),               .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("f"),      .implicit = String8__literal("zicsr"),           .requires = {0},                   .xlen =  0  },
        { .extension = String8__literal("b"),      .implicit = String8__literal("zba,zbb,zbc,zbs"), .requires = {0},                   .xlen =  0  },
};

// `g` is a group alias for the standard extension set.  It is expanded in place at parse time (see
// RISCV_Extensions__parse) rather than stored in the list, so it never appears in the architecture string (GNU as emits
// the expansion only).
global const String8 RISCV_extensions_g_components = String8__literal("i,m,a,f,d,zicsr,zifencei");

internal const RISCV_Extension *
RISCV_Extensions__find(const RISCV_Extension *extensions, U64 count, String8 name)
{
        const RISCV_Extension *result = 0;
        U64 index = 0;
        for (;;)
        {
                B32 break_should = result || index >= count;
                if (break_should)
                {
                        break;
                }

                if (String8__match_exact(extensions[index].name, name))
                {
                        result = &extensions[index];
                }

                index += 1;
        }

        return result;
}

// Canonical order position of a single-letter extension, or 0 if it is not a canonical single-letter extension.
internal U8
riscv_extension_single_order(U8 letter)
{
        U8 result = 0;
        U64 index = 0;
        for (;;)
        {
                B32 break_should = index >= RISCV_extensions_single_order.count
                                || RISCV_extensions_single_order.data[index] == letter;
                if (break_should)
                {
                        break;
                }

                index += 1;
        }

        B32 found = index < RISCV_extensions_single_order.count;
        if (found)
        {
                result = (U8)(index + 1);
        }

        return result;
}

// Case-insensitive comparison of the tail of two prefixed extension names.
internal B32
riscv_extension_name_less(const String8 a, const String8 b)
{
        B32 result = 0;
        U64 index = 1;
        for (;;)
        {
                B32 a_end = index >= a.count;
                B32 b_end = index >= b.count;
                B32 different = 0;
                if (!a_end && !b_end)
                {
                        U8 a_char = a.data[index];
                        U8 b_char = b.data[index];
                        different = a_char != b_char;
                        if (different)
                        {
                                result = a_char < b_char;
                        }
                }
                else
                {
                        // One name is a prefix of the other: the shorter sorts first.
                        result = a_end && !b_end;
                }

                B32 break_should = a_end || b_end || different;
                if (break_should)
                {
                        break;
                }

                index += 1;
        }

        return result;
}

// Whether `a` sorts before `b` in the canonical extension order, mirroring GNU as. Single-letter extensions first, in
// canonical order, then prefixed extensions.
internal B32
riscv_extensions_less(const String8 a, const String8 b)
{
        B32 result = 0;
        B32 a_single = a.count == 1;
        B32 b_single = b.count == 1;
        if (a_single && b_single)
        {
                result = riscv_extension_single_order(a.data[0]) < riscv_extension_single_order(b.data[0]);
        }
        else if (a_single != b_single)
        {
                result = a_single;
        }
        else
        {
                U8 a_second = riscv_extension_single_order(a.data[1]);
                U8 b_second = riscv_extension_single_order(b.data[1]);
                if (a_second != b_second)
                {
                        result = a_second < b_second;
                }
                else
                {
                        result = riscv_extension_name_less(a, b);
                }
        }

        return result;
}

// Add `name` at its canonical position, unless it is already present.  The list
// stays canonically sorted at all times, so no separate sort pass is needed.
internal void
RISCV_extensions_add(RISCV_Extensions *extensions, String8 name, U8 major, U8 minor)
{
        B32 present = RISCV_Extensions__find(extensions->data, extensions->count, name) != 0;
        B32 full    = extensions->count >= extensions->max;
        if (present || full)
        {
                return;
        }

        U64 insert_index = 0;
        for (;;)
        {
                B32 break_should = insert_index >= extensions->count
                                || riscv_extensions_less(name, extensions->data[insert_index].name);
                if (break_should)
                {
                        break;
                }

                insert_index += 1;
        }

        // Make room by shifting the tail right. It's okay since the list is short.
        U64 shift_index = extensions->count;
        for (;;)
        {
                B32 break_should = shift_index <= insert_index;
                if (break_should)
                {
                        break;
                }

                extensions->data[shift_index] = extensions->data[shift_index - 1];
                shift_index -= 1;
        }

        extensions->data[insert_index] = (RISCV_Extension)
        {
                .name  = name,
                .major = major,
                .minor = minor,
        };
        extensions->count += 1;
        return;
}

// Add a comma-separated list of extension names, resolving each to its default version. Unknown names are skipped.
internal void
RISCV_extensions_add_list(RISCV_Extensions *extensions, String8 list)
{
        String8 rest = list;
        for (;;)
        {
                U64 comma_index = 0;
                for (;;)
                {
                        B32 comma_break = comma_index >= rest.count || rest.data[comma_index] == ',';
                        if (comma_break)
                        {
                                break;
                        }

                        comma_index += 1;
                }

                String8 name = String8__substring(rest, comma_index);
                if (name.count > 0)
                {
                        const RISCV_Extension *extension = RISCV_Extensions__find(RISCV_Extension__defaults, array_count_m(RISCV_Extension__defaults), name);
                        if (extension)
                        {
                                RISCV_extensions_add(extensions, name, extension->major, extension->minor);
                        }
                }

                B32 list_break = comma_index >= rest.count;
                if (list_break)
                {
                        break;
                }
                rest = String8__skip(rest, comma_index + 1);
        }

        return;
}

internal void
RISCV_extensions_add_implicit(RISCV_Extensions *extensions, U8 xlen)
{
        U64 rule_index = 0;
        for (;;)
        {
                B32 break_should = rule_index >= array_count_m(RISCV_extensions_implicit);
                if (break_should)
                {
                        break;
                }

                const RISCV_Implicit_Extension *rule = &RISCV_extensions_implicit[rule_index];
                B32 present   = RISCV_Extensions__find(extensions->data, extensions->count, rule->extension) != 0;
                B32 satisfied = rule->requires.count == 0
                             || RISCV_Extensions__find(extensions->data, extensions->count, rule->requires) != 0;
                B32 xlen_ok   = rule->xlen == 0 || rule->xlen == xlen;
                if (present && satisfied && xlen_ok)
                {
                        RISCV_extensions_add_list(extensions, rule->implicit);
                }

                rule_index += 1;
        }

        return;
}

internal String8
RISCV_Extensions__update(Arena *arena, RISCV_Extensions *extensions, String8 string, U8 xlen)
{
        // `.option arch` expressions are assumed to be tightly formatted
        // (`+c,+zbb1p0`, `rv64imafd`): no whitespace of any kind is expected
        // or tolerated, so the string is used as-is.
        String8 error = {0};

        if (string.count == 0)
        {
                error = String8__format(arena, "ISA string cannot be empty");
        }
        else if (string.data[0] == '+')
        {
                // Incremental additions: `+ext[,...]`.
                String8 rest = string;
                for (;;)
                {
                        // Split off the next comma-separated entry.
                        U64 comma = 0;
                        for (;;)
                        {
                                B32 comma_break = comma >= rest.count || rest.data[comma] == ',';
                                if (comma_break)
                                {
                                        break;
                                }
                                comma += 1;
                        }
                        String8 entry = String8__substring(rest, comma);
                        B32 add_is = entry.count && entry.data[0] == '+';

                        String8 name    = {0};
                        String8 version = {0};
                        B32 base_is     = 0;
                        B32 version_is  = 0;

                        const RISCV_Extension *extension_default = 0;

                        if (add_is)
                        {
                                // `+name[<major>p<minor>]`: name is the run of letters
                                // up to the first digit or whitespace.
                                String8 name_and_version = String8__skip(entry, 1);
                                U64 name_count = 0;
                                for (;;)
                                {
                                        B32 name_break = name_count >= name_and_version.count
                                                      || U8__ascii_digit_is(name_and_version.data[name_count])
                                                      || name_and_version.data[name_count] == ' '
                                                      || name_and_version.data[name_count] == '\t';
                                        if (name_break)
                                        {
                                                break;
                                        }
                                        name_count += 1;
                                }
                                name    = String8__substring(name_and_version, name_count);
                                version = String8__skip(name_and_version, name_count);

                                // GNU as refuses to add the base extensions through `.option arch`.
                                base_is = name.count == 0
                                       || String8__match_exact(name, String8__literal("i"))
                                       || String8__match_exact(name, String8__literal("e"))
                                       || String8__match_exact(name, String8__literal("g"));
                                extension_default = RISCV_Extensions__find(RISCV_Extension__defaults, array_count_m(RISCV_Extension__defaults), name);
                                // Optional `<major>p<minor>` version suffix; leftover text is an invalid form.
                                version_is = version.count >= 3
                                           && U8__ascii_digit_is(version.data[0])
                                           && version.data[1] == 'p'
                                           && U8__ascii_digit_is(version.data[2]);
                        }

                        // Fail-fast chain: stop at the first invalid form, mirroring GNU as.
                        if (!add_is)
                        {
                                // Only additions are supported (GNU as has deprecated `-`).
                                error = error.count ? error : String8__format(arena, "unknown ISA extension in .option arch `%.*s'", String8__varg(string));
                        }
                        else if (base_is)
                        {
                                error = error.count ? error : String8__format(arena, "cannot + base extension `%.*s' in .option arch `%.*s'", String8__varg(name), String8__varg(string));
                        }
                        else if (extension_default == 0)
                        {
                                error = error.count ? error : String8__format(arena, "unknown ISA extension `%.*s' in .option arch `%.*s'", String8__varg(name), String8__varg(string));
                        }
                        else if (version.count && !version_is)
                        {
                                error = error.count ? error : String8__format(arena, "unknown ISA extension in .option arch `%.*s'", String8__varg(string));
                        }
                        else
                        {
                                U8 major = extension_default->major;
                                U8 minor = extension_default->minor;
                                if (version_is)
                                {
                                        major = version.data[0] - '0';
                                        minor = version.data[2] - '0';
                                }
                                RISCV_extensions_add(extensions, name, major, minor);
                        }

                        B32 more_is = comma < rest.count && error.count == 0;
                        if (more_is)
                        {
                                rest = String8__skip(rest, comma + 1);
                        }
                        B32 break_should = !more_is;
                        if (break_should)
                        {
                                break;
                        }
                }
        }
        else if (string.count)
        {
                // Re-parse of a full ISA string (rv32/rv64 prefix required).
                // `.option` may not change the XLEN, so the prefix must match the current one.

                B32 rv32_is = String8__match_prefix(string, String8__literal("rv32"));
                B32 rv64_is = String8__match_prefix(string, String8__literal("rv64"));
                if (rv32_is || rv64_is)
                {
                        B32 xlen_matches = (rv32_is && xlen == XLEN_32)
                                        || (rv64_is && xlen == XLEN_64);
                        if (xlen_matches)
                        {
                                String8 extensions_string = String8__skip(string, 4);
                                error = RISCV_Extensions__parse(arena, extensions, extensions_string, xlen);
                        }
                        else
                        {
                                error = String8__format(arena, ".option arch cannot change the XLEN (`rv%u' given, %u-bit ISA)", rv32_is ? 32 : 64, xlen);
                        }
                }
                else
                {
                        error = String8__format(arena, "ISA string `%.*s' must begin with rv32 or rv64", String8__varg(string));
                }
        }

        if (error.count == 0 && string.data[0] == '+')
        {
                RISCV_extensions_add_implicit(extensions, xlen);
                error = RISCV_extensions_check_conflicts(arena, extensions, xlen);
        }

        return error;
}

internal String8
RISCV_extensions_check_conflicts(Arena *arena, RISCV_Extensions *extensions, U8 xlen)
{
        String8 error = {0};
        if (xlen == 64 && RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("e")))
        {
                error = String8__format(arena, "rv64e is not supported");
        }
        else if (RISCV_Extensions__find(extensions->data, extensions->count,  String8__literal("e"))
                 && RISCV_Extensions__find(extensions->data, extensions->count,  String8__literal("h")))
        {
                error = String8__format(arena, "rv%de does not support the `h' extension", xlen);
        }
        else if (xlen == 32 && RISCV_Extensions__find(extensions->data, extensions->count,  String8__literal("q")))
        {
                error = String8__format(arena, "rv32 does not support the `q' extension");
        }
        else if (xlen == 64 && RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zcf")))
        {
                error = String8__format(arena, "rv64 does not support the `zcf' extension");
        }

        return error;
}

internal B32
RISCV_extensions_supports_class(const RISCV_Extensions *extensions, OPC class)
{
        B32 result = 0;
        switch (class)
        {
        case OPC__None:     { result = 1; } break;
        case OPC__I:        { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("i"))        != 0; } break;
        case OPC__C:        { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zca"))      != 0
                                    || RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zcd"))      != 0; } break;
        case OPC__M:        { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("m"))        != 0; } break;
        case OPC__ZMMUL:    { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zmmul"))    != 0; } break;
        case OPC__F:        { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("f"))        != 0; } break;
        case OPC__D:        { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("d"))        != 0; } break;
        case OPC__ZICOND:   { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zicond"))   != 0; } break;
        case OPC__ZBA:      { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zba"))      != 0; } break;
        case OPC__ZBC:      { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zbc"))      != 0; } break;
        case OPC__ZBS:      { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zbs"))      != 0; } break;
        case OPC__ZBB:      { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zbb"))      != 0; } break;
        case OPC__ZIFENCEI: { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zifencei")) != 0; } break;
        case OPC__ZICNTR:   { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zicntr"))   != 0; } break;
        case OPC__ZICSR:    { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zicsr"))    != 0; } break;
        case OPC__ZALRSC:   { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zalrsc"))   != 0; } break;
        case OPC__ZAAMO:    { result = RISCV_Extensions__find(extensions->data, extensions->count, String8__literal("zaamo"))    != 0; } break;
        default:            { result = 0; } break;
        }

        return result;
}

internal String8
RISCV_Extensions__parse(Arena *arena, RISCV_Extensions *extensions, String8 string, U8 xlen)
{

        extensions->count = 0;
        String8 error = {0};

        // ISA strings cannot contain uppercase letters.
        U64 index = 0;
        for (;;)
        {
                B32 break_should = error.count > 0 || index >= string.count;
                if (break_should)
                {
                        break;
                }

                if ('A' <= string.data[index] && string.data[index] <= 'Z')
                {
                        error = error.count ? error : String8__format(arena, "ISA string `%.*s' cannot contain uppercase letters", String8__varg(string));
                }

                index += 1;
        }

        // The first extension must be i, e or g.
        if (string.count == 0 || (string.data[0] != 'i' && string.data[0] != 'e' && string.data[0] != 'g'))
        {
                error = error.count ? error : String8__format(arena, "ISA string `%.*s': first ISA extension must be `e', `i' or `g'", String8__varg(string));
        }

        // Parse the extensions, separated by '_'. Single-letter standard extensions
        // are parsed consecutively without separators ("rv64imfdc"), while prefixed
        // extensions (z*/s*/x*) must be separated by '_' ("rv64i_zba_zbb").
        String8 cursor = string;
        for (;;)
        {
                B32 break_should = error.count > 0 || cursor.count == 0;
                if (break_should)
                {
                        break;
                }

                if (cursor.data[0] == '_')
                {
                        cursor = String8__skip(cursor, 1);
                }
                else
                {
                        // Prefixed extensions start with z, s or x (covering "zxm", "zaamo", ...).
                        B32 prefixed_is = cursor.data[0] == 'z'
                                       || cursor.data[0] == 's'
                                       || cursor.data[0] == 'x';

                        // The name is the run of letters leading up to the first digit. A
                        // single-letter standard extension is exactly one character.
                        String8 name = cursor;
                        if (prefixed_is)
                        {
                                for (;;)
                                {
                                        B32 name_break = cursor.count == 0
                                                     || cursor.data[0] == '_'
                                                     || U8__ascii_digit_is(cursor.data[0]);
                                        if (name_break)
                                        {
                                                break;
                                        }

                                        cursor = String8__skip(cursor, 1);
                                }
                        }
                        else
                        {
                                cursor = String8__skip(cursor, 1);
                        }
                        name.count -= cursor.count;

                        // The version is whatever follows the name, up to the next '_'.
                        U8 major = 0;
                        U8 minor = 0;
                        B32 valid_format = cursor.count >= 3
                                        && U8__ascii_digit_is(cursor.data[0])
                                        && cursor.data[1] == 'p'
                                        && U8__ascii_digit_is(cursor.data[2]);
                        if (valid_format)
                        {
                                major = cursor.data[0] - '0';
                                minor = cursor.data[2] - '0';
                                cursor = String8__skip(cursor, 3);
                        }

                        // `g` is a group alias: it is expanded here into its components, so it never enters the
                        // extension list (GNU as emits the expansion only).
                        B32 is_g = String8__match_exact(name, String8__literal("g"));
                        if (!is_g)
                        {
                                const RISCV_Extension *extension_default = RISCV_Extensions__find(RISCV_Extension__defaults, array_count_m(RISCV_Extension__defaults), name);
                                if (extension_default)
                                {
                                        if (!valid_format)
                                        {
                                                major = extension_default->major;
                                                minor = extension_default->minor;
                                        }
                                }
                                else
                                {
                                        error = error.count ? error : String8__format(arena, "ISA string `%.*s': unknown ISA extension `%.*s'", String8__varg(string), String8__varg(name));
                                }
                        }

                        // Prefixed extensions must be separated by '_'.
                        if (prefixed_is && cursor.count && cursor.data[0] != '_')
                        {
                                error = error.count ? error : String8__format(arena, "ISA string `%.*s': prefixed ISA extension must separate with '_'", String8__varg(string));
                        }

                        if (is_g)
                        {
                                RISCV_extensions_add_list(extensions, RISCV_extensions_g_components);
                        }
                        else
                        {
                                if (extensions->count >= extensions->max)
                                {
                                        error = error.count ? error : String8__format(arena, "ISA string `%.*s': too many extensions", String8__varg(string));
                                }
                                else
                                {
                                        RISCV_extensions_add(extensions, name, major, minor);
                                }
                        }
                }
        }

        RISCV_extensions_add_implicit(extensions, xlen);
        error = error.count ? error : RISCV_extensions_check_conflicts(arena, extensions, xlen);

        return error;
}
