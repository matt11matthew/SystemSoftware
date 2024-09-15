
#include "machine_types.h"
#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bof.h"


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


void handleBOFFile(char * file_name, int should_print);


void handleBOFFile(char * file_name, int should_print) {
    // BOFFILE boffile = bof_read_open(file_name);
    // BOFHeader header = bof_read_header(boffile);
    // printf("Text Length: %d\n",header.text_length);
    // printf("data_length: %d\n",header.data_length);
    // printf("magic: %s\n",header.magic);
}

int main(int argc, char **argv) {

    int shouldPrint = false;
    char* fileName = 0;

    if (argc==3 && strcmp(argv[1], "-p") == 0) {
        shouldPrint = true;
        fileName= argv[2];
    } else if (argc==2 && strcmp(argv[1], "-p")==1) {
        fileName = argv[1];
    } else {
        fprintf(stderr, "Usage: %s [-p] <BOF file>\n", argv[0]);
        return 0;

    }
    printf("File Name: %s\n", fileName);
    printf("Print?: %s\n", shouldPrint==1?"YES":"NO");
    handleBOFFile(fileName,shouldPrint);


    return 0;
}
