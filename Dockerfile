FROM ubuntu:16.04

# Set maintainer labels.
LABEL maintainer Keiran M. Raine <kr2@sanger.ac.uk>
LABEL maintainer Juan S. Medina <medinaj@mskcc.org>

# Install all dependencies in OPT.
ENV OPT /opt/bundle

# Export bundle bin and lib to PATH and PERL5LIB.
ENV PATH $OPT/bin:$PATH
ENV PERL5LIB $OPT/lib/perl5:$PERL5LIB

# Add repo.
COPY . /code

# Install package and dependencies.
RUN \
    # make dirs
    mkdir -p $OPT/bin $OPT/etc $OPT/lib $OPT/share && \
    \
    # Install system dependencies
    apt-get -yqq update --fix-missing && \
    apt-get -yqq install \
        build-essential \
        libbz2-dev \
        libcurl4-gnutls-dev \
        libgnutls-dev \
        liblzma-dev \
        libncurses5-dev \
        libssl-dev \
        locales \
        nettle-dev \
        wget \
        zlib1g-dev \
    && \
    apt-get clean && \
    \
    # Configure default locale, see below.
    echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen && \
    locale-gen en_US.utf8 && \
    /usr/sbin/update-locale LANG=en_US.UTF-8

# For best interoperability, it should be UTF-8.
# Some information about it here:
# https://github.com/rocker-org/rocker/issues/19
# http://jaredmarkell.com/docker-and-locales/
# http://click.pocoo.org/5/python3/
# https://crosbymichael.com/dockerfile-best-practices-take-2.html
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8

# Install package.
RUN \
    cd /code && \
    ./setup.sh $OPT && \
    cd ~ && \
    rm -rf /code

# Set volume to data as per:
# https://github.com/BD2KGenomics/cgl-docker-lib
VOLUME /data
WORKDIR /data

# Entry point set to wrapper.sh as per:
# https://github.com/BD2KGenomics/cgl-docker-lib
ENTRYPOINT ["bash"]
