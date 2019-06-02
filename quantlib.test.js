var QuantLib = null;

beforeAll(async () => {
    console.log("beforeAll");
});

test("Date", async () => {
    console.log('test("Date")');
    new Promise((resolve, reject) => {
        var loader = require("./quantlib")();
        loader.onRuntimeInitialized = () => {
            resolve(loader);
        };
    })
        .then((resp) => {
            QuantLib = resp;
        })
        .catch((error) => {
            console.log(error);
        });
    QuantLib = await p;
    console.log(QuantLib.Sweden.name());
});
