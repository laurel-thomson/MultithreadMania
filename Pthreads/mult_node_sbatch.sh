#!/bin/bash

for i in 1
do
	sbatch --constraint=elves --ntasks-per-node=4 --nodes=${i} --time=24:00:00 sbatch.sh
done
