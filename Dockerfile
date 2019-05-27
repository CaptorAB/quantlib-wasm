## -*- docker-image-name: "emscripten-quantlib" -*-
FROM trzeci/emscripten:latest
WORKDIR /src
RUN apt-get update 
RUN apt-get -y dist-upgrade
RUN apt-get -y install automake autoconf libtool

# Download and unzip Boost
# [Getting Started on Unix Variants](https://www.boost.org/doc/libs/1_70_0/more/getting_started/unix-variants.html)
# [Testing Emscripten with C++11 and Boost](https://gist.github.com/arielm/69a7488172611e74bfd4)

ENV EMSCRIPTEN /emsdk_portable/sdk
ENV BOOST /boost
ENV QUANTLIB /quantlib

RUN wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.bz2
RUN mkdir ${BOOST}
RUN tar --bzip2 -xf boost_1_70_0.tar.bz2 -C ${BOOST} --strip-components=1

# Build Boost with Emscripten

WORKDIR ${EMSCRIPTEN}
RUN ./embuilder.py build zlib
ENV NO_BZIP2 1

WORKDIR ${BOOST}
RUN ./bootstrap.sh
RUN rm -rf stage
RUN ./b2 -a -j8 toolset=clang-emscripten link=static threading=single variant=release --with-system --with-filesystem --with-iostreams stage

RUN rm -rf lib/emscripten
RUN mkdir -p lib/emscripten
RUN cp stage/lib/*.a lib/emscripten

# UNSETENV NO_BZIP2

# Download and unzip QuantLib

RUN wget https://bintray.com/quantlib/releases/download_file?file_path=QuantLib-1.15.tar.gz -O QuantLib-1.15.tar.gz
RUN mkdir ${QUANTLIB}
RUN tar xzf QuantLib-1.15.tar.gz -C ${QUANTLIB} --strip-components=1

# Build QuantLib with Boost and Emscripten

# How to use emconfigure and emmake, [see](https://emscripten.org/docs/compiling/Building-Projects.html)
# Also a good [guide](https://adamrehn.com/articles/creating-javascript-bindings-for-c-cxx-libraries-with-emscripten/)

WORKDIR ${QUANTLIB}
RUN emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten
RUN emmake make
RUN emmake make install
RUN ldconfig

WORKDIR /src
CMD ["/bin/bash"]