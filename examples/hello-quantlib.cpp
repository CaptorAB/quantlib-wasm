#include <iostream>
#include <boost/lexical_cast.hpp>
// #include <boost/timer.hpp>
#include <ql/time/date.hpp>
#include <ql/qldefines.hpp>

using namespace std;
using namespace QuantLib;
// using namespace boost;

int main(int, char* []) {
    // timer timer;

    Date date(15, February, 2002);

    // string input = "12345";
    // auto tmp = lexical_cast<int>(input);
    cout << "HELLO " << date << endl;

    return 0;
}
