//
// Created by Matt on 9/30/2024.
//
#include "machine_types.h"
#include "instruction.h"
#include <stdio.h>
#include <string.h>
#include "bof.h"
#include "regname.h"
#include <stdlib.h>

#ifndef HW1_VM_H
#define HW1_VM_H
#define MEMORY_SIZE_IN_WORDS 32768// a size for the memory (2^16 words = 32k words)
#define TRASH_VALUE -999
#define NEW_LINE_AFTER 59
#define NL 4
#define ON 1
#define OFF 0




// Function Prototypes
void handleInstruction( );
void compFormatInstr(comp_instr_t instruction, int address);
void otherCompInstr(other_comp_instr_t instruction, int address);
void immediateFormatInstr(immed_instr_t i, int address);
void jumpFormatInstr(jump_instr_t instruction, int address);
void executeSyscall(syscall_instr_t instruction, int i );
void exitErrorCode(int errorCode);
void readInInstructions(int length, BOFFILE file);
void openTraceFile(const char *currentTestCase);
void printRegContent(int lst);
void printProgramCounter();
void printTrace(bin_instr_t instruction, instr_type type, int address);
void printInstructions( int length);
void processInstructions(int length);
void initRegisters(BOFFILE file);
void printData(FILE* stream, int data_start, int data_end, int lst);
void handleBOFFile(char * file_name, int should_print);
#endif //HW1_VM_H
