#!/usr/bin/bash
echo calling make clean
sudo make clean
echo calling make compiler
sudo make compiler
clear
echo calling custom.spl
sudo ./compiler custom.spl > custom.out 2>&1

sleep 1
echo "Output of custom.out:"
echo "$(<custom.out)"