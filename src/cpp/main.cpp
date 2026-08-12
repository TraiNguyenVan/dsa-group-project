// Part C — C++ demo skeleton (PTIT INT1306 group project)
//
// Topic: TBD — replace this skeleton with your real-world demo.
// Rules:
//   - Implement the data structure yourself; STL is fine for *comparison*
//     only, not as a substitute for your implementation.
//   - Must compile with: g++ -std=c++17 -Wall
//   - Must run on n >= 100 000, print timings, and handle edge cases.

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

using clock_type = std::chrono::high_resolution_clock;

// Run `work` several times and return the fastest run in milliseconds.
// Best-of-5 reduces noise from cold caches / scheduling.
template <typename Work>
double best_of_5_ms(Work work, int repeats = 5) {
    double best = 1e18;
    for (int run = 0; run < repeats; ++run) {
        auto start = clock_type::now();
        work();
        auto end  = clock_type::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        best = std::min(best, ms);
    }
    return best;
}

int main(int argc, char* argv[]) {
    // ---- CLI for the live demo script -------------------------------------
    //   (no flag) -> full dataset, n >= 100 000, print timings
    //   --small   -> n = 50 subset, print output (correctness proof)
    //   --empty   -> empty input edge case
    //   --dups    -> duplicate keys edge case
    //   --oob     -> out-of-range access edge case
    // TODO: parse argv[] and run the matching mode.

    // ---- Build the input (placeholder: random ints) ----------------------
    constexpr std::size_t kInputSize = 100'000;
    std::vector<int> data(kInputSize);
    for (auto& value : data) {
        value = std::rand();
    }

    // ---- Time the reference operation (std::sort) ------------------------
    // Replace this lambda body with a call to YOUR structure's operation.
    double ms = best_of_5_ms([&] {
        std::sort(data.begin(), data.end());
    });

    // ---- Report -----------------------------------------------------------
    // TODO: also report operation count and peak memory if instrumented.
    std::cout << "n=" << data.size()
              << " best-of-5: " << ms << " ms\n";

    (void)argc;
    (void)argv;
    return 0;
}