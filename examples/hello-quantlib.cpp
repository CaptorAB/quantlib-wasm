// Build with:
// emcc -I${BOOST} -I${QUANTLIB} -o hello-quantlib.js hello-quantlib.cpp ${QUANTLIB}/ql/.libs/libQuantLib.a

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <ql/time/date.hpp>
#include <ql/qldefines.hpp>

using namespace std;
using namespace QuantLib;

int main(int, char *[])
{
    Date date(15, May, 2019);
    cout << "HELLO " << date << endl;

    return 0;
}
