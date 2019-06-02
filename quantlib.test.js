const fs = require("fs");
const path = require("path");

describe("captor/quantlib", () => {
    let QuantLib;

    beforeAll(async () => {
        const wasmPath = path.resolve(__dirname, "quantlib.wasm");
        const buffer = fs.readFileSync(wasmPath);
        const results = await WebAssembly.instantiate(buffer, {
            env: {
                memoryBase: 0,
                tableBase: 0,
                memory: new WebAssembly.Memory({ initial: 1024 }),
                table: new WebAssembly.Table({ initial: 16, element: "anyfunc" }),
                abort: console.log
            }
        });
        QuantLib = results.instance.exports;
    });

    test("Date", async () => {
        console.log(QuantLib.Sweden.name());
    });
});
