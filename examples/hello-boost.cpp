// Compile with:
// emcc -I${BOOST} -o hello-boost.js hello-boost.cpp

#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>

using namespace std;
using namespace boost;

int main(int, char *[])
{
    timer timer;

    string input = "12345";
    auto tmp = lexical_cast<int>(input);
    cout << "HELLO " << tmp << endl;

    return 0;
}
