cmake_minimum_required(VERSION 3.28)
project(hw1 C)

set(CMAKE_C_STANDARD 11)

add_executable(hw1
        ast.c
        bof.c
        asm_lexer.c
        asm_main.c
        asm_unparser.c
        assemble.c
        bof_bin_dump.c
        char_utilities.c
        disasm.c
        disasm_main.c
        file_location.c
        instruction.c
        lexer.c
        machine_types.c
        pass1.c
        regname.c
        symtab.c
        utilities.c)
