# Part C — Real-World C++ Demo

## Rules

- Solves a problem a **real user** would have (not "insert 5 numbers into a list").
- **Implement the structure yourself.** You may compare against the STL, but not substitute it.
- Must compile with `g++ -std=c++17 -Wall` and run live in class.
- Handles at least one dataset with **n ≥ 100 000** so the complexity is visible.
- Prints timing so the class sees the cost.
- Includes failure cases: **empty input, duplicate keys, out-of-range access**.

## Demo problem (proposed — get it approved before building)

_TODO: describe the real-world problem + why the structure is the right tool._

## Timing pattern (report best of 5 runs)

```cpp
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;

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
    return best; // report the best of 5 runs
}
```

Report for every n: input size n · operation count (if instrumented) · wall-clock time (best of 5) · peak memory (if relevant).

## Live demo script (45-min presentation, exact sequence)

1. **Data verification (1 min)** — show `head data.csv`; prove input is real, randomized, n ≥ 100 000.
2. **Edge case execution (2 min)** — empty file, duplicate keys, out-of-bounds access first; show graceful handling/rejection.
3. **Core algorithm & verification (3 min)** — run on n = 50 subset, print output to prove correctness visually.
4. **Stress test & profiling (6 min)** — full dataset (n ≥ 100 000), live terminal with the structured timing format.

## Test conditions (state these — a measurement without conditions is not a measurement)

- CPU & RAM:
- OS:
- Compiler & version:
- Optimisation flags:
- Dataset & how it was generated:
