//
// Created by matt1 on 10/18/2024.
//


#include "scope_check.h"
#include "ast.h"
#include <stddef.h>
void scope_check_program(block_t block) {
   // printf("1: %s", block.file_loc->filename); //We have the file name

    const_decl_t *current = block.const_decls.start;



//    printf("\n");
    while (current != NULL) {
        const_def_list_t list = current->const_def_list;

        const_def_t *start = list.start;
        while (start != NULL) {
            //printf("%s=%s\n",start->ident.name, start->number.text);
            start = start->next;



        }




        current = current->next;
    }
}