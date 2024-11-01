//
// Created by matt1 on 10/18/2024.
//
#include <stdio.h>
#include "scope_check.h"
#include "ast.h"
#include "scope.h"
#include "symtab.h"
#include <stddef.h>

void scope_check_program(block_t block) {
    symtab_enter_scope();
    scope_check_const_decls(block.const_decls);
    scope_check_varDecls(block.var_decls);
    scope_check_proc_decls(block.proc_decls);
    scope_check_stmts(block.stmts);
    symtab_leave_scope();
}


void scope_check_const_decls(const_decls_t const_decl){
    if (const_decl.start==NULL){
        printf("NULL const_decl\n");
    }
}

void scope_check_assign_stmt(assign_stmt_t assignStmt) {
    char* assignName = assignStmt.name;

    printf("%s =", assignStmt.name);
    id_use* idUsed = symtab_lookup(assignName);
    if (idUsed==NULL) {
        printf(" Unset\n");
    } else {
        printf(" Set\n");

    }

}
/*
 * typedef enum { assign_stmt, call_stmt, if_stmt, while_stmt,
	       read_stmt, print_stmt, block_stmt } stmt_kind_e;

 */
void scope_check_stmt(stmt_t *stmt) {
    if (stmt == NULL) return;

    switch (stmt->stmt_kind) {
        case assign_stmt:
            printf("- Assign statement\n");
            scope_check_assign_stmt(stmt->data.assign_stmt);
            break;
    }
}

void scope_check_stmts(stmts_t stmts) {
    if (stmts.stmt_list.start==NULL){
        printf("NULL stmt_list\n");
        return;
    }
    printf("Statements found:\n");
    stmt_t *current = stmts.stmt_list.start;
    while (current != NULL) {
        scope_check_stmt(current);

        current = current->next;
    }

}


void scope_check_proc_decls(proc_decls_t procDecls){
    proc_decl_t *current = procDecls.proc_decls;
//    while
}


void scope_check_varDecls(var_decls_t varDecls){
    if (varDecls.var_decls == NULL ) {
        printf("NULL var_decls\n");
        return;
    }
}