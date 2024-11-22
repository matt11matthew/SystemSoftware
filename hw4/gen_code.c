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

/*
 code_seq gen_code_rel_op(token_t rel_op, type_exp_e typ)
{
    // load top of the stack (the second operand) into AT
    code_seq ret = code_pop_stack_into_reg(AT, typ);
    // load next element of the stack into V0
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0, typ));

    // start out by doing the comparison
    // and skipping the next 2 instructions if it's true
    code_seq do_op = code_seq_empty();
    switch (rel_op.code) {
    case eqsym:
	if (typ == float_te) {
	    do_op = code_seq_singleton(code_bfeq(V0, AT, 2));
	} else {
	    do_op = code_seq_singleton(code_beq(V0, AT, 2));
	}
	break;
    case neqsym:
	if (typ == float_te) {
	    do_op = code_seq_singleton(code_bfne(V0, AT, 2));
	} else {
	    do_op = code_seq_singleton(code_bne(V0, AT, 2));
	}
	break;
    case ltsym:
	if (typ == float_te) {
	    do_op = code_seq_singleton(code_fsub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bfltz(V0, 2));
	} else {
	    do_op = code_seq_singleton(code_sub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bltz(V0, 2));
	}
	break;
    case leqsym:
	if (typ == float_te) {
	    do_op = code_seq_singleton(code_fsub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bflez(V0, 2));
	} else {
	    do_op = code_seq_singleton(code_sub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_blez(V0, 2));
	}
	break;
    case gtsym:
	if (typ == float_te) {
	    do_op = code_seq_singleton(code_fsub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bfgtz(V0, 2));
	} else {
	    do_op = code_seq_singleton(code_sub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bgtz(V0, 2));
	}
	break;
    case geqsym:
	if (typ == float_te) {
	    do_op = code_seq_singleton(code_fsub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bfgez(V0, 2));
	} else {
	    do_op = code_seq_singleton(code_sub(V0, AT, V0));
	    do_op = code_seq_add_to_end(do_op, code_bgez(V0, 2));
	}
	break;
    default:
	bail_with_error("Unknown token code (%d) in gen_code_rel_op",
			rel_op.code);
	break;
    }
    ret = code_seq_concat(ret, do_op);
    // rest of the code for the comparisons
    ret = code_seq_add_to_end(ret, code_add(0, 0, AT)); // put false in AT
    ret = code_seq_add_to_end(ret, code_beq(0, 0, 1)); // skip next instr
    ret = code_seq_add_to_end(ret, code_addi(0, AT, 1)); // put true in AT
    ret = code_seq_concat(ret, code_push_reg_on_stack(AT, bool_te));
    return ret;
}
 */

code_seq gen_code_rel_op(token_t rel_op) {

}

code_seq gen_code_arith_op(token_t rel_op) {


    code_seq do_op = code_seq_empty();
    switch (rel_op.code) {
        case plussym:
            code_seq_add_to_end(&do_op, code_add(SP, 0, SP, 1));
            do_op = code_seq_singleton(code_add(SP, 0, SP, 1));
            break;
//        case minussym:
//            do_op = code_seq_add_to_end(do_op, code_fsub(V0, AT, V0));
//            break;
//        case multsym:
//            do_op = code_seq_add_to_end(do_op, code_fmul(V0, AT, V0));
//            break;
//        case divsym:
//            do_op = code_seq_add_to_end(do_op, code_fdiv(V0, AT, V0));
//            break;
        default:
            bail_with_error("Unexpected arithOp (%d) in gen_code_arith_op",
                            rel_op.code);
            break;
    }
    return do_op;

}

code_seq gen_code_op(token_t op)
{
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
            bail_with_error("Unknown token code (%d) in gen_code_op",
                            op.code);
            break;
    }
    return code_seq_empty();
}

code_seq gen_code_expr_bin(char* varName, binary_op_expr_t expr, reg_num_type target_reg) {
    code_seq ret = gen_code_expr(varName, *expr.expr1, target_reg);

    code_seq_concat(&ret, gen_code_expr(varName, *(expr.expr2), target_reg));
    // check the types match
//    type_exp_e t1 = ast_expr_type(*(exp.expr1));
//    assert(ast_expr_type(*(expr.expr2)) == t1);
    // do the operation, putting the result on the stack
    code_seq_concat(&ret, gen_code_op(expr.arith_op));
    return ret;
}
/*
code_seq gen_code_op(token_t op, type_exp_e typ)
{
    switch (op.code) {
    case eqsym: case neqsym:
    case ltsym: case leqsym:
    case gtsym: case geqsym:
	return gen_code_rel_op(op, typ);
	break;
    case plussym: case minussym: case multsym: case divsym:
	assert(typ == float_te);
	return gen_code_arith_op(op);
	break;
    default:
	bail_with_error("Unknown token code (%d) in gen_code_op",
			op.code);
	break;
    }
    return code_seq_empty();
}



*/




