// Part C — C++ demo skeleton (PTIT INT1306_CLC group project)
// Topic: TBD. Replace this skeleton with your real-world demo.
// Rules: implement the structure yourself (STL comparison OK, substitution NOT).
// Must compile with: g++ -std=c++17 -Wall
// Must run on n >= 100 000, print timings, handle edge cases.

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>
using namespace std;
using namespace std::chrono;

// The required timing pattern — report the BEST of 5 runs.
template <typename F>
double timeIt(F work, int repeats = 5) {
    double best = 1e18;
    for (int r = 0; r < repeats; ++r) {
        auto t0 = high_resolution_clock::now();
        work();
        auto t1 = high_resolution_clock::now();
        double ms = duration<double, milli>(t1 - t0).count();
        best = min(best, ms);
    }
    return best;
}

// TODO: your structure implementation goes here.

int main(int argc, char* argv[]) {
    // TODO: CLI parsing for the live demo script:
    //   default  -> run on the full dataset (n >= 100 000), print timings
    //   --small  -> run on n = 50 subset, print output (correctness proof)
    //   --empty  -> empty input edge case
    //   --dups   -> duplicate keys edge case
    //   --oob    -> out-of-range access edge case

    vector<int> data(100000);
    for (auto& x : data) x = rand();

    double ms = timeIt([&] {
        sort(data.begin(), data.end());
    });
    cout << "n=" << data.size() << " best-of-5: " << ms << " ms\n";

    // TODO: report input size n, operation count (if instrumented),
    //       wall-clock time (best of 5), peak memory (if relevant).
    return 0;
}
