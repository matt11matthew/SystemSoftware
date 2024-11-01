
//
// Created by matt1 on 10/18/2024.
//

#ifndef HW3_SCOPE_CHECK_H
#define HW3_SCOPE_CHECK_H
#endif //HW3_SCOPE_CHECK_H
#include "ast.h"

void scope_check_program(block_t block);

void scope_check_const_decls(const_decls_t varDecls);


void scope_check_proc_decls(proc_decls_t varDecls);


void scope_check_assign_stmt(assign_stmt_t assignStmt);

void scope_check_stmts(stmts_t stmts);
void scope_check_stmt(stmt_t *stmt);

void scope_check_varDecls(var_decls_t varDecls);