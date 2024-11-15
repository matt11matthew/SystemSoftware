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
code_seq gen_code_stmts(stmts_t stmts);
void gen_code_initialize();
void gen_code_program(BOFFILE bf, block_t prog);

#endif //GEN_CODE_H
