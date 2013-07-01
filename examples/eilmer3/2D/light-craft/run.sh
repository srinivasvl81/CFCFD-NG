#! /bin/bash
# run.sh
module load openmpi-x86_64
date
mpirun -np 4 e3mpi.exe --job=lc --mpimap=lc.mpimap \
    --run > run.transcript
date

echo "At this point, we should have a flow solution"
echo "Use post.sh next"

