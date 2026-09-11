#include <chrono>
#include <iostream>

using namespace std;
using namespace std::chrono;

template <typename F>
void printTaskDuration(F work) {
    double ms = 1e18;
    auto t0 = high_resolution_clock::now();
    work();
    auto t1 = high_resolution_clock::now();
    ms = duration<double, milli>(t1 - t0).count();
    cout << "\nTook: " << ms << "ms.\n";
}