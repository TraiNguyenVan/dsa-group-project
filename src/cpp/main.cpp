// Part C — C++ demo skeleton (PTIT INT1306_CLC group project)
// Topic: TBD. Replace this skeleton with your real-world demo.
// Rules: implement the structure yourself (STL comparison OK, substitution
// NOT). Must compile with: g++ -std=c++17 -Wall Must run on n >= 100 000, print
// timings, handle edge cases.

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>
using namespace std;
using namespace std::chrono;

// The required timing pattern — report the BEST of 5 runs.
template <typename F> double timeIt(F work, int repeats = 5) {
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

void fr(){
    uint32_t s = 0;
    for (int i = 0; i < 1e6; ++i)
    {
        s += 67;
    }
}

int main(int argc, char *argv[]) {
  double ms = timeIt(fr, 5);
  cout << ms << "\n";
  return 0;
}
