## -*- docker-image-name: "emscripten-quantlib" -*-
FROM emscripten/emsdk:3.1.69

ENV EMSCRIPTEN /emsdk_portable/sdk

RUN apt-get update && \
        apt-get -y upgrade && \
        apt-get -y install software-properties-common  && \
        add-apt-repository ppa:edd/misc && \
        apt-get update  && \
        apt-get -y install ng-cjk automake autoconf

# https://packages.ubuntu.com/search?keywords=quantlib
# https://formulae.brew.sh/formula/quantlib
# https://formulae.brew.sh/formula/boost

# list distro and version
RUN cat /etc/*-release
# list installed version of boost
#RUN apt-cache policy libboost-dev
# list all versions available of libquantlib0-dev
#RUN apt list -a libquantlib0-dev
#RUN apt-get -y install libquantlib0-dev=${QUANTLIB_VERSION}-1.2204.1 libtool libboost-dev

ENV BOOST /boost
ENV BOOST_VERSION 1.86
ENV BOOST_UNDERSCORE_VERSION 1_86
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
# https://stackoverflow.com/questions/15724357/using-boost-with-emscripten/60550627#60550627

RUN echo ${PWD}
RUN mkdir -p ${BOOST}/lib/emscripten
RUN ./bootstrap.sh && rm -rf stage && \
	emconfigure ./b2 -a -j8 toolset=emscripten link=static threading=single variant=release \
	--with-date_time --with-system --with-filesystem --with-iostreams --with-timer \
	--with-math --with-random --with-thread stage \
	--prefix=${BOOST}/lib/emscripten --build-dir=./build install && rm -rf ./build

# We want boost to be installed natively x86_64 also ( for testing code without emscripten )
# we are not using apt-get install since we would get another version
WORKDIR ${BOOST}
RUN ./bootstrap.sh && rm -rf stage && \
	./b2 -a -j8 link=static threading=single variant=release \
	--with-date_time --with-system --with-filesystem --with-iostreams --with-timer \
	--with-math --with-random --with-thread stage \
	--build-dir=./build/natively install  && rm -rf ./build/natively 

# QuantLib
ENV QUANTLIB /quantlib
ENV QUANTLIB_NATIVE /quantlib_native 
ENV QUANTLIB_VERSION 1.36

WORKDIR /tmp
RUN wget https://github.com/lballabio/QuantLib/releases/download/v${QUANTLIB_VERSION}/QuantLib-${QUANTLIB_VERSION}.tar.gz -O QuantLib-${QUANTLIB_VERSION}.tar.gz && \
	mkdir ${QUANTLIB} && \
	tar xzf QuantLib-${QUANTLIB_VERSION}.tar.gz -C ${QUANTLIB} --strip-components=1 && \
	cp -rf ${QUANTLIB} ${QUANTLIB_NATIVE} && \
	rm -f QuantLib-${QUANTLIB_VERSION}.tar.gz 

# Build QuantLib with Boost and Emscripten

# How to use emconfigure and emmake, [see](https://emscripten.org/docs/compiling/Building-Projects.html)
# Also a good [guide](https://adamrehn.com/articles/creating-javascript-bindings-for-c-cxx-libraries-with-emscripten/)

WORKDIR ${QUANTLIB}
RUN echo $PWD
RUN autoreconf --force
# --enable-test-suite=no since it takes to much memory to link
RUN emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten --disable-shared --enable-test-suite=no --disable-dependency-tracking
RUN	emmake make -j1 && \
	rm -rf ${QUANTLIB}/Examples && \
	mv ${QUANTLIB}/ql/.libs/libQuantLib.a /tmp && \
	find ${QUANTLIB}/ql -type f  ! \( -name "*.h" -o -name "*.hpp" \) -delete && \
	mv /tmp/libQuantLib.a ${QUANTLIB}/ql/.libs && \
	rm -rf /usr/local/lib/libQuant*.* 


# We want also to install quantlib natively for x86_64  ( for testing code without emscripten )
# we are not using apt-get install since we would get another version
# This still generates a to big docker layer, but good enough for now.
# Be carefull with the -j switch. It could use up to much memory and docker will just kill it with "received SIGKILL (-9)"
# Most likely you need to increase the docker.settings memory usage in any case
# Compiling nativly should be last, since else /usr/local/lib/libQuantLib.so is lost 
WORKDIR ${QUANTLIB_NATIVE}
RUN autoreconf --force
RUN ./configure --enable-test-suite=no  --disable-dependency-tracking
RUN make -j1 && \
	make install && \
	ldconfig && \
	cd / && rm -rf ${QUANTLIB_NATIVE}


WORKDIR /src
CMD ["/bin/bash"]

