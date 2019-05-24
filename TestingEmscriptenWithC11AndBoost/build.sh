# BUILDING BOOST FOR EMSCRIPTEN...
# REFERENCE: https://www.boost.org/doc/libs/1_70_0/more/getting_started/unix-variants.html

EMSCRIPTEN=/emsdk_portable/sdk
BOOST=/src/boost_1_70_0

cd $EMSCRIPTEN; ./embuilder.py build zlib
export NO_BZIP2=1

cd $BOOST
./bootstrap.sh
rm -rf stage
./b2 -a -j8 toolset=clang-emscripten link=static threading=single variant=release --with-system --with-filesystem --with-iostreams stage

rm -rf lib/emscripten
mkdir -p lib/emscripten
cp stage/lib/*.a lib/emscripten

unset NO_BZIP2
