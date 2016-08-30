#!/bin/bash

# for bamstats
SOURCE_HTSLIB="https://github.com/samtools/htslib/archive/1.3.1.tar.gz"
# for fast merging of per-chr BW files
SOURCE_LIB_BW="https://github.com/dpryan79/libBigWig/archive/b36da5a06bffcc1b33c369e078b82f84625fd212.tar.gz"

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

set -e

if [ "$#" -ne "1" ] ; then
  echo "Please provide an installation path  such as /opt/ICGC"
  exit 0
fi

CPU=1

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

echo -n "Building htslib ..."
if [ -e $SETUP_DIR/htslib.success ]; then
  echo -n " previously installed ...";
else
  cd $SETUP_DIR
  get_distro "htslib" $SOURCE_HTSLIB
  mkdir -p htslib
  tar --strip-components 1 -C htslib -zxf htslib.tar.gz
  make -C htslib -j$CPU
  touch $SETUP_DIR/htslib.success
fi
echo

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

echo -n "Building cgpBigWig ..."
if [ -e $SETUP_DIR/cgpBigWig.success ]; then
  echo -n " previously installed ...";
else
  cd $INIT_DIR
  make -C c clean
  make -C c -j$CPU prefix=$INST_PATH LIB_LOCS="-L/Users/drj/homebrew/lib/"
  cp bin/bam2bedgraph $INST_PATH/bin/.
  cp bin/bwjoin $INST_PATH/bin/.
  cp bin/bam2bw $INST_PATH/bin/.
  cp bin/bwcat $INST_PATH/bin/.
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
