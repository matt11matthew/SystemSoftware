#include "gen_code.h"
#include "gen_code.h"
#include "utilities.h"
#include "utilities.h"
#include "code.h"
#include "code_seq.h"
#include "code_utils.h"

void gen_code_initialize() {

}
void gen_code_program(BOFFILE bf, block_t b) {
    BOFHeader header;

    // Set the magic number in the header
    bof_write_magic_to_header(&header);

    // Set text section parameters
    header.text_start_address = 0;
    header.text_length =1; // Set to the actual instruction size

    // Add a buffer to separate the text and data sections
    header.data_start_address = header.text_start_address + header.text_length + 4; // Add 4 bytes of buffer space
    header.data_length = 1024;

    // Set stack bottom address below the data section end
    header.stack_bottom_addr = header.data_start_address + header.data_length + 100; // Add some buffer space for the stack

    // Write header and single exit instruction
    bof_write_header(bf, header);
    instruction_write_bin_instr(bf, code_exit(0)->instr);
}

