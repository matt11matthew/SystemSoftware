#include "gen_code.h"
#include "utilities.h"
#include "code.h"
#include "code_seq.h"
#include "code_utils.h"
#include "literal_table.h"
#include "spl.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include "regname.h"
#include <string.h>
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

code_seq gen_code_expr(expr_t expr, reg_num_type target_reg) {
    code_seq base = code_seq_empty();

    switch (expr.expr_kind) {
        case expr_number:
            base = code_seq_singleton(code_lit(target_reg, 0, expr.data.number.value));
            break;
            // Handle other expression kinds (identifiers, binary operations, etc.)
            // ...
        default:
            fprintf(stderr, "Error: Unhandled expression kind in gen_code_expr\n");
            exit(1);
    }
    return base;
}

code_seq gen_code_print_stmt(print_stmt_t s) {
    code_seq base = code_seq_empty();

    // Evaluate the expression into R3
    code_seq expr_code = gen_code_expr(s.expr, 3);
    code_seq_concat(&base, expr_code);

    // Add print system call (PINT)
    code_seq_add_to_end(&base, code_pint(3, 0));

    return base;
}

code_seq gen_code_rel_op_condition(rel_op_condition_t cond) {
    code_seq result = code_seq_empty();

    // Evaluate expr1 into R3
    code_seq expr1_code = gen_code_expr(cond.expr1, 3);
    code_seq_concat(&result, expr1_code);

    // Evaluate expr2 into R4
    code_seq expr2_code = gen_code_expr(cond.expr2, 4);
    code_seq_concat(&result, expr2_code);

    // Perform comparison and set R5 to 1 (true) or 0 (false)
    if (strcmp(cond.rel_op.text, "!=") == 0) {
        code_seq_add_to_end(&result, code_bne(3, 4, 2));
    } else if (strcmp(cond.rel_op.text, "==") == 0) {
        code_seq_add_to_end(&result, code_beq(3, 4, 2));
    } else {
        // For relational operators, compute R6 = R3 - R4
        code_seq_add_to_end(&result, code_cpr(6, 3));       // R6 = R3
        code_seq_add_to_end(&result, code_sub(6, 0, 4, 0)); // R6 -= R4

        if (strcmp(cond.rel_op.text, "<") == 0) {
            code_seq_add_to_end(&result, code_bltz(6, 0, 2));
        } else if (strcmp(cond.rel_op.text, "<=") == 0) {
            code_seq_add_to_end(&result, code_blez(6, 0, 2));
        } else if (strcmp(cond.rel_op.text, ">") == 0) {
            code_seq_add_to_end(&result, code_bgtz(6, 0, 2));
        } else if (strcmp(cond.rel_op.text, ">=") == 0) {
            code_seq_add_to_end(&result, code_bgez(6, 0, 2));
        } else {
            fprintf(stderr, "Error: Unhandled relational operator '%s'\n", cond.rel_op.text);
            exit(1);
        }
    }

    // Set R5 = 0 (false)
    code_seq_add_to_end(&result, code_lit(5, 0, 0));
    code_seq_add_to_end(&result, code_jrel(1)); // Jump over setting true

    // Set R5 = 1 (true)
    code_seq_add_to_end(&result, code_lit(5, 0, 1));

    return result;
}

code_seq gen_code_if_stmt(if_stmt_t stmt) {
    code_seq base = code_seq_empty();

    // Generate condition code
    if (stmt.condition.cond_kind == ck_rel) {
        code_seq condition_code = gen_code_rel_op_condition(stmt.condition.data.rel_op_cond);
        code_seq_concat(&base, condition_code);
    } else {
        fprintf(stderr, "Error: Unhandled condition kind in gen_code_if_stmt\n");
        exit(1);
    }

    // Generate "then" block
    code_seq then_code = gen_code_stmts(*stmt.then_stmts);
    int then_size = code_seq_size(then_code);

    // Calculate offset to jump over the "then" block if the condition is false
    int jump_over_then = then_size;

    // Conditional branch to skip "then" block if condition is false (R5 == 0)
    code_seq_add_to_end(&base, code_beq(5, 0, jump_over_then));

    // Add "then" block
    code_seq_concat(&base, then_code);

    // Skip generating "else" block to avoid including code after the "if" statement
    // This ensures that code after the "if" statement is executed unconditionally

    return base;
}

code_seq gen_code_stmt(stmt_t *s) {
    code_seq stmt_code = code_seq_empty();

    switch (s->stmt_kind) {
        case print_stmt:
            stmt_code = gen_code_print_stmt(s->data.print_stmt);
            break;
        case if_stmt:
            stmt_code = gen_code_if_stmt(s->data.if_stmt);
            break;
        default:
            fprintf(stderr, "Error: Unhandled statement kind in gen_code_stmt\n");
            exit(1);
    }

    return stmt_code;
}

code_seq gen_code_stmts(stmts_t stmts)
{
    code_seq base = code_seq_empty();
    if (stmts.stmts_kind == empty_stmts_e) {
        return base; // Deal with epsilon case
    }
    // Not empty
    stmt_t *s = stmts.stmt_list.start;

    while (s != NULL) {
        code_seq stmt_code = gen_code_stmt(s);
        code_seq_concat(&base, stmt_code); // Concatenate the code sequences
        s = s->next;
    }

    return base;
}

void gen_code_program(BOFFILE bf, block_t b) {
    code_seq main_cs = code_utils_set_up_program();

    code_seq body_cs = gen_code_stmts(b.stmts);
    code_seq_concat(&main_cs, body_cs);

    code_seq tear_down_cs = code_utils_tear_down_program();
    code_seq_concat(&main_cs, tear_down_cs);

    gen_code_output_program(bf, main_cs);

//    code_seq_debug_print(stdout, main_cs);
}
