[![CircleCI](https://circleci.com/gh/CaptorAB/quantlib-wasm/tree/master.svg?style=svg)](https://circleci.com/gh/CaptorAB/quantlib-wasm/tree/master)
[![npm version](https://badge.fury.io/js/quantlib-wasm.svg)](https://badge.fury.io/js/quantlib-wasm)
[![install size](https://packagephobia.now.sh/badge?p=quantlib-wasm)](https://packagephobia.now.sh/result?p=quantlib-wasm)

# quantlib-wasm

A wrapper of the quantitative finance library [Quantlib](https://www.quantlib.org/). Compiled as a [WebAssembly](https://webassembly.org/) for use in browsers and in Node.JS.

WARNING: This is work in progress and in alpha mode.

## Install

```bash
npm install quantlib-wasm
```

## Usage

```js
var QuantLib = require("quantlib-wasm");

var loader = QuantLib();
loader.onRuntimeInitialized = () => {
    QuantLib = loader;
    console.log(`QuantLib v${QuantLib.version} loaded`);
};
```

## Introduction

[Quantlib](https://www.quantlib.org/) is a quantitative finance library, used for pricing, hedging and valuation of financial sequrities and derivatives. It's open source and widely used. The library is written in C++ and it has been exported to many languages such as Python, Java and C#.

How about exporting QuantLib to JavaScript? In Node.js there are many ways of importing external libraries, including node-gyp, addons and N-API. None of these techniques work client side in a browser. WebAssembly on the other hand, works both client and server-side.

[WebAssembly](https://webassembly.org/) (Wasm) is a sandboxed environment running inside the JavaScript virtual machine. High-level languages like C/C++ can be compiled into the Wasm. WebAssembly is supported by four major browsers (Chrome, Firefox, Safari and Edge). Therefore, QuantLib as WebAssembly can be used from JavaScript both client (web browser) and server-side (Node.js).

## Working with QuantLib in JavaScript

QuantLib is an object oriented library, rather than functional oriented. The QuantLib calculations are done with many objects, such as Date, Calendar, Schedule, PricingEngine, YieldCurve and all kind of instrument objects. These objects can be exported and used in JavaScript. The code in JavaScript will be similar to versions in Python or C++.

Here is a schedule generator example:

```js
const { Date, TimeUnit, Schedule, Period, BusinessDayConvention, DateGenerationRule } = QuantLib;
var effectiveDate = Date.fromISOString("2019-08-19");
var terminationDate = Date.fromISOString("2020-08-19");
var period = new Period(3, TimeUnit.Months);
var firstDate = new Date();
var nextToLastDate = new Date();
var schedule = new Schedule(
    effectiveDate,
    terminationDate,
    period,
    QuantLib.TARGET,
    BusinessDayConvention.ModifiedFollowing,
    BusinessDayConvention.ModifiedFollowing,
    DateGenerationRule.Backward,
    true,
    firstDate,
    nextToLastDate
);
var dates = schedule.dates();
for (let i = 0; i < dates.size(); i++) {
    let d = dates.get(i);
    console.log(d.toISOString());
    d.delete();
}
[effectiveDate, terminationDate, period, firstDate, nextToLastDate, dates, schedule].forEach((d) => d.delete());
```

## Emscripten

This implementation uses Emscripten to compile QuantLib. Emscripten compiles C++ into low level JavaScript called asm.js, which is highly optimizable and can be executed at close to native speed. A [long list of projects](https://github.com/emscripten-core/emscripten/wiki/Porting-Examples-and-Demos) are already using Emscripten to port codebases to JavaScript.

Embind is used to bind C++ functions and classes to JavaScript. The bindings are done with a few lines of code. The technique for defining bindings is similar to Boost Python.

The easiest way to run the Emscripten environment is in a prebuild Docker container. `trzeci/emscripten` is a good container and when running it compilations are done with the emcc compiler via the command prompt. The three projects Emscripten, QuantLib and Boost (which is a dependency of QuantLib) and wrapped together in a container called `captorab/emscripten-quantlib`. Running in a docker container saves a lot of time. The operating system issues and the configuration are done once and can easily be shared among developers.

## Memory management

When using Wasm and Embind, there is one catch though. Memory management must be handled in both the JavaScript and the Wasm environment. In JavaScript, objects are destructed automatically, but before leaving a QuantLib object in JavaScript a delete command needs to be sent to the Wasm, to destruct the C++ object. This must be done explicitly since the JavaScript objects do not have any finalizer. This is something high level programmers assume the environment will do automatically. Unfortunately this is not done automatically between the two memory areas, one in JavaScript and one in the Wasm. In the example above delete is called on the last line in the for loop and on the very last line of code.

Code like the example below cannot be used because it hides the destructor of the Date-object.

```js
console.log(Date.fromISOString("2019-08-19").toString()); // This causes a memory leak.
```

Here is the correct equivalent:

```js
var date = Date.fromISOString("2019-08-19");
console.log(date.toString());
date.delete();
```

Memory usage can be measured in the Wasm at any time, and memory leaks can be detected.

```js
const { Date, mallinfo } = QuantLib;
var m0 = mallinfo();
var date = Date.fromISOString("2019-08-19");
console.log(date.toString());
date.delete();
var m1 = mallinfo();
console.log(m1.uordblks - m0.uordblks + (m1.hblkhd - m0.hblkhd)); // Should print 0
```

## Using the wasm in a web page

When using the wasm client side, two files `quantlib.js` and `quantlib.wasm` need to be added to the web server. In the web page, e.g. index.html add the following lines of code.

```html
<script type="text/javascript" src="quantlib.js"></script>
<script>
    const loader = window.QuantLib();
    var QuantLib = null;
    loader.onRuntimeInitialized = () => {
        QuantLib = loader;
        console.log(`QuantLib v${QuantLib.version} loaded`);
    };
</script>
```

## Using the wasm in a React app

React in itself can easily use the Quantlib wasm. See the method above. When the app is build with `create-react-app`, webpack is used to load and build the source files. By default (version 3.1.1 or earlier of `react-scripts`), doesn't load wasm files. To bypass this problem Facebook's `react-scripts` can be forked and modified. How this is done is explained [here](https://auth0.com/blog/how-to-configure-create-react-app/).

One fork that loads wasm files is [@captor/react-scripts](https://www.npmjs.com/package/@captor/react-scripts). To create a new app that with the modifies script run:

```sh
npx create-react-app <app-name> --scripts-version @captor/react-scripts
```

Or, in an already existing app, change the installed script reference.

```sh
npm uninstall react-scripts
npm install @captor/react-scripts
```

Add `quantlib-wasm` to the app:

```sh
npm install quantlib-wasm
```

Using Quantlib in a source file, e.g. index.js or and other js file.

```js
import loader from "quantlib-wasm";
import wasm from "quantlib-wasm/dist/quantlib.wasm";

const QuantLib = loader({ locateFile: (path) => (path.endsWith(".wasm") ? wasm : path) });

QuantLib.onRuntimeInitialized = () => {
    console.log(QuantLib.version);
};
```

The WebAssembly instantiation is asynchronous. To handle this in React, the instantiation can start in the constructor of the component and the `onRuntimeInitialized` should call `this.setState`. See React [State and Lifecycle](https://reactjs.org/docs/state-and-lifecycle.html).

Example: Loading the wasm in an App component.

```js
import loader from "quantlib-wasm";
import wasm from "quantlib-wasm/dist/quantlib.wasm";

class App extends React.Component {
    constructor(props) {
        super(props);
        const QuantLib = loader({ locateFile: (path) => (path.endsWith(".wasm") ? wasm : path) });
        QuantLib.onRuntimeInitialized = () => {
            this.setState({ status: "Done!", QuantLib });
        };
        this.state = { status: "Loading...", QuantLib };
    }
    render() {
        const { status, QuantLib } = this.state;
        return <h2>{status + (QuantLib.version ? ` Version ${QuantLib.version} loaded` : ``)}</h2>;
    }
}

export default App;
```

## Status

Which objects and functions are exported? There is no documentation written yet. Until this project turns into alpha mode, the only reliable way is to check the code. In this case the binding file is the right place. It's found [here](https://github.com/CaptorAB/quantlib-wasm/blob/master/quantlib-embind.cpp).

## Versioning

quantlib-wasm does not follow https://semver.org/, but the version from [Quantlib](https://www.quantlib.org/) with an extra number to version the [quantlib-wasm](https://www.npmjs.com/package/quantlib-wasm) package.

## Development

In order to build a new version, when a new Quantlib version is available

```bash
git checkout -b $QUANTLIB_VERSION
```

In:

-   .circleci/config.yml
-   Dockerfile
-   Makefile
-   package.json

update from old version number to "${QUANTLIB_VERSION}"."${version}"

```bash
make build_docker_image
make build_bindings_from_unix
npm i
npm test
npm pack
npm publish
```
