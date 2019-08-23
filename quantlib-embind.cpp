// Build with:
// emcc --bind -I${EMSCRIPTEN}/system/include -I${QUANTLIB} -I${BOOST} -O3 -s MODULARIZE=1 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['addOnPostRun']" -s EXPORT_NAME=QuantLib -s TOTAL_MEMORY=64MB -o quantlib-embind.js quantlib-embind.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a

// https://www.quantlib.org/slides/dima-ql-intro-1.pdf

#include <math.h>
#include <iostream>
#include <malloc.h>
#include <ql/quantlib.hpp>
#include <emscripten/bind.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace QuantLib;
using namespace emscripten;

namespace
{

val emval_test_mallinfo()
{
    const auto &i = mallinfo();
    unsigned long t =
        std::chrono::system_clock::now().time_since_epoch() /
        std::chrono::milliseconds(1);
    val rv(val::object());
    rv.set("arena", val(i.arena));
    rv.set("ordblks", val(i.ordblks));
    rv.set("smblks", val(i.smblks));
    rv.set("hblks", val(i.hblks));
    rv.set("hblkhd", val(i.hblkhd));
    rv.set("usmblks", val(i.usmblks));
    rv.set("fsmblks", val(i.fsmblks));
    rv.set("uordblks", val(i.uordblks));
    rv.set("fordblks", val(i.fordblks));
    rv.set("keepcost", val(i.keepcost));
    rv.set("time", val(t));
    return rv;
}

void setValuationDate(Date &date)
{
    Settings::instance().evaluationDate() = date;
}

string timeUnitToString(Period &p)
{
    stringstream stream;
    stream << p;
    return stream.str();
}

double swapNpv(VanillaSwap &swap)
{
    return swap.NPV();
}

string calendarToISOString(Date &d)
{
    stringstream stream;
    stream << d.year() << "-" << setfill('0') << setw(2) << (int)d.month() << "-" << setfill('0') << setw(2) << d.dayOfMonth();
    return stream.str();
}

string calendarToString(Date &d)
{
    stringstream stream;
    stream << d;
    return stream.str();
}

Date calendarAdvance(Calendar &cal, Date &d, Integer n, TimeUnit unit, BusinessDayConvention c, bool endOfMonth)
{
    return cal.advance(d, n, unit, c, endOfMonth);
}

Date *dateFromISOString(string s)
{
    int y, m, d;
    sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d);
    return new Date(d, (Month)m, y);
}

void swapSetPricingEngine(VanillaSwap &swap, Handle<YieldTermStructure> &discountingTermStructure)
{
    boost::shared_ptr<PricingEngine> swapEngine(new DiscountingSwapEngine(discountingTermStructure));
    swap.setPricingEngine(swapEngine);
}

Handle<YieldTermStructure> *createLogLinearYieldTermStructure(vector<Date> &dates, vector<DiscountFactor> &discountFactor, DayCounter &dayCounter)
{
    // auto curve = new InterpolatedDiscountCurve<LogLinear>(dates, discountFactor, dayCounter);
    // InterpolatedDiscountCurve<LogLinear> curve(InterpolatedDiscountCurve<LogLinear>(dates, discountFactor, dayCounter));
    // auto yts = boost::make_shared<InterpolatedDiscountCurve<LogLinear>>(*curve);
    boost::shared_ptr<YieldTermStructure> yts(new InterpolatedDiscountCurve<LogLinear>(dates, discountFactor, dayCounter));
    return new Handle<YieldTermStructure>(yts);
}

VanillaSwap *createVanillaSwap(VanillaSwap::Type type, Real nominal, const Schedule &fixedSchedule, Rate fixedRate, DayCounter &fixedDayCount,
                               const Schedule &floatSchedule, IborIndex &iborIndex, Spread spread, DayCounter &floatingDayCount)
{
    boost::shared_ptr<IborIndex> iborIndexPtr = boost::make_shared<IborIndex>(iborIndex);
    VanillaSwap *res = new VanillaSwap(type, nominal, fixedSchedule, fixedRate, fixedDayCount, floatSchedule, iborIndexPtr, spread, floatingDayCount);
    return res;
}

Handle<Quote> *createQuoteHandle(Rate rate)
{
    boost::shared_ptr<SimpleQuote> ptrSimpleQuote = boost::make_shared<SimpleQuote>(rate);
    boost::shared_ptr<Quote> ptrQuote(ptrSimpleQuote);
    return new Handle<Quote>(ptrQuote);
}

