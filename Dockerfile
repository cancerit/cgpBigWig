FROM  ubuntu:16.04 as builder

USER  root

ARG VER_HTSLIB="1.9"
ARG VER_LIBBW="0.4.2"

ENV OPT /opt/wtsi-cgp
ENV PATH $OPT/bin:$PATH
ENV LD_LIBRARY_PATH $OPT/lib
ENV LC_ALL C

RUN apt-get -yq update
RUN apt-get install -yq --no-install-recommends\
  build-essential\
  apt-transport-https\
  curl\
  ca-certificates\
  make\
  bzip2\
  gcc\
  libtasn1-dev\
  libgnutls-dev\
  nettle-dev\
  libgmp-dev\
  libp11-kit-dev\
  zlib1g-dev\
  libbz2-dev\
  liblzma-dev\
  libcurl4-gnutls-dev\
  libncurses5-dev

RUN mkdir -p $OPT/bin

WORKDIR /install_tmp

ADD build/opt-build.sh build/
RUN bash build/opt-build.sh $OPT

COPY . .
RUN bash build/opt-build-local.sh $OPT

FROM  ubuntu:16.04

LABEL maintainer="cgphelp@sanger.ac.uk"\
      uk.ac.sanger.cgp="Cancer, Ageing and Somatic Mutation, Wellcome Sanger Institute" \
      version="1.1.0" \
      description="cgpBigWig"

ENV OPT /opt/wtsi-cgp
ENV PATH $OPT/bin:$PATH
ENV LD_LIBRARY_PATH $OPT/lib
ENV LC_ALL C

RUN apt-get -yq update
RUN apt-get install -yq --no-install-recommends \
apt-transport-https \
curl \
ca-certificates \
bzip2 \
zlib1g \
liblzma5 \
libncurses5 \
unattended-upgrades && \
unattended-upgrade -d -v && \
apt-get remove -yq unattended-upgrades && \
apt-get autoremove -yq

RUN mkdir -p $OPT
COPY --from=builder $OPT $OPT

## USER CONFIGURATION
RUN adduser --disabled-password --gecos '' ubuntu && chsh -s /bin/bash && mkdir -p /home/ubuntu

USER    ubuntu
WORKDIR /home/ubuntu

CMD ["/bin/bash"]
