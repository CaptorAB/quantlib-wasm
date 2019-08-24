// Compile with:
// emcc -I${BOOST} -I${QUANTLIB} -s BINARYEN_TRAP_MODE=clamp -o billiontrader-bootstrapping.js billiontrader-bootstrapping.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a

#include <ql/quantlib.hpp>

#ifdef BOOST_MSVC
/* Uncomment the following lines to unmask floating-point
exceptions. Warning: unpredictable results can arise...

See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
Is there anyone with a definitive word about this?
*/
// #include <float.h>
// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

#include <ql/quantlib.hpp>
//#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib
{

Integer sessionId() { return 0; }

} // namespace QuantLib
#endif

using namespace QuantLib;

int main(int argc, char *argv[])
{
    Calendar calendar = JointCalendar(UnitedKingdom(UnitedKingdom::Exchange), UnitedStates(UnitedStates::Settlement), JoinHolidays);

    Date settlementDate(18, February, 2015);
    settlementDate = calendar.adjust(settlementDate);
    Integer fixingDays = 2;
    Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
    Settings::instance().evaluationDate() = todaysDate;
    DayCounter depositDayCounter = Actual360();

    Rate d1wQuote = 0.001375;
    Rate d1mQuote = 0.001717;
    Rate d2mQuote = 0.002112;
    Rate d3mQuote = 0.002581;
    boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
    boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
    boost::shared_ptr<Quote> d2mRate(new SimpleQuote(d2mQuote));
    boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
    boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(Handle<Quote>(d1wRate), 7 * Days, fixingDays, calendar, ModifiedFollowing, true, depositDayCounter));
    boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(Handle<Quote>(d1mRate), 4 * Weeks, fixingDays, calendar, ModifiedFollowing, true, depositDayCounter));
    boost::shared_ptr<RateHelper> d2m(new DepositRateHelper(Handle<Quote>(d2mRate), 2 * Months, fixingDays, calendar, ModifiedFollowing, true, depositDayCounter));
    boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(Handle<Quote>(d3mRate), 3 * Months, fixingDays, calendar, ModifiedFollowing, true, depositDayCounter));

    DayCounter FutDayCounter = Actual360();
    Real fut1Quote = 99.725; // 0.2750
    Real fut2Quote = 99.585; // 0.4150
    Real fut3Quote = 99.385; //0.6150
    Real fut4Quote = 99.16;  // 0.84
    Real fut5Quote = 98.93;  // 1.07
    Real fut6Quote = 98.715; // 1.285
    boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));
    boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));
    boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));
    boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));
    boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));
    boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));
    Integer futMonths = 3;

    Date imm = IMM::nextDate(settlementDate);
    boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(Handle<Quote>(fut1Price), imm, futMonths, calendar, ModifiedFollowing, true, depositDayCounter));
    imm = IMM::nextDate(imm + 1);
    boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(Handle<Quote>(fut2Price), imm, futMonths, calendar, ModifiedFollowing, true, depositDayCounter));
    imm = IMM::nextDate(imm + 1);
    boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(Handle<Quote>(fut3Price), imm, futMonths, calendar, ModifiedFollowing, true, depositDayCounter));
    imm = IMM::nextDate(imm + 1);
    boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(Handle<Quote>(fut4Price), imm, futMonths, calendar, ModifiedFollowing, true, depositDayCounter));

    imm = IMM::nextDate(imm + 1);
    boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(Handle<Quote>(fut5Price), imm, futMonths, calendar, ModifiedFollowing, true, depositDayCounter));
    imm = IMM::nextDate(imm + 1);
    boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(Handle<Quote>(fut6Price), imm, futMonths, calendar, ModifiedFollowing, true, depositDayCounter));

    Rate s2yQuote = 0.0089268;
    Rate s3yQuote = 0.0123343;
    Rate s4yQuote = 0.0147985;
    Rate s5yQuote = 0.0165843;
    Rate s6yQuote = 0.0179191;
    boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
    boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
    boost::shared_ptr<Quote> s4yRate(new SimpleQuote(s4yQuote));
    boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
    boost::shared_ptr<Quote> s6yRate(new SimpleQuote(s6yQuote));

    Frequency swFixedLegFrequency = Annual;
    BusinessDayConvention swFixedLegConvention = Unadjusted;
    DayCounter swFixedLegDayCounter = Actual360();
    boost::shared_ptr<IborIndex> swFloatingLegIndex(new USDLibor(Period(3, Months)));

    boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
        Handle<Quote>(s2yRate), 2 * Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
        Handle<Quote>(s3yRate), 3 * Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s4y(new SwapRateHelper(
        Handle<Quote>(s4yRate), 4 * Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
        Handle<Quote>(s5yRate), 5 * Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));
    boost::shared_ptr<RateHelper> s6y(new SwapRateHelper(
        Handle<Quote>(s6yRate), 6 * Years,
        calendar, swFixedLegFrequency,
        swFixedLegConvention, swFixedLegDayCounter,
        swFloatingLegIndex));

    typedef boost::shared_ptr<RateHelper> SharedPtrRateHelper;
    vector<SharedPtrRateHelper> depoFutSwapInstruments;
    depoFutSwapInstruments.push_back(d1w);
    depoFutSwapInstruments.push_back(d1m);
    depoFutSwapInstruments.push_back(d2m);
    depoFutSwapInstruments.push_back(d3m);
    depoFutSwapInstruments.push_back(fut1);
    depoFutSwapInstruments.push_back(fut2);
    depoFutSwapInstruments.push_back(fut3);
    depoFutSwapInstruments.push_back(fut4);
    depoFutSwapInstruments.push_back(fut5);
    depoFutSwapInstruments.push_back(fut6);
    depoFutSwapInstruments.push_back(s2y);
    depoFutSwapInstruments.push_back(s3y);
    depoFutSwapInstruments.push_back(s4y);
    depoFutSwapInstruments.push_back(s5y);
    depoFutSwapInstruments.push_back(s6y);

    DayCounter termStructureDayCounter = Actual360();
    boost::shared_ptr<YieldTermStructure> depoFutSwapTermStructure(new PiecewiseYieldCurve<Discount,
                                                                                           Linear>(settlementDate, depoFutSwapInstruments, termStructureDayCounter, 1.0e-15));
    Date matDate1(25, February, 2015);
    Date matDate2(18, March, 2015);
    Date matDate3(20, April, 2015);
    Date matDate4(18, May, 2015);

    Date matDate5(17, June, 2015);
    Date matDate6(16, September, 2015);
    Date matDate7(16, December, 2015);
    Date matDate8(16, March, 2016);
    Date matDate9(15, June, 2016);

    Date matDate10(21, September, 2016);
    Date matDate11(21, February, 2017);
    Date matDate12(20, February, 2018);
    Date matDate13(19, February, 2019);
    Date matDate14(18, February, 2020);

    std::cout << "0.1375: " << depoFutSwapTermStructure->zeroRate(matDate1, depositDayCounter, Simple) << std::endl;
    std::cout << "0.1717: " << depoFutSwapTermStructure->zeroRate(matDate2, depositDayCounter, Simple) << std::endl;
    std::cout << "0.2112: " << depoFutSwapTermStructure->zeroRate(matDate3, depositDayCounter, Simple) << std::endl;
    std::cout << "0.2581: " << depoFutSwapTermStructure->zeroRate(matDate4, depositDayCounter, Simple) << std::endl;

    std::cout << "0.25093: " << depoFutSwapTermStructure->zeroRate(matDate5, FutDayCounter, Simple) << std::endl;
    std::cout << "0.32228: " << depoFutSwapTermStructure->zeroRate(matDate6, FutDayCounter, Simple) << std::endl;
    std::cout << "0.41111: " << depoFutSwapTermStructure->zeroRate(matDate7, FutDayCounter, Simple) << std::endl;
    std::cout << "0.51112: " << depoFutSwapTermStructure->zeroRate(matDate8, FutDayCounter, Simple) << std::endl;
    std::cout << "0.61698: " << depoFutSwapTermStructure->zeroRate(matDate9, FutDayCounter, Simple) << std::endl;

    std::cout << "0.73036: " << depoFutSwapTermStructure->zeroRate(matDate10, FutDayCounter, Compounded, Annual) << std::endl;
    std::cout << "0.89446: " << depoFutSwapTermStructure->zeroRate(matDate11, FutDayCounter, Compounded, Annual) << std::endl;
    std::cout << "1.23937: " << depoFutSwapTermStructure->zeroRate(matDate12, FutDayCounter, Compounded, Annual) << std::endl;
    std::cout << "1.49085: " << depoFutSwapTermStructure->zeroRate(matDate13, FutDayCounter, Compounded, Annual) << std::endl;
    std::cout << "1.67450: " << depoFutSwapTermStructure->zeroRate(matDate14, FutDayCounter, Compounded, Annual) << std::endl;
    std::cout << "Discount Rate : " << depoFutSwapTermStructure->discount(matDate14) << std::endl;
    std::cout << "Forward Rate : " << depoFutSwapTermStructure->forwardRate(matDate13, matDate14, FutDayCounter, Simple) << std::endl;

    /*
    std::ofstream myfile;
    myfile.open("example.txt");
    myfile << " 24 Feb: " << depoFutSwapTermStructure->zeroRate(matDate1, termStructureDayCounter, Simple) << std::endl;
    myfile << " 17 March: " << depoFutSwapTermStructure->zeroRate(matDate2, termStructureDayCounter, Simple) << std::endl;
    myfile << " 17 April: " << depoFutSwapTermStructure->zeroRate(matDate3, termStructureDayCounter, Simple) << std::endl;
    myfile << " 18 May: " << depoFutSwapTermStructure->zeroRate(matDate4, termStructureDayCounter, Simple) << std::endl;
    myfile << " 17 june: " << depoFutSwapTermStructure->zeroRate(matDate5, termStructureDayCounter, Simple) << std::endl;
    myfile << " 16 September: " << depoFutSwapTermStructure->zeroRate(matDate6, termStructureDayCounter, Simple) << std::endl;
    myfile.close();
    
    std::cout << " Zero Rate 1 Week : " << depoFutSwapTermStructure->zeroRate(settlementDate + 1 * Weeks, termStructureDayCounter, Simple) << std::endl;
    boost::shared_ptr<SimpleQuote> fiveYearsRate =boost::dynamic_pointer_cast<SimpleQuote>(d1wRate);
    fiveYearsRate->setValue(0.0400);	
    std::cout << " Zero Rate 1 Week : " << depoFutSwapTermStructure->zeroRate(settlementDate + 1 * Weeks, termStructureDayCounter, Simple) << std::endl;
    */

    return 0;
}