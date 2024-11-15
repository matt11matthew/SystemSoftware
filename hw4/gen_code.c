#include "gen_code.h"
#include "gen_code.h"
#include "utilities.h"
#include "utilities.h"
#include "code.h"
#include "code_seq.h"
#include "code_utils.h"

#define STACK_SPACE 4096

void gen_code_initialize() {

}

/*BOFHeader bh;
bof_write_magic_to_header(&bh);
bh.text_start_address = addr2address(prog.textSection.entryPoint);
bh.text_length = ast_list_length(prog.textSection.instrs.instrs);
bh.data_start_address = prog.dataSection.static_start_addr;
// have to write the header first, so need to know the true size
// of the data section before writing the header
bh.data_length = assemble_dataSection_words(prog.dataSection);
bh.stack_bottom_addr = prog.stackSection.stack_bottom_addr;
bof_write_header(bf, bh);
assembleTextSection(bf, prog.textSection);
assembleDataSection(bf, prog.dataSection);*/

BOFHeader gen_code_program_header(code_seq main_cs)
{
    BOFHeader ret;

    bof_write_magic_to_header(&ret);


    ret.text_start_address = 0;
    ret.text_length = code_seq_size(main_cs) * BYTES_PER_WORD;
    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    ret.data_start_address = dsa;

    // ret.ints_length = 0; // FLOAT has no int literals
    // ret.floats_length = literal_table_size() * BYTES_PER_WORD;

    int sba = dsa
    + ret.data_start_address
    +  STACK_SPACE;
    ret.stack_bottom_addr = sba;
    return ret;
}
static void gen_code_output_seq(BOFFILE bf, code_seq cs)
{
    while (!code_seq_is_empty(cs)) {
        bin_instr_t inst = code_seq_first(cs)->instr;
        instruction_write_bin_instr(bf, inst);
        cs = code_seq_rest(cs);
    }
}

void gen_code_output_program(BOFFILE bf, code_seq main_cs)
{
    BOFHeader bfh = gen_code_program_header(main_cs);
    bof_write_header(bf, bfh);
     gen_code_output_seq(bf, main_cs);
    // gen_code_output_literals(bf);
     bof_close(bf);
}
void gen_code_program(BOFFILE bf, block_t b) {



    code_seq main_cs;
    // We want to make the main program's AR look like all blocks... so:
    // allocate space and initialize any variables
    // main_cs = gen_code_var_decls(prog.var_decls);
    int vars_len_in_bytes
    = (code_seq_size(main_cs) / 2) * BYTES_PER_WORD;

    // there is no static link for the program as a whole,
    // so nothing to do for saving FP into A0 as would be done for a block
    code_seq_concat(&main_cs, code_utils_save_registers_for_AR());

    // main_cs= code_seq_concat(main_cs,
    //           gen_code_stmt(prog.stmt));

     code_seq_concat(&main_cs,
                  code_utils_restore_registers_from_AR());
    code_seq_concat(&main_cs,
                  code_utils_deallocate_stack_space(vars_len_in_bytes));
    code_seq_add_to_end(&main_cs, code_exit(0));


    gen_code_output_program(bf, main_cs);
}

