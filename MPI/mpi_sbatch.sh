#!/bin/bash -l
#SBATCH --job-name=enpayneMPI

#SBATCH --mem-per-cpu=4G   # Memory per core, use --mem= for memory per node
#SBATCH --time=10:00:00   # Use the form DD-HH:MM:SS
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=2
#SBATCH --constraint=elves

#SBATCH --mail-user=enpayne@ksu.edu
#SBATCH --mail-type=ALL   # same as =BEGIN,FAIL,END

module load OpenMPI
mpirun /homes/enpayne/OSProject4/MPI/mpi # change path to openMPI executable
