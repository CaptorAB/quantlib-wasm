// Useful links and projects
//
// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
// https://fr.slideshare.net/chadaustin/connecting-c-and-javascript-on-the-web-with-embind
//
// https://github.com/jvail/lmfit.js
// https://github.com/jvail/glpk.js

#include <emscripten/bind.h>

using namespace emscripten;

// int sum(double *buf, int len)
double sum(std::vector<double> v)
{
    double total = 0;
    for (int i = 0; i < v.size(); i++)
    {
        total += v[i];
    }
    printf("Hello");
    return total;
}

/*
double sum2(double *v, int n)
{
    double total = 0;
    for (int i = 0; i < n; i++)
    {
        double x = v[i];
        total += x * x;
    }
    return total;
}
*/

std::vector<double> createDoubleVector(int size)
{
    std::vector<double> v(size, 1);
    return v;
}

std::vector<int> createIntVector(int size)
{
    std::vector<int> v(size, 1);
    return v;
}

EMSCRIPTEN_BINDINGS(my_module)
{
    function("sum", &sum);
    //function("sum2", &sum2, allow_raw_pointers());
    function("createDoubleVector", &createDoubleVector);
    register_vector<int>("vector<int>");
    register_vector<double>("vector<double>");
}