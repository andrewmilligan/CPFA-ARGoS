#!/bin/bash
for i in 20 40 60 80 100 150 200 250
do
    ./runntimes.sh $1 experiments/CPFAU240minTargets${i}.xml results/CPFAU240minTargets${i}.txt &
done