# Development

### Pull from docker

```
docker pull captorab/emscripten-quantlib:1.15.2
```

### Run the container

See note on 'Share folder' (Windows only) below.

Then:

```
docker run -v ${pwd}:/src -it captorab/emscripten-quantlib:1.15.2 /bin/bash
```

Or continue execute and existing continer. Search with `docker ps -a`.

```
docker exec -it 590 /bin/bash
```

Where `590` is the current container id.

### Compile and test an example

In the container:

```
cd examples
emcc -I${BOOST} -I${QUANTLIB} -o hello-quantlib.js hello-quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

### Run

Outside the container:

```
cd examples
node hello-quantlib.js
```

### Build quantlib-embind

```
emcc --bind -I${EMSCRIPTEN}/system/include -I${QUANTLIB} -I${BOOST} -O3 -s MODULARIZE=1 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['addOnPostRun']" -s EXPORT_NAME=QuantLib -o quantlib-embind.js quantlib-embind.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

### Windows only: Share folder

Add a local user to the windows machine called `docker`. In Docker / Settings / Shared Drives, share the disk drive. Use the user `docker`.

## Setup the Emscripten container (from docker trzeci/emscripten)

### Start from trzeci/emscripten

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

### Download and unzip Boost

[Getting Started on Unix Variants](https://www.boost.org/doc/libs/1_70_0/more/getting_started/unix-variants.html) [Testing Emscripten with C++11 and Boost](https://gist.github.com/arielm/69a7488172611e74bfd4)

```
cd /tmp
wget -c https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.bz2
mkdir /boost
tar --bzip2 -xf boost_1_70_0.tar.bz2 -C /boost --strip-components=1
```

### Build Boost with Emscripten

```bash
EMSCRIPTEN=/emsdk_portable/sdk
BOOST=/boost

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

### Build a Boost hello world

```bash
EMSCRIPTEN=/emsdk_portable/sdk
BOOST=/src/boost_1_70_0
cd TestingBoost
c++ -I${BOOST} boost-hello-world.cpp -o boost-hello-world
```

### Download QuantLib

In the container (bash). See [QuantLib on Linux](https://www.quantlib.org/install/linux.shtml):

```bash
wget https://bintray.com/quantlib/releases/download_file?file_path=QuantLib-1.15.tar.gz -O QuantLib-1.15.tar.gz
tar xzf QuantLib-1.15.tar.gz -C /quantlib
QUANTLIB=/quantlib
cd $QUANTLIB
```

### Build QuantLib (with default Boost and without Emscripten)

```
./autogen.sh
./configure
make
make install
ldconfig
```

### Build QuantLib with Boost and Emscripten

How to use emconfigure and emmake, [see](https://emscripten.org/docs/compiling/Building-Projects.html) Also a good [guide](https://adamrehn.com/articles/creating-javascript-bindings-for-c-cxx-libraries-with-emscripten/)

```
cd $QUANTLIB
BOOST=/boost
emconfigure ./configure --with-boost-include=${BOOST} --with-boost-lib=${BOOST}/lib/emscripten
emmake make
emmake make install
ldconfig
```

### Build and run captorab/quantlib/emscripten docker image

Build using `Dockerfile` in the same folder.

```
docker build -t docker.io/captorab/emscripten-quantlib:1.15.2 .
```

Run it (update the container id):

```
docker run -v ${pwd}:/src -it quantlib/emscripten /bin/bash
```

On MacOSX

```
docker run --mount type=bind,source="${PWD}",target=/src -it captorab/emscripten-quantlib:1.15.2 /bin/bash
```

### Compile emscripten with boost

[Using Boost with Emscripten](https://stackoverflow.com/questions/15724357/using-boost-with-emscripten)

## Delete unwanted files

```
du -d1 -h /quantlib/ql
du -d1 -h /boost
du -d1 -h /usr/local/lib

mkdir /quantlib/libs
mv /quantlib/ql/.libs/libQuantLib.a /quantlib/libs
find /quantlib/ql -type f ! \( -name "_.h" -o -name "_.hpp" \) -delete
mv /quantlib/libs /quantlib/ql/.libs/libQuantLib.a
rm -rf /quantlib/Examples

find /boost/boost -type f ! \( -name "_.h" -o -name "_.hpp" -o -name "\*.ipp" \) -delete
rm -rf /boost/doc
rm -rf /boost/libs

rm -rf /usr/local/lib/libQuant*.*
```

### Build Quantlibs BermudanSwaption example

```
QUANTLIB=/src/QuantLib-1.15
cd QuantLib-1.15/Examples/BermudanSwaption/
emcc BermudanSwaption.cpp -o BermudanSwaption.js -std=c++11 -I${BOOST} -L${BOOST}/lib/emscripten -I${QUANTLIB} -I${QUANTLIB}/ql
```

Builds .o-file (LLVM bitcode):

```
emcc -DHAVE_CONFIG_H -I. -I../../ql  -I../.. -I../.. -I/src/boost_1_70_0  -g -O2 -MT BermudanSwaption.o -MD -MP -MF .deps/BermudanSwaption.Tpo -c -o BermudanSwaption.o BermudanSwaption.cpp
```

[gcc arguments](http://tigcc.ticalc.org/doc/comopts.html) [emcc arguments](https://emscripten.org/docs/tools_reference/emcc.html)

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

### Link error when building QuantLib with Emscripten

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

### BINARYEN_TRAP_MODE=clamp

This runtime error is handled with `BINARYEN_TRAP_MODE=clamp`

```
exception thrown: RuntimeError: float unrepresentable in integer range,RuntimeError: float unrepresentable in integer range
    at wasm-function[2063]:2701
    at wasm-function[1140]:1436
    at wasm-function[108]:1861
    at Object.Module._main (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:6006:33)
    at Object.callMain (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:6346:30)
    at doRun (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:6404:60)
    at run (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:6418:5)
    at runCaller (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:6323:29)
    at removeRunDependency (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:1517:7)
    at receiveInstance (C:\Projects\Nodejs\test\test190425\BermudanSwaption.js:1611:5)
```

```
emcc -I${BOOST} -I${QUANTLIB} -s BINARYEN_TRAP_MODE=clamp -s TOTAL_MEMORY=67108864 -o BermudanSwaption.js BermudanSwaption.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

## Build test examples

### hello-boost

```
cd examples
emcc -I${BOOST} -o hello-boost.js hello-boost.cpp
```

Expected output:

```
> node hello-boost.js
HELLO 12345
```

### hello-quantlib

```
cd examples
emcc -I${BOOST} -I${QUANTLIB} -o hello-quantlib.js hello-quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

Expected output:

```
> node hello-quantlib.js
HELLO May 15th, 2019
```

### hello-emscripten

```
cd examples
emcc -I${EMSCRIPTEN}/system/include --bind -o hello-emscripten.html hello-emscripten.cpp
```

Browse hello-emscripten.html. In the browser's console window

```
> Module.lerp(2,3,0.25)
2.25
```

Test in from Node.js. `Create main.js` with the following content:

```
var Module = require("./examples/hello-emscripten");

Module.onRuntimeInitialized = () => {
    console.log(Module.lerp(2, 3, 0.25));
};
```

Expected output:

```
> node main.js
2.25
```

### hello-array

Build with:

```
cd examples
emcc -I${EMSCRIPTEN}/system/include --bind -o hello-array.js hello-array.cpp
```

Test in from Node.js. `Create main.js` with the following content:

```
var Module = require("./examples/hello-array");

Module.onRuntimeInitialized = () => {
    var v = Module.createDoubleVector(5);
    for (let i = 0; i < 5; i++) {
        v.set(i, i + 1);
    }
    console.log(Module.sum(v));
};
```

Expected output:

```
> node main.js
15
```

### BermudanSwaption

Use either `-s TOTAL_MEMORY=67108864` or `-s ALLOW_MEMORY_GROWTH=1`

```
# emcc -I${BOOST} -I${QUANTLIB} -s BINARYEN_TRAP_MODE=clamp -s TOTAL_MEMORY=67108864 -o BermudanSwaption.js BermudanSwaption.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
cd examples
emcc -I${BOOST} -I${QUANTLIB} -s BINARYEN_TRAP_MODE=clamp -O3 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 -std=c++14 -o BermudanSwaption.js BermudanSwaption.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

Expected output:

```
> node BermudanSwaption.js

G2 (analytic formulae) calibration
1x5: model 10.04552 %, market 11.48000 % (-1.43448 %)
2x4: model 10.51233 %, market 11.08000 % (-0.56767 %)
3x3: model 10.70500 %, market 10.70000 % (+0.00500 %)
4x2: model 10.83816 %, market 10.21000 % (+0.62816 %)
5x1: model 10.94390 %, market 10.00000 % (+0.94390 %)
calibrated to:
a     = 0.050105, sigma = 0.0094504
b     = 0.050109, eta   = 0.0094505
rho   = -0.76333

Hull-White (analytic formulae) calibration
1x5: model 10.62037 %, market 11.48000 % (-0.85963 %)
2x4: model 10.62959 %, market 11.08000 % (-0.45041 %)
3x3: model 10.63414 %, market 10.70000 % (-0.06586 %)
4x2: model 10.64428 %, market 10.21000 % (+0.43428 %)
5x1: model 10.66132 %, market 10.00000 % (+0.66132 %)
calibrated to:
a = 0.046414, sigma = 0.0058693

Hull-White (numerical) calibration
1x5: model 10.31185 %, market 11.48000 % (-1.16815 %)
2x4: model 10.54619 %, market 11.08000 % (-0.53381 %)
3x3: model 10.66914 %, market 10.70000 % (-0.03086 %)
4x2: model 10.74020 %, market 10.21000 % (+0.53020 %)
5x1: model 10.79725 %, market 10.00000 % (+0.79725 %)
calibrated to:
a = 0.055229, sigma = 0.0061063

Payer bermudan swaption struck at 5.00000 % (ATM)
G2 (tree):      14.111
G2 (fdm) :      14.113
HW (tree):      12.904
HW (fdm) :      12.91
HW (num, tree): 13.158
HW (num, fdm) : 13.157
BK:             13.002
Payer bermudan swaption struck at 6.00000 % (OTM)
G2 (tree):       3.1943
G2 (fdm) :       3.1809
HW (tree):       2.4921
HW (fdm) :       2.4596
HW (num, tree):  2.615
HW (num, fdm):   2.5829
BK:              3.2751
Payer bermudan swaption struck at 4.00000 % (ITM)
G2 (tree):       42.61
G2 (fdm) :       42.706
HW (tree):       42.253
HW (fdm) :       42.215
HW (num, tree):  42.364
HW (num, fdm) :  42.311
BK:              41.825

Run completed in 1 m 51 s
```

### Billiontrader Bootstrapping

See article [here](http://billiontrader.com/2015/02/16/bootstrapping-with-quantlib/)

```
cd examples
emcc -I${BOOST} -I${QUANTLIB} -s BINARYEN_TRAP_MODE=clamp -o billiontrader-bootstrapping.js billiontrader-bootstrapping.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

Expected output:

```
> node billiontrader-bootstrapping.js
0.1375: 0.137499 % Actual/360 simple compounding
0.1717: 0.171700 % Actual/360 simple compounding
0.2112: 0.211200 % Actual/360 simple compounding
0.2581: 0.258100 % Actual/360 simple compounding
0.25093: 0.251098 % Actual/360 simple compounding
0.32228: 0.322259 % Actual/360 simple compounding
0.41111: 0.411112 % Actual/360 simple compounding
0.51112: 0.511346 % Actual/360 simple compounding
0.61698: 0.617716 % Actual/360 simple compounding
0.73036: 0.732486 % Actual/360 Annual compounding
0.89446: 0.890789 % Actual/360 Annual compounding
1.23937: 1.237068 % Actual/360 Annual compounding
1.49085: 1.489769 % Actual/360 Annual compounding
1.67450: 1.674417 % Actual/360 Annual compounding
 discount Rate : 0.919223
 Forward Rate : 2.419765 % Actual/360 simple compounding
```

### swap-example

```
cd examples
emcc -I${BOOST} -I${QUANTLIB} -s BINARYEN_TRAP_MODE=clamp -o swap-example.js swap-example.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
```

Expected output:

```
> node swap-example.js
-11836.3
```

## How to enable C++ in VS Code

In VS Code install the C/C++ `ms-vscode.cpptools` extension.

Add the file `c_cpp_properties.json` to the folder `.vscode` with the following content:

```
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": ["C:\\Repos\\QuantLib", "C:\\Repos\\boost", "C:\\Repos\\emscripten\\system\\include"],
            "defines": ["_DEBUG", "UNICODE", "_UNICODE"],
            "compilerPath": "C:\\\\Program Files (x86)\\\\Microsoft Visual Studio\\\\2019\\\\Community\\\\VC\\\\Tools\\\\MSVC\\\\14.20.27508\\\\bin\\\\Hostx64\\\\x64\\\\cl.exe",
            "windowsSdkVersion": "10.0.17763.0",
            "intelliSenseMode": "msvc-x64",
            "cStandard": "c11",
            "cppStandard": "c++17"
        }
    ],
    "version": 4
}
```

`compilerPath` needs to be adjusted to local installation of the c++-compiler. (Visual Studio)[https://visualstudio.microsoft.com/] And `includePath` needs to be adjusted to match local installations of the three projects QuantLib, Boost and Emscripten.

-   Install QuantLib with `git clone https://github.com/lballabio/QuantLib`
-   Install Boost from `https://www.boost.org/users/download/`
-   Install Emscripten with `git clone https://github.com/emscripten-core/emscripten`

## Useful links

-   (Emscripten)[https://emscripten.org/]
-   (Emscripten Wiki)[https://github.com/emscripten-core/emscripten/wiki]
-   (List of other emscripten projects)[https://github.com/emscripten-core/emscripten/wiki/Porting-Examples-and-Demos]
-   (WebAssembly Explorer)[https://mbebenita.github.io/WasmExplorer/]

## Coding details

-   (Memory allocation, mallinfo)[https://linux.die.net/man/3/mallinfo]
-   (Using shared pointers)[https://docs.microsoft.com/en-us/cpp/cpp/how-to-create-and-use-shared-ptr-instances?view=vs-2019]
-   (Difference in make-shared and normal shared-ptr)[https://stackoverflow.com/questions/20895648/difference-in-make-shared-and-normal-shared-ptr-in-c]
