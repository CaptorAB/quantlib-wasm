var QuantLibModule = require("./quantlib");
var QuantLib = null;

beforeAll(async () => {
    var loader = QuantLibModule();
    loader.ready = () =>
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
