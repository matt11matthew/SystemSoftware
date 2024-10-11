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
        fprintf(stderr, "Usage: %s <file>\n", argv[0]); //Usage
        return 0;
    }

    lexer_init(argv[1]); //Init lexer
    lexer_output(); //Print lexer output

    if (lexer_has_errors()){
        exit(1);
        return 1;
    }

    exit(0);
    return 0;
}
