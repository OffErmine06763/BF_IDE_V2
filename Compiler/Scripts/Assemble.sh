#!/bin/bash

while [ $# -ge 2 ]; do
    nasm -f elf64 "$1" -o "$2" -Wno-pp-macro-params-legacy

    status=$?
    if [ $status -ne 0 ]; then
        echo "Assembly failed with error $status"
        exit $status
    fi

    shift 2
done