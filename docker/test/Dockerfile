# Pull base image.
FROM ubuntu:20.04

# This is needed to prevent locales installation from being interactive.
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -qq update && apt-get -qq upgrade -y && \
    apt-get -qq install -y --no-install-recommends \
        build-essential scons autoconf automake libtool make g++ cmake \
        git pkg-config \
        wget ed curl bzip2 libbz2-dev unzip libarchive-tools \
        openssh-client libssl-dev \
        libgflags-dev libgoogle-glog-dev libgtest-dev && \
    apt-get -qq remove -y libprotobuf-dev protobuf-compiler && \
    rm -rf /var/lib/apt/lists/* /root/.cache


# Google Test
RUN cd /usr/src/gtest && cmake . && make && mv ./lib/libgtest* /usr/local/lib/

# Protobuf
RUN wget --no-check-certificate -q https://github.com/protocolbuffers/protobuf/archive/v3.9.0.tar.gz && \
    tar xf v3.9.0.tar.gz && rm v3.9.0.tar.gz && \
    cd protobuf-3.9.0 && ./autogen.sh &&  \
    ./configure --prefix=/usr/local && \
    make -j6 && \
    make install && ldconfig && \
    rm -rf /protobuf-3.9.0

# Install and compile client library

COPY aperturedb-client /aperturedb-client

RUN cd aperturedb-client && \
    scons -j16 && scons install

RUN ldconfig

# Setup entry point
RUN echo '#!/bin/bash'      > /start.sh && \
    echo 'cd /aperturedb-client/test' >> /start.sh && \
    echo './comm_test'     >> /start.sh && \
    chmod 755 /start.sh

CMD ["/start.sh"]
