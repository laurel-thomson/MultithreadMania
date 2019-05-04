#!/bin/bash

for i in 1 2 4 8
do
	sbatch --time=24:00:00 --ntasks=8 --nodes=${i} --constraint=elves mpi_sbatch.sh
done
