#ifndef HW3_SCOPE_CHECK_H
#define HW3_SCOPE_CHECK_H
#include "scope_check.h"
#include "ast.h"
#include "scope.h"
#include "utilities.h"
#include "symtab.h"

//Runs scope check for a s block
void scope_check_program_s(struct block_s block);

//Runs scope check for entire program
void scope_check_program(block_t block);

//Check if already exists
void handleAlreadyExist(const char *name, file_location loc, id_kind kind);

// Push any constant definition into list
void scope_push_constDefList(const_def_list_t identityList);

// Read through the constant declarations and push them to
// the constant definition list
void scope_check_const_decls(const_decls_t const_decl);

// Returns true or false based on if the ident being passed
// has already been declared or not
bool check_ident(const char* name, file_location loc);

// Ensure that both expressions used have
// been declared previously
void check_binary_expr(binary_op_expr_t bin);

// Call the appropriate function based on the
// expression kind being passed in.
void check_ident_express(struct expr_s xp);

// Check that the assignment statement is valid
void scope_check_assign_stmt(assign_stmt_t assignStmt);

// Check that each If statement has a condition
// and is followed by a then and else
void scope_check_if_stmt(if_stmt_t ifStmt);

// Check that each while statement has a condition
// and is followed by a body
void scope_check_while_stmt(while_stmt_t while_stmt);

//Checks individual statements
void scope_check_stmt(stmt_t *stmt);

//Checks statements
void scope_check_stmts(stmts_t stmts);

//Check proc decs
void scope_check_proc_decls(proc_decls_t procDecls);

//Pushes ident list into table and runs dec checks
void scope_push_identList(ident_list_t identityList);

//Scope checks var declarations
void scope_check_varDecls(var_decls_t varDecls);

#endif