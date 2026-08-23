#include "riscv_extensions.h"

// The single-letter standard extensions recognized by ras, in canonical order.
//
// Canonical order follows bfd/elfxx-riscv.c `riscv_ext_canonical_order`
// ("eigmafdqlcbkjtpvnh"), restricted to the extensions ras actually supports
// (those with an opcode class in `riscv_instruction.c`, plus `e`/`c`/`g` which
// have a direct effect on assembly options). A letter that appears here can be
// written consecutively in `-march` without a separator, e.g. "rv64imfdc".
global const String8 RISCV_extensions_standard_order = String8__literal("eigmfdc");

// Known prefixed extensions (z* and s*).
//
// A name that appears here can be written in `-march`, separated by '_', e.g.
// "rv64imfd_zba_zbb_zicond". Only extensions with an opcode class in
// `riscv_instruction.c` belong here.
//
// HOW TO ADD AN EXTENSION
// -----------------------
// 1. Add an OPC__* class in riscv_instruction.h and assign it to the matching
//    instructions in riscv_instruction.c.
// 2. Single-letter extension: insert the letter into
//    RISCV_extensions_standard_order, keeping the canonical RISC-V order
//    (e i g m a f d q l c b k j t p v n h).
// 3. Prefixed extension: insert the name into RISCV_extensions_prefixed,
//    keeping the list sorted alphabetically. (Order is otherwise irrelevant,
//    it is only scanned linearly for recognition.)
// 4. If the extension implies others, add a rule to RISCV_extensions_implicit.
//    Implicit names do NOT need to be listed here: they are added internally,
//    so only extensions that can appear explicitly in `-march` are validated.
global const String8 RISCV_extensions_prefixed[] =
{
        String8__literal("zba"),
        String8__literal("zbb"),
        String8__literal("zbc"),
        String8__literal("zbs"),
        String8__literal("zicntr"),
        String8__literal("zicond"),
        String8__literal("zicsr"), /* TODO(medium): In practice, zicsr isn't supported in its instructions yet */
        String8__literal("zifencei"),
        String8__literal("zmmul"),
};

global const RISCV_Extension RISCV_Extension__defaults[] =
{
        { .name = String8__literal("i"),        2, 1 },
        { .name = String8__literal("m"),        2, 0 },
        { .name = String8__literal("f"),        2, 2 },
        { .name = String8__literal("d"),        2, 2 },
        { .name = String8__literal("zba"),      1, 0 },
        { .name = String8__literal("zbb"),      1, 0 },
        { .name = String8__literal("zbc"),      1, 0 },
        { .name = String8__literal("zbs"),      1, 0 },
        { .name = String8__literal("zicntr"),   2, 0 },
        { .name = String8__literal("zicond"),   1, 0 },
        { .name = String8__literal("zicsr"),    2, 0 },
        { .name = String8__literal("zifencei"), 2, 0 },
        { .name = String8__literal("zmmul"),    1, 0 },
};
assert_static_m(array_count_m(RISCV_Extension__defaults) <= RISCV_Extensions__max, RISCV_Extension__defaults_size_check);

global const RISCV_Implicit_Extension RISCV_extensions_implicit[] =
{
        { String8__literal("g"),      String8__literal("i,m,a,f,d,zicsr,zifencei") },
        { String8__literal("e"),      String8__literal("i")                        },
        { String8__literal("zicntr"), String8__literal("zicsr")                    },
        { String8__literal("m"),      String8__literal("zmmul")                    },
        { String8__literal("a"),      String8__literal("zaamo,zalrsc")             },
        { String8__literal("d"),      String8__literal("f")                        },
        { String8__literal("f"),      String8__literal("zicsr")                    },
};

internal B32
RISCV_Extensions__supports(const RISCV_Extensions *extensions, String8 name)
{
        B32 result = 0;
        U64 index = 0;
        for (;;)
        {
                B32 break_should = result || index >= extensions->count;
                if (break_should)
                {
                        break;
                }

                if (String8__match_exact(extensions->data[index].name, name))
                {
                        result = 1;
                }

                index += 1;
        }

        return result;
}

