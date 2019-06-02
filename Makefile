build_bindings:
	emcc --bind -I${EMSCRIPTEN}/system/include -I${QUANTLIB} -I${BOOST} -O3 -s MODULARIZE=1 -s EXPORT_NAME=QuantLib -o quantlib.js quantlib-embind.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a
