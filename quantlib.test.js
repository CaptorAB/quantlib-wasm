var QuantLibModule = require("./quantlib-embind");

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
        expect(QuantLib.Sweden.name()).toBe("Sweden");
    });

    test("Calendar weekday", async () => {
        const { Date, Weekday } = QuantLib;
        var settlementDate = Date.fromISOString("2008-09-18");
        expect(settlementDate.weekday().value).toBe(Weekday.Thursday.value);
        settlementDate.delete();
    });

    test("Calendar adjust and advance", async () => {
        const { Date, BusinessDayConvention, TimeUnit } = QuantLib;
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
        //         .map((d) => bytesDiff(ms[0], d))
        //         .join(", ")
        // );
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
