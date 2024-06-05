#!/bin/bash

if [ $# -lt 1 ]; then
    echo "Usage: $0 <program> [program arguments]"
    exit 1
fi

# Check if lib/stb exists
if [ ! -d "./lib/stb" ]; then
    ./lib/setup.sh
else
    echo "lib/stb already exists, skipping setup.sh"
fi

# build the project
cmake -B build
cmake --build build

program_name=$(basename $1)
args="$*"

script=$(cat <<EOF
#!/usr/bin/env bpftrace
BEGIN
{   
    
    printf("Tracing the program $program_name... Hit CTRL+C to end\n");
}


tracepoint:syscalls:sys_enter_*
/comm == "$program_name"/
{
    @syscall_count[probe] = count();
}


tracepoint:exceptions:page_fault_user
/comm == "$program_name"/
{
    @page_faults = count();
}

hardware:cpu-cycles
/comm == "$program_name"/
{ 
    @cpu_cycles = count(); 
}


tracepoint:syscalls:sys_enter_openat
/comm == "$program_name"/
{
    printf("Opening file: %s\n", str(args->filename));
}

uretprobe:$1:distance
{
    @calls[probe] = count();
}
EOF
)

echo "$script" > bpftrace/load.bt

script=$(cat <<EOF
#!/usr/bin/env bpftrace
BEGIN
{   
    
    printf("Making an hist for the program $program_name... Hit CTRL+C to end\n");
}
tracepoint:syscalls:sys_enter_openat
/comm == "$program_name"/
{
    @start[tid] = nsecs;
}

tracepoint:syscalls:sys_exit_openat
/comm == "$program_name" && @start[tid]/
{
    @duration = hist(nsecs - @start[tid]);
    @start[tid] = 0;
}
EOF
)

echo "$script" > bpftrace/hist.bt

sudo chmod +x ./bpftrace/load.bt 
sudo ./bpftrace/load.bt &
PID=$!

sleep 5

eval "$args"

kill -INT $PID

wait $PID

sudo chmod +x ./bpftrace/hist.bt
sudo ./bpftrace/hist.bt &
PID=$!

sleep 5

eval "$args"

kill -INT $PID

wait $PID