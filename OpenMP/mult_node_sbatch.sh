#!/bin/bash

for i in 1 2 4 8 16
do
	 sbatch --constraint=elves --ntasks-per-node=8 --nodes=${i} --time=24:00:00 openmp_sbatch.sh
done
