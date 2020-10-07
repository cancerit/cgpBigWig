#!/bin/bash

set -ex

if [[ -z "${TMPDIR}" ]]; then
  TMPDIR=/tmp
fi

set -u

if [ "$#" -lt "1" ] ; then
  echo "Please provide an installation path such as /opt/ICGC"
  exit 1
fi

# get path to this script
SCRIPT_PATH=`dirname $0`;
SCRIPT_PATH=`(cd $SCRIPT_PATH && pwd)`

# get the location to install to
INST_PATH=$1
mkdir -p $1
INST_PATH=`(cd $1 && pwd)`
echo $INST_PATH

# get current directory
INIT_DIR=`pwd`

CPU=`grep -c ^processor /proc/cpuinfo`
if [ $? -eq 0 ]; then
  if [ "$CPU" -gt "6" ]; then
    CPU=6
  fi
else
  CPU=1
fi
echo "Max compilation CPUs set to $CPU"

mkdir -p $INST_PATH/bin

# make sure tools installed can see the install loc of libraries
set +u
export LD_LIBRARY_PATH=`echo $INST_PATH/lib:$LD_LIBRARY_PATH | perl -pe 's/:\$//;'`
export PATH=`echo $INST_PATH/bin:$PATH | perl -pe 's/:\$//;'`
export MANPATH=`echo $INST_PATH/man:$INST_PATH/share/man:$MANPATH | perl -pe 's/:\$//;'`
set -u

##### cgpBigWig installation
export LFLAGS="-L${INST_PATH}/lib"
make -C c clean
make -C c -j$CPU prefix=$INST_PATH HTSLIB=$INST_PATH/lib
cp bin/bam2bedgraph $INST_PATH/bin/.
cp bin/bwjoin $INST_PATH/bin/.
cp bin/bam2bw $INST_PATH/bin/.
cp bin/bwcat $INST_PATH/bin/.
cp bin/bam2bwbases $INST_PATH/bin/.
cp bin/bg2bw $INST_PATH/bin/.
cp bin/detectExtremeDepth $INST_PATH/bin/.
make -C c clean

cd $HOME
