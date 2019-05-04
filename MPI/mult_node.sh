#!/bin/bash

for i in 1 2 4 8 16
do
	sbatch --time=24:00:00 --ntasks-per-node=8 --nodes=${i} --constraint=elves mpi_sbatch.sh
done
