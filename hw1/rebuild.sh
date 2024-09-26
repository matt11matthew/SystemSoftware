#!/bin/bash

echo "Cleaning and rebuilding the VM..."
make clean
make vm
echo "Rebuild complete!"