#include <chrono>
#include <iostream>
#include <limits>

using namespace std;
using namespace std::chrono;

template <typename F>
double timeIt(F work) {
    auto t0 = high_resolution_clock::now();
    work();
    auto t1 = high_resolution_clock::now();
    double ms = duration<double, milli>(t1 - t0).count();
    return ms;
}

template <typename F>
double benchmark(F work, int repeats = 5) {
    if (repeats <= 0) {
        return 0.0;
    }
    double best = numeric_limits<double>::max();
    for (int r = 0; r < repeats; ++r) {
        double t = timeIt(work);
        if (best > t) best = t;
    }
    return best;
    // report the best of 5 runs
}

template <typename F>
void printTaskDuration(F work) {
    cout << "\nTook: " << timeIt(work) << "ms.\n";
}
