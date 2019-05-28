## -*- docker-image-name: "emscripten-quantlib" -*-
FROM trzeci/emscripten:latest

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y install automake autoconf libtool && \
    apt-get autoclean && \
    apt-get clean

ENV EMSCRIPTEN /emsdk_portable/sdk
ENV BOOST /boost
ENV QUANTLIB /quantlib

# Download and unzip Boost
# Remove unwanted files. Keep Emscripten as is.
# Keep Boost and QuantLib header files and lib files.

# Boost

WORKDIR /tmp
RUN wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.bz2 && \
    mkdir ${BOOST} && \
    tar --bzip2 -xf boost_1_70_0.tar.bz2 -C ${BOOST} --strip-components=1 && \
    rm -f boost_1_70_0.tar.bz2 && \
    rm -rf ${BOOST}/doc

# Build Boost for Emscripten

# [Getting Started on Unix Variants](https://www.boost.org/doc/libs/1_70_0/more/getting_started/unix-variants.html)
# [Testing Emscripten with C++11 and Boost](https://gist.github.com/arielm/69a7488172611e74bfd4)

WORKDIR ${EMSCRIPTEN}
RUN ./embuilder.py build zlib
ENV NO_BZIP2 1

WORKDIR ${BOOST}
RUN ./bootstrap.sh && rm -rf stage && \
    # ./b2 -a -j8 toolset=clang-emscripten link=static threading=single variant=release --with-system --with-filesystem --with-iostreams stage && \
    ./b2 -a -j8 toolset=clang-emscripten link=static threading=single variant=release \
        --with-date_time --with-system --with-filesystem --with-iostreams --with-timer --with-math --with-random --with-thread stage && \
    rm -rf libs && \
    rm -rf lib/emscripten && \
    mkdir -p lib/emscripten && \
    cp stage/lib/*.a lib/emscripten && \
    find ${BOOST}/boost -type f  ! \( -name "*.h" -o -name "*.hpp" -o -name "*.ipp" \) -delete

# QuantLib

WORKDIR /tmp
RUN wget https://bintray.com/quantlib/releases/download_file?file_path=QuantLib-1.15.tar.gz -O QuantLib-1.15.tar.gz && \
    mkdir ${QUANTLIB} && \
    tar xzf QuantLib-1.15.tar.gz -C ${QUANTLIB} --strip-components=1 && \
    rm -f QuantLib-1.15.tar.gz 

# UNSETENV NO_BZIP2

# Build QuantLib with Boost and Emscripten

# How to use emconfigure and emmake, [see](https://emscripten.org/docs/compiling/Building-Projects.html)
# Also a good [guide](https://adamrehn.com/articles/creating-javascript-bindings-for-c-cxx-libraries-with-emscripten/)

WORKDIR ${QUANTLIB}
RUN emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten && \
    emmake make && \
# emmake make install && \
# ldconfig && \
    rm -rf ${QUANTLIB}/Examples && \
    mv ${QUANTLIB}/ql/.libs/libQuantLib.a /tmp && \
    find ${QUANTLIB}/ql -type f  ! \( -name "*.h" -o -name "*.hpp" \) -delete && \
    mv /tmp/libQuantLib.a ${QUANTLIB}/ql/.libs && \
    rm -rf /usr/local/lib/libQuant*.* 

# RUN apt-get clean

WORKDIR /src
CMD ["/bin/bash"]