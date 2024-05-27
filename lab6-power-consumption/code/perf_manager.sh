#!/bin/bash

mkfifo perf_fifo.ctl
mkfifo perf_fifo.ack

exec {perf_ctl_fd}<>perf_fifo.ctl
exec {perf_ack_fd}<>perf_fifo.ack

export PERF_CTL_FD=${perf_ctl_fd}
export PERF_ACK_FD=${perf_ack_fd}

events=("power/energy-psys/" "power/energy-cores/" "power/energy-gpu/" "power/energy-pkg/")

for evt in ${events[@]}; do
    perf stat \
        -e $evt \
        --delay=-1 \
        --control fd:${perf_ctl_fd},${perf_ack_fd} \
        -- ./build/segmentation_perf_simd img/nyc.png 10 out.png
done

rm perf_fifo.*