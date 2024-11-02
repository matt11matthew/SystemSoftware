#!/usr/bin/bash

echo "calling make clean"
 make clean

echo "calling make compiler"
 make compiler

clear

echo "calling custom.spl"
if [[ "$1" == "-v" ]]; then
   valgrind ./compiler custom.spl
elif [[ "$1" == "-g" ]]; then
   gdb ./compiler custom.spl
else
   ./compiler custom.spl
fi


