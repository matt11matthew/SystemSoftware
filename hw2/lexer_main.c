//
// Created by Matt on 10/6/2024.
//
// Includes
#include <stdio.h>

#include "lexer.h"
#include "parser_types.h"
#include "spl_lexer.h"


int main(int argc, char **argv) {

    if (argc!=2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 0;
    }
    lexer_init(argv[1]);
    lexer_print_output_header();
//

    if(!lexer_has_errors()){// If lexer has no errors display output
//        printf("NO ERRORS");
        lexer_output();
    } else {
//        printf("ERRORS");
    }
////    loadFile(argv[1]);
//
    yywrap();// Used to close file
    return 0;
}