# This is just a sample PBS script. You should adapt the
# following such that you can run all your experiments.
# For more information about Torque and PBS, check the following sources:
#    https://support.cc.gatech.edu/facilities/instructional-labs/how-to-run-jobs-on-the-instructional-hpc-clusters
#    http://www.democritos.it/activities/IT-MC/documentation/newinterface/pages/runningcodes.html


# allocate 4 of the (24 total) sixcore nodes for up to 5 minutes

#PBS -q class
#PBS -l nodes=4:sixcore
#PBS -l walltime=02:00:00
#PBS -N cse6220-findmotifs

# TODO: change this to your project directory relative to your home directory
#       (= $HOME)
#export PBS_O_WORKDIR=$HOME/path/to/your/project 
export PBS_O_WORKDIR=$HOME/dev


# TODO:
# try various different configurations and different input files
# it could be a good idea to create loops over different input files
# (or randomly generated on the fly using `generate_input` using different
# input sizes)
# and trying different computation depths for the master
# (again, you might want to add this to a loop and increase the total all
#  time accordingly)
INPUT_FILE=$PBS_O_WORKDIR/input.txt
OUTPUT_FILE=$PBS_O_WORKDIR/32_10_result.txt
MASTER_DEPTH=6
EXE=$PBS_O_WORKDIR/findmotifs

#---------------------------------------------------
# Set by adsouza31
# 
export PBS_O_WORKDIR=$HOME/dev
export PBS_NODEFILE=$PBS_O_WORKDIR/host-mpich.txt
INPUT_FILE=$PBS_O_WORKDIR/input.txt
OUTPUT_FILE=$PBS_O_WORKDIR/result.txt
MASTER_DEPTH=6
EXE=$PBS_O_WORKDIR/findmotifs
#
#
#---------------------------------------------------

# loop over number of processors (just an example, uncomment to use the loop)
 for p in 4 8 12 16 24 
 do
  for MASTER_DEPTH in 4 10 14
  do
  # test for each file
  for file in 200_30_17 300_33_15 400_34_15 500_35_15 600_36_16
  do
    INPUT_FILE=$PBS_O_WORKDIR/input/$file.txt
    OUTPUT_FILE=$PBS_O_WORKDIR/fresult/$file.out.txt
    OMPI_MCA_mpi_yield_when_idle=0 mpirun --hostfile $PBS_NODEFILE -np $p $EXE $MASTER_DEPTH $INPUT_FILE $OUTPUT_FILE > $PBS_O_WORKDIR/fresult/p-$p-dp-$MASTER_DEPTH-fn-$file.txt 2>&1
  done
  done
done

#    cat $PBS_O_WORKDIR/input/$file.sol |sort > $PBS_O_WORKDIR/result/${file}.sol
#    cat $OUTPUT_FILE |sort > $PBS_O_WORKDIR/result/r_p${p}_${file}.sol
#    diff $PBS_O_WORKDIR/result/${file}.sol $PBS_O_WORKDIR/result/r_p${p}_${file}.sol >  $PBS_O_WORKDIR/result/p${p}_${file}.diff
# loop over number of processors (just an example, uncomment to use the loop)
# for p in 2 4 8 12 16 24
# do
#OMPI_MCA_mpi_yield_when_idle=0 mpirun --hostfile $PBS_NODEFILE -np $p $EXE $MASTER_DEPTH $INPUT_FILE $OUTPUT_FILE
# done

