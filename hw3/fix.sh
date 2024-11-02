#!/bin/bash

# Set the directory path (current directory by default)
directory="."

# Loop through each file in the current directory
for file in "$directory"/*; do
    # Check if it is a regular file (not a directory)
    if [ -f "$file" ]; then
        chmod +x "$file"
        echo "Added execute permission to: $file"
    fi
done