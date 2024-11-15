#include "gen_code.h"
#include "utilities.h"
#include "code.h"
#include "code_seq.h"
#include "code_utils.h"
#include "literal_table.h"

#define STACK_SPACE 4096

void gen_code_initialize() {
    literal_table_initialize();
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

    ret.text_length = count;

    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    ret.data_start_address = dsa;
    ret.data_length = literal_table_size() * BYTES_PER_WORD;
    int sba = dsa
    + ret.data_start_address
    + ret.data_length + STACK_SPACE;
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
code_seq gen_code_print_stmt(print_stmt_t s, code_seq base) {

   // code_pstr(code_utils_())
    return base;
}
code_seq gen_code_stmt(stmt_t *s, code_seq base) {
    switch (s->stmt_kind) {
        case print_stmt:
            base = gen_code_print_stmt(s->data.print_stmt, base);
            break;
    }
    return base;
}

code_seq gen_code_stmts(stmts_t stmts)
{

    code_seq base = code_seq_empty();
    if (stmts.stmts_kind == empty_stmts_e) {
        return base; //Deal with epsilon case
    }
    //Not empty
    stmt_t *s = stmts.stmt_list.start;

    while (s != NULL) {
        base = gen_code_stmt(s, base);
        s = s->next;
    }

    return base;
}

void gen_code_program1(BOFFILE bf, block_t b)
{
    code_seq main_cs = code_utils_set_up_program();  // Initialize main_cs to an empty sequence

    // We want to make the main program's AR look like all blocks... so:
    // allocate space and initialize any variables
    // main_cs = gen_code_var_decls(prog.var_decls);
    int vars_len_in_bytes
    = (code_seq_size(main_cs) / 2) * BYTES_PER_WORD;
    // there is no static link for the program as a whole,
    // so nothing to do for saving FP into A0 as would be done for a block
    code_seq_concat(&main_cs, code_utils_save_registers_for_AR());
    code_seq_concat(&main_cs, gen_code_stmts(b.stmts));
    code_seq_concat(&main_cs,
                  code_utils_restore_registers_from_AR());
    code_seq_concat(&main_cs,
                  code_utils_deallocate_stack_space(vars_len_in_bytes));
    code_seq_add_to_end(&main_cs, code_exit(0));
    gen_code_output_program(bf, main_cs);

    code_seq_debug_print(stdout, main_cs);
}


void gen_code_program(BOFFILE bf, block_t b) {
    // code_seq main_cs =  code_utils_set_up_program();  // Initialize main_cs to an empty sequence
    code_seq main_cs = code_utils_set_up_program();  // Initialize main_cs to an empty sequence

    // Generate code for variable declarations (uncomment and implement if necessary)
    // main_cs = gen_code_var_decls(prog.var_decls);

  //  int vars_len_in_bytes = (code_seq_size(main_cs) / 2) * BYTES_PER_WORD;

    // Save registers for the AR setup
   // code_seq_concat(&main_cs, code_utils_save_registers_for_AR());
    code_seq_concat(&main_cs, code_utils_save_registers_for_AR());
    // Generate code for main statement block (uncomment and implement if necessary)
    code_seq_concat(&main_cs, gen_code_stmts(b.stmts));

  //  code_seq_concat(&main_cs, code_utils_restore_registers_from_AR());
   // code_seq_concat(&main_cs, code_utils_deallocate_stack_space(vars_len_in_bytes));

    code_seq_add_to_end(&main_cs, code_exit(0));  // Ensure this adds an exit instruction

    gen_code_output_program(bf, main_cs);


    code_seq_debug_print(stdout, main_cs);
}
