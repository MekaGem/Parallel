#!/bin/sh
mpic++ -O3 ./integral.cpp
echo "Processes: $1 Intervals: $2 Temerature: $3"
mpirun -np $1 -mca btl ^openib ./a.out $2 $3
