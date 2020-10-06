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


SETUP_DIR=$INIT_DIR/install_tmp
mkdir -p $SETUP_DIR/distro # don't delete the actual distro directory until the very end
mkdir -p $INST_PATH/bin
cd $SETUP_DIR


# make sure tools installed can see the install loc of libraries
set +u
export LD_LIBRARY_PATH=`echo $INST_PATH/lib:$LD_LIBRARY_PATH | perl -pe 's/:\$//;'`
export PATH=`echo $INST_PATH/bin:$PATH | perl -pe 's/:\$//;'`
export MANPATH=`echo $INST_PATH/man:$INST_PATH/share/man:$MANPATH | perl -pe 's/:\$//;'`
set -u

##### DEPS for cgpBigWig #####

## libdeflate
if [ ! -e $SETUP_DIR/libdeflate.success ]; then
  rm -rf tmp_deflate
  mkdir -p tmp_deflate
  curl -sSL --retry 10 https://github.com/ebiggers/libdeflate/archive/${VER_LIBDEFLATE}.tar.gz > distro.tar.gz
  tar --strip-components 1 -C tmp_deflate -zxf distro.tar.gz
  cd tmp_deflate
  PREFIX=$INST_PATH make -j$CPU CFLAGS="-fPIC -O3" install
  cd ../
  rm -rf distro.* tmp_deflate
  touch $SETUP_DIR/libdeflate.success
fi

## HTSLIB (tar.bz2)
if [ ! -e $SETUP_DIR/htslib.success ]; then
  rm -rf htslib
  mkdir -p htslib
  curl -sSL --retry 10 https://github.com/samtools/htslib/releases/download/${VER_HTSLIB}/htslib-${VER_HTSLIB}.tar.bz2 > distro.tar.bz2
  tar --strip-components 1 -C htslib -jxf distro.tar.bz2
  cd htslib
  ./configure --enable-plugins --enable-libcurl --prefix=$INST_PATH \
    --with-libdeflate \
    CPPFLAGS="-I$INST_PATH/include" \
    LDFLAGS="-L${INST_PATH}/lib -Wl,-R${INST_PATH}/lib"
  make clean
  make -j$CPU
  make install
  cd $SETUP_DIR
  rm -rf distro.*
  touch $SETUP_DIR/htslib.success
fi

## LIB-BW (tar.gz)
if [ ! -e $SETUP_DIR/libBigWig.success ]; then
  curl -sSL --retry 10 https://github.com/dpryan79/libBigWig/archive/${VER_LIBBW}.tar.gz > distro.tar.gz
  rm -rf distro/*
  tar --strip-components 1 -C distro -xzf distro.tar.gz
  make -C distro clean
  make -C distro -j$CPU install prefix=$INST_PATH
  rm -rf distro.* distro/*
  touch $SETUP_DIR/libBigWig.success
fi

cd $HOME
rm -rf $SETUP_DIR
