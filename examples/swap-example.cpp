#include <iostream>
#include <ql/quantlib.hpp>

using namespace std;
using namespace QuantLib;
using namespace boost;

int main()
{
    vector<Date> dates; 
    vector<DiscountFactor> discountFactor; 

    Date valuationDate(31,December,2012);
    Settings::instance().evaluationDate()=valuationDate;

    dates.push_back(valuationDate); discountFactor.push_back(1.0); 
    dates.push_back(Date(31,December, 2013));  discountFactor.push_back(0.99); 
    dates.push_back(Date(31,December, 2024));  discountFactor.push_back(0.80); 
    boost::shared_ptr<YieldTermStructure> forwardCurve(new InterpolatedDiscountCurve<LogLinear>(dates,discountFactor,Actual360())); 

    discountFactor.pop_back();discountFactor.pop_back();

    discountFactor.push_back(0.999);
    discountFactor.push_back(0.89);

    boost::shared_ptr<YieldTermStructure> oisCurve(new InterpolatedDiscountCurve<LogLinear>(dates,discountFactor,Actual360())); 

    Handle<YieldTermStructure> discountingTermStructure(oisCurve);
    Handle<YieldTermStructure> forwardingTermStructure(forwardCurve);

    Real nominal = 1000000.0;
    Date previousResetDate(20,November,2012);
    Date maturity(20,November,2022);
    double spread = 0.02;
    double fixedRate=0.04;

    boost::shared_ptr<IborIndex> euribor(new Euribor(3*Months,forwardingTermStructure));
    euribor->addFixing(euribor->fixingDate(previousResetDate),0.01,true);
            
    VanillaSwap::Type swapType = VanillaSwap::Payer;

    Schedule fixedSchedule(previousResetDate, maturity,1*Years,
                                TARGET(), ModifiedFollowing,ModifiedFollowing,
                                DateGeneration::Forward, false);

    Schedule floatSchedule(previousResetDate,maturity,3*Months,
                                TARGET(),ModifiedFollowing ,ModifiedFollowing,
                                DateGeneration::Forward, false);
            
    VanillaSwap swap(VanillaSwap::Payer, nominal,fixedSchedule, fixedRate, Thirty360(),
                floatSchedule, euribor, spread,Actual360());
        
    boost::shared_ptr<PricingEngine> swapEngine(new DiscountingSwapEngine(discountingTermStructure));

    swap.setPricingEngine(swapEngine);
        
    double res=swap.NPV();
    cout << res << endl;
}