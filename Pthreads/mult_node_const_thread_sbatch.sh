#!/bin/bash

for i in 1 2 4 8 16
do
	sbatch --constraint=elves --ntasks=8 --nodes=${i} --time=24:00:00 sbatch.sh
done