OISRateHelper *createOISRateHelper(Natural settlementDays, Period &tenor, Handle<Quote> &fixedRate, OvernightIndex &overnightIndex
                                   /*, Handle<YieldTermStructure> &discountingCurve, bool telescopicValueDates, Natural paymentLag,
                                   BusinessDayConvention paymentConvention, Frequency paymentFrequency, Calendar &paymentCalendar, Period &forwardStart,
                                   Spread overnightSpread */
)
{
    ext::shared_ptr<OvernightIndex> ptrOvernightIndex(&overnightIndex);
    return new OISRateHelper(settlementDays, tenor, fixedRate, ptrOvernightIndex /*, discountingCurve,
                                  telescopicValueDates, paymentLag, paymentConvention, paymentFrequency, paymentCalendar, forwardStart, overnightSpread*/
    );
}

DatedOISRateHelper *createDatedOISRateHelper(Date &startDate, Date &endDate, Handle<Quote> &fixedRate, OvernightIndex &overnightIndex)
{
    ext::shared_ptr<OvernightIndex> ptrOvernightIndex(&overnightIndex);
    return new DatedOISRateHelper(startDate, endDate, fixedRate, ptrOvernightIndex);
}

FuturesRateHelper *createFuturesRateHelper(Real price, Date &iborStartDate, Natural lengthInMonths, Calendar &calendar, BusinessDayConvention convention, bool endOfMonth, DayCounter &dayCounter)
{
    return new FuturesRateHelper(price, iborStartDate, lengthInMonths, calendar, convention, endOfMonth, dayCounter);
}

SwapRateHelper *createSwapRateHelper(Handle<Quote> &rate, SwapIndex &swapIndex, Handle<Quote> &spread,
                                     Period &fwdStart, Handle<YieldTermStructure> &discountingCurve, Pillar::Choice pillar, Date customPillarDate)
{
    ext::shared_ptr<SwapIndex> ptrSwapIndex(&swapIndex);
    return new SwapRateHelper(rate, ptrSwapIndex, spread, fwdStart, discountingCurve, pillar, customPillarDate);
}

// class MyClassA
// {
// public:
//     MyClassA(int x, string y)
//         : x(x), y(y)
//     {
//     }

//     ~MyClassA()
//     {
//         // cout << "MyClassA destructor" << endl;
//     }

//     void incrementX()
//     {
//         ++x;
//     }

//     int getX() const { return x; }
//     void setX(int value) { x = value; }

//     static string getStringFromInstance(const MyClassA &instance)
//     {
//         return instance.y;
//     }

// private:
//     int x;
//     string y;
// };

// class MyClassB
// {
// public:
//     MyClassB(MyClassA &aref)
//     {
//         // cout << "x = " << aref.getX() << endl;
//         a = std::make_shared<MyClassA>(aref);
//         // cout << "x = " << a->getX() << endl;
//     }

//     ~MyClassB()
//     {
//         // cout << "MyClassB destructor" << endl;
//     }

//     int getX() const { return a->getX(); }
//     void setX(int value) { a->setX(value); }

// private:
//     std::shared_ptr<MyClassA> a;
// };

// EMSCRIPTEN_BINDINGS(dev)
// {
//     class_<MyClassA>("MyClassA")
//         .constructor<int, string>()
//         .function("incrementX", &MyClassA::incrementX)
//         .property("x", &MyClassA::getX, &MyClassA::setX)
//         .class_function("getStringFromInstance", &MyClassA::getStringFromInstance);
//     class_<MyClassB>("MyClassB")
//         .constructor<MyClassA &>()
//         .property("x", &MyClassB::getX, &MyClassB::setX);
// }

