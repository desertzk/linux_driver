#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "用法: $0 <PID>"
    exit 1
fi

pid="$1"

# Read the pagemap file in binary and convert to hex using xxd
xxd -p -c 8 -g 8 "/proc/$pid/pagemap" | while read entry; do
    echo "Page Entry: 0x$entry"
done