## Pull from docker

```
docker pull trzeci/emscripten
```

## Compile

```
docker run --rm -v ${pwd}:/src -u emscripten trzeci/emscripten emcc helloworld.cpp -o helloworld.js --closure 1
```

## Run

```
node helloworld.js
```

## Windows only: Share folder

Add a local user to the windows machine called `docker`.
In Docker / Settings / Shared Drives, share the disk drive. Use the user `docker`.

## Setup the Emscripten container (from docker trzeci/emscripten)

Start an emscripten container

Share folder ´/share´

```
docker run -v ${pwd}:/share -it -d trzeci/emscripten /bin/bash
docker exec -it ee5 /bin/bash
```

In bash install extra packages. Boost should be installed and build from sourcecode. Link to [libboost](https://packages.debian.org/jessie/libboost-all-dev)

```
apt-get update
apt-get -y dist-upgrade
# apt-get install libboost-all-dev
apt-get -y install automake autoconf libtool
```

## Download and unzip Boost

[Getting Started on Unix Variants](https://www.boost.org/doc/libs/1_70_0/more/getting_started/unix-variants.html)
[Testing Emscripten with C++11 and Boost](https://gist.github.com/arielm/69a7488172611e74bfd4)

```
wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.bz2
tar --bzip2 -xf boost_1_70_0.tar.bz2
```

## Build Boost with Emscripten

```
EMSCRIPTEN=/emsdk_portable/sdk
BOOST=/src/boost_1_70_0

cd $EMSCRIPTEN
./embuilder.py build zlib
export NO_BZIP2=1

cd $BOOST
./bootstrap.sh
rm -rf stage
./b2 -a -j8 toolset=clang-emscripten link=static threading=single variant=release --with-system --with-filesystem --with-iostreams stage

rm -rf lib/emscripten
mkdir -p lib/emscripten
cp stage/lib/*.a lib/emscripten

unset NO_BZIP2
```

## Build a Boost hello world

```
EMSCRIPTEN=/emsdk_portable/sdk
BOOST=/src/boost_1_70_0
cd TestingBoost
c++ -I${BOOST} boost-hello-world.cpp -o boost-hello-world
```

## Download QuantLib

In the container (bash). See [QuantLib on Linux](https://www.quantlib.org/install/linux.shtml):

```
wget https://bintray.com/quantlib/releases/download_file?file_path=QuantLib-1.15.tar.gz -O QuantLib-1.15.tar.gz
tar xzf QuantLib-1.15.tar.gz
cd QuantLib-1.15
```

## Build QuantLib (with default Boost and without Emscripten)

```
./autogen.sh
./configure
make
make install
ldconfig
```

## Build QuantLib with Boost and Emscripten

How to use emconfigure and emmake, [see](https://emscripten.org/docs/compiling/Building-Projects.html)
Also a good [guide](https://adamrehn.com/articles/creating-javascript-bindings-for-c-cxx-libraries-with-emscripten/)

```
cd QuantLib-1.15
BOOST=/src/boost_1_70_0
emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten
emmake make
emmake make install
ldconfig
```

## Build and run quantlib/emscripten docker image

Build using `Dockerfile` in the same folder.

```
docker build -t quantlib/emscripten .
```

Run it (update the container id):

```
docker run -v ${pwd}:/src -it -d quantlib/emscripten /bin/bash
docker exec -it ee5 /bin/bash
```

## Compile emscripten with boost

[Using Boost with Emscripten](https://stackoverflow.com/questions/15724357/using-boost-with-emscripten)

## Build Emscripten with Boost

```
cd TestingEmscriptenWithC11AndBoost
emcc hello-boost.cpp -o hello-boost.js --closure 1 -v -Wno-warn-absolute-paths -std=c++11 -I${BOOST} -L${BOOST}/lib/emscripten -lboost_system -lboost_filesystem -lboost_iostreams
```

Or simply:

```
emcc hello-boost.cpp -o hello-boost.js -std=c++11 -I${BOOST}
```

Where hello-boost.cpp contains:

```
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

int main() {
    string input = "12345";
	auto tmp = boost::lexical_cast<int>(input);
    std::cout << "HELLO " << tmp << std::endl;
    return 0;
}
```

## Build BermudanSwaption example

```
QUANTLIB=/src/QuantLib-1.15
cd QuantLib-1.15/Examples/BermudanSwaption/
emcc BermudanSwaption.cpp -o BermudanSwaption.js -std=c++11 -I${BOOST} -L${BOOST}/lib/emscripten -I${QUANTLIB} -I${QUANTLIB}/ql
```

Builds .o-file (LLVM bitcode):

```
emcc -DHAVE_CONFIG_H -I. -I../../ql  -I../.. -I../.. -I/src/boost_1_70_0  -g -O2 -MT BermudanSwaption.o -MD -MP -MF .deps/BermudanSwaption.Tpo -c -o BermudanSwaption.o BermudanSwaption.cpp
```

[gcc arguments](http://tigcc.ticalc.org/doc/comopts.html)
[emcc arguments](https://emscripten.org/docs/tools_reference/emcc.html)

```
emcc -I. -I../../ql -I../.. -I${BOOST} -L${BOOST}/lib/emscripten -O2 -MT BermudanSwaption.o -MD -MP -o BermudanSwaption.js BermudanSwaption.cpp
emcc -I. -I../../ql -I../.. -I${BOOST} -L${BOOST}/lib/emscripten -O2 -o hello-boost.js hello-boost.cpp
```

Output from `make install`

```
/bin/bash ../libtool --mode=install /usr/bin/install -c libQuantLib.la '/usr/local/lib'
libtool: install: /usr/bin/install -c .libs/libQuantLib.so.0.0.0 /usr/local/lib/libQuantLib.so.0.0.0
libtool: install: (cd /usr/local/lib && { ln -s -f libQuantLib.so.0.0.0 libQuantLib.so.0 || { rm -f libQuantLib.so.0 && ln -s libQuantLib.so.0.0.0 libQuantLib.so.0; }; })
libtool: install: (cd /usr/local/lib && { ln -s -f libQuantLib.so.0.0.0 libQuantLib.so || { rm -f libQuantLib.so && ln -s libQuantLib.so.0.0.0 libQuantLib.so; }; })
libtool: install: /usr/bin/install -c .libs/libQuantLib.lai /usr/local/lib/libQuantLib.la
libtool: install: /usr/bin/install -c .libs/libQuantLib.a /usr/local/lib/libQuantLib.a
libtool: install: chmod 644 /usr/local/lib/libQuantLib.a
libtool: install: /emsdk_portable/sdk/emranlib /usr/local/lib/libQuantLib.a
libtool: finish: PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/sbin" ldconfig -n /usr/local/lib
ldconfig: /usr/local/lib/libQuantLib.so.0.0.0 is not an ELF file - it has the wrong magic bytes at the start.
ldconfig: /usr/local/lib/libQuantLib.so.0 is not an ELF file - it has the wrong magic bytes at the start.
ldconfig: /usr/local/lib/libQuantLib.so is not an ELF file - it has the wrong magic bytes at the start.
```

```
BOOST=/src/boost_1_70_0
QUANTLIB=/src/QuantLib-1.15
emcc -I${QUANTLIB} -I${BOOST} -O2 -o hello-boost.js hello-boost.cpp
emcc -I${QUANTLIB} -I${BOOST} -o hello-quantlib.js hello-quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

## Link error when building QuantLib with Emscripten

```
root@ee5ee9687d4f:/src# emcc -I${QUANTLIB} -I${BOOST} -O2 -o hello-quantlib.js hello-quantlib.cpp
error: undefined symbol: _ZN8QuantLib4DateC1EiNS_5MonthEi
warning: To disable errors for undefined symbols use `-s ERROR_ON_UNDEFINED_SYMBOLS=0`
error: undefined symbol: _ZN8QuantLiblsERNSt3__213basic_ostreamIcNS0_11char_traitsIcEEEERKNS_4DateE
Error: Aborting compilation due to previous errors
shared:ERROR: '/emsdk_portable/node/bin/node /emsdk_portable/sdk/src/compiler.js /tmp/tmpT3Jd9s.txt /emsdk_portable/sdk/src/library_pthread_stub.js' failed (1)
```

Adding the right lib solves the problem:

```
emcc -I${QUANTLIB} -I${BOOST} -o hello-quantlib.js hello-quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```
