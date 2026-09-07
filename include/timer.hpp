#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace timing {

using Clock = std::chrono::high_resolution_clock;
using Microseconds = long long;

// Holds an operation's result plus how long it took.
template <typename T>
struct Timed {
    T value;
    Microseconds microseconds;
};

template <>
struct Timed<void> {
    Microseconds microseconds;
};

// Run func(), return its result + elapsed microseconds.
// Does NOT print — caller prints after its own output so
// "[Label] elapsed:" stays last (same order as before).
template <typename Func>
auto measure(Func&& func) -> Timed<decltype(func())> {
    using Ret = decltype(func());
    auto start = Clock::now();
    if constexpr (std::is_void_v<Ret>) {
        std::forward<Func>(func)();
        auto end = Clock::now();
        return Timed<void>{
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()};
    } else {
        auto result = std::forward<Func>(func)();
        auto end = Clock::now();
        return Timed<Ret>{
            std::move(result),
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()};
    }
}

inline void printElapsed(const std::string& label, Microseconds us) {
    double ms = us / 1000.0;
    std::ostringstream oss;
    oss << "[" << label << "] elapsed: " << std::fixed << std::setprecision(3) << ms << " ms";
    std::cout << oss.str() << "\n";
}

// Convenience wrapper: measure + print immediately, forward the value.
// Use when there is no domain output that must appear before the timing line.
template <typename Func>
decltype(auto) timed(const std::string& label, Func&& func) {
    using Ret = decltype(func());
    if constexpr (std::is_void_v<Ret>) {
        auto t = measure(std::forward<Func>(func));
        printElapsed(label, t.microseconds);
    } else {
        auto t = measure(std::forward<Func>(func));
        printElapsed(label, t.microseconds);
        return std::move(t.value);
    }
}

}  // namespace timing
