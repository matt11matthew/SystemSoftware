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


    header.text_start_address = 0;
    header.text_length = 1;
    header.data_start_address = 1024;
    header.data_length = 1024;
    header.stack_bottom_addr = 0;
    // code_seq_debug_print(stdout, code_seq_singleton(code_exit(0)));

    bof_write_header(bf, header);
    bof_write_word(bf,code_exit(0));
    bof_write_word(bf,);
//Reference HW1 
}

