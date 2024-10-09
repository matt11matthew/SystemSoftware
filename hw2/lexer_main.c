//
// Created by Matt on 10/6/2024.
//
// Includes
#include <stdio.h>

#include "lexer.h"
#include "parser_types.h"
#include "spl_lexer.h"
#include "spl.tab.h"


void t_lexer_output()
{
    lexer_print_output_header();
    AST dummy;
    yytoken_kind_t t;
    do {
        t = yylex(&dummy);

        if (t == YYEOF) {

            break;
        }
        lexer_print_token(t, yylineno, yytext);
    } while (t != YYEOF);
}


int main(int argc, char **argv) {

    if (argc!=2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 0;
    }
    lexer_init(argv[1]);
    t_lexer_output();
    return 0;
}
