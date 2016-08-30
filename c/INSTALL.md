#Install:
`./setup.sh /install/to/here`

`'/install/to/here'` is where you want the bin/lib folders to be created.

setup.sh will install
- cgpBigWig tools bwjoin, bwcat, bam2bw and bam2bedgraph

#OS:
  This distribution will only work on *NIX type systems at present.

#Other Software
  For installation to proceed you require the following packages:



##For Ubuntu (tested with 14.04)
```
apt-get update && \
apt-get -y install \
build-essential zlib1g-dev libncurses5-dev libcurl4-gnutls-dev libssl-dev libexpat1-dev nettle-dev &&\
apt-get clean
```

##For Amazon Linux AMI (2016.03.0 x86_64)
```
yum -q -y update && \
yum -y install \
make glibc-devel gcc patch ncurses-devel expat-devel perl-core openssl-devel libcurl-devel gnutls-devel libtasn1-devel p11-kit-devel gmp-devel nettle-devel
```

**Should nettle-devel not exist**
Use the following

```
yum -q -y install autoconf
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
