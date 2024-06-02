#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: $0 <program> [program arguments]"
    exit 1
fi

args="$*"

sudo ./bpftrace/load.bt "$args" &
PID=$!

sleep 5

eval "$args"

kill -INT $PID

wait $PID