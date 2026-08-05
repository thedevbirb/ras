// This is your build script. You only need to "bootstrap" it once with `cc -o nob nob.c`
// (you can call the executable whatever actually) or `cl nob.c` on MSVC. After that every
// time you run the `nob` executable if it detects that you modifed nob.c it will rebuild
// itself automatically thanks to NOB_GO_REBUILD_URSELF (see below)

// nob.h is an stb-style library https://github.com/nothings/stb/blob/master/docs/stb_howto.txt
// What that means is that it's a single file that acts both like .c and .h files, but by default
// when you include it, it acts only as .h. To make it include implementations of the functions
// you must define NOB_IMPLEMENTATION macro. This is done to give you full control over where
// the implementations go.
#define NOB_IMPLEMENTATION

// Always keep a copy of nob.h in your repo. One of my main pet peeves with build systems like CMake
// and Autotools is that the codebases that use them naturally rot. That is if you do not actively update
// your build scripts, they may not work with the latest version of the build tools. Here we basically
// include the entirety of the source code of the tool along with the code base. It will never get
// outdated (unless you got no standard compliant C compiler lying around, but at that point why are
// you trying to build a C project?)
//
// (In these examples we actually symlinking nob.h, but this is to keep nob.h-s synced among all the
// examples)
#include "nob.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER               "build/"
#define SRC_FOLDER                 "src/"
#define INCLUDE_FOLDER             "thirdparty/"
#define CLION_COMPILATION_DATABASE "compile_commands.json"

typedef struct Options Options;
struct Options
{
        int release;
        int clion;
        int clion_clean;
};

static void
print_usage(const char *program)
{
        fprintf(stdout,
                        "Usage: %s [options]\n"
                        "\n"
                        "Options:\n"
                        "    --debug              Build the debug configuration (default): -g -O0 + sanitizers.\n"
                        "    --release            Build the release configuration: -O2"
                        "    --clion              Update compile_commands.json for CLion (accumulates variants).\n"
                        "    --clion-clean        Delete compile_commands.json.\n"
                        "    -h, --help           Print this help message.\n",
                        program);
}

static Options
parse_options(int argc, char **argv)
{
        Options options = {0};
        for (int i = 1; i < argc; ++i)
        {
                if (strcmp(argv[i], "--clion") == 0)
                {
                        options.clion = true;
                }
                else if (strcmp(argv[i], "--clion-clean") == 0)
                {
                        options.clion_clean = true;
                }
                else if (strcmp(argv[i], "--release") == 0)
                {
                        options.release = true;
                }
                else if (strcmp(argv[i], "--debug") == 0)
                {
                        options.release = false;
                }
                else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
                {
                        print_usage(argv[0]);
                        exit(0);
                }
                else
                {
                        nob_log(NOB_ERROR, "Unknown argument: %s", argv[i]);
                        print_usage(argv[0]);
                        exit(1);
                }
        }
        return options;
}

// Escape a C string so it can be embedded in a JSON string literal.
static void
nob_json_escape(Nob_String_Builder *sb, const char *s)
{
        for (const unsigned char *p = (const unsigned char *) s; *p; ++p)
        {
                switch (*p)
                {
                        case '"':  nob_sb_append_cstr(sb, "\\\""); break;
                        case '\\': nob_sb_append_cstr(sb, "\\\\"); break;
                        case '\b': nob_sb_append_cstr(sb, "\\b"); break;
                        case '\f': nob_sb_append_cstr(sb, "\\f"); break;
                        case '\n': nob_sb_append_cstr(sb, "\\n"); break;
                        case '\r': nob_sb_append_cstr(sb, "\\r"); break;
                        case '\t': nob_sb_append_cstr(sb, "\\t"); break;
                        default:   nob_da_append(sb, *p); break;
                }
        }
}

