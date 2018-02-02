FROM  ubuntu:16.04 as builder

USER root

ENV OPT /opt/wtsi-cgp
ENV PATH $OPT/bin:$PATH
ENV LD_LIBRARY_PATH $OPT/lib

RUN apt-get -yq update
RUN apt-get install -yq --no-install-recommends\
 curl\
 build-essential\
 zlib1g-dev\
 libcurl4-openssl-dev\
 liblzma-dev\
 nettle-dev\
 libncurses5-dev

RUN mkdir -p $OPT/bin

ADD build/opt-build.sh build/
RUN bash build/opt-build.sh $OPT

#TODO add scropt for mapping.



FROM  ubuntu:16.04

MAINTAINER  drj@sanger.ac.uk

LABEL uk.ac.sanger.cgp="Cancer Genome Project, Wellcome Trust Sanger Institute" \
      version="1.0.0" \
      description="cgpBigWig for dockstore.org"

ENV OPT /opt/wtsi-cgp
ENV PATH $OPT/bin:$PATH
ENV LD_LIBRARY_PATH $OPT/lib
ENV LC_ALL C

RUN apt-get -yq update
RUN apt-get install -yq --no-install-recommends\
  curl\
  bzip2\
  zlib1g\
  liblzma5\
  libncurses5

RUN mkdir -p $OPT
COPY --from=builder $OPT $OPT

## USER CONFIGURATION
RUN adduser --disabled-password --gecos '' ubuntu && chsh -s /bin/bash && mkdir -p /home/ubuntu

USER    ubuntu
WORKDIR /home/ubuntu

CMD ["/bin/bash"]
