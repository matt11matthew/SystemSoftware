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

code_seq gen_code_expr(char* varName, expr_t expr, reg_num_type target_reg) {
    code_seq base = code_seq_empty();

    switch (expr.expr_kind) {
        case expr_bin:
            printf("bin stmt\n");
            break;
        case expr_negated:
            printf("negated stmt\n");
            break;
        case expr_ident:
           // base = code_seq_singleton(code_cpr(target_reg, 0, expr.data.number.value));
            break;
        case expr_number:
            if (varName==NULL){
                base = code_seq_singleton(code_lit(target_reg, 0, expr.data.number.value));
            } else {
                return gen_code_number(varName, expr.data.number);
                //PROCESS NUMBER
            }
            break;
        default:
            fprintf(stderr, "Error: Unhandled expression kind in gen_code_expr\n");
            exit(1);
    }
    return base;
}

code_seq gen_code_number(char* cName, number_t num)
{
    unsigned int global_offset
            = literal_table_lookup(cName, num.value);
    printf("OFFSET SET: %d\n", global_offset);
    return code_seq_singleton(code_cpw(SP, global_offset,GP, global_offset));
}

code_seq gen_code_print_stmt(print_stmt_t s) {
    code_seq base = code_seq_empty();

    // Evaluate the expression into R3
    code_seq expr_code = gen_code_expr(NULL, s.expr, SP);
    code_seq_concat(&base, expr_code);

    // Add print system call (PINT)
    if (s.expr.expr_kind==expr_ident){

        code_seq_add_to_end(&base, code_pint(SP,id_use_get_attrs(s.expr.data.ident.idu)->offset_count ));
    } else {

        code_seq_add_to_end(&base, code_pint(SP,0 ));
    }

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

/*code_seq gen_code_assign_stmt(assign_stmt_t stmt)
{
    // can't call gen_code_ident,
    // since stmt.name is not an ident_t
    code_seq ret;
    // put value of expression in $v0
    ret = gen_code_expr(*(stmt.expr));
    assert(stmt.idu != NULL);
    assert(id_use_get_attrs(stmt.idu) != NULL);
    type_exp_e typ = id_use_get_attrs(stmt.idu)->type;
    ret = code_seq_concat(ret, code_pop_stack_into_reg(V0, typ));
    // put frame pointer from the lexical address of the name
    // (using stmt.idu) into $t9
    ret = code_seq_concat(ret,
			  code_compute_fp(T9, stmt.idu->levelsOutward));
    unsigned int offset_count = id_use_get_attrs(stmt.idu)->offset_count;
    assert(offset_count <= USHRT_MAX); // it has to fit!
    switch (id_use_get_attrs(stmt.idu)->type) {
    case float_te:
	ret = code_seq_add_to_end(ret,
				  code_fsw(T9, V0, offset_count));
	break;
    case bool_te:
	ret = code_seq_add_to_end(ret,
				  code_sw(T9, V0, offset_count));
	break;
    default:
	bail_with_error("Bad var_type (%d) for ident in assignment stmt!",
			id_use_get_attrs(stmt.idu)->type);
	break;
    }
    return ret;
}
 */

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

code_seq gen_code_call_stmt(call_stmt_t stmt){
    code_seq base = code_seq_empty();
    return base;
}

code_seq gen_code_while_stmt(while_stmt_t stmt){
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
            // stmt_code = gen_code_FIX_stmt(s->data.FIX_stmt);
            break;
        case print_stmt:
            stmt_code = gen_code_print_stmt(s->data.print_stmt);
            break;
        case block_stmt:
            printf("block stmt\n");
            // stmt_code = gen_code_FIX_stmt(s->data.assign_stmt);
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

        printf("VALUE: %s\n",cName);
        code_seq_concat(&base, gen_code_number(cName, def->number));


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


//
//    printf("Offset: %u\n", literal_table_lookup("A", 1.0));  // Should add and return 0
//    printf("Offset: %u\n", literal_table_lookup("B", 2.0));  // Should add and return 1
//    printf("Offset: %u\n", literal_table_lookup("A", 1.0));  // Should return 0 (existing)
//    printf("Offset: %u\n", literal_table_lookup("A", 3.0));  // Should add and return 2 (new due to different value)
//


    code_seq body_cs = gen_code_stmts(b.stmts);
    code_seq_concat(&main_cs, body_cs);

    code_seq tear_down_cs = code_utils_tear_down_program();
    code_seq_concat(&main_cs, tear_down_cs);

    gen_code_output_program(bf, main_cs);

 //   code_seq_debug_print(stdout, main_cs);
}