EMSCRIPTEN_BINDINGS(quantlib)
{
    emscripten::constant<string>("version", QL_VERSION);
    enum_<BusinessDayConvention>("BusinessDayConvention")
        .value("Following", Following)
        .value("ModifiedFollowing", ModifiedFollowing)
        .value("Preceding", Preceding)
        .value("ModifiedPreceding", ModifiedPreceding)
        .value("Unadjusted", Unadjusted)
        .value("HalfMonthModifiedFollowing", HalfMonthModifiedFollowing)
        .value("Nearest", Nearest);
    enum_<Month>("Month")
        .value("January", January)
        .value("February", February)
        .value("March", March)
        .value("April", April)
        .value("May", May)
        .value("June", June)
        .value("July", July)
        .value("August", August)
        .value("September", September)
        .value("October", October)
        .value("November", November)
        .value("December", December)
        .value("Jan", Jan)
        .value("Feb", Feb)
        .value("Mar", Mar)
        .value("Apr", Apr)
        .value("May", May)
        .value("Jun", Jun)
        .value("Jul", Jul)
        .value("Aug", Aug)
        .value("Sep", Sep)
        .value("October", Oct)
        .value("Nov", Nov)
        .value("Dec", Dec);
    enum_<TimeUnit>("TimeUnit")
        .value("D", Days)
        .value("W", Weeks)
        .value("M", Months)
        .value("Y", Years)
        .value("Days", Days)
        .value("Weeks", Weeks)
        .value("Months", Months)
        .value("Years", Years)
        .value("Hours", Hours)
        .value("Minutes", Minutes)
        .value("Seconds", Seconds)
        .value("Milliseconds", Milliseconds)
        .value("Microseconds", Microseconds);
    enum_<DateGeneration::Rule>("DateGenerationRule")
        .value("Backward", DateGeneration::Backward)
        .value("Forward", DateGeneration::Forward)
        .value("Zero", DateGeneration::Zero)
        .value("ThirdWednesday", DateGeneration::ThirdWednesday)
        .value("Twentieth", DateGeneration::Twentieth)
        .value("TwentiethIMM", DateGeneration::TwentiethIMM)
        .value("OldCDS", DateGeneration::OldCDS)
        .value("CDS", DateGeneration::CDS)
        .value("CDS2015", DateGeneration::CDS2015);
    enum_<Thirty360::Convention>("Thirty360Convention")
        .value("USA", Thirty360::USA)
        .value("BondBasis", Thirty360::BondBasis)
        .value("European", Thirty360::European)
        .value("EurobondBasis", Thirty360::EurobondBasis)
        .value("Italian", Thirty360::Italian)
        .value("German", Thirty360::German);
    enum_<ActualActual::Convention>("ActualActualConvention")
        .value("ISMA", ActualActual::ISMA)
        .value("Bond", ActualActual::Bond)
        .value("ISDA", ActualActual::ISDA)
        .value("Historical", ActualActual::Historical)
        .value("Actual365", ActualActual::Actual365)
        .value("AFB", ActualActual::AFB)
        .value("Euro", ActualActual::Euro);
    enum_<VanillaSwap::Type>("VanillaSwapType")
        .value("Payer", VanillaSwap::Payer)
        .value("Receiver", VanillaSwap::Receiver);
    enum_<Weekday>("Weekday")
        .value("Sunday", Sunday)
        .value("Monday", Monday)
        .value("Tuesday", Tuesday)
        .value("Wednesday", Wednesday)
        .value("Thursday", Thursday)
        .value("Friday", Friday)
        .value("Saturday", Saturday)
        .value("Sun", Sun)
        .value("Mon", Mon)
        .value("Tue", Tue)
        .value("Wed", Wed)
        .value("Thu", Thu)
        .value("Fri", Fri)
        .value("Sat", Sat);
    enum_<Pillar::Choice>("PillarChoice")
        .value("MaturityDate", Pillar::Choice::MaturityDate)
        .value("LastRelevantDate", Pillar::Choice::LastRelevantDate)
        .value("CustomDate", Pillar::Choice::CustomDate);
    enum_<UnitedStates::Market>("UnitedStatesMarket")
        .value("Settlement", UnitedStates::Market::Settlement)
        .value("NYSE", UnitedStates::Market::NYSE)
        .value("GovernmentBond", UnitedStates::Market::GovernmentBond)
        .value("NERC", UnitedStates::Market::NERC)
        .value("LiborImpact", UnitedStates::Market::LiborImpact)
        .value("FederalReserve", UnitedStates::Market::FederalReserve);
    enum_<UnitedKingdom::Market>("UnitedKingdomMarket")
        .value("Settlement", UnitedKingdom::Market::Settlement)
        .value("Exchange", UnitedKingdom::Market::Exchange)
        .value("Metals", UnitedKingdom::Market::Metals);
    enum_<JointCalendarRule>("JointCalendarRule")
        .value("JoinHolidays", JointCalendarRule::JoinHolidays)
        .value("JoinBusinessDays", JointCalendarRule::JoinBusinessDays);
    class_<Date>("Date")
        .constructor<>()
        .constructor<int>()
        .constructor<int, Month, int>()
        .function("serialNumber", &Date::serialNumber)
        .function("weekday", &Date::weekday)
        .function("dayOfMonth", &Date::dayOfMonth)
        .function("dayOfYear", &Date::dayOfYear)
        .function("month", &Date::month)
        .function("year", &Date::year)
        .function("toISOString", &calendarToISOString)
        .function("toString", &calendarToString)
        .class_function("fromISOString", &dateFromISOString, allow_raw_pointers())
        .class_function("isLeap", &Date::isLeap);
    class_<Schedule>("Schedule")
        .constructor<vector<Date>>()
        .constructor<Date, Date, Period, Calendar, BusinessDayConvention, BusinessDayConvention, DateGeneration::Rule, bool, Date, Date>()
        .function("size", &Schedule::size)
        .function("dates", &Schedule::dates);
    class_<Calendar>("Calendar")
        .function("name", &Calendar::name)
        .function("toString", &calendarToString)
        .function("isBusinessDay", &Calendar::isBusinessDay)
        .function("adjust", &Calendar::adjust)
        .function("advance", &calendarAdvance);
    class_<TARGET, base<Calendar>>("TARGET").constructor<>();
    class_<NullCalendar, base<Calendar>>("NullCalendar").constructor<>();
    class_<UnitedKingdom, base<Calendar>>("UnitedKingdom").constructor<UnitedKingdom::Market>();
    class_<UnitedStates, base<Calendar>>("UnitedStates").constructor().constructor<UnitedStates::Market>();
    class_<Sweden, base<Calendar>>("Sweden").constructor<>();
    class_<Period>("Period")
        .constructor<int, TimeUnit>()
        .function("toString", &timeUnitToString);
    class_<DayCounter>("DayCounter")
        .function("name", &DayCounter::name)
        .function("dayCount", &DayCounter::dayCount)
        .function("yearFraction", &DayCounter::yearFraction);
    class_<Thirty360, base<DayCounter>>("Thirty360")
        .constructor<>()
        .constructor<Thirty360::Convention, bool>();
    class_<Actual360, base<DayCounter>>("Actual360")
        .constructor<>()
        .constructor<bool>();
    class_<Actual365Fixed, base<DayCounter>>("Actual365Fixed").constructor<>();
    class_<ActualActual, base<DayCounter>>("ActualActual")
        .constructor<>()
        .constructor<ActualActual::Convention, Schedule>();
    class_<Business252, base<DayCounter>>("Business252").constructor<>();
    class_<boost::optional<BusinessDayConvention>>("OptionalBusinessDayConvention");
    class_<VanillaSwap>("VanillaSwap")
        .constructor(&createVanillaSwap, allow_raw_pointers())
        // .constructor<VanillaSwap::Type, Rate, Schedule, Rate, DayCounter, Schedule, ext::shared_ptr<IborIndex>, Spread, DayCounter, boost::optional<BusinessDayConvention>>()
        .class_function("create", &createVanillaSwap, allow_raw_pointers())
        .function("setPricingEngine", &swapSetPricingEngine)
        .function("NPV", &swapNpv);
    class_<Handle<YieldTermStructure>>("Handle<YieldTermStructure>");
    emscripten::function("createVanillaSwap", &createVanillaSwap, allow_raw_pointers());
    emscripten::function("mallinfo", &emval_test_mallinfo);
    emscripten::function("setValuationDate", &setValuationDate);
    emscripten::function("createLogLinearYieldTermStructure", &createLogLinearYieldTermStructure, allow_raw_pointers());
    register_vector<int>("Vector<int>")
        .constructor<int>();
    register_vector<double>("Vector<double>")
        .constructor<int>();
    register_vector<Date>("Vector<Date>")
        .constructor<int>();
    class_<DepositRateHelper>("DepositRateHelper")
        .constructor<Handle<Quote>, Period, Natural, Calendar, BusinessDayConvention, bool, DayCounter>();
    class_<Quote>("Quote");
    class_<SimpleQuote, base<Quote>>("SimpleQuote")
        .constructor<Real>();
    class_<Handle<Quote>>("QuoteHandle")
        .constructor(&createQuoteHandle, allow_raw_pointers());

    class_<Index>("Index")
        .function("addFixing", &Index::addFixing);
    class_<InterestRateIndex, base<Index>>("InterestRateIndex")
        .function("fixingDate", &InterestRateIndex::fixingDate);
    class_<IborIndex, base<InterestRateIndex>>("IborIndex");
    class_<Euribor, base<IborIndex>>("Euribor")
        .constructor<Period, Handle<YieldTermStructure>>();
    class_<OvernightIndex, base<IborIndex>>("OvernightIndex");
    class_<Eonia, base<OvernightIndex>>("Eonia").constructor();
    class_<Libor, base<IborIndex>>("Libor");
    class_<USDLibor, base<Libor>>("USDLibor")
        .constructor<Period, Handle<YieldTermStructure>>();

    class_<OISRateHelper>("OISRateHelper").constructor(&createOISRateHelper, allow_raw_pointers());
    class_<DatedOISRateHelper>("DatedOISRateHelper").constructor(&createDatedOISRateHelper, allow_raw_pointers());
    class_<FuturesRateHelper>("FuturesRateHelper").constructor(&createFuturesRateHelper, allow_raw_pointers());
    class_<SwapRateHelper>("SwapRateHelper").constructor(&createSwapRateHelper, allow_raw_pointers());
}

} // namespace

// In [9]: eonia_curve_c = PiecewiseLogCubicDiscount(0, TARGET(),
// helpers, Actual365Fixed())
// eonia_curve_c.enableExtrapolation()
