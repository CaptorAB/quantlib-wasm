var QuantLib = require(".");

const dateToSerialNumber = (d) => d.getTime() / 86400000 + 25569;
const serialNumberToDate = (n) => new Date((n - 25569) * 86400000);
const addDays = (d, n) => new Date(d.getTime() + 86400000 * n);
const bytesDiff = (m0, m1) => m1.uordblks - m0.uordblks + (m1.hblkhd - m0.hblkhd);

const toWasmVector = (arr, type) => {
    let res = new type(arr.length);
    for (let j = 0; j < arr.length; j++) {
        res.set(j, arr[j]);
    }
    return res;
};

const toCurveItem = (date, value) => ({ date, value });
const hrtimeDiffToMs = (t0, t1) => 1000 * (t1[0] - t0[0]) + (t1[1] - t0[1]) / 1000000;

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

    var arr = [...Array(400000)].map((d, i) => i + 1);
    var t0 = process.hrtime();
    var s0 = stdev(arr);
    var t1 = process.hrtime();
    var v1 = toWasmVector(arr, QuantLib.Vector$double$);
    var s1 = QuantLib.stdev(v1);
    var t2 = process.hrtime();
    var v2 = toWasmVector(arr, QuantLib.Vector$double$);
    QuantLib.stdevDummy(v2);
    var t3 = process.hrtime();
    v1.delete();
    v2.delete();
    console.log(s0, s1);
    console.log("JavaScript " + hrtimeDiffToMs(t0, t1));
    console.log("WebAssembly " + hrtimeDiffToMs(t1, t2));
    console.log("WebAssembly copy data " + hrtimeDiffToMs(t2, t3));
}

class Dummy {
    delete() {}
}

function replicateSwapExample2() {
    var ms = [];
    ms.push(QuantLib.mallinfo());

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
        Euribor,
        Month
    } = QuantLib;
    const { January, February, March, April, May, June, July, August, September, October, November, December } = Month;

    var valuationDate = new Date(31, December, 2012);
    setValuationDate(valuationDate);

    var nominal = 1000000.0;
    var previousResetDate = new Date(20, November, 2012);
    var maturity = new Date(20, November, 2022);
    var spread = 0.02;
    var fixedRate = 0.04;
    var previousResetValue = 0.01;

    var fixedTenor = new Period(1, TimeUnit.Years);
    var floatTenor = new Period(3, TimeUnit.Months);

    var curveDates = new QuantLib.Vector$Date$(3);
    var curveDateObjs = [new Date(31, December, 2013), new Date(31, December, 2024)];
    curveDates.set(0, valuationDate);
    curveDateObjs.forEach((d, i) => curveDates.set(i + 1, d));

    var forwardCurveDfs = new QuantLib.Vector$double$(3);
    forwardCurveDfs.set(0, 1);
    forwardCurveDfs.set(1, 0.99);
    forwardCurveDfs.set(2, 0.8);

    var discountCurveDfs = new QuantLib.Vector$double$(3);
    discountCurveDfs.set(0, 1);
    discountCurveDfs.set(1, 0.999);
    discountCurveDfs.set(2, 0.89);

    var actual360 = new Actual360();
    var forwardingTermStructure = QuantLib.createLogLinearYieldTermStructure(curveDates, forwardCurveDfs, actual360);
    var discountTermStructure = QuantLib.createLogLinearYieldTermStructure(curveDates, discountCurveDfs, actual360);

    var calendar = new QuantLib.Sweden();
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

    ms.push(QuantLib.mallinfo());

    [
        ...curveDateObjs,
        valuationDate,
        maturity,
        fixedTenor,
        floatTenor,
        curveDates,
        forwardCurveDfs,
        discountCurveDfs,
        forwardingTermStructure,
        discountTermStructure,
        firstDate,
        nextToLastDate,
        fixedDayCount,
        fixedSchedule,
        floatingDayCount,
        floatSchedule,
        previousResetDate,
        euribor,
        previousFixingDate,
        swap,
        actual360,
        calendar
    ].forEach((d) => d.delete());

    ms.push(QuantLib.mallinfo());
    // ms.forEach((d) => console.log(JSON.stringify(d)));
    // console.log(
    //     ms
    //         .filter((d, i) => i !== 0)
    //         .map((d) => bytesDiff(d, ms[0]))
    //         .join(", ")
    // );

    return v;
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

