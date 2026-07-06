#ifndef LANGUAGE_REGISTER_H
#define LANGUAGE_REGISTER_H

typedef struct Register Register;
struct Register
{
    const char *name;
    U8 number; // Bad padding but that's it.
};

#define register_tp 4

static const Register register_map[] =
{
        {"x0",  0},  {"x1",  1},  {"x2",  2},  {"x3",  3},
        {"x4",  4},  {"x5",  5},  {"x6",  6},  {"x7",  7},
        {"x8",  8},  {"x9",  9},  {"x10", 10}, {"x11", 11},
        {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15},
        {"x16", 16}, {"x17", 17}, {"x18", 18}, {"x19", 19},
        {"x20", 20}, {"x21", 21}, {"x22", 22}, {"x23", 23},
        {"x24", 24}, {"x25", 25}, {"x26", 26}, {"x27", 27},
        {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31},

        {"zero", 0},
        {"ra",   1},
        {"sp",   2},
        {"gp",   3},
        {"tp",   4},
        {"fp",   8},  // fp is alias for s0.

        {"t0",  5},  {"t1",  6},  {"t2",  7},
        {"t3", 28},  {"t4", 29},  {"t5", 30}, {"t6", 31},

        {"s0",  8},  {"s1",  9},
        {"s2", 18},  {"s3", 19},  {"s4", 20}, {"s5", 21},
        {"s6", 22},  {"s7", 23},  {"s8", 24}, {"s9", 25},
        {"s10", 26}, {"s11", 27},

        {"a0", 10},  {"a1", 11},  {"a2", 12}, {"a3", 13},
        {"a4", 14},  {"a5", 15},  {"a6", 16}, {"a7", 17},

        // Extra buffer to avoid faults when doing length-based checks.
        {"", 0xFF}
};

#define register_map_size (sizeof(register_map) / sizeof(register_map[0])) - 1
#define register_invalid 0xFF

// Returns 0xFF on not found.
U8
register_lookup(String8 string);

#endif // LANGUAGE_REGISTER_H

