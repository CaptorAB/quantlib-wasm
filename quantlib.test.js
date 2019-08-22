var QuantLibModule = require(".");

var QuantLib = null;
const bytesDiff = (m0, m1) => m1.uordblks - m0.uordblks + (m1.hblkhd - m0.hblkhd);

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
        var calendar = new QuantLib.Sweden();
        expect(calendar.name()).toBe("Sweden");
        calendar.delete();
    });

    test("Calendar weekday", async () => {
        const { Date, Weekday } = QuantLib;
        var settlementDate = Date.fromISOString("2008-09-18");
        expect(settlementDate.weekday().value).toBe(Weekday.Thursday.value);
        settlementDate.delete();
    });

    test("Calendar adjust and advance", async () => {
        const { Date, BusinessDayConvention, TimeUnit, TARGET } = QuantLib;
        var settlementDate = Date.fromISOString("2008-09-18");
        var calendar = new TARGET();
        var settlementDateAdj = calendar.adjust(settlementDate, BusinessDayConvention.Following);
        var fixingDays = 3;
        var todaysDate = calendar.advance(settlementDateAdj, -fixingDays, TimeUnit.Days, BusinessDayConvention.Following, true);
        expect(todaysDate.toISOString()).toBe("2008-09-15");
        [settlementDate, settlementDateAdj, todaysDate, calendar].forEach((d) => d.delete());
    });

    test("Date", async () => {
        const { Date, Month } = QuantLib;
        var myDate = new Date(12, Month.Aug, 2009);
        expect(myDate.toISOString()).toBe("2009-08-12");
        expect(myDate.weekday().value).toBe(4);
        expect(myDate.dayOfMonth()).toBe(12);
        expect(myDate.dayOfYear()).toBe(224);
        expect(myDate.month().value).toBe(8);
        expect(myDate.year()).toBe(2009);
        expect(myDate.serialNumber()).toBe(40037);
    });

    test("DayCounters", async () => {
        const { Date, Month, Thirty360, Actual360, Actual365Fixed, ActualActual, Business252 } = QuantLib;
        var start = new Date(22, Month.January, 2017);
        var end = new Date(22, Month.August, 2019);

        var expects = [930, 942, 942, 942, 645];
        [Thirty360, Actual360, Actual365Fixed, ActualActual, Business252].forEach((type, i) => {
            var dc = new type();
            var d = dc.dayCount(start, end);
            expect(d).toBe(expects[i]);
            dc.delete();
        });
        [start, end].forEach((d) => d.delete());
    });

    test("Vector", async () => {
        const { Vector$double$ } = QuantLib;
        var m0 = QuantLib.mallinfo();
        for (let i = 0; i < 5; i++) {
            let n = 100000;
            let arr = new Vector$double$(n);
            for (let j = 0; j < n; j++) {
                arr.set(j, j + 1);
            }
            arr.delete();
        }
        var m1 = QuantLib.mallinfo();
        expect(bytesDiff(m0, m1)).toBe(0);
    });
});
