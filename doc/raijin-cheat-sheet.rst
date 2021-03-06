======================================
Cheat Sheet for Using Eilmer on Raijin
======================================
:Author: Rowan J. Gollan
:Date: 05-Feb-2014
:History: 27-Apr-2017
          Updated instructions to include NENZFr

Logging in
----------

The hostname for the supercompute cluster is ``raijin.nci.org.au``.

Login is via ssh. From a linux terminal, for example::

  > ssh username@raijin.nci.org.au

Compile and install your own SWIG
---------------------------------

Download and copy across the latest version of SWIG.
At the time of writing, that version is 3.0.12.

Unpack and get ready for compilation::

  tar -xvf swig-3.0.12.tar.gz
  cd swig-3.0.12
  module unload intel-fc
  module unload intel-cc
  module load intel-cc/17.0.1.132
  module load python/2.7.5
  ./configure CC=icc CXX=icpc --prefix=$HOME/opt/swig-3.0.12

Then compile and install::

  make
  make install

Setting your computational environment
--------------------------------------

Edit your ``.bashrc`` file and comment out the lines
that load the intel compilers::

  #module load intel-fc
  #module load intel-cc
  
The lines might be at ``.login`` and ``.profile`` files as well,
so check these files and comment out the lines if possible.

Eilmer needs newer versions of the intel compilers and these lines would load the default versions.

Then add these lines to the end of your ``.bashrc``::

  module load intel-cc/17.0.1.132
  module load mercurial
  module load python/2.7.5
  module load openmpi/2.0.0
  
  export PATH=${HOME}/opt/swig-3.0.12/bin:${PATH}:${HOME}/e3bin
  export LUA_PATH=${HOME}/e3bin/?.lua
  export LUA_CPATH=${HOME}/e3bin/?.so
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${HOME}/e3bin

This sets your environment ready for compiling and running.
It's easiest to log out and back in to have the changes take effect.

If you are an OpenFOAM user on raijin cluster, then add the following lines to the end of your ``.bashrc``::

  module load openmpi/1.6.3
  module load OpenFOAM/2.2.1
  export WM_PROJECT_VERSION=2.2.1
  export WM_PROJECT_USER_DIR=$HOME/$WM_PROJECT/$USER-$WM_PROJECT_VERSION
  export FOAM_USER_APPBIN=$WM_PROJECT_USER_DIR/platforms/$WM_OPTIONS/bin
  export FOAM_USER_LIBBIN=$WM_PROJECT_USER_DIR/platforms/$WM_OPTIONS/lib
  export PATH=$PATH:$FOAM_USER_APPBIN
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$FOAM_USER_LIBBIN
  
Please note that the current version of OpenFOAM on raijin cluster is 2.2.1. You should change
your ``.bashrc`` file if higher version is used.

17-04-15: Now OpenFOAM/2.3.1 is also available on raijin.

Compiling eilmer
----------------

Get a fresh copy of the repository::

  > hg clone https://source.eait.uq.edu.au/hg/cfcfd3 cfcfd3

Username: cfcfd-user@svn.itee.uq.edu.au
Password: hyper-socks

Copy the ``cea2`` source directory to ``cfcfd3/app/extern/``.

Compile eilmer::

  > cd cfcfd3/app/eilmer3/build
  > make TARGET=for_openmpi_intel install

Note that the default (and recommended) MPI environment is openmpi.
This version has been compiled with the intel compiler, that's why we load the intel compiler above.

UPDATE: 28-Jan-2015
If the warning that's issued when compiling block_io.cxx bothers you,
it can be suppressed by ignoring that particular warning:

  > OMPI_CXXFLAGS=-wd823 make TARGET=for_openmpi_intel install

Compiling NENZFr
----------------

After completing the install of Eilmer, then you can follow that
with a NENZFr installation if required::

  > cd cfcfd3/app/nenzfr
  > make TARGET=for_openmpi_intel install

Submitting a job
----------------

You should run your jobs from the ``/short`` disk, so copy your working files over to that partition.
(It's ok to leave the source code in your home area).

Your ``/short`` directory is of the form::

  /short/"project-no"/"username"

For example, my ``/short`` directory is::
  
  /short/fb5/rog564
  
The actual job submission would look something like::

  > cd /short/fb5/rog564
  > cd sim_dir
  --- prepare submit script ---
  > qsub submit-script

An example submit script is::

  #!/bin/bash
  #PBS -N test-job
  #PBS -P fb5
  #PBS -l walltime=1:00:00
  #PBS -l ncpus=64
  #PBS -l mem=5GB
  #PBS -l wd
  
  echo "Start MPI job...."
  date
  mpirun e3mpi.exe --job=mms --run
  echo "End MPI job."
  date

Note that on raijin, a node has 16 processors. If you request more than one node, then
you need to request in multiples of 16, even if you don't use all of those processors.
The account will be charged though as if you used all processors requested.
This is not really a problem using eilmer: you can use the SuperBlock facility
to split one large block into many smaller blocks and the ``e3loadbalance`` utility
along with the ``--mpimap`` option to run the code on a desired number of
processors (which will typically be a multiple of 16).

Account usage
-------------
The available compute hours in each quarter, and how many of those
hours have been used are available via the ``nci_account`` command:

  > nci_account -P fb5 -p 2014.q1

More information
----------------
The National Facility maintains a User Guide for Raijin on a wiki page::

  http://nf.nci.org.au/wiki/RaijinUserGuide

Some more general information about the National Facility, such as the
available file systems, is available at::

  http://nf.nci.org.au/facilities/userguide




  
