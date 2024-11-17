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
    //Remember to add total amount of literals in table
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
        case expr_bin:
            printf("bin stmt\n");
            break;
        case expr_negated:
            printf("negated stmt\n");
            break;
        case expr_ident:

            base = code_seq_singleton(code_cpr(target_reg, 0, expr.data.number.value));



            break;
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
    code_seq expr_code = gen_code_expr(s.expr, SP);
    code_seq_concat(&base, expr_code);

    // Add print system call (PINT)
    code_seq_add_to_end(&base, code_pint(SP, 0));

    return base;
}

code_seq gen_code_if_ck_db(db_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();
    printf("db_condition_t");
    return base;
}

code_seq gen_code_if_ck_rel(rel_op_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();

    code_seq_concat(&base, gen_code_expr(stmt.expr1, SP));
    code_seq_concat(&base, gen_code_expr(stmt.expr2, FP));

    code_seq_add_to_end(&base,  code_sub(SP, 0, FP, 0)); //PUTS subtracted value into SP

    if (strcmp(stmt.rel_op.text, "<")==0) {
        code_seq_add_to_end(&base, code_bgtz(SP,0,thenSize+2));
    }


    return base;
}


code_seq gen_code_if_stmt(if_stmt_t stmt) {
    code_seq base = code_seq_empty();

    condition_t c = stmt.condition;

    code_seq thenSeq = gen_code_stmts(*stmt.then_stmts);
    code_seq elseSeq = gen_code_stmts(*stmt.else_stmts);

    int thenSeqLength = code_seq_size(thenSeq);
    int elseSeqLength = code_seq_size(elseSeq);


    if (c.cond_kind==ck_db) {
        code_seq_concat(&base, gen_code_if_ck_db(c.data.db_cond,thenSeqLength));
    }
    if (c.cond_kind==ck_rel) {
        code_seq_concat(&base, gen_code_if_ck_rel(c.data.rel_op_cond,thenSeqLength));
    }


    code_seq_concat(&base, thenSeq);
    code_seq_add_to_end(&base, code_jrel(elseSeqLength + 1));

    code_seq_concat(&base, elseSeq);

    return base;
}

code_seq gen_code_stmt(stmt_t *s) {
    code_seq stmt_code = code_seq_empty();

    switch (s->stmt_kind) {
        case assign_stmt:
            printf("Constant definitions\n");
            break;
        case call_stmt:
            printf("call stmt\n");
            break;
        case if_stmt:
            stmt_code = gen_code_if_stmt(s->data.if_stmt);
            break;
        case while_stmt:
            printf("While stmt");
            break;
        case read_stmt:
            printf("Read definitions\n");
            break;
        case print_stmt:
            stmt_code = gen_code_print_stmt(s->data.print_stmt);
            break;
        case block_stmt:
            printf("block stmt\n");
            break;
        default:
            fprintf(stderr, "Error: Unhandled statement kind in gen_code_stmt\n");
            exit(1);
    }

    return stmt_code;
}

code_seq gen_code_const(const_def_t  def) {
    code_seq base = code_seq_empty();

    //Handle single for now

     // printf("%s=%d\n", def.ident.name, def.number.value);
    code_seq_add_to_end(&base, code_lit(GP,    literal_table_lookup(def.ident.name,  def.number.value),  def.number.value));

    return base;
}

code_seq gen_code_consts(const_decls_t  decls) {

    code_seq base = code_seq_empty();

    const_decl_t *start = decls.start;
    while (start != NULL) {
        const_def_t* d = start->const_def_list.start;
        while (d != NULL) {
            code_seq_concat(&base,gen_code_const(*d));
            d = d->next;
        }
        start= start->next;
    }

    return base;
}

code_seq gen_code_stmts(stmts_t stmts) {
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

    code_seq_concat(&main_cs, gen_code_consts(b.const_decls));

    code_seq body_cs = gen_code_stmts(b.stmts);
    code_seq_concat(&main_cs, body_cs);

    code_seq tear_down_cs = code_utils_tear_down_program();
    code_seq_concat(&main_cs, tear_down_cs);

    gen_code_output_program(bf, main_cs);

    code_seq_debug_print(stdout, main_cs);
}
