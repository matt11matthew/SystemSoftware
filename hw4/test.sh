#!/usr/bin/bash

echo "calling make clean"
sudo make clean

echo "calling make compiler"
sudo make compiler

clear

echo "calling custom.spl"
if [[ "$1" == "-v" ]]; then
  sudo valgrind ./compiler custom.spl > custom.out 2>&1
elif [[ "$1" == "-g" ]]; then
  sudo gdb ./compiler custom.spl
else
  sudo ./compiler custom.spl > custom.out 2>&1
fi

sleep 1

echo "Output of custom.out:"
echo "$(<custom.out)"
