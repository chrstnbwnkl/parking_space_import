ARG VALHALLA_BUILDER_IMAGE=ghcr.io/valhalla/valhalla:latest
# some build args
ARG ENABLE_TESTS=Off
ARG BUILD_TOOLS=On

FROM $VALHALLA_BUILDER_IMAGE AS valhalla_base
LABEL org.opencontainers.image.authors="Christian Beiwinkel <chrstn@bwnkl.de>"

FROM ubuntu:24.04 AS builder
LABEL org.opencontainers.image.authors="Christian Beiwinkel <chrstn@bwnkl.de>"

COPY --from=valhalla_base /usr/local /usr/local
COPY --from=valhalla_base /usr/local/lib/python3.12/dist-packages/valhalla /usr/local/lib/python3.12/dist-packages/

WORKDIR /usr/local/src/parking_spaces

# install deps
RUN apt-get update > /dev/null && \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get install -y git pkgconf cmake build-essential libosmium2-dev libprotozero-dev zlib1g-dev libsqlite3-mod-spatialite libspatialite-dev  libgdal-dev libgeos-dev curl libcurl4-openssl-dev libluajit-5.1-dev libzmq3-dev libprotobuf-dev libczmq-dev protobuf-compiler libgeotiff-dev libtiff-dev  
RUN rm -rf /var/lib/apt/lists/*

# prepare build
COPY CMakeLists.txt CMakeLists.txt
COPY cmake/ cmake/
COPY include/ include/
COPY third_party/ third_party/
COPY src/ src/
COPY tools/ tools/
COPY test/ test/

RUN ls -la
RUN rm -rf build && mkdir build

WORKDIR /usr/local/src/parking_spaces
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc  -DPS_BUILD_TESTS=${ENABLE_TESTS:-"Off"} -DPS_BUILD_TOOLS=${BUILD_TOOLS:-"On"}
RUN cd build && make all -j${CONCURRENCY:-$(nproc)} && make install

# remove the source
WORKDIR /usr/local/src
RUN rm -rf parking_spaces

FROM ghcr.io/valhalla/valhalla:latest AS runner
LABEL org.opencontainers.image.authors="Christian Beiwinkel <chrstn@bwnkl.de>"

# install some additional runtime deps
RUN apt-get update > /dev/null && \
  export DEBIAN_FRONTEND=noninteractive && \
  apt-get install -y zlib1g-dev libcurl4-openssl-dev   libgdal-dev 

# also just copy what's needed to not overwrite valhalla's installs
COPY --from=builder /usr/local/ /usr/local

ENV LD_LIBRARY_PATH="/usr/local/lib:${LD_LIBRARY_PATH}"

