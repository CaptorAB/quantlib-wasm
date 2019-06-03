// Build with:
// emcc --bind -I${EMSCRIPTEN}/system/include -I${QUANTLIB} -I${BOOST} -O3 -s MODULARIZE=1 -s "EXTRA_EXPORTED_RUNTIME_METHODS=['addOnPostRun']" -s EXPORT_NAME=QuantLib -o quantlib-embind.js quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a

// https://www.quantlib.org/slides/dima-ql-intro-1.pdf

#include <math.h>
#include <iostream>
#include <malloc.h>
#include <ql/quantlib.hpp>
#include <emscripten/bind.h>
#include <boost/foreach.hpp>

// using namespace QuantLib;
// using namespace emscripten;
// using namespace std;

// using emscripten::class_;
// using emscripten::enum_;
// using emscripten::function;
// using std::string;

using namespace std;
using namespace QuantLib;
using namespace emscripten;

namespace
{

val emval_test_mallinfo()
{
    const auto &i = mallinfo();
    val rv(val::object());
    rv.set("arena", val(i.arena));
    rv.set("ordblks", val(i.ordblks));
    rv.set("smblks", val(i.smblks));
    rv.set("hblks", val(i.hblks));
    rv.set("usmblks", val(i.usmblks));
    rv.set("fsmblks", val(i.fsmblks));
    rv.set("uordblks", val(i.uordblks));
    rv.set("fordblks", val(i.fordblks));
    rv.set("keepcost", val(i.keepcost));
    return rv;
}

enum DayCountConvention
{
    DayCountConventionThirty360,
    DayCountConventionActual360,
    DayCountConventionActual365,
    DayCountConventionActualActual,
    DayCountConventionBusiness252
};

const Calendar swedenCalendar = Sweden();
const Calendar nullCalendar = NullCalendar();

void setValuationDate(Date &date)
{
    Settings::instance().evaluationDate() = date;
}

DayCounter toDayCounter(DayCountConvention dc)
{
    if (dc == DayCountConventionThirty360)
    {
        return QuantLib::Thirty360();
    }
    if (dc == DayCountConventionActual360)
    {
        return QuantLib::Actual360();
    }
    if (dc == DayCountConventionActual365)
    {
        return QuantLib::Actual365Fixed();
    }
    if (dc == DayCountConventionActualActual)
    {
        return QuantLib::ActualActual();
    }
    if (dc == DayCountConventionBusiness252)
    {
        return QuantLib::Business252();
    }
    //default
    return QuantLib::Actual360();
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

string toISOString(Date &d)
{
    stringstream stream;
    stream << d.year() << "-" << setfill('0') << setw(2) << (int)d.month() << "-" << setfill('0') << setw(2) << d.dayOfMonth();
    return stream.str();
}

// std::shared_ptr<Date> fromISOString(string s)
Date *dateFromISOString(string s)
{
    int y, m, d;
    sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d);
    // return std::make_shared<Date>(d, (Month)m, y);
    return new Date(d, (Month)m, y);
}

void swapSetPricingEngine(VanillaSwap &swap, Handle<YieldTermStructure> &discountingTermStructure)
{
    boost::shared_ptr<PricingEngine> swapEngine(new DiscountingSwapEngine(discountingTermStructure));
    swap.setPricingEngine(swapEngine);
}

double swapNpvExample(double nominal, double fixedRate, double spread, int valuationDateAsSerialNumber, int maturityAsSerialNumber,
                      int previousResetDateAsSerialNumber, double previousResetValue, int fixedScheduleCount, TimeUnit fixedScheduleTimeUnit,
                      int floatScheduleCount, TimeUnit floatScheduleTimeUnit,
                      DayCountConvention forwardCurveDayCountConvention, DayCountConvention oisCurveDayCountConvention,
                      vector<int> &forwardCurveDatesAsSerialNumbers, vector<double> &forwardCurveDfs, vector<int> &oisCurveDatesAsSerialNumbers, vector<double> &oisCurveDfs)
{
    cout << forwardCurveDayCountConvention << endl;
    Settings::instance().evaluationDate() = Date(valuationDateAsSerialNumber);

    int i;
    vector<Date> forwardCurveDates = vector<Date>(forwardCurveDatesAsSerialNumbers.size());
    for (i = 0; i < forwardCurveDatesAsSerialNumbers.size(); i++)
        forwardCurveDates[i] = Date(forwardCurveDatesAsSerialNumbers[i]);
    vector<Date> oisCurveDates = vector<Date>(oisCurveDatesAsSerialNumbers.size());
    for (i = 0; i < oisCurveDatesAsSerialNumbers.size(); i++)
        oisCurveDates[i] = Date(oisCurveDatesAsSerialNumbers[i]);

    // cout << "Fixed Period = " << (fixedScheduleCount * fixedScheduleTimeUnit) << endl;
    // cout << "Float Period = " << (floatScheduleCount * floatScheduleTimeUnit) << endl;
    // for (int i = 0; i < oisCurveDates.size(); i++)
    // {
    //     Date date(oisCurveDates[i]);
    //     cout << date << endl;
    // }

    boost::shared_ptr<YieldTermStructure> forwardCurve(new InterpolatedDiscountCurve<LogLinear>(forwardCurveDates, forwardCurveDfs, toDayCounter(forwardCurveDayCountConvention)));
    boost::shared_ptr<YieldTermStructure> oisCurve(new InterpolatedDiscountCurve<LogLinear>(oisCurveDates, oisCurveDfs, toDayCounter(oisCurveDayCountConvention)));

    Handle<YieldTermStructure> forwardingTermStructure(forwardCurve);
    Handle<YieldTermStructure> discountingTermStructure(oisCurve);

    Date previousResetDate(previousResetDateAsSerialNumber);
    Date maturity(maturityAsSerialNumber);

    boost::shared_ptr<IborIndex> euribor(new Euribor(floatScheduleCount * floatScheduleTimeUnit, forwardingTermStructure));
    euribor->addFixing(euribor->fixingDate(previousResetDate), previousResetValue, true);

    VanillaSwap::Type swapType = VanillaSwap::Payer;
    Schedule fixedSchedule(previousResetDate, maturity, fixedScheduleCount * fixedScheduleTimeUnit,
                           TARGET(), ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Forward, false);

    Schedule floatSchedule(previousResetDate, maturity, floatScheduleCount * floatScheduleTimeUnit,
                           TARGET(), ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Forward, false);

    VanillaSwap swap(VanillaSwap::Payer, nominal, fixedSchedule, fixedRate, QuantLib::Thirty360(),
                     floatSchedule, euribor, spread, QuantLib::Actual360());

    boost::shared_ptr<PricingEngine> swapEngine(new DiscountingSwapEngine(discountingTermStructure));
    swap.setPricingEngine(swapEngine);
    double res = swap.NPV();

    return res;
}

double stdevDummy(vector<double> &xs)
{
    return 0;
}

double stdev(vector<double> &xs)
{
    int n = xs.size();
    double sx = 0;
    double sy = 0;
    double sx2 = 0;

    for (int i = 0; i < n; i++)
    {
        double x = xs[i];
        sx += x;
        sx2 += x * x;
    }
    return sqrt((sx2 - sx * sx / n) / (n - 1));
}

Handle<YieldTermStructure> *createLogLinearYieldTermStructure(vector<Date> &dates, vector<DiscountFactor> &discountFactor, DayCounter &dayCounter)
{
    boost::shared_ptr<YieldTermStructure> yts(new InterpolatedDiscountCurve<LogLinear>(dates, discountFactor, dayCounter));
    Handle<YieldTermStructure> *res = new Handle<YieldTermStructure>(yts);
    return res;
}

VanillaSwap *createVanillaSwap(VanillaSwap::Type type, Real nominal, const Schedule &fixedSchedule, Rate fixedRate, DayCounter &fixedDayCount, const Schedule &floatSchedule, IborIndex &iborIndex, Spread spread, DayCounter &floatingDayCount)
{
    boost::shared_ptr<IborIndex> iborIndexPtr(&iborIndex);
    return new VanillaSwap(type, nominal, fixedSchedule, fixedRate, fixedDayCount, floatSchedule, iborIndexPtr, spread, floatingDayCount);
}

EMSCRIPTEN_BINDINGS(my_module)
{
    enum_<DayCountConvention>("DayCountConvention")
        .value("Thirty360", DayCountConventionThirty360)
        .value("Actual360", DayCountConventionActual360)
        .value("Actual365", DayCountConventionActual365)
        .value("ActualActual", DayCountConventionActualActual)
        .value("Business252", DayCountConventionBusiness252);
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
    emscripten::constant("Sweden", swedenCalendar);
    emscripten::constant("NullCalendar", nullCalendar);
    class_<Date>("Date")
        .constructor<>()
        .constructor<int>()
        .constructor<int, Month, int>()
        // .smart_ptr_constructor("Date", &std::make_shared<C>)
        .function("serialNumber", &Date::serialNumber)
        .function("weekday", &Date::weekday)
        .function("dayOfMonth", &Date::dayOfMonth)
        .function("dayOfYear", &Date::dayOfYear)
        .function("month", &Date::month)
        .function("year", &Date::year)
        .function("toISOString", &toISOString)
        .class_function("fromISOString", &dateFromISOString, allow_raw_pointers())
        .class_function("isLeap", &Date::isLeap);
    class_<Schedule>("Schedule")
        .constructor<vector<Date>>()
        .constructor<Date, Date, Period, Calendar, BusinessDayConvention, BusinessDayConvention, DateGeneration::Rule, bool, Date, Date>()
        .function("size", &Schedule::size)
        .function("dates", &Schedule::dates);
    class_<Calendar>("Calendar")
        .function("name", &Calendar::name)
        .function("isBusinessDay", &Calendar::isBusinessDay);
    class_<Period>("Period")
        .constructor<int, TimeUnit>()
        .function("toString", &timeUnitToString);
    class_<DayCounter>("DayCounter")
        .function("name", &DayCounter::name);
    class_<Thirty360, base<DayCounter>>("Thirty360")
        .constructor<>()
        .constructor<Thirty360::Convention>()
        .constructor<Thirty360::Convention, bool>();
    class_<Actual360, base<DayCounter>>("Actual360")
        .constructor<>()
        .constructor<bool>();
    class_<ActualActual, base<DayCounter>>("ActualActual")
        .constructor<>()
        .constructor<ActualActual::Convention>()
        .constructor<ActualActual::Convention, Schedule>();
    class_<Index>("Index")
        .function("addFixing", &Index::addFixing);
    class_<InterestRateIndex, base<Index>>("InterestRateIndex")
        .function("fixingDate", &InterestRateIndex::fixingDate);
    class_<IborIndex, base<InterestRateIndex>>("IborIndex");
    class_<Euribor, base<IborIndex>>("Euribor")
        .constructor<Period, Handle<YieldTermStructure>>();
    class_<boost::optional<BusinessDayConvention>>("OptionalBusinessDayConvention");
    class_<VanillaSwap>("VanillaSwap")
        .constructor(&createVanillaSwap, allow_raw_pointers())
        // .constructor<VanillaSwap::Type, Rate, Schedule, Rate, DayCounter, Schedule, ext::shared_ptr<IborIndex>, Spread, DayCounter, boost::optional<BusinessDayConvention>>()
        .function("setPricingEngine", &swapSetPricingEngine)
        .function("NPV", &swapNpv);
    class_<Handle<YieldTermStructure>>("Handle<YieldTermStructure>");
    emscripten::function("swapNpvExample", &swapNpv);
    emscripten::function("stdev", &stdev);
    emscripten::function("stdevDummy", &stdevDummy);
    emscripten::function("createVanillaSwap", &createVanillaSwap, allow_raw_pointers());
    emscripten::function("mallinfo", &emval_test_mallinfo);
    emscripten::function("setValuationDate", &setValuationDate);
    emscripten::function("createLogLinearYieldTermStructure", &createLogLinearYieldTermStructure, allow_raw_pointers());
    register_vector<int>("vector<int>")
        .constructor<int>();
    register_vector<double>("vector<double>")
        // .smart_ptr<std::shared_ptr<vector<double>>>("std::shared_ptr<vector<double>>")
        // .smart_ptr_constructor("std::shared_ptr<vector<double>>", &std::make_shared<vector<double>, int>);
        .constructor<int>();
    register_vector<Date>("vector<Date>")
        .constructor<int>();
}

} // namespace
