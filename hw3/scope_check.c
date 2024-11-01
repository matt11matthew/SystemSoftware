//
// Created by matt1 on 10/18/2024.
//
#include <stdio.h>
#include "scope_check.h"
#include "ast.h"
#include "scope.h"
#include "utilities.h"
#include "symtab.h"
#include <stddef.h>

bool DEBUG = false;

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
        if (DEBUG) {

            printf("NULL const_decl\n");
        }
    }
}

void scope_check_assign_stmt(assign_stmt_t assignStmt) {
    char* assignName = assignStmt.name;
    if (DEBUG) {

        printf("%s =",assignName);
    }
    bool idUsed = symtab_declared(assignName);
    if (!idUsed) {
        bail_with_prog_error(*assignStmt.file_loc,
                             "identifier \"%s\" is not declared!",
                             assignName);
        //hw3-declerrtest0.spl: line 3 identifier "x" is not declared!
        if (DEBUG) {

            printf(" Unset\n");
        }
    } else {
        if (DEBUG) {

            printf(" Set\n");
        }

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
            if (DEBUG) {

                printf("- Assign statement\n");
            }
            scope_check_assign_stmt(stmt->data.assign_stmt);
            break;
    }
}

void scope_check_stmts(stmts_t stmts) {
    if (stmts.stmt_list.start==NULL){
        if (DEBUG) {

            printf("NULL stmt_list\n");
        }
        return;
    }
    if (DEBUG) {

        printf("Statements found:\n");
    }
    stmt_t *current = stmts.stmt_list.start;
    while (current != NULL) {
        scope_check_stmt(current);

        current = current->next;
    }

}


void scope_check_proc_decls(proc_decls_t procDecls){
    proc_decl_t *current = procDecls.proc_decls;
//    while(current != NULL){}

}

void scope_push_identList(  ident_list_t identityList){
    //ADD TO SCOPE

    ident_t * st = identityList.start;


    while (st!=NULL) {
        //TODO PUSH
//        printf("Name: %s", st->name);


        symtab_insert(st->name,  id_use_create(create_id_attrs(*identityList.file_loc,variable_idk,

                                                               0)

                                               ,1));

        st = st->next;

    }
//    symtab_insert(/**/)
//    symtab_declared()
//    symtab_insert()

}

void scope_check_identList(  ident_list_t identityList){

}

void scope_check_varDecls(var_decls_t varDecls){
    if (varDecls.var_decls == NULL ) {
        if (DEBUG) {

            printf("NULL var_decls\n");
        }
        return;
    }
    var_decl_t* cur = varDecls.var_decls;
    while (cur != NULL) {

        scope_check_identList( cur->ident_list);
        scope_push_identList( cur->ident_list);
        cur = cur->next;
    }
}