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
ENV PROTOBUF_VERSION=3.20.0
RUN wget --no-check-certificate -q https://github.com/protocolbuffers/protobuf/archive/v$PROTOBUF_VERSION.tar.gz && \
    tar xf v$PROTOBUF_VERSION.tar.gz && rm v$PROTOBUF_VERSION.tar.gz && \
    cd protobuf-$PROTOBUF_VERSION && ./autogen.sh &&  \
    ./configure --prefix=/usr/local && \
    make -j6 && \
    make install && ldconfig && \
    rm -rf /protobuf-$PROTOBUF_VERSION

# JSON for Modern C++
RUN wget --no-check-certificate -q https://github.com/nlohmann/json/archive/refs/tags/v3.10.2.zip && \
    unzip v3.10.2.zip -d /usr/local/src && \
    rm v3.10.2.zip && \
    cmake -S /usr/local/src/json-3.10.2 -B /usr/local/build/nlohmann-json -DCMAKE_BUILD_TYPE=Release -DJSON_BuildTests=OFF && \
    cmake --build /usr/local/build/nlohmann-json --target install && \
    rm -rf /usr/local/build/nlohmann-json /usr/local/src/json-3.10.2

#Install prometheus-cpp
RUN wget --no-check-certificate -q https://github.com/jupp0r/prometheus-cpp/releases/download/v1.0.0/prometheus-cpp-with-submodules.tar.gz && \
    tar xf prometheus-cpp-with-submodules.tar.gz && \
    rm prometheus-cpp-with-submodules.tar.gz && \
    cd prometheus-cpp-with-submodules && mkdir _build && cd _build && \
    cmake .. -DBUILD_SHARED_LIBS=ON -DENABLE_PUSH=OFF -DENABLE_COMPRESSION=OFF && \
    cmake --build . --parallel 16 && ctest -V && \
    cmake --install . && cmake --install ./pull && \
    rm -rf /prometheus-cpp-with-submodules

# Install and compile client library

COPY aperturedb-client /aperturedb-client

RUN cd aperturedb-client && \
    scons -j16 && scons install

RUN ldconfig

# Setup entry point
RUN echo '#!/bin/bash'      > /start.sh && \
    echo 'cd /aperturedb-client/test' >> /start.sh && \
    echo './comm_test'     >> /start.sh && \
    echo 'cd /aperturedb-client/tools/prometheus_ambassador' >> /start.sh && \
    echo './prometheus_ambassador_test'     >> /start.sh && \
    chmod 755 /start.sh

CMD ["/start.sh"]
