# 03 — Timing Harness

The smallest building block of the benchmark: **how a single function call is timed**.

## The primitive: `timeIt`

Canonical version — `include/timer.hpp` (28 lines, the source of truth):

```cpp
#include <chrono>
using namespace std::chrono;

template <typename F>
double timeIt(F work) {
    auto t0 = high_resolution_clock::now();
    work();                                   // ← the thing you measure
    auto t1 = high_resolution_clock::now();
    return duration<double, milli>(t1 - t0).count(); // ms
}
```

That's it: record time, run `work`, record time again, subtract. Returns **milliseconds** as `double`.

### Per-language ports

Same logic, different clocks — all faithful ports of `timer.hpp`:

| Lang | File | Clock | Returns |
|------|------|-------|---------|
| C++ | `include/timer.hpp` | `high_resolution_clock::now()` | `double` ms |
| Python | `python/phonebook/timer.py` | `time.perf_counter()` | `float` ms |
| Go | `go/phonebook/timer.go` | `time.Now()` | `float64` ms |
| JS | `javascript/phonebook/src/timer.js` | `perf_hooks.performance.now()` | `number` ms |
| Java | `java/phonebook/src/com/phonebook/Timer.java` | `System.nanoTime()` | `double` ms |

```python
# python/phonebook/timer.py
def time_it(work):
    t0 = time.perf_counter()
    work()
    return (time.perf_counter() - t0) * 1000.0
```

```go
// go/phonebook/timer.go
func TimeIt(work func()) float64 {
    t0 := time.Now()
    work()
    return float64(time.Since(t0).Nanoseconds()) / 1e6
}
```

```js
// javascript/phonebook/src/timer.js
function timeIt(work) {
    const t0 = performance.now();
    work();
    return performance.now() - t0;
}
```

```java
// java/phonebook/src/com/phonebook/Timer.java
public static double timeIt(Runnable work) {
    long t0 = System.nanoTime();
    work.run();
    return (System.nanoTime() - t0) / 1e6;
}
```

> All five write `ms` to CSV in their own float format (`setprecision(17)`, `repr`, `%g`, `Double.toString`). `benchmark/plot.py:parse_ms()` handles all of them with a defensive `float()`.

## `benchmark` — best-of-5

One run is noisy (OS jitter, GC, cache). The harness runs 5 times and keeps the **minimum**:

```cpp
template <typename F>
double benchmark(F work, int repeats = 5) {
    double best = numeric_limits<double>::max();
    for (int r = 0; r < repeats; ++r)
        best = min(best, timeIt(work));
    return best; // the best of 5
}
```

Why **minimum** and not mean?

- The minimum discards outliers (a context switch that added 2 ms).
- The mean would include them.
- The CSV keeps **all 5 raw runs** so you can compute mean/stdev/CV offline if you want — the live CLI just prints the best.

```
Run 1: 0.42 ms  ← cold cache, slow
Run 2: 0.36 ms
Run 3: 0.35 ms  ← best
Run 4: 0.37 ms
Run 5: 0.36 ms
→ benchmark() returns 0.35 ms
```

### Where `benchmark` is used

- **Interactive mode** (option `0`): `runSearchBenchmark()` calls `benchmark()` for each of the 12 cells (4 cases × 3 algos) and prints `Linear best of 5: 0.365ms`.
- **Batch mode** (`--benchmark-csv`): `runSearchBenchmarkBatch()` calls `timeIt()` 5 times per cell and writes **every run** as a separate CSV row (60 rows per language per dataset). `plot.py` then takes `min()` per cell — same result, but auditable.

### `printTaskDuration` — the menu helper

```cpp
template <typename F>
void printTaskDuration(F work) {
    cout << "\nTook: " << timeIt(work) << "ms.\n";
}
```

Used by menu options `1` (load), `2` (save), `3` (insert) — not part of the benchmark, just user-facing feedback. Ported to every language.

## Statistical fairness

Every cell in `benchmark/results.csv` has 5 raw `ms` values. To compute mean/stdev/CV:

```python
import csv, statistics
from collections import defaultdict

rows = defaultdict(list)
with open("benchmark/results.csv") as f:
    for r in csv.DictReader(f):
        rows[(r["language"], r["case"], r["algo"], r["n"])].append(float(r["ms"]))

for key, vals in rows.items():
    mean = statistics.mean(vals)
    stdev = statistics.stdev(vals) if len(vals) > 1 else 0
    cv = stdev / mean if mean else 0
    print(key, f"mean={mean:.4f} stdev={stdev:.4f} CV={cv:.2f} best={min(vals):.4f}")
```

**JIT caveat:** Java/JS run 1 is slow (interpreted). Until the planned 3 discarded warmup runs are added, compare `min(best)` or `mean(runs 2–5)` for those languages. See [08 — Interpreting Results](../benchmark/08-interpreting-results.md).

---

Next: [04 — Benchmark Modes](../benchmark/04-benchmark-modes.md) — interactive vs batch.
