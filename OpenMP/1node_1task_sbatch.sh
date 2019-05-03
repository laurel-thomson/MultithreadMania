#!/bin/bash

for i in 1
do
	 sbatch --constraint=elves --ntasks-per-node=${i} --nodes=1 --time=24:00:00 openmp_sbatch.sh
done
