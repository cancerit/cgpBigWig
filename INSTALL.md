#Install:
`./setup.sh /install/to/here`

`'/install/to/here'` is where you want the bin/lib folders to be created.

setup.sh will install
- cgpBigWig tools bwjoin, bwcat, bam2bw, bam2bwbases and bam2bedgraph

#OS:
  This distribution will only work on *NIX type systems at present.

#Other Software
  For installation to proceed you require the following packages:

##For Ubuntu (tested with 14.04)
```
apt-get update && \
apt-get -y install \
build-essential zlib1g-dev libncurses5-dev libcurl4-gnutls-dev libssl-dev nettle-dev libbz2-dev liblzma-dev &&\
apt-get clean
```

##For Amazon Linux AMI (2016.03.3 x86_64)
```
yum -q -y update && \
yum -y install \
make glibc-devel gcc patch ncurses-devel openssl-devel libcurl-devel gnutls-devel libtasn1-devel p11-kit-devel gmp-devel nettle-devel libbz2-devel liblzma-devel
```

**Should nettle-devel not exist**
Use the following

```
yum -q -y install autoconf ghostscript texinfo-tex
wget https://git.lysator.liu.se/nettle/nettle/repository/archive.tar.gz?ref=nettle_3.2_release_20160128 -O nettle.tar.gz
mkdir -p nettle
tar --strip-components 1 -C nettle -zxf nettle.tar.gz
cd nettle
./.bootstrap
./configure --disable-pic --disable-shared && \
sudo make && \
sudo make check && \
sudo make install && \
cd .. && \
rm -rf nettle nettle.tar.gz
```

##OSX
  In order for cgpBigWig to install you may need the following packages installed
```
brew install gnutls
# fixes gnutls include
export LIBRARY_PATH="$HOME/homebrew/lib"
export LDFLAGS="-L$HOME/homebrew/opt/libffi/lib"
export PKG_CONFIG_PATH="$HOME/homebrew/opt/libffi/lib/pkgconfig"
brew install libffi
# from https://p11-glue.freedesktop.org/doc/p11-kit/devel-building.html
wget https://p11-glue.freedesktop.org/releases/p11-kit-0.23.2.tar.gz
tar zxf p11-kit-0.23.2.tar.gz
cd p11-kit-0.23.2
./configure --prefix=$HOME/local --without-trust-paths
make
```