function eoniaCurveBootstrapping() {
    const { Date, Period, TimeUnit, BusinessDayConvention, Schedule, Month, setValuationDate, SimpleQuote } = QuantLib;
    const { TARGET, QuoteHandle, DepositRateHelper, OISRateHelper, DatedOISRateHelper } = QuantLib;
    const { Actual360, Eonia } = QuantLib;
    const { Years, Months, Weeks, Days } = TimeUnit;
    const { Following } = BusinessDayConvention;
    const { January, February, March, April, May, June, July, August, September, October, November, December } = Month;

    var today = new Date(11, Month.December, 2002);

    setValuationDate(today);
    var calendar = new TARGET();
    var dc = new Actual360();

    var depositRates = [0.04, 0.04, 0.04];
    var fixingDays = [0, 1, 2];
    var period = new Period(1, Days);

    var helpers = [];

    depositRates.forEach((rate, i) => {
        var fixingDay = fixingDays[i];
        var quoteHandle = new QuoteHandle(rate / 100);
        helpers.push(new DepositRateHelper(quoteHandle, period, fixingDay, calendar, Following, false, dc));
    });

    var eonia = new Eonia();

    var shortOisData = [[0.07, 1, Weeks], [0.069, 2, Weeks], [0.078, 3, Weeks], [0.074, 1, Months]];
    shortOisData.forEach((data, i) => {
        var quoteHandle = new QuoteHandle(data[0] / 100);
        var tenor = new Period(data[1], data[2]);
        helpers.push(new OISRateHelper(2, tenor, quoteHandle, eonia));
    });

    var datedOisData = [
        [0.046, new Date(16, January, 2013), new Date(13, February, 2013)],
        [0.016, new Date(13, February, 2013), new Date(13, March, 2013)],
        [-0.007, new Date(13, March, 2013), new Date(10, April, 2013)],
        [-0.013, new Date(10, April, 2013), new Date(8, May, 2013)],
        [-0.014, new Date(8, May, 2013), new Date(12, June, 2013)]
    ];
    datedOisData.forEach((data, i) => {
        var quoteHandle = new QuoteHandle(data[0] / 100);
        helpers.push(new DatedOISRateHelper(data[1], data[2], quoteHandle, eonia));
    });

    // In [8]: helpers += [ OISRateHelper(2, Period(*tenor),
    //     QuoteHandle(SimpleQuote(rate/100)), eonia)
    //     for rate, tenor in [(0.002, (15,Months)), (0.008, (18,Months)),
    //     (0.021, (21,Months)), (0.036, (2,Years)),
    //     (0.127, (3,Years)), (0.274, (4,Years)),
    //     (0.456, (5,Years)), (0.647, (6,Years)),
    //     (0.827, (7,Years)), (0.996, (8,Years)),
    //     (1.147, (9,Years)), (1.280, (10,Years)),
    //     (1.404, (11,Years)), (1.516, (12,Years)),
    //     (1.764, (15,Years)), (1.939, (20,Years)),
    //     (2.003, (25,Years)), (2.038, (30,Years))] ]

    var longOisData = [
        [0.002, 15, Months],
        [0.008, 18, Months],
        [0.021, 21, Months],
        [0.036, 2, Years],
        [0.127, 3, Years],
        [0.274, 4, Years],
        [0.456, 5, Years],
        [0.647, 6, Years],
        [0.827, 7, Years],
        [0.996, 8, Years],
        [1.147, 9, Years],
        [1.28, 10, Years],
        [1.404, 11, Years],
        [1.516, 12, Years],
        [1.764, 15, Years],
        [1.939, 20, Years],
        [2.003, 25, Years],
        [2.038, 30, Years]
    ];

    longOisData.forEach((data, i) => {
        var quoteHandle = new QuoteHandle(data[0] / 100);
        var tenor = new Period(data[1], data[2]);
        helpers.push(new OISRateHelper(2, tenor, quoteHandle, eonia));
    });

    console.log(today.toISOString());

    // var previousResetDate = Date.fromISOString("2012-11-20");
    // var maturity = Date.fromISOString("2022-11-20");
    // var fixedTenor = new Period(1, TimeUnit.Years);
    // var calendar = QuantLib.TARGET;
    // var convention = BusinessDayConvention.ModifiedFollowing;
    // var terminationDateConvention = BusinessDayConvention.ModifiedFollowing;
    // var rule = DateGenerationRule.Forward;
    // var endOfMonth = false;
    // var firstDate = new Date();
    // var nextToLastDate = new Date();

    // var fixedSchedule = new Schedule(
    //     previousResetDate,
    //     maturity,
    //     fixedTenor,
    //     calendar,
    //     convention,
    //     terminationDateConvention,
    //     rule,
    //     endOfMonth,
    //     firstDate,
    //     nextToLastDate
    // );
    // var dates = fixedSchedule.dates();
    // for (let i = 0; i < dates.size(); i++) {
    //     let d = dates.get(i);
    //     console.log(d.toISOString());
    // }

    [today, calendar, dc, period, eonia, ...tenors].forEach((d) => d.delete());
}

