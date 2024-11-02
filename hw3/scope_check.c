// Includes
#include <stdio.h>
#include "scope_check.h"
#include "ast.h"
#include "scope.h"
#include "utilities.h"
#include "symtab.h"
#include <stddef.h>

// Functions

//Runs scope check for as block
void scope_check_program_s(struct block_s block) {
    symtab_enter_scope();
    scope_check_const_decls(block.const_decls);
    scope_check_varDecls(block.var_decls);
    scope_check_proc_decls(block.proc_decls);
    scope_check_stmts(block.stmts);
    symtab_leave_scope();
}

//Runs scope check for entire program
void scope_check_program(block_t block) {
    symtab_enter_scope();
    scope_check_const_decls(block.const_decls);
    scope_check_varDecls(block.var_decls);
    scope_check_proc_decls(block.proc_decls);
    scope_check_stmts(block.stmts);
    symtab_leave_scope();
}

//Check if already exists
void handleAlreadyExist(const char *name, file_location loc, id_kind kind) {
    id_use *id = symtab_lookup(name);
    bail_with_prog_error(loc,
                         "%s \"%s\" is already declared as a %s",
                         kind2str(kind),
                         name, kind2str(id->attrs->kind));
}

// Push any constant definition into list
void scope_push_constDefList(const_def_list_t identityList) {
    const_def_t *st = identityList.start;

    while (st != NULL) {
        const char *name = st->ident.name;
        if (symtab_declared_in_current_scope(name)) {
            handleAlreadyExist(name, *st->file_loc, constant_idk);
            st = st->next;
            continue;
        }

        id_attrs *attrs = create_id_attrs(*identityList.file_loc, constant_idk, 0);

        symtab_insert(st->ident.name, attrs);
        st = st->next;
    }
}

// Read through the constant declarations and push them to
// the constant definition list
void scope_check_const_decls(const_decls_t const_decl) {
    if (const_decl.start == NULL) {
        return;
    }
    const_decl_t *cur = const_decl.start;

    while (cur != NULL) {
        scope_push_constDefList(cur->const_def_list);
        cur = cur->next;
    }
}


bool check_ident(const char *name, file_location loc) {
    bool idUsed = symtab_declared(name);

    if (!idUsed) {
        bail_with_prog_error(loc,
                             "identifier \"%s\" is not declared!",
                             name);
        return false;
    }
    return true;
}

void check_binary_expr(binary_op_expr_t bin) {
    struct expr_s *xp1 = bin.expr1;
    if (xp1 != NULL) {
        check_ident_express(*xp1);
    }
    struct expr_s *xp2 = bin.expr2;
    if (xp2 != NULL) {
        check_ident_express(*xp2);
    }
}

void check_ident_express(struct expr_s xp) {
    switch (xp.expr_kind) {
        case expr_bin:
            check_binary_expr(xp.data.binary);
            break;
        case expr_ident:
            ident_t ident = xp.data.ident;
            check_ident(ident.name, *ident.file_loc);
            break;
        case expr_number:
            break;
        case expr_negated:
            break;
        default:
            break;
    }
}

void scope_check_assign_stmt(assign_stmt_t assignStmt) {
    const char *assignName = assignStmt.name;
    if (check_ident(assignName, *assignStmt.file_loc)) {
        struct expr_s xp = *assignStmt.expr;
        check_ident_express(xp);
    }
}

void scope_check_if_stmt(if_stmt_t ifStmt) {
    condition_t cond = ifStmt.condition;

    if (cond.cond_kind == ck_db) {
        db_condition_t db = cond.data.db_cond;
        check_ident_express(db.dividend);
        check_ident_express(db.divisor);
    }

    if (cond.cond_kind == ck_rel) {
        rel_op_condition_t rel = cond.data.rel_op_cond;
        check_ident_express(rel.expr1);
        check_ident_express(rel.expr2);
    }

    if (ifStmt.then_stmts != NULL) {
        scope_check_stmts(*ifStmt.then_stmts);
    }
    if (ifStmt.else_stmts != NULL) {
        scope_check_stmts(*ifStmt.else_stmts);
    }
}

void scope_check_while_stmt(while_stmt_t while_stmt) {
    if (while_stmt.body != NULL) {
        scope_check_stmts(*while_stmt.body);
    }
}

void scope_check_stmt(stmt_t *stmt) {
    if (stmt == NULL)return; // NULL Check

    switch (stmt->stmt_kind) {
        case print_stmt:
            check_ident_express(stmt->data.print_stmt.expr);
            break;
        case read_stmt:
            check_ident(stmt->data.read_stmt.name, *stmt->data.read_stmt.file_loc);
            break;
        case block_stmt:
            struct block_s *block = stmt->data.block_stmt.block;
            scope_check_program_s(*block);
            break;
        case if_stmt:
            scope_check_if_stmt(stmt->data.if_stmt);
            break;
        case while_stmt:
            scope_check_while_stmt(stmt->data.while_stmt);
            break;
        case assign_stmt:
            scope_check_assign_stmt(stmt->data.assign_stmt);
            break;
        default:
            break;
    }
}


void scope_check_stmts(stmts_t stmts) {
    if (stmts.stmts_kind == empty_stmts_e) {
        return; //Epsilon case
    }

    if (stmts.stmt_list.start == NULL) {
        return;
    }

    stmt_t *current = stmts.stmt_list.start;

    while (current != NULL) {
        scope_check_stmt(current);
        current = current->next;
    }
}

void scope_check_proc_decls(proc_decls_t procDecls) {
    proc_decl_t *current = procDecls.proc_decls;

    while (current != NULL) {
        if (current->block != NULL) {
            scope_check_program_s(*current->block);
        }
        current = current->next;
    }
}

void scope_push_identList(ident_list_t identityList) {
    ident_t *st = identityList.start;

    while (st != NULL) {
        if (symtab_declared_in_current_scope(st->name)) {
            handleAlreadyExist(st->name, *st->file_loc, variable_idk);
            st = st->next;
            continue;
        }

        id_attrs *attrs = create_id_attrs(*identityList.file_loc, variable_idk, 0);

        symtab_insert(st->name, attrs);
        st = st->next;
    }
}

void scope_check_varDecls(var_decls_t varDecls) {
    if (varDecls.var_decls == NULL) {
        return;
    }

    var_decl_t *cur = varDecls.var_decls;

    while (cur != NULL) {
        scope_push_identList(cur->ident_list);
        cur = cur->next;
    }
}
