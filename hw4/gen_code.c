#include "gen_code.h"
#include "utilities.h"
#include "code.h"
#include "code_seq.h"
#include "code_utils.h"

#define STACK_SPACE 4096

void gen_code_initialize() {
    // Initialization logic, if necessary
}

BOFHeader gen_code_program_header(code_seq main_cs) {
    BOFHeader ret;
    bof_write_magic_to_header(&ret);

    ret.text_start_address = 0;
    ret.text_length = code_seq_size(main_cs) * BYTES_PER_WORD;
    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    ret.data_start_address = dsa;

    int sba = dsa + ret.data_start_address + STACK_SPACE;
    ret.stack_bottom_addr = sba;
    return ret;
}

void gen_code_output_seq(BOFFILE bf, code_seq cs) {
    while (!code_seq_is_empty(cs)) {
        bin_instr_t inst = code_seq_first(cs)->instr;
        instruction_write_bin_instr(bf, inst);
        cs = code_seq_rest(cs);
    }
}

void gen_code_output_program(BOFFILE bf, code_seq main_cs) {
    BOFHeader bfh = gen_code_program_header(main_cs);
    bof_write_header(bf, bfh);
    gen_code_output_seq(bf, main_cs);
    bof_close(bf);
}

void gen_code_program(BOFFILE bf, block_t b) {
    code_seq main_cs = code_seq_empty();  // Initialize main_cs to an empty sequence

    // Generate code for variable declarations (uncomment and implement if necessary)
    // main_cs = gen_code_var_decls(prog.var_decls);

    int vars_len_in_bytes = (code_seq_size(main_cs) / 2) * BYTES_PER_WORD;

    // Save registers for the AR setup
    code_seq_concat(&main_cs, code_utils_save_registers_for_AR());

    // Generate code for main statement block (uncomment and implement if necessary)
    // main_cs = code_seq_concat(main_cs, gen_code_stmt(prog.stmt));

    code_seq_concat(&main_cs, code_utils_restore_registers_from_AR());
    code_seq_concat(&main_cs, code_utils_deallocate_stack_space(vars_len_in_bytes));
    code_seq_add_to_end(&main_cs, code_exit(0));  // Ensure this adds an exit instruction

    gen_code_output_program(bf, main_cs);
}
