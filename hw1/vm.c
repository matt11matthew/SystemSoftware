
//Includes
#include "machine_types.h"
#include "instruction.h"
#include <stdio.h>
#include <string.h>
#include "bof.h"
#include "regname.h"

//Defines
#define MEMORY_SIZE_IN_WORDS 32768// a size for the memory (2^16 words = 32k words)
//#define BUFFER_SIZE 1024  // Adjust size as necessary

//Global variables
int PC = 0;
int HI = 0;
int LO = 0;
int exitCode = -999;
int planStopTracing = 0;
static int GPR[MEMORY_SIZE_IN_WORDS];
char finalPrintBuffer[256];
int tracing = 1;
int firstExitTrace = -999;
FILE *traceFile;
BOFHeader globalHeader;


static union mem_u{// Used to represent the memory of the VM
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
} memory;

// Function Prototypes
void handleInstruction(bin_instr_t instruction, instr_type type, int address);
void otherCompInstr(other_comp_instr_t instruction, int address);
void immediateFormatInstr(immed_instr_t i, int address);
void jumpFormatInstr(jump_instr_t instruction, int address);
void executeSyscall(syscall_instr_t instruction, int i );
void handleBOFFile(char * file_name, int should_print);
void compFormatInstr(comp_instr_t instruction, int address);
void readInInstructions(int length, BOFFILE file);
void openTraceFile(const char *currentTestCase);
void printRegContent(int lst);
void printProgramCounter();
void printTrace(bin_instr_t instruction, instr_type type, int address);
void printInstructions( int length);
void processInstructions(int length);
void initRegisters(BOFFILE file);
void printData(FILE* stream, int data_start, int data_end, int lst);
void exitErrorCode(int errorCode);

//Functions
// Handles a given instruction, calling the appropriate function based on the type of instruction.
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
        case syscall_instr_type://System Calls
            executeSyscall(instruction.syscall, address);

            break;
        case error_instr_type:
            //stdeer
            break;
    }
    printTrace(instruction, type, address);
}

// Handles computational format instructions based on the function code
void compFormatInstr(comp_instr_t instruction, int address) {
    switch (instruction.func) {// Switch to handle operation based on func code
        case NOP_F://Does nothing
            break;
        case ADD_F:// Add
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)]
            = memory.words[GPR[SP]] + (memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case SUB_F:// Subtract
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.words[GPR[SP]] - (memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case CPW_F:// Copy Word
            memory.words[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.words[GPR[instruction.rs] + machine_types_formOffset(instruction.os)];
            break;
        case AND_F:// Bitwise And
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.uwords[GPR[SP]] & (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case BOR_F:// Bitwise Or
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    memory.uwords[GPR[SP]] | (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]);
            break;
        case NOR_F:// Bitwise Not-Or
            memory.uwords[GPR[instruction.rt] + machine_types_formOffset(instruction.ot)] =
                    ~(memory.uwords[GPR[SP]] | (memory.uwords[GPR[instruction.rs] + machine_types_formOffset(instruction.os)]));
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

// Handles other computational format instructions based on the function code
void otherCompInstr(other_comp_instr_t i, int address) {
    switch (i.func) {
        case LIT_F: // Literal
            memory.words[GPR[i.reg + machine_types_formOffset(i.offset)]] = machine_types_sgnExt(i.arg);
            break;
        case ARI_F: // Add register immediate
            GPR[i.reg] = (GPR[i.reg] + machine_types_sgnExt(i.arg));
            break;
        case SRI_F: // Subtract register immediate
            GPR[i.reg] = (GPR[i.reg] - machine_types_sgnExt(i.arg));
            break;
        case MUL_F: // Multiply
            long long result = (long long) memory.words[GPR[SP]] *
                               (long long) (memory.words[GPR[i.reg]] + machine_types_formOffset(i.offset));

            // Store the result in HI and LO
            HI = (int) (result >> 32);  // Most significant 32 bits
            LO = (int) (result & 0xFFFFFFFF);  // Least significant 32 bits
            break;
        case DIV_F: // Divide
            // Check if we are dividing by 0
            if((memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]) == 0){
                //TODO ADD ERROR
            }

            // Divisor
            HI = memory.words[GPR[SP]]
                 % (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]);

            // Quotient
            LO = memory.words[GPR[SP]]
                 / (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]);
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
            PC = ((PC - 1) + machine_types_formOffset(i.offset));
            break;
        case SYS_F: // System Call
            //Assign variables
            syscall_instr_t si;
            si.op = OTHC_O;
            si.reg = i.reg;
            si.offset = i.offset;
            si.code = i.op;
            si.func = SYS_F;
            executeSyscall(si, address);// Pass back to Sys call
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
                    memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] | machine_types_zeroExt(i.immed);
            break;
        case NORI_O:  // Bitwise NOR Immediate
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    ~memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] | machine_types_zeroExt(i.immed);
            break;
        case XORI_O:// Bitwise Exclusive-Or Immediate
            memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] =
                    (memory.uwords[GPR[i.reg] + machine_types_formOffset(i.offset)] ^ machine_types_zeroExt(i.immed));
            break;
        case BEQ_O:// Branch on Equal
            if (memory.words[GPR[SP] == memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]]) {
                PC = ((PC - 1) + machine_types_formOffset(i.immed));
            }
            break;
        case BGEZ_O:// Branch >= 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] >= 0) {
                PC = ((PC - 1) + machine_types_formOffset(i.immed));
            }
            break;
        case BGTZ_O:// Branch > 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] > 0) {
                PC = ((PC - 1) + machine_types_formOffset(i.immed));
            }
            break;
        case BLEZ_O:// Branch <= 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] <= 0) {
                PC = ((PC - 1) + machine_types_formOffset(i.immed));
            }
            break;
        case BLTZ_O:// Branch < 0
            if (memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)] < 0) {
                PC = ((PC - 1) + machine_types_formOffset(i.immed));
            }
            break;
        case BNE_O:// Branch Not Equal
            if (memory.words[GPR[SP]] != memory.words[GPR[i.reg] + machine_types_formOffset(i.offset)]) {
                PC = ((PC - 1) + machine_types_formOffset(i.immed));
            }
            break;
    }
}

