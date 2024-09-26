//Includes
#include "machine_types.h"
#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bof.h"
#include "regname.h"

// a size for the memory (2^16 words = 32k words)
#define MEMORY_SIZE_IN_WORDS 32768

//Global variables
int PC = 0;
int HI = 0 ;
int LO = 0;
static int GPR[MEMORY_SIZE_IN_WORDS];

static union mem_u{// Used to represent the memory of the VM
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

// Function Prototypes
void otherCompInstr(other_comp_instr_t instruction, int address);
void immediateFormatInstr(immed_instr_t i, int address);
void jumpFormatInstr(jump_instr_t instruction, int address);
void executeSyscall(syscall_instr_t instruction, int i );
void handleBOFFile(char * file_name, int should_print);
void compFormatInstr(comp_instr_t instruction, int address);
void readInInstructions(int length, BOFFILE file);
void printInstructions( int length);
void processInstructions(int length);

//Functions
void handleInstruction(bin_instr_t instruction, instr_type type, int address) {
    PC++;//Increment the Program Counter
    switch (type) {//Call appropriate function based on the type being fed
        case comp_instr_type://Computational Instructions, with opcode 0
            compFormatInstr(instruction.comp, address);
            break;
        case other_comp_instr_type://Computational Instructions, with opcode 1
            otherCompInstr(instruction.othc, address);
            break;
        case immed_instr_type://Immediate Format instructions
            immediateFormatInstr(instruction.immed, address);
            break;
        case jump_instr_type://Jump Format Instructions
            jumpFormatInstr(instruction.jump, address);
            break;
        case syscall_instr_type://System Callse
            executeSyscall(instruction.syscall, address);
        case error_instr_type:
            //stdeer

            break;
    }
}

void compFormatInstr(comp_instr_t instruction, int address) {
    switch (instruction.func) {// Switch to handle operation based on func code
        case NOP_F:
            break; //Does nothing
        case ADD_F:// Add
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.words[GPR[SP]] + (memory.words[GPR[instruction.rs]] + machine_types_formOffset(instruction.os));
            break;
        case SUB_F:// Subtract
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] = 
                memory.words[GPR[SP]] - (memory.words[GPR[instruction.rs]] + machine_types_formOffset(instruction.os));
            break;
        case CPW_F:// Copy Word
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)]
            = memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)];
            break;
        case AND_F:// Bitwise And
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] = 
                memory.uwords[GPR[SP]] & (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case BOR_F:// Bitwise Or
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.uwords[GPR[instruction.rs]] | (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case NOR_F:// Bitwise Not-Or
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    ~(memory.uwords[GPR[instruction.rs]] | (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]));
            break;
        case XOR_F:// Bitwise Exclusive-Or
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.uwords[GPR[SP]] ^ (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case LWR_F:// Load word into Register
            GPR[instruction.rt] = memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)];
            break;
        case SWR_F:// Store word from register
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] = GPR[instruction.rs];
            break;
        case SCA_F:// Store Computed address
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    (GPR[instruction.rs] + machine_types_formOffset(instruction.rs));
            break;
        case LWI_F:// Load Word Indirect
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                memory.words[memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]];
            break;
        case NEG_F:// Negate
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    -memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)];
            break;
    }
}

void otherCompInstr(other_comp_instr_t i, int address) {
    switch (i.func) {
        case LIT_F: // Literal
            memory.words[GPR[i.reg + machine_types_formOffset(i.offset)]] = machine_types_sgnExt(i.arg);
            break;

        case ARI_F: // Add register immediate
            memory.words[GPR[i.reg]] = GPR[i.reg] + machine_types_sgnExt(i.arg);
            break;

        case SRI_F: // Subtract register immediate
            memory.words[GPR[i.reg]] = GPR[i.reg] - machine_types_sgnExt(i.arg);
            break;

        case MUL_F: // Multiply
            // Multiply stack top by memory[GPR[s] + formOffset(o)],
            // putting the most significant bits in HI
            // and the least significant bits in LO.
            // (HI, LO) ← memory[GPR[$sp]] × (memory[GPR[s] + formOffset(o)])
            memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    memory.words[GPR[SP]] * (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]);
            break;

        case DIV_F: // Divide
            HI = memory.words[GPR[SP]] % (memory.words[GPR[i.reg]] + machine_types_formOffset(i.offset));
            LO = memory.words[GPR[SP]] / (memory.words[GPR[i.reg]] + machine_types_formOffset(i.offset));
            break;

        case CFHI_F: // Copy from HI
            memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] = HI;
            break;

        case CFLO_F: // Copy from LO
            memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] = LO;
            break;

        case SLL_F: // Shift Left Logical
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] = memory.uwords[GPR[SP]] << i.arg;
            break;

        case SRL_F: // Shift Right Logical
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] = memory.uwords[GPR[SP]] >> i.arg;
            break;

        case JMP_F: // Jump
            PC = memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)];
            break;

        case CSI_F: // Call Subroutine Indirectly
            GPR[RA] = PC;
            PC = memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)];
            break;

        case JREL_F: // Jump Relative to address
            PC = (PC - 1) + machine_types_formOffset(i.offset);
            break;

        case SYS_F: // System Call
            syscall_instr_t si;
            si.op = OTHC_O;
            si.reg = i.reg;
            si.offset = i.offset;
            si.code = i.op;
            si.func = SYS_F;
            executeSyscall(si, address);
            break;
    }
}

