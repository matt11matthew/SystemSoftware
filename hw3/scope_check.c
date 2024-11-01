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

void handleAlreadyExist(char *name, file_location loc) {
    id_use* id = symtab_lookup(name);
    printf("FOUND: %s (lvl: %d) %s", name, id->levelsOutward, kind2str(id->attrs->kind));

    if (id->attrs->kind==constant_idk) {

        bail_with_prog_error(loc,
                             "constant \"%s\" is already declared as a constant",
                             name);
    } else   if (id->attrs->kind==variable_idk) {

        bail_with_prog_error(loc,
                             "variable \"%s\" is already declared as a variable",
                             name);
    }
}

void scope_push_constDefList(const_def_list_t identityList) {
    const_def_t * st = identityList.start;


    while (st!=NULL) {
        char* name = st->ident.name;
        if (symtab_declared_in_current_scope(name)){

            handleAlreadyExist(name,*st->file_loc);
            st = st->next;
            continue;
        }


        symtab_insert(name,  id_use_create(create_id_attrs(*identityList.file_loc,constant_idk,
                                                               0)
                ,0));
        st = st->next;
    }

}
void scope_check_const_decls(const_decls_t const_decl){
    if (const_decl.start==NULL){
        if (DEBUG) {

            printf("NULL const_decl\n");
        }
        return;
    }
    //DO
    const_decl_t* cur = const_decl.start;

    while (cur != NULL) {
/*
        const_def_list_t def_list = cur->const_def_list;
        const_def_t *def = def_list.start;
        while(def != NULL){
            //TODO: declare identifiers
            def = def->next;
        }
*/
//        scope_check_constDefList( cur->const_def_list );
        scope_push_constDefList( cur->const_def_list);
        cur = cur->next;
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
            scope_check_assign_stmt(stmt->data.assign_stmt);
            break;
    }
}

void scope_check_stmts(stmts_t stmts) {
    if (stmts.stmt_list.start==NULL){
        return;
    }

    stmt_t *current = stmts.stmt_list.start;

    while (current != NULL) {
        scope_check_stmt(current);
        current = current->next;
    }
}


void scope_check_proc_decls(proc_decls_t procDecls){
    proc_decl_t *current = procDecls.proc_decls;
    while (current != NULL) {
        //declare identifier

        symtab_enter_scope();
        scope_check_program(*(current->block));
        symtab_leave_scope();

        current = current->next;
    }

}

void scope_push_identList(  ident_list_t identityList){

    ident_t * st = identityList.start;

    while (st != NULL) {
        if (symtab_declared_in_current_scope(st->name)){
            handleAlreadyExist(st->name,*st->file_loc);
            st = st->next;
            continue;
        }
        
        symtab_insert(st->name, (id_attrs *) id_use_create(create_id_attrs(*identityList.file_loc, variable_idk,
                                                                           0), 0));
        st = st->next;
    }
}

void scope_check_identList(  ident_list_t identityList, id_kind  type){
    ident_list_t curIdentList = identityList;

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

        scope_check_identList( cur->ident_list , variable_idk);
        scope_push_identList( cur->ident_list);
        cur = cur->next;
    }
}