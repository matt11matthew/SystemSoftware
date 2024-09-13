#include "machine_types.h"
#include "instruction.h"
#include <stdio.h>
// a size for the memory (2^16 words = 32k words)
#define MEMORY_SIZE_IN_WORDS 32768

static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

int main(int argc, char **argv){
    if (argc > 1)
    {
        // Print each argument as a string
        for (int i = 1; i < argc; i++)
        {
            printf("Argument %d as string: %s\n", i, argv[i]);
        }
    }
    else
    {
        printf("No command-line arguments provided.\n");
    }
    return 0;
}