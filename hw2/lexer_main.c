//
// Created by Matt on 10/6/2024.
//
// Includes
#include <stdio.h>

#include "lexer.h"
#include "parser_types.h"
#include "spl_lexer.h"
#include "spl.tab.h"





int main(int argc, char **argv) {

    if (argc!=2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 0;
    }
    lexer_init(argv[1]);
    lexer_output();
////    lexer_output();
//lexer_print_output_header();
//
//    AST dummy;
//    yytoken_kind_t t = yylex(&dummy);
//    lexer_print_token(t,1,"test");
//    fflush(stdout);
//

    return 0;
}
