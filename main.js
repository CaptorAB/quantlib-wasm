var QuantLib = require("./quantlib");

const dateToSerialNumber = d => d.getTime() / 86400000 + 25569;
const serialNumberToDate = n => new Date((n - 25569) * 86400000);
const addDays = (d, n) => new Date(d.getTime() + 86400000 * n);

const toWasmIntVector = arr => {
	var v = QuantLib.createIntVector(arr.length);
	for (let i = 0; i < arr.length; i++) {
		v.set(i, arr[i]);
	}
	return v;
};

const toWasmDoubleVector = arr => {
	var v = QuantLib.createDoubleVector(arr.length);
	for (let i = 0; i < arr.length; i++) {
		v.set(i, arr[i]);
	}
	return v;
};

const toCurveItem = (date, value) => ({ date, value });

function performanceTest() {
	const { sqrt } = Math;
	const sqr = x => x * x;
	const stdev = xs => {
		var n = xs.length;
		var sx = 0;
		var sy = 0;
		var sx2 = 0;

		for (let i = 0; i < n; i++) {
			let x = xs[i];
			sx += x;
			sx2 += sqr(x);
		}

		return sqrt((sx2 - sqr(sx) / n) / (n - 1));
	};
	const hrtimeDiffToMs = (t0, t1) => (t1[0] - t0[0] + (t1[1] - t0[1]) / 1000000) / 1000;

	var arr = [...Array(100000)].map((d, i) => i + 1);
	var t0 = process.hrtime();
	var s0 = stdev(arr);
	var t1 = process.hrtime();
	var t2 = process.hrtime();
	var s1 = QuantLib.stdev(toWasmDoubleVector(arr));
	var t3 = process.hrtime();
	console.log(s0, s1, hrtimeDiffToMs(t0, t1), hrtimeDiffToMs(t2, t3));
}

QuantLib.onRuntimeInitialized = () => {
	// var arr = [...Array(100000)].map((d, i) => i + 1);
	// for (let index = 0; index < 1000; index++) {
	// 	let a = toWasmDoubleVector(arr);
	// 	let v = QuantLib.stdev(a);
	// 	a.delete();
	// }
	var s = QuantLib.createScheduleFromDates(toWasmIntVector([35000, 36000]));

	var valuationDate = new Date("2012-12-31");

	var nominal = 1000000.0;
	var previousResetDate = new Date("2012-11-20");
	var maturity = new Date("2022-11-20");
	var spread = 0.02;
	var fixedRate = 0.04;
	var previousResetValue = 0.01;

	var fixedScheduleCount = 1;
	var fixedScheduleTimeUnit = QuantLib.TimeUnit.Years;
	var floatScheduleCount = 3;
	var floatScheduleTimeUnit = QuantLib.TimeUnit.Months;

	var forwardCurveDayCountConvention = QuantLib.DayCountConvention.Actual360;
	var oisCurveDayCountConvention = QuantLib.DayCountConvention.Actual360;

	var d0 = new Date("2013-12-31");
	var d1 = new Date("2024-12-31");
	var forwardCurve = [toCurveItem(valuationDate, 1), toCurveItem(d0, 0.99), toCurveItem(d1, 0.8)];
	var oisCurve = [toCurveItem(valuationDate, 1), toCurveItem(d0, 0.999), toCurveItem(d1, 0.89)];

	console.log(QuantLib.DayCountConvention.Actual360);

	var wasmValuationDate = dateToSerialNumber(valuationDate);
	var wasmPreviousResetDate = dateToSerialNumber(previousResetDate);
	var wasmMaturity = dateToSerialNumber(maturity);
	var wasmForwardCurveDates = toWasmIntVector(forwardCurve.map(d => dateToSerialNumber(d.date)));
	var wasmForwardCurveDfs = toWasmDoubleVector(forwardCurve.map(d => d.value));
	var wasmOisCurveDates = toWasmIntVector(oisCurve.map(d => dateToSerialNumber(d.date)));
	var wasmOisCurveDfs = toWasmDoubleVector(oisCurve.map(d => d.value));

	console.log(
		QuantLib.swapNpv(
			nominal,
			fixedRate,
			spread,
			wasmValuationDate,
			wasmMaturity,
			wasmPreviousResetDate,
			previousResetValue,
			fixedScheduleCount,
			fixedScheduleTimeUnit,
			floatScheduleCount,
			floatScheduleTimeUnit,
			forwardCurveDayCountConvention,
			oisCurveDayCountConvention,
			wasmForwardCurveDates,
			wasmForwardCurveDfs,
			wasmOisCurveDates,
			wasmOisCurveDfs
		)
	);
};