code_seq gen_code_ident(ident_t id)
{

    assert(id.idu != NULL);
    code_seq ret = code_compute_fp(T9, id.idu->levelsOutward);

    assert(id_use_get_attrs(id.idu) != NULL);
    unsigned int offset_count = id_use_get_attrs(id.idu)->offset_count;
    assert(offset_count <= USHRT_MAX); // it has to fit!
    type_exp_e typ = id_use_get_attrs(id.idu)->type;
    if (typ == float_te) {
        ret = code_seq_add_to_end(ret,
                                  code_flw(T9, V0, offset_count));
    } else {
        ret = code_seq_add_to_end(ret,
                                  code_lw(T9, V0, offset_count));
    }
    return code_seq_concat(ret, code_push_reg_on_stack(V0, typ));
}

// Generate code for the expression exp
// putting the result on top of the stack,
// and using V0 and AT as temporary registers
// May also modify SP, HI,LO when executed
code_seq gen_code_expr(expr_t exp)
{
    switch (exp.expr_kind) {
        case expr_bin:
//            return gen_code_binary_op_expr(exp.data.binary);
            break;
        case expr_ident:
            return gen_code_ident(exp.data.ident);
            break;
        case expr_number:
            return gen_code_number(exp.data.number);
            break;
        case expr_logical_not:
            return gen_code_logical_not_expr(*(exp.data.logical_not));
            break;
        default:
            bail_with_error("Unexpected expr_kind_e (%d) in gen_code_expr",
                            exp.expr_kind);
            break;
    }
    // never happens, but suppresses a warning from gcc
    return code_seq_empty();
}

code_seq gen_code_number(char* cName, number_t num, bool negate) {
    word_type i = negate ? -(num.value) : num.value;
    //unsigned int offset_count = id_use_get_attrs(num.idu)->offset_count;
    unsigned int global_offset
            = literal_table_lookup(cName,i);
    printf("OFFSET SET (%d): %d\n",i,  global_offset);
    return code_seq_singleton(code_cpw(SP, 0,GP, global_offset));
}

code_seq gen_code_print_stmt(print_stmt_t s) {
    code_seq base = code_seq_empty();

    code_seq expr_code = gen_code_expr(NULL, s.expr, SP);

    code_seq_concat(&base, expr_code);
    code_seq_add_to_end(&base, code_pint(SP,0 ));

    return base;
}

code_seq gen_code_if_ck_db(db_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();
    printf("db_condition_t");
    return base;
}

code_seq gen_code_if_ck_rel(rel_op_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();

    code_seq_concat(&base, gen_code_expr(NULL,stmt.expr1, SP));
    code_seq_concat(&base, gen_code_expr(NULL,stmt.expr2, FP));

    code_seq_add_to_end(&base,  code_sub(SP, 0, FP, 0)); //PUTS subtracted value into SP

    if (strcmp(stmt.rel_op.text, "<")==0) {
        code_seq_add_to_end(&base, code_bgtz(SP,0,thenSize+2));
    }


    return base;
}

code_seq gen_code_assign_stmt(assign_stmt_t stmt){

    code_seq base = code_seq_empty();
    printf("VAR: %s\n", stmt.name);

    code_seq_concat(&base,gen_code_expr(stmt.name,*stmt.expr, SP));
    unsigned int offset_count = id_use_get_attrs(stmt.idu)->offset_count;
    printf("THE OFFSET: %u\n",offset_count);

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
//            printf("Constant definitions\n");
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
            printf("Read definitions\n");
            stmt_code = gen_code_read_stmt(s->data.read_stmt);
            break;
        case print_stmt:
            stmt_code = gen_code_print_stmt(s->data.print_stmt);
            break;
        case block_stmt:
            printf("block stmt\n");
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
        char* cName = def->number.text;

//        printf("\n-0-0-0-0-\nC NAME: %s (%s)\n-0-0-0-0-\n",cName, def->ident.name);
        code_seq_concat(&base, gen_code_number( def->ident.name, def->number,false));


        def = def->next;
    }
    //Handle single for now

   // literal_table_debug_print();
//    int v =     literal_table_lookup(def.ident.name,20);

     // printf("%s=%d\n", def.ident.name, def.number.value);
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
    code_seq_debug_print(stdout, main_cs);
}
