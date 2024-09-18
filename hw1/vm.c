
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




void handleBOFFile(char * file_name, int should_print);


void printStats() {
    printf("PC: %d\n", PC);
    printf("SP: %d\n", SP);
    printf("FP: %d\n", FP);
    printf("HI: %d\n", HI);
    printf("LO: %d\n", LO);
}

void executeSyscall(bin_instr_t instruction) {

}




void handleInstruction(bin_instr_t instruction, instr_type type, int i ) {
    switch (type) {
        case comp_instr_type:
            printf("comp_instr_type");
            break;
        case other_comp_instr_type:
            printf("other_comp_instr_type");
            break;
        case immed_instr_type:
            printf("immed_instr_type");
            break;
        case jump_instr_type:
            printf("jump_instr_type");
            break;
        case syscall_instr_type:
            printf("syscall_instr_type");
            executeSyscall(instruction);
        return;
        case error_instr_type:
            printf("error_instr_type");
            break;
    }
    if (type != syscall_instr_type) {
        memory.instrs[PC + i] = instruction;
    }
}

void handleBOFFile(char * file_name, int should_print) {
    BOFFILE file = bof_read_open(file_name);
    BOFHeader header = bof_read_header(file);

    if (!bof_has_correct_magic_number(header)) {
        fprintf(stderr, "Error: Invalid magic number in BOF file.\n");
        bof_close(file);
        return;
    }

    PC = header.text_start_address;

    SP = MEMORY_SIZE_IN_WORDS - 1;
    // FP = header.stack_bottom_addr;


    // for (int i = 0; i < header.text_length; i++) { //loop header
    //     if (i > MEMORY_SIZE_IN_WORDS) {
    //         fprintf(stderr, "Error: Too many words in BOF file.\n");
    //         bof_close(file);
    //         return;
    //     }
    //
    //     bin_instr_t instruction = instruction_read(file);
    //     instr_type  type = instruction_type(instruction);
    //     handleInstruction(instruction, type, i );
    //
    //
    // }
    bin_instr_t instruction = instruction_read(file);
    instr_type  type = instruction_type(instruction);
    handleInstruction(instruction, type, 0 );
    bof_close(file);
}

int main(int argc, char **argv) {

    int shouldPrint = false;
    char* fileName = 0;

    if (argc==3 && strcmp(argv[1], "-p") == 0) {
        shouldPrint = true;
        fileName= argv[2];
    } else if (argc == 2 && strcmp(argv[1], "-p") ==1) {
        fileName = argv[1];
    } else {
        fprintf(stderr, "Usage: %s [-p] <BOF file>\n", argv[0]);
        return 0;
    }
    handleBOFFile(fileName,shouldPrint);


    return 0;
}
