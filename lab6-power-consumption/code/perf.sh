#!/bin/bash

events=("power/energy-psys/" "power/energy-cores/" "power/energy-gpu/" "power/energy-pkg/")

for evt in ${events[@]}; do
    perf stat \
        -e $evt \
        -- ./build/segmentation img/nyc.png 100 out.png
done

rm out.png