void immediateFormatInstr(immed_instr_t i, int address) {
    switch (i.op) {
        case ADDI_O:// Add Immediate
            memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] + machine_types_sgnExt(i.immed);
            break;
        case ANDI_O:// Bitwise And immediate
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    (memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)]) & machine_types_zeroExt(i.immed);
            break;
        case BORI_O:// Bitwise Or Immediate
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    (memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)]) | machine_types_zeroExt(i.immed);
            break;
        case NORI_O:  // Bitwise NOR Immediate
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    ~(memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] | machine_types_zeroExt(i.immed));
            break;
        case XORI_O:// Bitwise Exclusive-Or Immediate
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    (memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] ^ machine_types_zeroExt(i.immed));
            break;
        case BEQ_O:// Branch on Equal
            if (memory.words[GPR[SP] = memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]]) {
                PC = (PC - 1 + machine_types_formOffset(i.immed));
            }
            break;
        case BGEZ_O:// Branch >= 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] >= 0) {
                PC = (PC - 1 + machine_types_formOffset(i.immed));
            }
            break;
        case BGTZ_O:// Branch > 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] > 0) {
                PC = (PC - 1 + machine_types_formOffset(i.immed));
            }
            break;
        case BLEZ_O:// Branch <= 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] <= 0) {
                PC = (PC - 1 + machine_types_formOffset(i.immed));
            }
            break;
        case BLTZ_O:// Branch < 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] < 0) {
                PC = (PC - 1 + machine_types_formOffset(i.immed));
            }
            break;
        case BNE_O:// Branch Not Equal
            if (memory.words[GPR[SP] != memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]]) {
                PC = (PC - 1 + machine_types_formOffset(i.immed));
            }
            break;
    }
}

void jumpFormatInstr(jump_instr_t instruction, int address) {
    switch (instruction.addr) {// Handle jump instruction based on op code
        case JMPA_O:// Jump To given Address
            PC = machine_types_formAddress(PC - 1, instruction.addr);
            break;
        case CALL_O:// Call Subroutine
            GPR[RA] = PC;
            PC = machine_types_formAddress(PC - 1, instruction.addr);
            break;
        case RTN_O:// Return from Subroutine
            PC = GPR[RA];
            break;
    }
}

void executeSyscall(syscall_instr_t instruction, int i) {

    switch (instruction.code) {
        case exit_sc://EXIT
//            exit(machine_types_sgnExt(instruction.offset));
            break;
        case print_str_sc://PSTR
            memory.words[GPR[SP]]= printf("%s",&memory.words[GPR[instruction.reg] + machine_types_formOffset(instruction.offset)]);
            break;
        case print_char_sc://PCH
            memory.words[GPR[SP]] = fputc(memory.words[GPR[instruction.reg] + machine_types_formOffset(instruction.offset)], stdout);
            break;
        case read_char_sc://RCH
            memory.words[GPR[instruction.reg] + machine_types_formOffset(instruction.offset)] = getc(stdin);
            break;
        case start_tracing_sc://Start VM tracing output
            break;
        case stop_tracing_sc://No VM tracing; Stop the tracing output
            break;
    }
}

void readInInstructions(int length,   BOFFILE file) {
    for (int i = 0; i < length; i++) { //loop header
        if (i > MEMORY_SIZE_IN_WORDS) {
            fprintf(stderr, "Error: Too many words in BOF file.\n");
            bof_close(file);
            return;
        }
        memory.instrs[i] = instruction_read(file);
    }
}

void printInstructions( int length) {
    printf("%s %s\n", "Address", "Instruction");
    for (int i = 0; i < length; i++) {
        bin_instr_t instruction = memory.instrs[i];
        printf( "%8u: %s\n", i, instruction_assembly_form(i, instruction));
    }
}

void processInstructions(int length) {
    for (int i = 0; i < length; i++) {
        bin_instr_t instruction = memory.instrs[i];
        instr_type type = instruction_type(instruction);
        handleInstruction(instruction, type, i);
    }
}
/*
 *
 */

void initRegisters(BOFFILE file,BOFHeader header) {
    PC = header.text_start_address;

    GPR[GP] = header.data_start_address;
    GPR[SP] = header.stack_bottom_addr;
    GPR[FP] = header.stack_bottom_addr;
    GPR[RA] = 0;


    for (int i = 0; i < header.data_length; i++){

        printf("TEST INPUT: %d\n", bof_read_word(file));
    }
}

void handleBOFFile(char * file_name, int should_print) {
    BOFFILE file = bof_read_open(file_name);
    BOFHeader header = bof_read_header(file);
    int tempCount = 0;//Track if a \n needs to be printed

    if (!bof_has_correct_magic_number(header)) {
        fprintf(stderr, "Error: Invalid magic number in BOF file.\n");
        bof_close(file);
        return;
    }

    initRegisters(file, header);


    int length = header.text_length;

    readInInstructions(length, file);




    if (should_print) {
        printInstructions(length);
    }

    processInstructions(length);

    if (should_print) {
        int data_start = header.data_start_address;
        int data_end = data_start + header.data_length+1;

        // Iterate through the memory words
        for (int i = data_start; i < data_end; i++) {
            printf("%8u: %d", i, memory.words[i]);
            
            // Print a newline after every 5th value or at the end
            if (tempCount == 4 ) {
                tempCount = 0;
                printf("\n");  // Add newline after every 5th value or at the end
            } else {
                printf("\t");  // Add tab between values for alignment
            }
            tempCount++;
        }

        printf("        ...\n");

        printf("\n");
    }

    bof_close(file);
}

int main(int argc, char **argv) {

    int shouldPrint = false;
    char *fileName;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        shouldPrint = true;  // Enable printing
        fileName = argv[2];   // File name is the second argument
    } else if (argc == 2) {
        fileName = argv[1];   // File name is the first argument
    } else {
        fprintf(stderr, "Usage: %s [-p] <BOF file>\n", argv[0]);
        return 1;  // Return non-zero for usage error
    }

    // Begin reading the BOF file and handle printing if enabled
    handleBOFFile(fileName, shouldPrint);

    return 0;
}