#include "gen_code.h"
#include "utilities.h"
#include "code.h"
#include "code_seq.h"
#include "code_utils.h"
#include "literal_table.h"

#define STACK_SPACE 4096

void gen_code_initialize() {
    // Initialization logic, if necessary
}

void gen_code_output_literals(BOFFILE bf)
{
    literal_table_start_iteration();
    while (literal_table_iteration_has_next()) {
        word_type w = literal_table_iteration_next();
        // debug_print("Writing literal %f to BOF file\n", w);
        bof_write_word(bf, w);
    }
    literal_table_end_iteration(); // not necessary
}

int gen_code_output_seq_count(code_seq cs) {


    int res = 0;

    while (!code_seq_is_empty(cs)) {
        bin_instr_t inst = code_seq_first(cs)->instr;
        res++;
        cs = code_seq_rest(cs);


    }
    return res;
}

BOFHeader gen_code_program_header(code_seq main_cs) {
    BOFHeader ret;
    bof_write_magic_to_header(&ret);

    ret.text_start_address = 0;

    int count = gen_code_output_seq_count(main_cs);
    // printf("Count Sequence: %d\n", count);

    //int textLength = BYTES_PER_WORD * code_seq_size(main_cs);

    ret.text_length = count;

    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    // printf("DSA: %d\n", dsa);
    ret.data_start_address = dsa;

  ret.data_length = dsa;
  int sba = dsa + ret.data_start_address + STACK_SPACE;
  // printf("sba: %d\n", sba);
  ret.stack_bottom_addr = sba;
  return ret;
}

void gen_code_output_seq(BOFFILE bf, code_seq cs) {

    while (!code_seq_is_empty(cs)) {
        bin_instr_t inst = code_seq_first(cs)->instr;
        instruction_write_bin_instr(bf, inst);
        //printf(" %s\n", instruction_assembly_form(0, inst) );
        cs = code_seq_rest(cs);
    }
}

void gen_code_output_program(BOFFILE bf, code_seq main_cs) {
    BOFHeader bfh = gen_code_program_header(main_cs);
    bof_write_header(bf, bfh);
    gen_code_output_seq(bf, main_cs);
    gen_code_output_literals(bf);
    bof_close(bf);
}

code_seq gen_code_stmt(stmts_t stmt)
{

    // The following can never execute, but this quiets gcc's warning
    return code_seq_empty();
}

void gen_code_program(BOFFILE bf, block_t b) {
    // code_seq main_cs =  code_utils_set_up_program();  // Initialize main_cs to an empty sequence
    code_seq main_cs = code_seq_empty();  // Initialize main_cs to an empty sequence

    // Generate code for variable declarations (uncomment and implement if necessary)
    // main_cs = gen_code_var_decls(prog.var_decls);

  //  int vars_len_in_bytes = (code_seq_size(main_cs) / 2) * BYTES_PER_WORD;

    // Save registers for the AR setup
   // code_seq_concat(&main_cs, code_utils_save_registers_for_AR());

    // Generate code for main statement block (uncomment and implement if necessary)
    //code_seq_concat(&main_cs, gen_code_stmt(b.stmts));

  //  code_seq_concat(&main_cs, code_utils_restore_registers_from_AR());
   // code_seq_concat(&main_cs, code_utils_deallocate_stack_space(vars_len_in_bytes));

    code_seq_add_to_end(&main_cs, code_exit(0));  // Ensure this adds an exit instruction

    gen_code_output_program(bf, main_cs);


    code_seq_debug_print(stdout, main_cs);
}
