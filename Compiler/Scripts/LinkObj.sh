#!/bin/bash

ld -o "$1" -e _start $2

status=$?
if [ $status -ne 0 ]; then
    echo "Linking failed with error $status"
    exit $status
fi