//
// Created by Matt on 9/30/2024.
//
#include <stdio.h>
#include <string.h>
#include "bof.h"
#include "vm.h"

int main(int argc, char **argv) {

    int shouldPrint = false;
    char *fileName;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        shouldPrint = true;  // Enable printing
        fileName = argv[2];   // File name is the second argument
    }
    else if (argc == 2) {
        fileName = argv[1];   // File name is the first argument
    }
    else {
        fprintf(stderr, "Usage: %s [-p] <BOF file>\n", argv[0]);
        return 1;  // Return non-zero for usage error
    }

    handleBOFFile(fileName, shouldPrint);// Begin reading the BOF file and handle printing if enabled

    return 0;
}