void jumpFormatInstr(jump_instr_t instruction, int address) {
    switch (instruction.op) {// Handle jump instruction based on op code
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
        case print_int_sc:
            break;
        case exit_sc://EXIT
            exitCode = machine_types_sgnExt(instruction.offset);
            //exitErrorCode(exitCode);
//            firstExitTrace = 1;
            firstExitTrace = -999; //SETS TO TRASH VALUE
            break;
        case print_str_sc://PSTR
            memory.words[GPR[SP]] = printf("%ls", (&memory.words[GPR[instruction.reg] + machine_types_formOffset(instruction.offset)]));
            break;
        case print_char_sc://PCH
            memory.words[GPR[SP]] =
                    fputc(memory.words[GPR[instruction.reg] +
                    machine_types_formOffset(instruction.offset)], traceFile);
            fflush(traceFile);
            break;
        case read_char_sc://RCH
            memory.words[GPR[instruction.reg] + machine_types_formOffset(instruction.offset)] = getc(stdin);
            break;
        case start_tracing_sc://Start VM tracing output
            tracing = 1;
            break;
        case stop_tracing_sc://No VM tracing; Stop the tracing output
            planStopTracing = 1;
            break;
    }
}

void exitErrorCode(int errorCode){
    switch(errorCode){
        case 0:// Dividing by 0
            fprintf(stderr, "can't divide by 0\n");
            break;
        case 1:
            fprintf(stderr, "0 <= GPR[$gp]");
            break;
        case 2:
            fprintf(stderr, "GPR[$gp] < GPR[$sp]");
            break;
        case 3:
            fprintf(stderr, "GPR[$sp] < GPR[$fp]");
            break;
        case 4:
            fprintf(stderr, "GPR[$fp] < MEMORY_SIZE_IN_WORDS");
            break;
        case 5:
            fprintf(stderr, "0 <= PC");
            break;
        case 6:
            fprintf(stderr, "PC < MEMORY_SIZE_IN_WORDS");
            break;
    }
}

void readInInstructions(int length,   BOFFILE file) {
    for (int i = 0; i < length; i++) { //loop header
        if (i >= MEMORY_SIZE_IN_WORDS) {
            fprintf(stderr, "Error: Too many words in BOF file.\n");
            bof_close(file);
            return;
        }
        memory.instrs[i] = instruction_read(file);
    }
}

void openTraceFile(const char *currentTestCase) {//Function to handle opening file
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.myo", currentTestCase); // Format filename with .myo extension

    traceFile = fopen(filename, "w");
    if (traceFile == NULL) {
        perror("Failed to open file");
        return;
    }
}

void printRegContent(int lst) {
    if (exitCode != -999)return;

    if (planStopTracing) {
        tracing = 0;
        planStopTracing = 0;
        return;
    }

    //Print the info for each register
    for (int i = 0; i < 8; i++) {
        fprintf(traceFile, "GPR[%s]: %d\t", regname_get(i), GPR[i]);
        if(i == 4){
            fprintf(traceFile, "\n");
        }
    }
    fprintf(traceFile, "\n");

    printData(traceFile, globalHeader.data_start_address,  GPR[SP], lst);

    printData(traceFile, GPR[SP],  GPR[FP]+1, 0);
}

