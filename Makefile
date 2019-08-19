
# this needs to be the first target if target build_bindings_from_unix is going to work
build_bindings:
	emcc --bind -I${EMSCRIPTEN}/system/include -I${QUANTLIB} -I${BOOST} -O3 -s MODULARIZE=1 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['addOnPostRun']" -s EXPORT_NAME=QuantLib -s TOTAL_MEMORY=64MB -o dist/quantlib.js quantlib-embind.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a

build_bindings_from_unix:
	docker pull captorab/emscripten-quantlib:1.16.1
	docker run --mount type=bind,source="${PWD}",target=/src -it captorab/emscripten-quantlib:1.16.1 make
