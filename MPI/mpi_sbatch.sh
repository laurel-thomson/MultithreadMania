#!/bin/bash -l
##$ -l h_rt=1-00:00:00
#SBATCH -o openMPI.out

module load OpenMPI

mpirun /samant4/openMPI # change path to openMPI code
