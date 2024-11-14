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
    header.text_length = sizeof(instruction_t); // Set to the actual instruction size, e.g., sizeof(code_exit(0)->instr)

    // Set data section parameters
    header.data_start_address = header.text_start_address + header.text_length;
    header.data_length = 1024;
    header.stack_bottom_addr = header.data_start_address + header.data_length;

    // Write header and single exit instruction
    bof_write_header(bf, header);
    instruction_write_bin_instr(bf, code_exit(0)->instr);
}

