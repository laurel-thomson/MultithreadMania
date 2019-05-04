#!/bin/bash

for i in 1 2 4 8 16
do
	sbatch --time=24:00:00 --ntasks-per-node=${i} --nodes=1 --constraint=elves sbatch2.sh
done