internal RISCV_Extension *
RISCV_Extensions__find(RISCV_Extensions *extensions, String8 name)
{
        RISCV_Extension *result = 0;
        U64 index = 0;
        for (;;)
        {
                B32 break_should = result || index >= extensions->count;
                if (break_should)
                {
                        break;
                }

                if (String8__match_exact(extensions->data[index].name, name))
                {
                        result = &extensions->data[index];
                }

                index += 1;
        }

        return result;
}

internal B32
RISCV_extension_name_known(String8 name)
{
        B32 result = 0;
        if (name.count > 0)
        {
                if (name.count == 1)
                {
                        U64 index = 0;
                        for (;;)
                        {
                                B32 break_should = result || index >= RISCV_extensions_standard_order.count;
                                if (break_should)
                                {
                                        break;
                                }

                                result = name.data[0] == RISCV_extensions_standard_order.data[index];

                                index += 1;
                        }
                }
                else if (name.data[0] == 'x')
                {
                        // Custom extensions: any well-formed name except the bare "x".
                        result = name.count > 1;
                }
                else if (name.data[0] == 'z' || name.data[0] == 's')
                {
                        U64 index = 0;
                        for (;;)
                        {
                                B32 break_should = result || index >= array_count_m(RISCV_extensions_prefixed);
                                if (break_should)
                                {
                                        break;
                                }

                                result = String8__match_exact(name, RISCV_extensions_prefixed[index]);

                                index += 1;
                        }
                }
        }

        return result;
}

internal U64
RISCV_extensions_parse_version(String8 version, U8 *major_out, U8 *minor_out)
{
        U32 major    = 0;
        U32 minor    = 0;
        U32 value    = 0;
        B32 in_major = 1;
        B32 stop     = 0;
        U64 index    = 0;
        for (;;)
        {
                B32 break_should = stop || index >= version.count;
                if (break_should)
                {
                        break;
                }

                B32 consumed = 0;
                U8 c = version.data[index];
                if (c == 'p')
                {
                        U8 next = index + 1 < version.count ? version.data[index + 1] : 0;
                        if (U8__ascii_digit_is(next))
                        {
                                major = value;
                                value = 0;
                                in_major = 0;
                                consumed = 1;
                        }
                        else
                        {
                                stop = 1;
                        }
                }
                else if (U8__ascii_digit_is(c))
                {
                        value = value * 10 + (c - '0');
                        consumed = 1;
                }
                else
                {
                        stop = 1;
                }

                index += consumed;
        }

        if (in_major)
        {
                major = value;
        }
        else
        {
                minor = value;
        }

        *major_out = (U8)major;
        *minor_out = (U8)minor;
        return index;
}

internal void
RISCV_extensions_add_implicit(RISCV_Extensions *extensions)
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
                RISCV_Extension *found = RISCV_Extensions__find(extensions, rule->extension);
                if (found == 0)
                {
                        // This rule does not apply; move on to the next rule.
                }
                else
                {
                        String8 rest = rule->implicit;
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
                                if (name.count > 0 && RISCV_Extensions__find(extensions, name) == 0)
                                {
                                        if (extensions->count < RISCV_Extensions__max)
                                        {
                                                extensions->data[extensions->count++] = (RISCV_Extension)
                                                {
                                                        .name  = name,
                                                        .major = 0,
                                                        .minor = 0,
                                                };
                                        }
                                }

                                B32 implicit_break = comma_index >= rest.count;
                                if (implicit_break)
                                {
                                        break;
                                }
                                rest = String8__skip(rest, comma_index + 1);
                        }
                }

                rule_index += 1;
        }

        return;
}

