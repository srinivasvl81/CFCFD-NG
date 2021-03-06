#! /bin/bash
# run.sh
#PBS -S /bin/bash
#PBS -N mCyl_kw_4block
#PBS -l walltime=3:00:00
#PBS -l select=1:ncpus=4:mpiprocs=4 -A uq-MechMinEng
#PBS -V

job=mallinson_cylinder
np=4

. /usr/share/modules/init/bash
module load intel-mpi
module load intel-cc-11
module load python

echo "Where are my nodes?"
echo $PBS_NODEFILE
cat $PBS_NODEFILE
echo "----------------------------------------"
cd $PBS_O_WORKDIR
echo "Start time: "; date
mpirun -np $np e3mpi.exe --job=$job --run > LOGFILE
echo "Finish time: "; date
