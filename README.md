[![CircleCI](https://circleci.com/gh/CaptorAB/quantlib-wasm/tree/master.svg?style=svg)](https://circleci.com/gh/CaptorAB/quantlib-wasm/tree/master)

# quantlib-wasm

A wrapper of the quantitative finance library [Quantlib](https://www.quantlib.org/). Compiled as a [WebAssembly](https://webassembly.org/) for use in browsers and in Node.JS.

WARNING: This is work in progress and in alpha mode.

## Install

```bash
npm install quantlib-wasm
```

## Usage

```JavaScript
var QuantLib = require("quantlib-wasm");

var loader = QuantLib();
loader.onRuntimeInitialized = () => {
    QuantLib = loader;
    console.log(`QuantLib v${QuantLib.version} loaded`);
};
```

## Versioning

quantlib-wasm does not follow https://semver.org/, but the version from [Quantlib](https://www.quantlib.org/) with an extra number to version the [quantlib-wasm](https://www.npmjs.com/package/quantlib-wasm) package.