internal String8
RISCV_extensions_check_conflicts(Arena *arena, RISCV_Extensions *extensions, U8 xlen)
{
        String8 error = {0};
        if (xlen == 64 && RISCV_Extensions__supports(extensions, String8__literal("e")))
        {
                error = String8__format(arena, "rv64e is not supported");
        }
        else if (RISCV_Extensions__supports(extensions, String8__literal("e"))
                 && RISCV_Extensions__supports(extensions, String8__literal("h")))
        {
                error = String8__format(arena, "rv%de does not support the `h' extension", xlen);
        }
        else if (xlen == 32 && RISCV_Extensions__supports(extensions, String8__literal("q")))
        {
                error = String8__format(arena, "rv32 does not support the `q' extension");
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
        case OPC__I:        { result = RISCV_Extensions__supports(extensions, String8__literal("i")); } break;
        case OPC__M:        { result = RISCV_Extensions__supports(extensions, String8__literal("m")); } break;
        case OPC__ZMMUL:    { result = RISCV_Extensions__supports(extensions, String8__literal("zmmul")); } break;
        case OPC__F:        { result = RISCV_Extensions__supports(extensions, String8__literal("f")); } break;
        case OPC__D:        { result = RISCV_Extensions__supports(extensions, String8__literal("d")); } break;
        case OPC__ZICOND:   { result = RISCV_Extensions__supports(extensions, String8__literal("zicond")); } break;
        case OPC__ZBA:      { result = RISCV_Extensions__supports(extensions, String8__literal("zba")); } break;
        case OPC__ZBC:      { result = RISCV_Extensions__supports(extensions, String8__literal("zbc")); } break;
        case OPC__ZBS:      { result = RISCV_Extensions__supports(extensions, String8__literal("zbs")); } break;
        case OPC__ZBB:      { result = RISCV_Extensions__supports(extensions, String8__literal("zbb")); } break;
        case OPC__ZIFENCEI: { result = RISCV_Extensions__supports(extensions, String8__literal("zifencei")); } break;
        case OPC__ZICNTR:   { result = RISCV_Extensions__supports(extensions, String8__literal("zicntr")); } break;
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
        U64 cursor = 0;
        for (;;)
        {
                B32 break_should = error.count > 0 || cursor >= string.count;
                if (break_should)
                {
                        break;
                }

                if (string.data[cursor] == '_')
                {
                        cursor += 1;
                }
                else
                {
                        // Prefixed extensions start with z, s or x (covering "zxm", "zaamo", ...).
                        B32 prefixed_is = string.data[cursor] == 'z'
                                       || string.data[cursor] == 's'
                                       || string.data[cursor] == 'x';

                        // The name is the run of letters leading up to the first digit. A
                        // single-letter standard extension is exactly one character.
                        U64 name_begin = cursor;
                        if (prefixed_is)
                        {
                                for (;;)
                                {
                                        B32 name_break = cursor >= string.count
                                                     || string.data[cursor] == '_'
                                                     || ('0' <= string.data[cursor] && string.data[cursor] <= '9');
                                        if (name_break)
                                        {
                                                break;
                                        }

                                        cursor += 1;
                                }
                        }
                        else
                        {
                                cursor += 1;
                        }
                        String8 name = String8__substring(String8__skip(string, name_begin), cursor - name_begin);

                        // The version is whatever follows the name, up to the next '_'.
                        String8 version = String8__skip(string, cursor);
                        U8 major = 0;
                        U8 minor = 0;
                        U64 version_count = RISCV_extensions_parse_version(version, &major, &minor);
                        cursor += version_count;

                        if (name.count == 0)
                        {
                                error = error.count ? error : String8__format(arena, "ISA string `%.*s': missing extension name", String8__varg(string));
                        }
                        else if (!RISCV_extension_name_known(name))
                        {
                                error = error.count ? error : String8__format(arena, "ISA string `%.*s': unknown ISA extension `%.*s'", String8__varg(string), String8__varg(name));
                        }
                        // Prefixed extensions must be separated by '_'.
                        else if (prefixed_is && cursor < string.count && string.data[cursor] != '_')
                        {
                                error = error.count ? error : String8__format(arena, "ISA string `%.*s': prefixed ISA extension must separate with '_'", String8__varg(string));
                        }
                        else if (extensions->count >= extensions->max)
                        {
                                error = error.count ? error : String8__format(arena, "ISA string `%.*s': too many extensions", String8__varg(string));
                        }
                        else
                        {
                                extensions->data[extensions->count] = (RISCV_Extension)
                                {
                                        .name  = name,
                                        .major = major,
                                        .minor = minor,
                                };
                                extensions->count += 1;
                        }
                }
        }

        // Add the implicit extensions.
        if (error.count == 0)
        {
                RISCV_extensions_add_implicit(extensions);
        }

        // Check for conflicts.
        if (error.count == 0)
        {
                error = error.count ? error : RISCV_extensions_check_conflicts(arena, extensions, xlen);
        }

        return error;
}
