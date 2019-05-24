#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

int main() {
    string input = "12345";
	auto tmp = boost::lexical_cast<int>(input);
    std::cout << "HELLO " << tmp << std::endl;
    return 0;
}
