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

    // Example initialization for the other fields (customize these values as needed)
    header.text_start_address = 0; // Starting address for the text section
    header.text_length = 1;           // Length of the text section in words
    header.data_start_address = 1024; // Starting address for the data section
    header.data_length = 1024;            // Length of the data section in words
    header.stack_bottom_addr = 0;  // Address of the bottom of the stack
    // code_seq_debug_print(stdout, code_seq_singleton(code_exit(0)));
    // Write the header to the file
    //
    bof_write_header(bf, header);
    bof_write_word(bf,code_exit(0));
    // code_add()
    printf("word: %u", code_exit(0));
    // bof_write
}

