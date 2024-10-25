#!/usr/bin/bash
echo calling make clean
make clean
echo calling make compiler
make compiler
clear
echo calling custom.spl
./compiler custom.spl > custom.out

sleep 1
echo "Output of custom.out:"
echo "$(<custom.out)"
history -c
history -s "./test.sh", "make compiler"