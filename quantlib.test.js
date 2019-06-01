var QuantLib = null;

beforeAll(async () => {});

test("Date", async () => {
	new Promise((resolve, reject) => {
		var loader = require("./quantlib")();
		loader.onRuntimeInitialized = () => {
			resolve(loader);
		};
	})
		.then(resp => {
			QuantLib = resp;
		})
		.catch(error => {
			console.log(error);
		});
	QuantLib = await p;
	console.log(QuantLib.Sweden.name());
});