// Append the current build command to the compilation database. Entries that are
// already recorded (same rendered command) are skipped so repeated runs do not
// grow the file. Each build variant (`./nob --release --clion`) records its own
// entry, which lets CLion switch between compilations of the same file.
static int
nob_generate_clion_compilation_database(const char *db_path, Nob_Cmd cmd, Nob_File_Paths sources)
{
        const char *cwd = nob_get_current_dir_temp();
        if (cwd == NULL) return false;

        Nob_String_Builder db = {0};
        bool initialized = false;
        if (nob_file_exists(db_path))
        {
                if (!nob_read_entire_file(db_path, &db)) return false;
                initialized = db.count > 0;
        }

        for (size_t i = 0; i < sources.count; ++i)
        {
                Nob_String_Builder command = {0};
                nob_cmd_render(cmd, &command);
                nob_sb_append_null(&command);

                if (initialized)
                {
                        nob_sb_append_null(&db);
                        bool already_recorded = strstr(db.items, command.items) != NULL;
                        db.count -= 1; // drop the null we just appended
                        if (already_recorded) continue;
                }

                Nob_String_Builder entry = {0};
                nob_sb_append_cstr(&entry, "  {\n");
                nob_sb_append_cstr(&entry, "    \"directory\": \"");
                nob_json_escape(&entry, cwd);
                nob_sb_append_cstr(&entry, "\",\n");
                nob_sb_append_cstr(&entry, "    \"command\": \"");
                nob_json_escape(&entry, command.items);
                nob_sb_append_cstr(&entry, "\",\n");
                nob_sb_append_cstr(&entry, "    \"file\": \"");
                nob_json_escape(&entry, sources.items[i]);
                nob_sb_append_cstr(&entry, "\"\n");
                nob_sb_append_cstr(&entry, "  }");

                if (initialized)
                {
                        // Insert the new entry before the closing ']' of the existing file.
                        char *closing_bracket = strrchr(db.items, ']');
                        if (closing_bracket == NULL)
                        {
                                // Malformed database: rebuild it from scratch.
                                db.count = 0;
                                initialized = false;
                        } else {
                                db.count = (size_t)(closing_bracket - db.items);
                                nob_sb_append_cstr(&db, ",\n");
                        }
                }

                if (!initialized)
                {
                        nob_sb_append_cstr(&db, "[\n");
                }
                nob_sb_append_buf(&db, entry.items, entry.count);
                nob_sb_append_cstr(&db, "\n]");
                initialized = true;
        }

        if (!nob_write_entire_file(db_path, db.items, db.count)) return false;
        nob_log(NOB_INFO, "Updated %s", db_path);
        return true;
}

int
main(int argc, char **argv)
{
        // This line enables the self-rebuilding. It detects when nob.c is updated and auto rebuilds it then
        // runs it again.
        NOB_GO_REBUILD_URSELF(argc, argv);

        Options options = parse_options(argc, argv);

        // It's better to keep all the building artifacts in a separate build folder. Let's create it if it
        // does not exist yet.
        //
        // Majority of the nob command return bool which indicates whether operation has failed or not (true -
        // success, false - failure). If the operation returned false you don't need to log anything, the
        // convention is usually that the function logged what happened to itself. Just do
        // `if (!nob_function()) return;`
        if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

        // The working horse of nob is the Nob_Cmd structure. It's a Dynamic Array of strings which represent
        // command line that you want to execute.
        Nob_Cmd cmd = {0};

        // Flags that are shared between all build configurations.
        nob_cmd_append(&cmd, "cc",
                "-std=c11",
                // "-w",
                "-Wall", "-Wextra", "-Wpedantic",
                "-Wno-override-init",
                "-Wno-unused-function",
                "-Werror=shadow",
                "-Werror=incompatible-pointer-types",
                "-Werror=int-conversion",
                "-Werror=sign-compare",
                "-Werror=parentheses",
                "-I.",
                "-I"SRC_FOLDER,
                // "-DRAS_DEBUG_TOKEN_DUMP",
                "-DU8_AS_UNSIGNED_CHAR");

        // Flags that are specific to the selected configuration.
        if (options.release)
        {
                // Release build: optimized, no sanitizers.
                nob_cmd_append(&cmd, "-O2");
        }
        else
        {
                // Debug build (default): debug info, no optimization, all the sanitizers.
                nob_cmd_append(&cmd, "-g", "-O0",
                        "-fsanitize=address",                            // ASan: out-of-bounds, use-after-free, use-after-return, etc.
                        "-fsanitize-address-use-after-scope",            // ASan: poison stack vars after their scope ends.
                        "-fsanitize-address-use-after-return=always",    // ASan: detect use-after-return (stack use after the call).
                        "-fno-omit-frame-pointer",                       // keep frame pointers for usable stack traces.
                        "-fno-sanitize-recover=all");                    // abort on first sanitizer hit instead of continuing.
        }

        nob_cmd_append(&cmd, "-o", BUILD_FOLDER"ras");

        // The compilation database needs the list of source files as separate entries,
        // so collect them here instead of appending directly to the command.
        Nob_File_Paths sources = {0};
        nob_da_append(&sources, SRC_FOLDER"ras_main.c");
        for (size_t i = 0; i < sources.count; ++i)
        {
                nob_cmd_append(&cmd, sources.items[i]);
        }

        if (options.clion_clean)
        {
                if (nob_file_exists(CLION_COMPILATION_DATABASE) && !nob_delete_file(CLION_COMPILATION_DATABASE)) return 1;
        }

        if (options.clion)
        {
                if (!nob_generate_clion_compilation_database(CLION_COMPILATION_DATABASE, cmd, sources)) return 1;
        }

        // Let's execute the command.
        if (!nob_cmd_run(&cmd)) return 1;

        return 0;
}
