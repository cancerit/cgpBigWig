#!/bin/bash

########## LICENSE ##########
# Copyright (c) 2016 Genome Research Ltd.
#
# Author: Cancer Genome Project cgpit@sanger.ac.uk
#
# This file is part of cgpBigWig.
#
# cgpBigWig is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#    1. The usage of a range of years within a copyright statement contained within
#    this distribution should be interpreted as being equivalent to a list of years
#    including the first and last year specified and all consecutive years between
#    them. For example, a copyright statement that reads ‘Copyright (c) 2005, 2007-
#    2009, 2011-2012’ should be interpreted as being identical to a statement that
#    reads ‘Copyright (c) 2005, 2007, 2008, 2009, 2011, 2012’ and a copyright
#    statement that reads ‘Copyright (c) 2005-2012’ should be interpreted as being
#    identical to a statement that reads ‘Copyright (c) 2005, 2006, 2007, 2008,
#    2009, 2010, 2011, 2012’."
#
###########################

# for bamstats
SOURCE_HTSLIB="https://github.com/samtools/htslib/releases/download/1.3.2/htslib-1.3.2.tar.bz2"
#libBigWig - bigwig access library
SOURCE_LIB_BW="https://github.com/dpryan79/libBigWig/archive/0.1.7.tar.gz"

get_distro () {
  EXT=""
  if [[ $2 == *.tar.bz2* ]] ; then
    EXT="tar.bz2"
  elif [[ $2 == *.zip* ]] ; then
    EXT="zip"
  elif [[ $2 == *.tar.gz* ]] ; then
    EXT="tar.gz"
  else
    echo "I don't understand the file type for $1"
    exit 1
  fi
  if hash curl 2>/dev/null; then
    curl -sS -o $1.$EXT -L $2
  else
    wget -nv -O $1.$EXT $2
  fi
}

get_file () {
# output, source
  if hash curl 2>/dev/null; then
    curl -sS -o $1 -L $2
  else
    wget -nv -O $1 $2
  fi
}

if [ "$#" -ne "1" ] ; then
  echo "Please provide an installation path  such as /opt/ICGC"
  exit 0
fi

CPU=1
ls /proc/cpuinfo >& /dev/null # very noddy attempt to figure out thread count
if [ $? -eq 0 ]; then
  CPU=`grep -c ^processor /proc/cpuinfo`
else
  CPU=`sysctl -a | grep machdep.cpu | grep thread_count | awk '{print $2}'`
  if [ $? -ne 0 ]; then
    # fall back to unthreaded core test
    CPU=`sysctl -a | grep machdep.cpu | grep core_count | awk '{print $2}'`
  fi
fi
if [ "$CPU" -gt "6" ]; then
  CPU=6
fi
echo "Max compilation CPUs set to $CPU"

set -e

INST_PATH=$1

# get current directory
INIT_DIR=`pwd`

# cleanup inst_path
mkdir -p $INST_PATH
cd $INST_PATH
INST_PATH=`pwd`
mkdir -p $INST_PATH/bin
cd $INIT_DIR

#create a location to build dependencies
SETUP_DIR=$INIT_DIR/install_tmp
mkdir -p $SETUP_DIR

echo -n "Get htslib ..."
if [ -e $SETUP_DIR/htslibGet.success ]; then
  echo " already staged ...";
else
  echo
  cd $SETUP_DIR
  get_distro "htslib" $SOURCE_HTSLIB
  touch $SETUP_DIR/htslibGet.success
fi

echo -n "Building htslib ..."
if [ -e $SETUP_DIR/htslib.success ]; then
  echo " previously installed ...";
else
  echo
  mkdir -p htslib
  tar --strip-components 1 -C htslib -jxf htslib.tar.bz2
  cd htslib
  ./configure --enable-plugins --enable-libcurl --prefix=$INST_PATH
  make -j$CPU
  make install
  rm -f $INST_PATH/lib/libhts.so*
  cd $SETUP_DIR
  touch $SETUP_DIR/htslib.success
fi

cd $INIT_DIR

echo -n "Building libBigWig ..."
if [ -e $SETUP_DIR/libBigWig.success ]; then
  echo -n " previously installed ...";
else
  cd $SETUP_DIR
  get_distro "libBigWig" $SOURCE_LIB_BW
  mkdir -p libBigWig
  tar --strip-components 1 -C libBigWig -zxf libBigWig.tar.gz
  make -C libBigWig -j$CPU install prefix=$INST_PATH
  rm -f $INST_PATH/lib/libBigWig.so
  touch $SETUP_DIR/libBigWig.success
fi
echo

export HTSLIB="$SETUP_DIR/htslib"
cd $INIT_DIR


echo -n "Building cgpBigWig ..."
if [ -e $SETUP_DIR/cgpBigWig.success ]; then
  echo -n " previously installed ...";
else
  cd $INIT_DIR
  make -C c clean
  make -C c -j$CPU prefix=$INST_PATH HTSLIB=$SETUP_DIR/htslib
  cp bin/bam2bedgraph $INST_PATH/bin/.
  cp bin/bwjoin $INST_PATH/bin/.
  cp bin/bam2bw $INST_PATH/bin/.
  cp bin/bwcat $INST_PATH/bin/.
  cp bin/bam2bwbases $INST_PATH/bin/.
  cp bin/bg2bw $INST_PATH/bin/.
  cp bin/detectExtremeDepth $INST_PATH/bin/.
  touch $SETUP_DIR/cgpBigWig.success
  # need to clean up as will clash with other version
  make -C c clean
fi
echo

# cleanup all junk
rm -rf $SETUP_DIR

echo
echo
echo "Please add the following to beginning of path:"
echo "  $INST_PATH/bin"
echo

exit 0
