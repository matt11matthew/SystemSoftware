//
// Created by Matt on 11/13/2024.
//

#ifndef GEN_CODE_H
#define GEN_CODE_H

#include <stdio.h>
#include <stdlib.h>

#include "bof.h"
#include "lexer.h"
#include "parser.h"
#include "unparser.h"
#include "ast.h"
#include "code.h"
#include "code_seq.h"
#include "utilities.h"
#include "symtab.h"
#include "scope_check.h"

void gen_code_initialize();

// Pushes a register's value onto the stack with an optional offset.
code_seq push_reg_on_stack(reg_num_type reg, offset_type offset, offset_type second, reg_num_type sp);

// Outputs all literals stored in the literal table to a BOF file.
void gen_code_output_literals(BOFFILE bf);

// Counts the number of instructions in a given code sequence.
int gen_code_output_seq_count(code_seq cs);

// Generates a program header for the BOF file, including segment sizes and stack information.
BOFHeader gen_code_program_header(code_seq main_cs);

// Writes a code sequence to a BOF file.
void gen_code_output_seq(BOFFILE bf, code_seq cs);

// Outputs a complete program, including the main code, literals, and header, to a BOF file.
void gen_code_output_program(BOFFILE bf, code_seq main_cs);

// Generates code for an arithmetic operation.
code_seq gen_code_arith_op(token_t rel_op);
code_seq gen_code_bin_op_expr(binary_op_expr_t expr);

// Generates code for a binary expression.
// code_seq gen_code_expr_bin(char* name, binary_op_expr_t expr, reg_num_type reg);
// code_seq gen_code_bin_op_expr(binary_op_expr_t expr, offset_type second, reg_num_type reg);

// Generates code to retrieve the value of an identifier.
code_seq gen_code_ident(ident_t ident, offset_type second, reg_num_type reg);

// Generates code for an expression of various kinds (identifiers, numbers, binary expressions, etc.).
code_seq gen_code_expr(char* name, expr_t exp, offset_type second, reg_num_type reg);

// Generates code for a number (either constant or negated).
code_seq gen_code_number( char* varName, number_t num, bool negate, offset_type second, reg_num_type sp);

// Generates code for a print statement.
code_seq gen_code_print_stmt(print_stmt_t s);

// Generates code for a "divisible by" condition in an if-statement.
code_seq gen_code_if_ck_db(db_condition_t stmt, int thenSize);

// Generates code for a relational condition in an if-statement.
code_seq gen_code_if_ck_rel(rel_op_condition_t stmt, int elseSize, int thenSize, bool norm);

// Generates code for an assignment statement.
code_seq gen_code_assign_stmt(assign_stmt_t stmt);

// Determines whether a relational condition should use normal comparison logic.
bool isNormalRev(condition_t c);

// Generates code for an if-statement.
code_seq gen_code_if_stmt(if_stmt_t stmt);

// Generates code for a while-statement.
code_seq gen_code_while_stmt(while_stmt_t stmt);

// Generates code for a procedure call statement.
code_seq gen_code_call_stmt(call_stmt_t stmt);

// Generates code for a read statement.
code_seq gen_code_read_stmt(read_stmt_t stmt);

// Generates code for a block of statements.
code_seq gen_code_block_stmt(block_stmt_t stmt);

// Generates code for a single statement of any kind.
code_seq gen_code_stmt(stmt_t *s);

// Generates code for a single constant definition.
code_seq gen_code_const(const_def_t*  def);

// Generates code for a list of constant definitions.
code_seq gen_code_consts(const_decls_t  decls);

// Generates code for a list of statements.
code_seq gen_code_stmts(stmts_t* stmts);

// Wrapper function to start processing instructions
void gen_code_program(BOFFILE bf, block_t b);
#endif //GEN_CODE_H