#!/bin/bash

command -v nasm >/dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "Error: NASM not found in PATH"
    exit $?
fi