var QuantLibModule = require("./quantlib");
var QuantLib = null;

describe("captor/quantlib", () => {
    beforeAll(async () => {
        var loader = QuantLibModule();
        loader.ready = () =>
            // https://github.com/emscripten-core/emscripten/issues/5820
            new Promise((resolve, reject) => {
                delete loader.then;
                loader.onAbort = reject;
                loader.addOnPostRun(() => {
                    resolve(loader);
                });
            });
        QuantLib = await loader.ready();
    });

    test("Sweden Calendar", async () => {
        expect(QuantLib.Sweden.name()).toBe("Sweden");
    });
});
