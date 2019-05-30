// Compile with:
// emcc -I${EMSCRIPTEN}/system/include -I${QUANTLIB} -I${BOOST} --bind -o quantlib.js quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a

// https://www.quantlib.org/slides/dima-ql-intro-1.pdf

#include <math.h>
#include <iostream>
#include <ql/quantlib.hpp>
#include <emscripten/bind.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace QuantLib;
using namespace emscripten;

namespace Captor
{

enum DayCountConvention
{
    Thirty360,
    Actual360,
    Actual365,
    ActualActual,
    Business252
};

const Calendar swedenCalendar = Sweden();

vector<double> createDoubleVector(int size)
{
    // vector<double> *v = new vector<double>();
    // v->reserve(size);
    // return *v;
    return vector<double>(size, 1);
}

vector<int> createIntVector(int size)
{
    // vector<int> *v = new vector<int>();
    // v->reserve(size);
    // return *v;
    return vector<int>(size, 1);
}

vector<int> scheduleDates(Schedule &schedule)
{
    vector<int> res = vector<int>(schedule.size(), 1);
    for (int i = 0; i < schedule.size(); i++)
        res[i] = schedule[i].serialNumber();
    return res;
}

Schedule generateSchedule(int effectiveDateAsSerialNumber, int terminationDateAsSerialNumber, int periodCount, TimeUnit periodTimeUnit, BusinessDayConvention convention,
                          BusinessDayConvention terminationDateConvention, DateGeneration::Rule rule, bool endOfMonth,
                          int firstDateAsSerialNumber = 0, int nextToLastDateAsSerialNumber = 0)
{
    Date effectiveDate = Date(effectiveDateAsSerialNumber);
    Date terminationDate = Date(terminationDateAsSerialNumber);
    Period tenor = Period(periodCount, periodTimeUnit);
    Calendar calendar = Sweden();
    Date firstDate = (firstDateAsSerialNumber == 0) ? Date() : Date(firstDateAsSerialNumber);
    Date nextToLastDate = (nextToLastDateAsSerialNumber == 0) ? Date() : Date(nextToLastDateAsSerialNumber);
    return Schedule(effectiveDate, terminationDate, tenor, calendar, convention, terminationDateConvention, rule, endOfMonth, firstDate, nextToLastDate);
}

Schedule createScheduleFromDates(vector<int> &datesAsSerialNumber)
{
    vector<Date> dates = vector<Date>(datesAsSerialNumber.size());
    for (int i = 0; i < datesAsSerialNumber.size(); i++)
    {
        dates[i] = Date(datesAsSerialNumber[i]);
    }
    return Schedule(dates);
    // Date begin (30 , September ,2009) , end (15 , Jun ,2012);
    // Calendar myCal = Japan ();
    // BusinessDayConvention bdC = BusinessDayConvention ( Following );
    // Period myTenor (6 , Months );
    // DateGeneration :: Rule myRule = DateGeneration :: Forward ;
    // Schedule mySched ( begin , end , myTenor , myCal , bdC , bdC , myRule , true );
    // return mySched;
}

DayCounter toDayCounter(DayCountConvention dc)
{
    if (dc == Thirty360)
    {
        return QuantLib::Thirty360();
    }
    if (dc == Actual360)
    {
        return QuantLib::Actual360();
    }
    if (dc == Actual365)
    {
        return QuantLib::Actual365Fixed();
    }
    if (dc == ActualActual)
    {
        return QuantLib::ActualActual();
    }
    if (dc == Business252)
    {
        return QuantLib::Business252();
    }
    //default
    return QuantLib::Actual360();
}

std::string timeUnitToString(Period &p)
{
    std::stringstream stream;
    stream << p;
    return stream.str();
}

double swapNpv(double nominal, double fixedRate, double spread, int valuationDateAsSerialNumber, int maturityAsSerialNumber,
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

    cout << "Fixed Period = " << (fixedScheduleCount * fixedScheduleTimeUnit) << endl;
    cout << "Float Period = " << (floatScheduleCount * floatScheduleTimeUnit) << endl;
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

// int main(int, char *[])
// {
//     // timer timer;

//     Date date(34000);

//     // string input = "12345";
//     // auto tmp = lexical_cast<int>(input);
//     cout << "HELLO " << date << endl;

//     return 0;
// }

inline double sqr(double x)
{
    return x * x;
}

double stdev(vector<double> xs)
{
    int n = xs.size();
    double sx = 0;
    double sy = 0;
    double sx2 = 0;

    for (int i = 0; i < n; i++)
    {
        double x = xs[i];
        sx += x;
        sx2 += sqr(x);
    }

    return sqrt((sx2 - sqr(sx) / n) / (n - 1));
}

// vector<Date> serialNumbersToDates(vector<int> &serialNumbers)
// {
//     vector<Date> *res = new vector<Date>();
//     res->reserve(serialNumbers.size());
//     for (int i = 0; i < serialNumbers.size(); i++)
//     {
//         Date d = new Date()
//         (*res)[i] = (Date)(new Date(serialNumbers[i]));
//     }
//     return *res;
// }

EMSCRIPTEN_BINDINGS(my_module)
{
    enum_<DayCountConvention>("DayCountConvention")
        .value("Thirty360", Thirty360)
        .value("Actual360", Actual360)
        .value("Actual365", Actual365);
    enum_<BusinessDayConvention>("BusinessDayConvention")
        .value("Following", Following)
        .value("ModifiedFollowing", ModifiedFollowing)
        .value("Preceding", Preceding)
        .value("ModifiedPreceding", ModifiedPreceding)
        .value("Unadjusted", Unadjusted)
        .value("HalfMonthModifiedFollowing", HalfMonthModifiedFollowing)
        .value("Nearest", Nearest);
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
    emscripten::constant("Sweden", swedenCalendar);
    class_<Schedule>("Schedule")
        .function("dates", &scheduleDates);
    class_<Calendar>("Calendar");
    class_<Period>("Period")
        .constructor<int, TimeUnit>()
        .function("toString", &timeUnitToString);
    emscripten::function("swapNpv", &swapNpv);
    emscripten::function("generateSchedule", &generateSchedule);
    emscripten::function("createScheduleFromDates", &createScheduleFromDates);
    emscripten::function("stdev", &stdev);
    emscripten::function("createIntVector", &createIntVector);
    emscripten::function("createDoubleVector", &createDoubleVector);
    register_vector<int>("vector<int>");
    register_vector<double>("vector<double>");
}

} // namespace Captor
