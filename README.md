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

```
docker run -v ${pwd}:/src -it -d trzeci/emscripten /bin/bash
docker exec -it ee5 /bin/bash
```

In bash install extra packages. Link to [libboost](https://packages.debian.org/jessie/libboost-all-dev)

```
apt-get update
apt-get dist-upgrade
# Should be build instead: apt-get install libboost-all-dev
apt-get install automake autoconf libtool
```

## Download QuantLib

In the container (bash). See [QuantLib on Linux](https://www.quantlib.org/install/linux.shtml):

```
wget https://bintray.com/quantlib/releases/download_file?file_path=QuantLib-1.15.tar.gz
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

```
cd QuantLib-1.15
BOOST=/src/boost_1_70_0
emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten
emmake make
make install
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
