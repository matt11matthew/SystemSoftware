#ifndef HW3_SCOPE_CHECK_H
#define HW3_SCOPE_CHECK_H
#include "scope_check.h"
#include "ast.h"
#include "scope.h"
#include "utilities.h"
#include "symtab.h"

// void scope_check_stmt(stmt_t *stmt);
void scope_check_program_s(struct block_s block);

void scope_check_program(block_t block);

void handleAlreadyExist(const char *name, file_location loc, id_kind kind);

void scope_push_constDefList(const_def_list_t identityList);

void scope_check_const_decls(const_decls_t const_decl);

bool check_ident(const char* name, file_location loc);

void  check_binary_expr(binary_op_expr_t bin);

void check_ident_express(struct expr_s xp);

void scope_check_assign_stmt(assign_stmt_t assignStmt);

void scope_check_if_stmt(if_stmt_t ifStmt);

void scope_check_while_stmt(while_stmt_t while_stmt);

void scope_check_stmt(stmt_t *stmt);

void scope_check_stmts(stmts_t stmts);

void scope_check_proc_decls(proc_decls_t procDecls);

void scope_push_identList(ident_list_t identityList);

void scope_check_varDecls(var_decls_t varDecls);

#endif