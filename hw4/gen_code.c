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

code_seq push_reg_on_stack(reg_num_type reg, offset_type offset, offset_type second, reg_num_type sp ) {
    return code_seq_singleton(code_cpw(sp,  second, reg, offset));
}

void gen_code_output_literals(BOFFILE bf) {
    literal_table_start_iteration();
    while (literal_table_iteration_has_next()) {
        word_type w = literal_table_iteration_next();
        bof_write_word(bf, w);
    }
}

int gen_code_output_seq_count(code_seq cs) {
    int res = 0;

    while (!code_seq_is_empty(cs)) {
//        bin_instr_t inst = code_seq_first(cs)->instr;
        res++;
        cs = code_seq_rest(cs);
    }
    return res;
}

BOFHeader gen_code_program_header(code_seq main_cs) {
    BOFHeader ret;
    bof_write_magic_to_header(&ret);

    ret.text_start_address = 0;
    ret.text_length = code_seq_size(main_cs);
    int dsa = MAX(ret.text_length, 1024) + BYTES_PER_WORD;
    ret.data_start_address = dsa;
    ret.data_length = literal_table_size() ;
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
code_seq gen_code_expr(char* name, expr_t exp, offset_type second, reg_num_type reg) {
    code_seq base = code_seq_empty();
    switch (exp.expr_kind) {
        case expr_ident:
            base = gen_code_ident(exp.data.ident, second, reg);
        break;
        case expr_bin:
            base = gen_code_bin_op_expr(exp.data.binary);
        break;
        case expr_negated:
            base = gen_code_number(NULL, exp.data.negated.expr->data.number, true, 0, SP);
        break;
        case expr_number:
            base = gen_code_number(NULL, exp.data.number, false, 0, SP);
        break;
        default:
            bail_with_error("Unexpected expr_kind_e (%d) in gen_code_expr", exp.expr_kind);
        break;
    }
    return base;
}


code_seq gen_code_arith_op(token_t rel_op) {
    code_seq base = code_seq_empty();
    return base;
}
int AC = 4;

code_seq gen_code_ident(ident_t ident, offset_type second, reg_num_type reg) {
    code_seq base = code_seq_empty();
    int offset = literal_table_lookup(ident.name, 0);

    // Adjust SP to allocate space
    code_seq_add_to_end(&base, code_sri(SP, 1)); // SP = SP - 1

    // Copy value from GP + offset to SP + 0
    code_seq_add_to_end(&base, code_cpw(SP, 0, GP, offset));

    return base;
}

code_seq gen_code_bin_op_expr(binary_op_expr_t expr) {
    code_seq base = code_seq_empty();

    // Evaluate left operand (SP adjusted within gen_code_expr)
    code_seq left_code = gen_code_expr(NULL, *expr.expr1, 0, SP);
    code_seq_concat(&base, left_code);

    // Evaluate right operand (SP adjusted within gen_code_expr)
    code_seq right_code = gen_code_expr(NULL, *expr.expr2, 0, SP);
    code_seq_concat(&base, right_code);

    // Now the stack has:
    // [SP + 1]: Left operand (expr1)
    // [SP + 0]: Right operand (expr2)

    // Perform the operation
    switch (expr.arith_op.code) {
        case plussym:
            code_seq_add_to_end(&base, code_add(SP, 1, SP, 0));
        break;
        case minussym:
            code_seq_add_to_end(&base, code_sub(SP, 1, SP, 0));
        break;
        case multsym:
            code_seq_add_to_end(&base, code_lwr(AC, SP, 1));
        code_seq_add_to_end(&base, code_mul(SP, 0));
        code_seq_add_to_end(&base, code_cflo(SP, 1));
        break;
        case divsym:
            code_seq_add_to_end(&base, code_lwr(AC, SP, 1));
        code_seq_add_to_end(&base, code_div(SP, 0));
        code_seq_add_to_end(&base, code_cflo(SP, 1));
        break;
        default:
            bail_with_error("Unhandled binary operation in gen_code_bin_op_expr");
    }

    // Adjust SP to deallocate right operand
    code_seq_add_to_end(&base, code_ari(SP, 1)); // SP = SP + 1

    // Now the result is at SP + 0 (since SP has been incremented)

    return base;
}



code_seq gen_code_number(char* varName, number_t num, bool negate, offset_type second, reg_num_type sp) {
    code_seq base = code_seq_empty();
    word_type val = negate ? -(num.value) : num.value;

    // Adjust SP to allocate space
    code_seq_add_to_end(&base, code_sri(SP, 1)); // SP = SP - 1

    // Store the literal at SP + 0
    code_seq_add_to_end(&base, code_lit(SP, 0, val));

    return base;
}


code_seq gen_code_print_stmt(print_stmt_t s) {
    code_seq base = code_seq_empty();

    // Evaluate the expression (SP adjusted within gen_code_expr)
    code_seq expr_code = gen_code_expr(NULL, s.expr, 0, SP);
    code_seq_concat(&base, expr_code);

    // Print the value
    code_seq_add_to_end(&base, code_pint(SP, 0));

    // Deallocate the expression from the stack
    code_seq_add_to_end(&base, code_ari(SP, 1)); // SP = SP + 1

    return base;
}

code_seq gen_code_if_ck_db(db_condition_t stmt, int thenSize) {
    code_seq base = code_seq_empty();
    code_seq_concat(&base, gen_code_expr(NULL,stmt.dividend, false, SP));
    code_seq_add_to_end(&base, code_lit(SP, 1, stmt.divisor.data.number.value));
    code_seq_add_to_end(&base, code_cfhi(SP, 1));
    code_seq_add_to_end(&base, code_beq(SP, 0, thenSize + 2));
    return base;
}

code_seq gen_code_if_ck_rel(rel_op_condition_t stmt, int elseSize, int thenSize, bool norm) {
    code_seq base = code_seq_empty();

    // Evaluate first expression
    code_seq left_expr = gen_code_expr(NULL, stmt.expr1, 0, SP);
    code_seq_concat(&base, left_expr);

    // Evaluate second expression
    code_seq right_expr = gen_code_expr(NULL, stmt.expr2, 0, SP);
    code_seq_concat(&base, right_expr);

    // Now the stack has:
    // [SP + 1]: expr1
    // [SP + 0]: expr2

    // Perform the comparison
    code_seq_add_to_end(&base, code_sub(SP, 1, SP, 0));

    // Deallocate expr2 from the stack
    code_seq_add_to_end(&base, code_ari(SP, 1));

    // Deallocate expr1 from the stack
    code_seq_add_to_end(&base, code_ari(SP, 1));

    // Branch based on the comparison result
    if (norm) {
        if (strcmp(stmt.rel_op.text, "==") == 0) {
            code_seq_add_to_end(&base, code_bne(SP, 0, thenSize + 2));
        } else if (strcmp(stmt.rel_op.text, "!=") == 0) {
            code_seq_add_to_end(&base, code_beq(SP, 0, thenSize + 2));
        }
    } else {
        // For relational operators, SP now points to the result of `expr1 - expr2`
        if (strcmp(stmt.rel_op.text, "<") == 0) {
            code_seq_add_to_end(&base, code_bltz(SP, 0, elseSize + 2));
        } else if (strcmp(stmt.rel_op.text, "<=") == 0) {
            code_seq_add_to_end(&base, code_blez(SP, 0, elseSize + 2));
        } else if (strcmp(stmt.rel_op.text, ">") == 0) {
            code_seq_add_to_end(&base, code_bgtz(SP, 0, elseSize + 2));
        } else if (strcmp(stmt.rel_op.text, ">=") == 0) {
            code_seq_add_to_end(&base, code_bgez(SP, 0, elseSize + 2));
        }
    }

    return base;
}


code_seq gen_code_assign_stmt(assign_stmt_t stmt) {
    code_seq base = code_seq_empty();
    int offset = literal_table_lookup(stmt.name, 0);

    // Evaluate the expression (SP adjusted within gen_code_expr)
    code_seq expr_code = gen_code_expr(stmt.name, *stmt.expr, 0, SP);
    code_seq_concat(&base, expr_code);

    // Store the value into the variable
    code_seq_add_to_end(&base, code_cpw(GP, offset, SP, 0));

    // Deallocate the expression from the stack
    code_seq_add_to_end(&base, code_ari(SP, 1)); // SP = SP + 1

    return base;
}


bool isNormalRev(condition_t c) {
    if (strcmp(c.data.rel_op_cond.rel_op.text, "==") == 0)return true;
    if (strcmp(c.data.rel_op_cond.rel_op.text, "!=") == 0)return true;
    return false;
}

code_seq gen_code_if_stmt(if_stmt_t stmt) {
    code_seq base = code_seq_empty();
    condition_t c = stmt.condition;

    bool rel = false;
    if (c.cond_kind == ck_rel) {
        rel = true;
    }

    code_seq thenSeq = gen_code_stmts(stmt.then_stmts);
    code_seq elseSeq = gen_code_stmts(stmt.else_stmts);

    int thenSeqLength = code_seq_size(thenSeq);
    int elseSeqLength = code_seq_size(elseSeq);

    bool norm = false;

    if (rel && isNormalRev(c)) {
        norm=true;
    }
    if (c.cond_kind == ck_db) {
        code_seq_concat(&base, gen_code_if_ck_db(c.data.db_cond,elseSeqLength));
    }
    if (rel) {
        code_seq_concat(&base, gen_code_if_ck_rel(c.data.rel_op_cond,elseSeqLength, thenSeqLength,  norm));
    }
    if (rel && isNormalRev(c)) {
        code_seq_concat(&base, thenSeq);
        code_seq_add_to_end(&base, code_jrel(elseSeqLength + 1));
        code_seq_concat(&base, elseSeq);
    } else {
        code_seq_concat(&base, elseSeq);
        code_seq_add_to_end(&base, code_jrel(thenSeqLength + 1));
        code_seq_concat(&base, thenSeq);
    }
    return base;
}

code_seq gen_code_while_stmt(while_stmt_t stmt) {
    code_seq base = code_seq_empty();
    code_seq bodyCode = gen_code_stmts(stmt.body);
    int bodySeqSize = code_seq_size(bodyCode);

    code_seq conditionCode = code_seq_empty();
    int conditionSize = 0;


    if (stmt.condition.cond_kind == ck_rel) {
        rel_op_condition_t rel = stmt.condition.data.rel_op_cond;

        // Generate code for both expressions
        code_seq_concat(&conditionCode, gen_code_expr(NULL,rel.expr1, false, SP));
        code_seq_concat(&conditionCode, gen_code_expr(NULL,rel.expr2, true, SP));

        conditionSize = code_seq_size(conditionCode); // Update condition size

        // Generate branching code based on relational operator

        if (rel.expr1.expr_kind==expr_number &&   rel.expr2.expr_kind==expr_number) {
            int num1 = rel.expr1.data.number.value;
            int num2 = rel.expr2.data.number.value;
            if (num1==num2) {
                if (strcmp(rel.rel_op.text, "<") == 0) {
                    code_seq_add_to_end(&conditionCode, code_jrel(bodySeqSize + 2)); // Jump if false
                } else {
                    bail_with_error("Unhandled relational operator in while condition");
                }
            }
        }else {
            if (strcmp(rel.rel_op.text, "<") == 0) {
                code_seq_add_to_end(&conditionCode, code_sub(SP, 0, SP, 1));
                code_seq_add_to_end(&conditionCode, code_bgtz(SP, 0, bodySeqSize + 2)); // Jump if false
            } else if (strcmp(rel.rel_op.text, "<=") == 0) {
                code_seq_add_to_end(&conditionCode, code_sub(SP, 0, SP, 1));
                code_seq_add_to_end(&conditionCode, code_bgez(SP, 0, bodySeqSize + 2)); // Jump if false
            } else if (strcmp(rel.rel_op.text, ">") == 0) {
                code_seq_add_to_end(&conditionCode, code_sub(SP, 0, SP, 1));
                code_seq_add_to_end(&conditionCode, code_bltz(SP, 0, bodySeqSize + 2)); // Jump if false
            } else if (strcmp(rel.rel_op.text, ">=") == 0) {
                code_seq_add_to_end(&conditionCode, code_sub(SP, 0, SP, 1));
                code_seq_add_to_end(&conditionCode, code_blez(SP, 0, bodySeqSize + 2)); // Jump if false
            } else if (strcmp(rel.rel_op.text, "==") == 0) {
                code_seq_add_to_end(&conditionCode, code_bne(SP, 1, bodySeqSize + 2)); // Jump if false
               // code_seq_add_to_end(&base, code_jrel(bodySeqSize));
            } else if (strcmp(rel.rel_op.text, "!=") == 0) {
                code_seq_add_to_end(&conditionCode, code_beq(SP, 1, bodySeqSize + 2)); // Jump if false
            } else {
                bail_with_error("Unhandled relational operator in while condition");
            }
        }
    }
    else {
        bail_with_error("Unhandled condition kind in while statement");
    }

    // Concatenate condition code and body code
    code_seq_concat(&base, conditionCode);
    code_seq_concat(&base, bodyCode);

    // Add jump to the start of the condition
    code_seq_add_to_end(&base, code_jrel(-(conditionSize + bodySeqSize + 2)));

    return base;
}

code_seq gen_code_call_stmt(call_stmt_t stmt) {
    code_seq base = code_seq_empty();
    return base;
}

// Generate code for the read statment given by stmt
code_seq gen_code_read_stmt(read_stmt_t stmt) {
    code_seq base = code_seq_empty();
    int offset = literal_table_lookup(stmt.name,0);
    code_seq_add_to_end(&base, code_rch(GP, offset));
    return base;
}

code_seq gen_code_block_stmt(block_stmt_t stmt) {
    code_seq base = code_seq_empty();
    struct block_s * b = stmt.block;
    code_seq_concat(&base, gen_code_consts(b->const_decls));
    code_seq body_cs = gen_code_stmts(&b->stmts);
    code_seq_concat(&base, body_cs);
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
        bool negate = false;
        code_seq_concat(&base, gen_code_number( def->ident.name,  def->number, negate, false, SP));
        def = def->next;
    }
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

code_seq gen_code_stmts(stmts_t* stmts) {
    code_seq base = code_seq_empty();
    if (stmts == NULL) {
        return base;
    }

    stmts_t stmt = *stmts;

    if (stmt.stmts_kind == empty_stmts_e) {
        return base; // Deal with epsilon case
    }

    // Not empty
    stmt_t *s = stmt.stmt_list.start;

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
    code_seq body_cs = gen_code_stmts(&b.stmts);
    code_seq_concat(&main_cs, body_cs);
    code_seq tear_down_cs = code_utils_tear_down_program(); //BROKEN
    code_seq_concat(&main_cs, tear_down_cs);
    gen_code_output_program(bf, main_cs);
//    literal_table_debug_print();
//    code_seq_debug_print(stdout, main_cs);
}