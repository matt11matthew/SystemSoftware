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
//    literal_table_test();
}

code_seq push_reg_on_stack(reg_num_type reg, offset_type offset) {
    return code_seq_singleton(code_cpw(SP, 0, reg, offset));
}

void gen_code_output_literals(BOFFILE bf) {
    literal_table_start_iteration();
    int count = 0;
    while (literal_table_iteration_has_next()) {
        word_type w = literal_table_iteration_next();
        //printf("Literal[%d] %s: %u\n", count++, "?",  w);
        // debug_print("Writing literal %f to BOF file\n", w);
        bof_write_word(bf, w);
    }
//    literal_table_end_iteration(); // not necessary
    //printf("Total literals written: %d\n", count);
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
    // remember, the unit of length in the BOF format is a byte!
    ret.text_length = code_seq_size(main_cs);
    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    ret.data_start_address = dsa;
    ret.data_length = literal_table_size() ;
    int sba = dsa
              + ret.data_start_address
              + ret.data_length + STACK_SPACE;
    ret.stack_bottom_addr = sba;

//    ret.text_start_address = 0;
//
//    int count = gen_code_output_seq_count(main_cs);
//
//    ret.text_length = count;
//
//    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
//    ret.data_start_address = dsa;
//    ret.data_length = literal_table_size() * BYTES_PER_WORD;
//    int sba = dsa
//              + ret.data_start_address
//              + ret.data_length + STACK_SPACE;
//    //Remember to add total amount of literals in table
//    ret.stack_bottom_addr = dsa + ret.data_length + STACK_SPACE;

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

code_seq gen_code_rel_op(token_t rel_op) {

//    switch(rel_op.code){
//        case eqsym:
//            break;
//        case neqsym:
//            break;
//
//        default:
//            break;
//    }

    return code_seq_empty();
}

/*
 * code_seq gen_code_expr_bin(char* varName, binary_op_expr_t expr, reg_num_type target_reg){
    code_seq ret = gen_code_expr(varName, *expr.expr1,target_reg);

    code_seq_concat(&ret, gen_code_expr(varName,*(expr.expr2), target_reg));
    // check the types match
//    type_exp_e t1 = ast_expr_type(*(exp.expr1));
//    assert(ast_expr_type(*(expr.expr2)) == t1);
    // do the operation, putting the result on the stack
    code_seq_concat(&ret, gen_code_op(expr.arith_op));
    return ret;
}
 */
code_seq gen_code_arith_op(token_t rel_op) {
    code_seq base = code_seq_empty();
    switch (rel_op.code) {
        case plussym:
            code_seq_add_to_end(&base, code_add( SP, 0,GP, 0));
            //do_op = code_seq_singleton(code_add(SP, 0, SP, 1));
            break;
        case minussym:
            break;
        case multsym:
            break;
        case divsym:
            code_seq_add_to_end(&base, code_div( GP, 0));
            code_seq_add_to_end(&base, code_cflo( SP, 0));
            break;
        default:
            return base;
    }
    return base;
}
//    switch (rel_op.code) {
//        case plussym:
//            code_seq_add_to_end(&do_op, code_add(GP, 0, GP, 1));
//            //do_op = code_seq_singleton(code_add(SP, 0, SP, 1));
//            break;
////        case minussym:
////            do_op = code_seq_add_to_end(do_op, code_fsub(V0, AT, V0));
////            break;
////        case multsym:
////            do_op = code_seq_add_to_end(do_op, code_fmul(V0, AT, V0));
////            break;
////        case divsym:
////            do_op = code_seq_add_to_end(do_op, code_fdiv(V0, AT, V0));
////            break;
//        default:
//            bail_with_error("Unexpected arithOp (%d) in gen_code_arith_op", rel_op.code);
//            break;
//    }
//    return do_op;


code_seq gen_code_op(token_t op) {
    switch (op.code) {
        case eqsym: case neqsym:
        case ltsym: case leqsym:
        case gtsym: case geqsym:
            return gen_code_rel_op(op);
            break;
        case plussym: case minussym: case multsym: case divsym:
            return gen_code_arith_op(op);
            break;
        default:
            bail_with_error("Unknown token code (%d) in gen_code_op", op.code);
            break;
    }
    return code_seq_empty();
}

code_seq gen_code_expr_bin(binary_op_expr_t expr){
    code_seq seq = code_seq_empty();
    if (expr.arith_op.code==plussym) {
        code_seq_concat(&seq, gen_code_expr(*expr.expr1));
        code_seq_concat(&seq, gen_code_expr(*expr.expr2));
        code_seq_concat(&seq, gen_code_arith_op(expr.arith_op));
    }else if (expr.arith_op.code==divsym){ //2/1
        code_seq_concat(&seq, gen_code_expr(*expr.expr2));
        code_seq_concat(&seq, gen_code_expr(*expr.expr1));
        code_seq_concat(&seq, gen_code_arith_op(expr.arith_op));
    } else {
//        code_seq_concat(&seq, gen_code_expr(*expr.expr1));
//        code_seq_concat(&seq, gen_code_expr(*expr.expr2));
//        code_seq_concat(&seq, gen_code_arith_op(expr.arith_op));
    }

    return seq;
}

code_seq gen_code_ident(ident_t ident) {

//    gen_code_number(ident.name,ident);
//    printf("GEN CODE _IDENT: OFFSET %s: %d\n", ident.name, ident.idu->levelsOutward);

    int offset = literal_table_lookup(ident.name, -3223);
    return push_reg_on_stack(GP, offset);
}

//PUSHES TO HEAD OF STACK
code_seq gen_code_expr(expr_t exp) {
    switch (exp.expr_kind) {
        case expr_ident:
            return gen_code_ident(exp.data.ident);
        case expr_bin:
            return gen_code_expr_bin(exp.data.binary);
        case expr_negated:
//            return gen_code_expr(*exp.data.negated.expr);
            return gen_code_number(NULL, exp.data.negated.expr->data.number,true);
        case expr_number:
            return gen_code_number(NULL, exp.data.number,false);
        default:
            bail_with_error("Unexpected expr_kind_e (%d) in gen_code_expr", exp.expr_kind);
            break;
    }
    // never happens, but suppresses a warning from gcc
    return code_seq_empty();
}

code_seq gen_code_number( char* varName, number_t num, bool negate) {

    word_type val = num.value;

//
    if (negate) {
//        if (num.value > 0) {
//            val = (word_type)(-((int)num.value));
//        } else {
//            val = num.value;
//        }
        val = -(num.value);
    } else {
        val = num.value;
    }

    printf("GEN CODE NUm: %s %d\n", varName, val);


    if (varName==NULL){
//        unsigned int global_offset
//                = literal_table_lookup(num.text, num.value);

        return code_seq_singleton(code_lit(SP, 0, val));
    }
    unsigned int global_offset
            = literal_table_lookup(varName, val);

    return push_reg_on_stack(GP, global_offset);
}

code_seq gen_code_print_stmt(print_stmt_t s) {
    code_seq base = code_seq_empty();

    // Evaluate the expression into R3
    code_seq expr_code = gen_code_expr(s.expr);
    code_seq_concat(&base, expr_code);

    code_seq_add_to_end(&base, code_pint(SP,0 ));
    // Add print system call (PINT)
//    if (s.expr.expr_kind == expr_ident) {
//        int offset = literal_table_lookup(s.expr.data.ident.name, 0);
//        printf("L: %s %d\n", s.expr.data.ident.name,offset);
//
////        code_seq_add_to_end(&base, code_pint(SP,offset));
//        code_seq_add_to_end(&base, code_pint(SP,        id_use_get_attrs(s.expr.data.ident.idu)->offset_count));
//
//    } else {
//        code_seq_add_to_end(&base, code_pint(SP,0 ));
//    }

    return base;
}

code_seq gen_code_if_ck_db(db_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();
    printf("db_condition_t");
    return base;
}

code_seq gen_code_if_ck_rel(rel_op_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();

    code_seq_concat(&base, gen_code_expr(stmt.expr2));
    code_seq_add_to_end(&base, code_cpw(FP, 0, SP,0));

    code_seq_concat(&base, gen_code_expr(stmt.expr1));


    code_seq_add_to_end(&base,  code_sub(SP, 0,FP, 0 )); //PUTS subtracted value into SP

    if (strcmp(stmt.rel_op.text, "<")==0) {
        code_seq_add_to_end(&base, code_bgtz(SP,0,thenSize+2));
    }


    return base;
}

code_seq gen_code_assign_stmt(assign_stmt_t stmt){
    code_seq base = code_seq_empty();




    int offset = literal_table_lookup(stmt.name,0);

    code_seq_concat(&base,gen_code_expr(*stmt.expr));
//    unsigned int offset_count = id_use_get_attrs(stmt.idu)->offset_count;
//    printf("THE OFFSET: %u\n",offset_count);

    //PULL FROM FRONT
    code_seq_add_to_end(&base, code_cpw(GP, offset, SP,0));


    return base;
}

code_seq gen_code_if_stmt(if_stmt_t stmt) {
    code_seq base = code_seq_empty();

    condition_t c = stmt.condition;

    code_seq thenSeq = gen_code_stmts(*stmt.then_stmts);
    code_seq elseSeq = gen_code_stmts(*stmt.else_stmts);

    int thenSeqLength = code_seq_size(thenSeq);
    int elseSeqLength = code_seq_size(elseSeq);


    if (c.cond_kind == ck_db) {
        code_seq_concat(&base, gen_code_if_ck_db(c.data.db_cond,thenSeqLength));
    }
    if (c.cond_kind == ck_rel) {
        code_seq_concat(&base, gen_code_if_ck_rel(c.data.rel_op_cond,thenSeqLength));
    }

    code_seq_concat(&base, thenSeq);
    code_seq_add_to_end(&base, code_jrel(elseSeqLength + 1));
    code_seq_concat(&base, elseSeq);

    return base;
}

code_seq gen_code_call_stmt(call_stmt_t stmt) {
    code_seq base = code_seq_empty();
    return base;
}

code_seq gen_code_while_stmt(while_stmt_t stmt){
    code_seq base = code_seq_empty();
    return base;
}

code_seq gen_code_read_stmt(read_stmt_t stmt) {
    code_seq base = code_seq_empty();
    return base;
}

code_seq gen_code_block_stmt(block_stmt_t stmt) {
    code_seq base = code_seq_empty();
    return base;
}

code_seq gen_code_stmt(stmt_t *s) {
    code_seq stmt_code = code_seq_empty();

    switch (s->stmt_kind) {
        case assign_stmt:
            stmt_code = gen_code_assign_stmt(s->data.assign_stmt);
            break;
        case call_stmt:
            stmt_code = gen_code_call_stmt(s->data.call_stmt);
            break;
        case if_stmt:
            stmt_code = gen_code_if_stmt(s->data.if_stmt);
            break;
        case while_stmt:
            stmt_code = gen_code_while_stmt(s->data.while_stmt);
            break;
        case read_stmt:
            stmt_code = gen_code_read_stmt(s->data.read_stmt);
            break;
        case print_stmt:
            stmt_code = gen_code_print_stmt(s->data.print_stmt);
            break;
        case block_stmt:
            stmt_code = gen_code_block_stmt(s->data.block_stmt);
            break;
        default:
            fprintf(stderr, "Error: Unhandled statement kind in gen_code_stmt\n");
            exit(1);
    }

    return stmt_code;
}

code_seq gen_code_const(const_def_t*  def) {
    code_seq base = code_seq_empty();

    while (def!=NULL) {
        //PROCESS;

//        printf("\n-0-0-0-0-\nC NAME: %s (%s)\n-0-0-0-0-\n",cName, def->ident.name);

        bool negate = false;
        code_seq_concat(&base, gen_code_number( def->ident.name,  def->number, negate));
        def = def->next;
    }
    //Handle single for now
    // literal_table_debug_print();
//    int v =     literal_table_lookup(def.ident.name,20);

//    printf("%s=%d\n", def.ident.name, def.number.value);
//    code_seq_add_to_end(&base, code_lit(GP,    literal_table_lookup(def.ident.name,  def.number.value),  def.number.value));

    return base;
}

code_seq gen_code_consts(const_decls_t  decls) {
    code_seq base = code_seq_empty();

    const_decl_t *start = decls.start;
    while (start != NULL) {
        const_def_t* d = start->const_def_list.start;
        while (d != NULL) {
            code_seq_concat(&base,gen_code_const(d));
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
    code_seq_add_to_end(&main_cs, code_exit(0));
    code_seq tear_down_cs = code_utils_tear_down_program(); //BROKEN
    // code_seq_concat(&main_cs, tear_down_cs);

    gen_code_output_program(bf, main_cs);

    literal_table_debug_print();
//    code_seq_debug_print(stdout, main_cs);
}