void printProgramCounter() {
    if(exitCode != -999)return;
    if(tracing == 0)return;
    if(planStopTracing)return;

    fprintf(traceFile, "\tPC: %d",PC);//Print the Program counter

    if (HI!=0 || LO !=0 ) {//Check if HI needs to be printed
        fprintf(traceFile, "\tHI: %d", HI);
        fprintf(traceFile, "\tLO: %d", LO);
    }



    fprintf(traceFile, "\n");
}

void printTrace( bin_instr_t instruction, instr_type type, int address) {
    if (!tracing)return;

    if (exitCode!=-999 ){


        if (firstExitTrace!= -999) {
            firstExitTrace = 0;
            return;
        }
    }

    fprintf(traceFile,  "\n==> %8d: %s\n",address, instruction_assembly_form(address, instruction));//Print Current instruction info
    printProgramCounter();//Print info for program counter, HI, and LO
    printRegContent(0);//print the contents of each register.
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

void initRegisters(BOFFILE file) {
    //Declarations
    BOFHeader header = bof_read_header(file);
    int startIndex = 0;

    //Store values
    PC = header.text_start_address;
    GPR[GP] = header.data_start_address;
    GPR[SP] = header.stack_bottom_addr;
    GPR[FP] = header.stack_bottom_addr;
    GPR[RA] = 0;

    for (int  j = 0; j< header.data_length+header.text_length; j++) {
        int t = bof_read_word(file);
        if (j >=header.text_length){

            int memoryIndex = header.data_start_address+startIndex;

            memory.words[memoryIndex] = t;

            startIndex++;
        }
    }


    printProgramCounter();
    printRegContent(0);

    bof_close(file);//Close file
}

void printData(FILE* stream, int data_start, int data_end, int lst) {

    //Variable Declarations
    int consecZero = 0;
    int printedChars = 0;

    for (int i = data_start; i < data_end; i++) {
        if (memory.words[i] == 0) {
            if (consecZero == 0) {//First time a 0 is encountered
                printedChars += fprintf(stream, "%8d: %d\t", i, memory.words[i]);
                consecZero = 1;
            } else if (consecZero == 1) {// Start of a zero loop
                printedChars += fprintf(stream, "        ...     ");
                consecZero = 2;//Don't print another "..."
            }
        }
        else {// Outside of zero loop
            printedChars += fprintf(stream, "%8d: %d\t", i, memory.words[i]);
            consecZero = 0;//Reset consecZero
        }

        if (printedChars > 59) {// Print a newline after 59 chars then reset
            fprintf(stream, "\n");
            printedChars = 0;
        }
    }
    fprintf(stream, "\n");
}

void handleBOFFile(char * file_name, int should_print) {
    char base_name[256];
    strncpy(base_name, file_name, sizeof(base_name));
    base_name[sizeof(base_name) - 1] = '\0';  // Ensure null termination

    // Find the last occurrence of ".bof" and terminate the string there
    char *dot = strrchr(base_name, '.');
    if (dot != NULL && strcmp(dot, ".bof") == 0) {
        *dot = '\0';  // Remove the ".bof" extension
    }

    BOFFILE file = bof_read_open(file_name);
    BOFHeader header = bof_read_header(file);

    if (!bof_has_correct_magic_number(header)) {
        fprintf(stderr, "Error: Invalid magic number in BOF file.\n");
        bof_close(file);
        return;
    }

    openTraceFile(base_name);  // Pass the modified base name to openTraceFile
    int length = header.text_length;

    readInInstructions(length, file);
    globalHeader = header;
    initRegisters(bof_read_open(file_name));

    if (should_print) {
        printInstructions(length);
        printData(stdout, globalHeader.data_start_address,  GPR[SP]-1, 1);
    }


    processInstructions(length);


    bof_close(file);
}

int main(int argc, char **argv) {

    int shouldPrint = false;
    char *fileName;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        shouldPrint = true;  // Enable printing
        fileName = argv[2];   // File name is the second argument
    }
    else if (argc == 2) {
        fileName = argv[1];   // File name is the first argument
    }
    else {
        fprintf(stderr, "Usage: %s [-p] <BOF file>\n", argv[0]);
        return 1;  // Return non-zero for usage error
    }

    handleBOFFile(fileName, shouldPrint);// Begin reading the BOF file and handle printing if enabled

    return 0;
}