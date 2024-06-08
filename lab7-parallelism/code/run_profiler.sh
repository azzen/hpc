#!/bin/bash

nb_iterations=$1

if [ $# -ne 3 ]; then
    echo "Usage: $0 <nb_iterations> <min> <max>"
    exit 1
fi

while [ $nb_iterations -gt 0 ]; do
    rand=$(($2 + $RANDOM % $3))
    params="$(seq 1 $rand | shuf | tr -d '\n') --nthreads=16"
    echo params: $params
    coz run --- ./build/find_sequence inputs/1b.txt $params >/dev/null 2>&1
    nb_iterations=$((nb_iterations - 1))
done

nb_iterations=$1

while [ $nb_iterations -gt 0 ]; do
    rand=$(($2 + $RANDOM % $3))
    params="$(seq 1 $rand | shuf | tr -d '\n') --omp"
    echo params: $params
    coz run --- ./build/find_sequence inputs/1b.txt $params >/dev/null 2>&1
    nb_iterations=$((nb_iterations - 1))
done

nb_iterations=$1

while [ $nb_iterations -gt 0 ]; do
    rand=$(($2 + $RANDOM % $3))
    params="$(seq 1 $rand | shuf | tr -d '\n')"
    echo params: $params
    coz run --- ./build/find_sequence inputs/1b.txt $params >/dev/null 2>&1
    nb_iterations=$((nb_iterations - 1))
done