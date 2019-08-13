var QuantLib = require("./quantlib-embind");

const dateToSerialNumber = (d) => d.getTime() / 86400000 + 25569;
const serialNumberToDate = (n) => new Date((n - 25569) * 86400000);
const addDays = (d, n) => new Date(d.getTime() + 86400000 * n);

const toWasmIntVector = (arr) => {
    var v = QuantLib.createIntVector(arr.length);
    for (let i = 0; i < arr.length; i++) {
        v.set(i, arr[i]);
    }
    return v;
};

const toWasmDoubleVector = (arr) => {
    var v = QuantLib.createDoubleVector(arr.length);
    for (let i = 0; i < arr.length; i++) {
        v.set(i, arr[i]);
    }
    return v;
};

const toCurveItem = (date, value) => ({ date, value });

function performanceTest() {
    const { sqrt } = Math;
    const sqr = (x) => x * x;
    const stdev = (xs) => {
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
    const hrtimeDiffToMs = (t0, t1) => 1000 * (t1[0] - t0[0]) + (t1[1] - t0[1]) / 1000000;

    var arr = [...Array(400000)].map((d, i) => i + 1);
    var t0 = process.hrtime();
    var s0 = stdev(arr);
    var t1 = process.hrtime();
    var s1 = QuantLib.stdev(toWasmDoubleVector(arr));
    var t2 = process.hrtime();
    var s2 = QuantLib.stdevDummy(toWasmDoubleVector(arr));
    var t3 = process.hrtime();
    console.log(s0, s1);
    console.log("JavaScript " + hrtimeDiffToMs(t0, t1));
    console.log("WebAssembly " + hrtimeDiffToMs(t1, t2));
    console.log("WebAssembly excl copy data " + (hrtimeDiffToMs(t1, t2) - hrtimeDiffToMs(t2, t3)));
}

function replicateSwapExample2() {
    const {
        Date,
        Period,
        TimeUnit,
        BusinessDayConvention,
        DateGenerationRule,
        Schedule,
        VanillaSwapType,
        VanillaSwap,
        setValuationDate,
        Thirty360,
        Actual360,
        Euribor
    } = QuantLib;
    var valuationDate = Date.fromISOString("2012-12-31");
    setValuationDate(valuationDate);

    var nominal = 1000000.0;
    var previousResetDate = Date.fromISOString("2012-11-20");
    var maturity = Date.fromISOString("2022-11-20");
    var spread = 0.02;
    var fixedRate = 0.04;
    var previousResetValue = 0.01;

    var fixedTenor = new Period(1, TimeUnit.Years);
    var floatTenor = new Period(3, TimeUnit.Months);

    var curveDates = new QuantLib.vector$Date$(3);
    curveDates.set(0, valuationDate);
    curveDates.set(1, Date.fromISOString("2013-12-31"));
    curveDates.set(2, Date.fromISOString("2024-12-31"));

    var forwardCurveDfs = new QuantLib.vector$double$(3);
    forwardCurveDfs.set(0, 1);
    forwardCurveDfs.set(1, 0.99);
    forwardCurveDfs.set(2, 0.8);

    var discountCurveDfs = new QuantLib.vector$double$(3);
    discountCurveDfs.set(0, 1);
    discountCurveDfs.set(1, 0.999);
    discountCurveDfs.set(2, 0.89);

    var actual360 = new Actual360();

    var forwardingTermStructure = QuantLib.createLogLinearYieldTermStructure(curveDates, forwardCurveDfs, actual360);
    var discountTermStructure = QuantLib.createLogLinearYieldTermStructure(curveDates, discountCurveDfs, actual360);

    var calendar = QuantLib.Sweden;
    var convention = BusinessDayConvention.ModifiedFollowing;
    var terminationDateConvention = BusinessDayConvention.ModifiedFollowing;
    var rule = DateGenerationRule.Forward;
    var endOfMonth = false;
    var firstDate = new Date();
    var nextToLastDate = new Date();

    var fixedDayCount = new Thirty360();
    var floatingDayCount = new Actual360();

    var fixedSchedule = new Schedule(
        previousResetDate,
        maturity,
        fixedTenor,
        calendar,
        convention,
        terminationDateConvention,
        rule,
        endOfMonth,
        firstDate,
        nextToLastDate
    );
    var floatSchedule = new Schedule(
        previousResetDate,
        maturity,
        floatTenor,
        calendar,
        convention,
        terminationDateConvention,
        rule,
        endOfMonth,
        firstDate,
        nextToLastDate
    );

    var previousResetDate = Date.fromISOString("2012-11-20");
    var previousResetValue = 0.01;

    var euribor = new Euribor(floatTenor, forwardingTermStructure);
    var previousFixingDate = euribor.fixingDate(previousResetDate);
    euribor.addFixing(previousFixingDate, previousResetValue, true);

    var swap = new VanillaSwap(
        VanillaSwapType.Payer,
        nominal,
        fixedSchedule,
        fixedRate,
        fixedDayCount,
        floatSchedule,
        euribor,
        spread,
        floatingDayCount
    );
    swap.setPricingEngine(discountTermStructure);
    var v = swap.NPV();

    valuationDate.delete();
    maturity.delete();
    fixedTenor.delete();
    floatTenor.delete();
    curveDates.delete();
    forwardCurveDfs.delete();
    discountCurveDfs.delete();
    forwardingTermStructure.delete();
    discountTermStructure.delete();
    firstDate.delete();
    nextToLastDate.delete();
    fixedDayCount.delete();
    fixedSchedule.delete();
    floatSchedule.delete();
    previousResetDate.delete();
    euribor.delete();
    previousFixingDate.delete();
    swap.delete();
    actual360.delete();

    return v;
}

function replicateSwapExample1() {
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

    var wasmValuationDate = dateToSerialNumber(valuationDate);
    var wasmPreviousResetDate = dateToSerialNumber(previousResetDate);
    var wasmMaturity = dateToSerialNumber(maturity);
    var wasmForwardCurveDates = toWasmIntVector(forwardCurve.map((d) => dateToSerialNumber(d.date)));
    var wasmForwardCurveDfs = toWasmDoubleVector(forwardCurve.map((d) => d.value));
    var wasmOisCurveDates = toWasmIntVector(oisCurve.map((d) => dateToSerialNumber(d.date)));
    var wasmOisCurveDfs = toWasmDoubleVector(oisCurve.map((d) => d.value));

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
}

function generateSchedule() {
    const { Month, Date, Period, TimeUnit, BusinessDayConvention, DateGenerationRule, Schedule } = QuantLib;
    var effectiveDate = new Date(15, Month.May, 2019);
    var terminationDate = new Date(31, Month.May, 2023);
    var calendar = QuantLib.Sweden;
    var tenor = new Period(6, TimeUnit.Months);
    var convention = BusinessDayConvention.ModifiedFollowing;
    var terminationDateConvention = BusinessDayConvention.ModifiedFollowing;
    var rule = DateGenerationRule.Backward;
    var endOfMonth = false;
    var firstDate = new Date();
    var nextToLastDate = new Date();

    var schedule = new QuantLib.Schedule(
        effectiveDate,
        terminationDate,
        tenor,
        calendar,
        convention,
        terminationDateConvention,
        rule,
        endOfMonth,
        firstDate,
        nextToLastDate
    );

    var dates = schedule.dates();
    for (let i = 0; i < dates.size(); i++) {
        console.log(dates.get(i).toISOString());
    }

    var schedule2 = new QuantLib.Schedule(dates);
    var dates2 = schedule2.dates();
    for (let i = 0; i < dates2.size(); i++) {
        console.log(dates2.get(i).toISOString());
    }

    // cleanup
    effectiveDate.delete();
    terminationDate.delete();
    tenor.delete();
    firstDate.delete();
    nextToLastDate.delete();
    dates.delete();
    schedule.delete();
    dates2.delete();
    schedule2.delete();
}

function testDate() {
    const { Date, Month } = QuantLib;
    var myDate = new Date(12, Month.Aug, 2009);
    console.log(" Original Date: " + myDate.toISOString());
    console.log(" Weekday: " + myDate.weekday());
    console.log(" Day of Month: " + myDate.dayOfMonth());
    console.log(" Day of Year: " + myDate.dayOfYear());
    console.log(" Month: " + myDate.month());
    var month = myDate.month();
    console.log(" Month via Integer: " + month.value);
    console.log(" Year: " + myDate.year());
    var serialNum = myDate.serialNumber();
    console.log("Serial Number: " + serialNum);
}

function testVector() {
    for (let i = 0; i < 10; i++) {
        let n = 500000;
        let arr = new QuantLib.vector$double$(n);
        for (let j = 0; j < n; j++) {
            arr.set(j, j + 1);
        }
        arr.delete();
        var v = QuantLib.mallinfo();
        console.log(JSON.stringify(v));
    }
}

var QuantLibLoader = QuantLib();
QuantLibLoader.onRuntimeInitialized = () => {
    QuantLib = QuantLibLoader;

    // testVector();
    // var d = new QuantLib.Date(34000);
    // console.log(d.toISOString());
    // generateSchedule();
    // BusinessDayConvention terminationDateConvention, DateGeneration::Rule rule, bool endOfMonth,
    // int firstDateAsSerialNumber = 0, int nextToLastDateAsSerialNumber = 0
    // var arr = [...Array(100000)].map((d, i) => i + 1);
    // for (let index = 0; index < 1000; index++) {
    // 	let a = toWasmDoubleVector(arr);
    // 	let v = QuantLib.stdev(a);
    // 	a.delete();
    // }
    // var s = QuantLib.createScheduleFromDates(toWasmIntVector([35000, 36000]));

    // Works fine
    // const { MyClass } = QuantLib;
    // for (let i = 0; i < 1000000; i++) {
    //     let instance = new MyClass(10, "hello");
    //     instance.incrementX();
    //     let x = instance.x; // 11
    //     instance.x = 20; // 20
    //     let s = MyClass.getStringFromInstance(instance); // "hello"
    //     instance.delete();
    // }

    var v;
    for (let i = 0; i < 3; i++) {
        v = replicateSwapExample2();
    }
    let m = QuantLib.mallinfo();
    console.log(JSON.stringify(m));
    console.log(v);
    // performanceTest();
    // testDate();
};
