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

#if defined(__clang__)
#  define NOB_CC_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#  define NOB_CC_GCC 1
#else
#  error "unsupported compiler"
#endif

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
#define CODEGEN_FOLDER             "codegen/"
#define GENERATED_FOLDER           "generated/"
#define INCLUDE_FOLDER             "thirdparty/"

typedef struct Options Options;
struct Options
{
        int release;
        int codegen;
        // Build and run the object comparator on the two provided objects.
        int compare_objects;
        const char *compare_objects_ras;
        const char *compare_objects_gnu;
        // Name of a different compiler to use, by default `cc`.
        const char *compiler;
};
static const char *cc_default = "cc";

static void
print_usage(const char *program)
{
        fprintf(stdout,
                        "Usage: %s [options]\n"
                        "\n"
                        "Options:\n"
                        "    --codegen     Regenerate "GENERATED_FOLDER"instruction_hashes.h from "CODEGEN_FOLDER"instruction_hashes.c.\n"
                        "    --compare-objects <obj1> <obj2>  Build the object comparator and compare the two ELF objects.\n"
                        "    --cc <cc>     Use another C compiler to build the program, by default `cc`\n"
                        "    --debug       Build the debug configuration (default): -g -O0 + sanitizers.\n"
                        "    --release     Build the release configuration: -O2\n"
                        "    -h, --help    Print this help message.\n",
                        program);
}

static Options
parse_options(int argc, char **argv)
{
        Options options = { .compiler = cc_default };
        for (int i = 1; i < argc; ++i)
        {
                if (strcmp(argv[i], "--codegen") == 0)
                {
                        options.codegen = true;
                }
                else if (strcmp(argv[i], "--compare-objects") == 0)
                {
                        if (i + 2 >= argc)
                        {
                                fprintf(stderr, "expected two object files after --compare-objects\n");
                                print_usage(argv[0]);
                                exit(1);
                        }
                        options.compare_objects = true;
                        options.compare_objects_ras = argv[i + 1];
                        options.compare_objects_gnu = argv[i + 2];
                        i += 2;
                }
                else if (strcmp(argv[i], "--release") == 0)
                {
                        options.release = true;
                }
                else if (strcmp(argv[i], "--debug") == 0)
                {
                        options.release = false;
                }
                else if (strcmp(argv[i], "--cc") == 0)
                {
                        if (i + 1 == argc)
                        {
                                fprintf(stderr, "missing argument for option %s\n", argv[i]);
                                print_usage(argv[0]);
                                exit(1);
                        }
                        else
                        {
                                i += 1;
                                options.compiler = argv[i];
                        }
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

        if (options.codegen)
        {
                Nob_Cmd cmd = {0};

                nob_cmd_append(&cmd, options.compiler,
                        "-std=c11",
                        "-Wall", "-Wextra",
                        "-o", BUILD_FOLDER"instruction_hashes",
                        CODEGEN_FOLDER"instruction_hashes.c");

                if (!nob_cmd_run(&cmd)) return 1;

                // The codegen writes the generated hashes to stdout; capture them into
                // GENERATED_FOLDER"instruction_hashes.h".
                Nob_Cmd run = {0};
                nob_cmd_append(&run, BUILD_FOLDER"instruction_hashes");
                // This `.stdout_path` option is very very cool.
                if (!nob_cmd_run(&run, .stdout_path = GENERATED_FOLDER"instruction_hashes.h")) return 1;

                return 0;
        }

        if (options.compare_objects)
        {
                Nob_Cmd cmd = {0};

                // Same shared flags as the main `ras` build, so the comparator is
                // held to the same standard.
                nob_cmd_append(&cmd, options.compiler,
                        "-std=c11",
                        "-Wall", "-Wextra",
                        "-Wno-override-init",
                        "-Wno-unused-function",
                        "-Werror=shadow",
                        "-Werror=incompatible-pointer-types",
                        "-Werror=int-conversion",
                        "-Werror=sign-compare",
                        "-Werror=parentheses",
                        "-I.",
                        "-I"SRC_FOLDER,
                        "-DU8_AS_UNSIGNED_CHAR",
                        "-o", BUILD_FOLDER"compare_objects",
                        "compare_objects.c");

#ifdef NOB_CC_CLANG
                nob_cmd_append(&cmd, "-Wno-gnu-designator");
#endif

                if (!nob_cmd_run(&cmd)) return 1;

                // Run the comparison. The tool exits non-zero when differences
                // are found, which is propagated as a failure here.
                Nob_Cmd run = {0};
                nob_cmd_append(&run, BUILD_FOLDER"compare_objects",
                        options.compare_objects_ras,
                        options.compare_objects_gnu);
                if (!nob_cmd_run(&run)) return 1;

                return 0;
        }

        // The working horse of nob is the Nob_Cmd structure. It's a Dynamic Array of strings which represent
        // command line that you want to execute.
        Nob_Cmd cmd = {0};

        // Flags that are shared between all build configurations.
        nob_cmd_append(&cmd, options.compiler,
                "-std=c11",
                "-Wall", "-Wextra",
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

#ifdef NOB_CC_CLANG
        nob_cmd_append(&cmd, "-Wno-gnu-designator");
#endif

        // Flags that are specific to the selected configuration.
        if (options.release)
        {
                nob_cmd_append(&cmd, "-O2", "-march=native");
        }
        else
        {
                // Debug build (default): debug info, no optimization, all the sanitizers.
                nob_cmd_append(&cmd, "-g3", "-O0");
                nob_cmd_append(&cmd, "-ftrapv");                             // trap on signed integer overflow/underflow.
                nob_cmd_append(&cmd,
                        "-fsanitize=address",                            // ASan: out-of-bounds, use-after-free, use-after-return, etc.
                        "-fsanitize-address-use-after-scope",            // ASan: poison stack vars after their scope ends.
#ifdef NOB_CC_CLANG
                        "-fsanitize-address-use-after-return=always",    // ASan: detect use-after-return (stack use after the call).
#endif
                        "-fno-omit-frame-pointer",                       // keep frame pointers for usable stack traces.
                        "-fno-sanitize-recover=all");                    // abort on first sanitizer hit instead of continuing.
        }

        nob_cmd_append(&cmd, "-o", BUILD_FOLDER"ras");

        Nob_File_Paths sources = {0};
        nob_da_append(&sources, SRC_FOLDER"ras_main.c");
        for (size_t i = 0; i < sources.count; ++i)
        {
                nob_cmd_append(&cmd, sources.items[i]);
        }

        // Let's execute the command.
        if (!nob_cmd_run(&cmd)) return 1;

        return 0;
}