function billiontraderBootstrapping() {
    const { Date, UnitedKingdom, UnitedStates, JointCalendar, UnitedKingdomMarket, UnitedStatesMarket, JointCalendarRule } = QuantLib;
    const { TimeUnit, BusinessDayConvention, Month, setValuationDate, Actual360, QuoteHandle, Period, DepositRateHelper } = QuantLib;
    const { IMM, FuturesRateHelper, SwapRateHelper, Frequency, USDLibor, Compounding } = QuantLib;
    const { January, February, March, April, May, June, July, August, September, October, November, December } = Month;
    const { Days, Weeks, Months, Years } = TimeUnit;
    const { Simple, Compounded } = Compounding;

    var cal1 = new UnitedKingdom(UnitedKingdomMarket.Exchange);
    var cal2 = new UnitedStates(UnitedStatesMarket.Settlement);
    var calendar = new JointCalendar(cal1, cal2, JointCalendarRule.JoinHolidays);

    var d0 = new Date(18, February, 2015);
    var settlementDate = calendar.adjust(d0, BusinessDayConvention.Following);
    var fixingDays = 2;
    var todaysDate = calendar.advance(settlementDate, -fixingDays, TimeUnit.Days, BusinessDayConvention.Following, false);
    setValuationDate(todaysDate);

    var depositDayCounter = new Actual360();
    var depositsBusinessDayConvention = BusinessDayConvention.ModifiedFollowing;

    var trashcan = [cal1, cal2, calendar, d0, settlementDate, todaysDate, depositDayCounter];

    var depoFutSwapInstruments = [];

    var deposits = [
        { rate: 0.001375, periods: 7, unit: Days },
        { rate: 0.001717, periods: 4, unit: Weeks },
        { rate: 0.002112, periods: 2, unit: Months },
        { rate: 0.002581, periods: 3, unit: Months }
    ];
    deposits.forEach((d) => {
        var quote = new QuoteHandle(d.rate);
        var period = new Period(d.periods, d.unit);
        depoFutSwapInstruments.push(
            new DepositRateHelper(quote, period, fixingDays, calendar, depositsBusinessDayConvention, true, depositDayCounter)
        );
        trashcan.push(quote);
        trashcan.push(period);
    });

    var futDayCounter = new Actual360();
    var futMonths = 3;
    var futures = [{ rate: 99.725 }, { rate: 99.585 }, { rate: 99.385 }, { rate: 99.16 }, { rate: 98.93 }, { rate: 98.715 }];
    var imm = IMM.nextDate(settlementDate, true);
    trashcan.push(futDayCounter);
    futures.forEach((d) => {
        var quote = new QuoteHandle(d.rate);
        depoFutSwapInstruments.push(
            new FuturesRateHelper(quote, imm, futMonths, calendar, BusinessDayConvention.ModifiedFollowing, true, depositDayCounter)
        );
        trashcan.push(imm);
        imm = IMM.nextDate(imm, true);
        trashcan.push(quote);
    });
    trashcan.push(imm);

    var swFixedLegFrequency = Frequency.Annual;
    var swFixedLegConvention = BusinessDayConvention.Unadjusted;
    var swFixedLegDayCounter = new Actual360();
    var swFloatingLegIndexPeriod = new Period(3, Months);
    var swFloatingLegIndex = new USDLibor(swFloatingLegIndexPeriod);
    trashcan.push(swFixedLegDayCounter);
    trashcan.push(swFloatingLegIndexPeriod);
    trashcan.push(swFloatingLegIndex);

    var swaps = [
        { rate: 0.0089268, periods: 2, unit: Years },
        { rate: 0.0123343, periods: 3, unit: Years },
        { rate: 0.0147985, periods: 4, unit: Years },
        { rate: 0.0165843, periods: 5, unit: Years },
        { rate: 0.0179191, periods: 6, unit: Years }
    ];
    swaps.forEach((d) => {
        var quote = new QuoteHandle(d.rate);
        var period = new Period(d.periods, d.unit);
        depoFutSwapInstruments.push(
            new SwapRateHelper(quote, period, calendar, swFixedLegFrequency, swFixedLegConvention, swFixedLegDayCounter, swFloatingLegIndex)
        );
        trashcan.push(quote);
        trashcan.push(period);
    });

    var termStructureDayCounter = new Actual360();
    trashcan.push(termStructureDayCounter);
    var instrs = toWasmVector(depoFutSwapInstruments, QuantLib.Vector$RateHelper$);
    var depoFutSwapTermStructure = new QuantLib.PiecewiseYieldCurve$Discount$Linear$(
        settlementDate,
        instrs,
        termStructureDayCounter,
        1.0e-15
    );

    var maturities = [
        [0.1375, new Date(25, February, 2015), Simple],
        [0.1717, new Date(18, March, 2015), Simple],
        [0.2112, new Date(20, April, 2015), Simple],
        [0.2581, new Date(18, May, 2015), Simple],
        [0.25093, new Date(17, June, 2015), Simple],
        [0.32228, new Date(16, September, 2015), Simple],
        [0.41111, new Date(16, December, 2015), Simple],
        [0.51112, new Date(16, March, 2016), Simple],
        [0.61698, new Date(15, June, 2016), Simple],
        [0.73036, new Date(21, September, 2016), Compounded],
        [0.89446, new Date(21, February, 2017), Compounded],
        [1.23937, new Date(20, February, 2018), Compounded],
        [1.49085, new Date(19, February, 2019), Compounded],
        [1.6745, new Date(18, February, 2020), Compounded]
    ];
    var log = [];
    maturities.forEach((d, i) => {
        var interestRate = depoFutSwapTermStructure.zeroRate(d[1], depositDayCounter, d[2], Frequency.Annual, false);
        log.push(`${d[0]}: ${interestRate.toString()}`);
        interestRate.delete();
    });
    log.push(`Discount Rate : ${depoFutSwapTermStructure.discount(maturities[13][1], false).toFixed(6)}`);
    var forwardRate = depoFutSwapTermStructure.forwardRate(
        maturities[12][1],
        maturities[13][1],
        futDayCounter,
        Simple,
        Frequency.Annual,
        false
    );
    log.push(`Forward Rate : ${forwardRate}`);
    trashcan.push(forwardRate);
    maturities.forEach((d) => trashcan.push(d[1]));

    trashcan.push(depoFutSwapTermStructure);
    trashcan.push(instrs);

    trashcan.forEach((d) => d.delete());
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

    for (let i = 0; i < 1; i++) {
        // var m0 = QuantLib.mallinfo();
        console.log(replicateSwapExample2());
        // var m1 = QuantLib.mallinfo();
        // console.log(bytesDiff(m0, m1));
    }
};
