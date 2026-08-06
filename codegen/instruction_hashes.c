#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef uint32_t U32;
typedef uint8_t  U8;
typedef int32_t  B32;

////////////////////////////////
// Hash

U32
hash_FNV_1a(const char *data, size_t count)
{
        U32 hash = 2166136261u;

        U32 index = 0;
        for (;;)
        {
                B32 break_should = index >= count;
                if (break_should)
                {
                        break;
                }

                hash ^= (U8)data[index];
                hash *= 16777619u;

                index += 1;
        }

        return hash;
}

////////////////////////////////
// Instruction list

const char *Instruction_Kind_strings[] =
{
        "", // None

        "lui","auipc","jal","jalr",

        "beq","bne","blt","bge","bltu","bgeu",

        "lb","lh","lw","ld","lbu","lhu","lwu",

        "sb","sh","sw","sd",

        "addi","slti","sltiu","xori","ori","andi",

        "slli","srli","srai",

        "add","sub","sll","slt","sltu","xor","srl","sra","or","and",

        "addiw","slliw","srliw","sraiw",
        "addw","subw","sllw", "srlw","sraw",

        "ecall","ebreak","fence", "fence.tso", "pause",

        // Pseudo instructions
        "nop","ret","mv","not","neg","negw","sext.w","seqz","snez","sltz","sgtz","beqz","bnez","blez",
        "bgez","bltz","bgtz","bgt","ble","bgtu","bleu","j","call","tail","jr","li","la", "lla",

        "csrrw","csrrs","csrrc","csrrwi","csrrsi","csrrci",

        // M
        "mul","mulh","mulhsu","mulhu","div","divu","rem","remu",
        "mulw","divw","divuw","remw","remuw",

        // A
        "lr.w","sc.w","amoswap.w","amoadd.w","amoxor.w","amoand.w","amoor.w",
        "amomin.w","amomax.w","amominu.w","amomaxu.w",

        "lr.d","sc.d","amoswap.d","amoadd.d","amoxor.d","amoand.d","amoor.d",
        "amomin.d","amomax.d","amominu.d","amomaxu.d",
};

////////////////////////////////
// Helpers

// Replace from source '.' and '-' into '_', and add a null terminator at the end.
void
make_safe(char *destination, const char *source)
{
        for (;;)
        {
                B32 break_should = *source == 0;
                if (break_should)
                {
                        break;
                }

                char character = *source;
                if (character == '.' || character == '-')
                {
                        character = '_';
                }

                *destination = character;

                source += 1;
                destination += 1;
        }

        *destination = 0;
        return;
}

////////////////////////////////

int
main(void)
{
        size_t instructions_count =
                sizeof(Instruction_Kind_strings) /
                sizeof(Instruction_Kind_strings[0]);

        char safe_names[256][64];
        U32 instruction_hashes[256];

        size_t maximum_name_length = 0;

        ////////////////////////////////
        // Build names + hashes

        size_t instruction_index = 1;
        for (;;)
        {
                B32 break_should = instruction_index >= instructions_count;
                if (break_should)
                {
                        break;
                }

                const char *instruction_string =
                        Instruction_Kind_strings[instruction_index];

                make_safe(
                        safe_names[instruction_index],
                        instruction_string
                );

                size_t string_length = strlen(instruction_string);

                instruction_hashes[instruction_index] =
                        hash_FNV_1a(
                                instruction_string,
                                string_length
                        );

                size_t safe_name_length =
                        strlen(safe_names[instruction_index]);

                B32 update_should =
                        safe_name_length > maximum_name_length;

                if (update_should)
                {
                        maximum_name_length = safe_name_length;
                }

                instruction_index += 1;
        }

        ////////////////////////////////
        // Collision check (PANIC)

        size_t outer_index = 1;
        for (;;)
        {
                B32 break_outer = outer_index >= instructions_count;
                if (break_outer)
                {
                        break;
                }

                size_t inner_index = outer_index + 1;
                for (;;)
                {
                        B32 break_inner = inner_index >= instructions_count;
                        if (break_inner)
                        {
                                break;
                        }

                        B32 collision_found =
                                instruction_hashes[outer_index] ==
                                instruction_hashes[inner_index];

                        if (collision_found)
                        {
                                fprintf(stderr,
                                        "FATAL: hash collision!\n"
                                        "  \"%s\" and \"%s\"\n"
                                        "  hash = 0x%08X\n",
                                        Instruction_Kind_strings[outer_index],
                                        Instruction_Kind_strings[inner_index],
                                        instruction_hashes[outer_index]);

                                exit(1);
                        }

                        inner_index += 1;
                }

                outer_index += 1;
        }

        ////////////////////////////////
        // Output

        printf("// =====================\n");
        printf("// HASH DEFINES (FNV-1a)\n");
        printf("// =====================\n\n");

        size_t output_index = 1;
        for (;;)
        {
                B32 break_should = output_index >= instructions_count;
                if (break_should)
                {
                        break;
                }

                printf("#define HASH_%-*s 0x%08X\n",
                        (int)maximum_name_length,
                        safe_names[output_index],
                        instruction_hashes[output_index]);

                output_index += 1;
        }

        return 0;
}
