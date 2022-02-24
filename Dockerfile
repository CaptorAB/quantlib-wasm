## -*- docker-image-name: "emscripten-quantlib" -*-
FROM emscripten/emsdk:3.1.1

RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y install automake autoconf libtool && \
    apt-get autoclean && \
    apt-get clean

ENV EMSCRIPTEN /emsdk_portable/sdk
ENV BOOST /boost
ENV BOOST_VERSION 1.75
ENV BOOST_UNDERSCORE_VERSION 1_75
ENV QUANTLIB /quantlib
ENV QUANTLIB_VERSION 1.25

# Download and unzip Boost
# Remove unwanted files. Keep Emscripten as is.
# Keep Boost and QuantLib header files and lib files.

# Boost

WORKDIR /tmp
RUN wget -c https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}.0/source/boost_${BOOST_UNDERSCORE_VERSION}_0.tar.bz2 && \
    mkdir ${BOOST} && \
    tar --bzip2 -xf boost_${BOOST_UNDERSCORE_VERSION}_0.tar.bz2 -C ${BOOST} --strip-components=1 && \
    rm -f boost_${BOOST_UNDERSCORE_VERSION}_0.tar.bz2 && \
    rm -rf ${BOOST}/doc

# Build Boost for Emscripten

# [Getting Started on Unix Variants](https://www.boost.org/doc/libs/1_70_0/more/getting_started/unix-variants.html)
# [Testing Emscripten with C++11 and Boost](https://gist.github.com/arielm/69a7488172611e74bfd4)

WORKDIR ${EMSCRIPTEN}
RUN embuilder.py build zlib
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
RUN wget https://github.com/lballabio/QuantLib/releases/download/QuantLib-v${QUANTLIB_VERSION}/QuantLib-${QUANTLIB_VERSION}.tar.gz -O QuantLib-${QUANTLIB_VERSION}.tar.gz && \
	mkdir ${QUANTLIB} && \
	tar xzf QuantLib-${QUANTLIB_VERSION}.tar.gz -C ${QUANTLIB} --strip-components=1 && \
	rm -f QuantLib-${QUANTLIB_VERSION}.tar.gz 

# UNSETENV NO_BZIP2

# Build QuantLib with Boost and Emscripten

# How to use emconfigure and emmake, [see](https://emscripten.org/docs/compiling/Building-Projects.html)
# Also a good [guide](https://adamrehn.com/articles/creating-javascript-bindings-for-c-cxx-libraries-with-emscripten/)

WORKDIR ${QUANTLIB}
RUN echo $PWD
RUN emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten --disable-shared && \
	emmake make -j4 && \
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
