
#include "machine_types.h"
#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// a size for the memory (2^16 words = 32k words)
#define MEMORY_SIZE_IN_WORDS 32768

//Global variables
int PC = 0;
int SP = 0;
int FP = 0;
int HI = 0;
int LO = 0;

static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

//Function Prototypes

void readFile(char* name, char** output) {
    FILE *file = fopen(name, "r");
    if (file == NULL) {
        perror("Failed to open file");
        *output = NULL;
        return;
    }

    // Move the file pointer to the end to get the size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file); // Reset pointer to the beginning

    // Allocate memory for the file contents plus a null terminator
    *output = (char*)malloc((fileSize + 1) * sizeof(char));
    if (*output == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }

    // Read the file into the allocated buffer
    fread(*output, sizeof(char), fileSize, file);
    (*output)[fileSize] = '\0'; // Null-terminate the string

    fclose(file);
}


int main(int argc, char **argv) {
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        char *fileContents = NULL;
        readFile(argv[2], &fileContents);

        if (fileContents != NULL) {
            printf("File Contents:\n%s\n", fileContents);
            free(fileContents); // Remember to free the allocated memory
        } else {
            printf("Failed to read file or allocate memory.\n");
        }
    } else {
        printf("Invalid arguments. Use: %s -p <filename>\n", argv[0]);
    }

    return 0;
}
