var QuantLibModule = require("./quantlib-embind");

var QuantLib = null;
var Date,
    Weekday,
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
    Euribor;
const bytesDiff = (m1, m0) => m1.uordblks - m0.uordblks + (m1.hblkhd - m0.hblkhd);

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
        Date = QuantLib.Date;
        Weekday = QuantLib.Weekday;
        Period = QuantLib.Period;
        TimeUnit = QuantLib.TimeUnit;
        BusinessDayConvention = QuantLib.BusinessDayConvention;
        DateGenerationRule = QuantLib.DateGenerationRule;
        Schedule = QuantLib.Schedule;
        VanillaSwapType = QuantLib.VanillaSwapType;
        VanillaSwap = QuantLib.VanillaSwap;
        setValuationDate = QuantLib.setValuationDate;
        Thirty360 = QuantLib.Thirty360;
        Actual360 = QuantLib.Actual360;
        Euribor = QuantLib.Euribor;
    });

    test("Sweden Calendar", async () => {
        expect(QuantLib.Sweden.name()).toBe("Sweden");
    });

    test("Calendar weekday", async () => {
        var settlementDate = Date.fromISOString("2008-09-18");
        expect(settlementDate.weekday().value).toBe(Weekday.Thursday.value);
        settlementDate.delete();
    });

    test("Calendar adjust and advance", async () => {
        // var ms = [];
        // ms.push(QuantLib.mallinfo());
        var settlementDate = Date.fromISOString("2008-09-18");
        var calendar = QuantLib.TARGET;
        var settlementDateAdj = calendar.adjust(settlementDate, BusinessDayConvention.Following);
        var fixingDays = 3;
        var todaysDate = calendar.advance(settlementDateAdj, -fixingDays, TimeUnit.Days, BusinessDayConvention.Following, true);
        expect(todaysDate.toISOString()).toBe("2008-09-15");
        // ms.push(QuantLib.mallinfo());
        [settlementDate, settlementDateAdj, todaysDate].forEach((d) => d.delete());
        // ms.push(QuantLib.mallinfo());
        // console.log(
        //     ms
        //         .filter((d, i) => i !== 0)
        //         .map((d) => bytesDiff(d, ms[0]))
        //         .join(", ")
        // );
    });
});
