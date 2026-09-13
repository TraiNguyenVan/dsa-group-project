# 01 — Concepts (from zero)

You don't need prior profiling experience. This page defines every term the harness uses, in the order you'll encounter it.

## 1. What is a benchmark?

A **benchmark** is a controlled experiment: run the same work many times, measure how long it takes and how much memory it uses, and report the numbers with the conditions (machine, compiler, dataset). Without conditions a number is meaningless — `0.36 ms` on one laptop is not comparable to `0.36 ms` on another.

This repo benchmarks **phonebook lookup**: given a phone number, find the contact. Three algorithms are compared on the same data:

| Algo | Idea | Complexity |
|------|------|------------|
| `linear` | scan from index 0 until `phone == target` | O(n) — position matters |
| `hash` | `hash(phone) → bucket → walk chain` | O(1) expected — position-independent |
| `binary` | binary search on a sorted phone index | O(log n) — position-independent |

## 2. Wall-clock time

**Wall-clock time** is what a wall clock shows: real elapsed time from start to finish, including everything (CPU work, waiting, OS scheduling). The harness measures wall-clock, not CPU-only time, because that's what a user feels.

Per-language clocks (all measure wall-clock):

| Language | Call | Resolution |
|----------|------|------------|
| C++ | `std::chrono::high_resolution_clock::now()` | nanoseconds |
| Python | `time.perf_counter()` | nanoseconds |
| Go | `time.Now()` | nanoseconds |
| JS | `perf_hooks.performance.now()` | microseconds |
| Java | `System.nanoTime()` | nanoseconds |

All are converted to **milliseconds (ms)** before writing to CSV.

> **Why not `clock()` / CPU time?** CPU time excludes time the OS spent scheduling other processes. Wall-clock is the honest user-visible cost and is what the guide's `timeIt` template uses.

## 3. `timeIt` vs `benchmark` (best-of-5)

```cpp
// include/timer.hpp — the canonical version
template <typename F> double timeIt(F work) {
    auto t0 = high_resolution_clock::now();
    work();
    auto t1 = high_resolution_clock::now();
    return duration<double, milli>(t1 - t0).count();
}
template <typename F> double benchmark(F work, int repeats = 5) {
    double best = numeric_limits<double>::max();
    for (int r = 0; r < repeats; ++r) best = min(best, timeIt(work));
    return best; // report the best of 5
}
```

- `timeIt` — one timed run.
- `benchmark` — run `timeIt` 5 times, return the **minimum**. The minimum is less noisy than the mean: it discards outliers caused by OS jitter, GC pauses, or a context switch. The CSV keeps all 5 raw runs so you can compute mean/stdev offline; the live CLI (option `0`) prints only the best.

Every language ports this exactly (`timer.py:time_it`, `timer.go:TimeIt`, `timer.js:timeIt`, `Timer.java:timeIt`).

## 4. Memory — the three layers

Think of memory as three nested boxes:

```
┌─────────────────────────────────────────────┐
│  Virtual address space (what the process     │
│  thinks it has — may be larger than RAM)    │
│  ┌───────────────────────────────────────┐  │
│  │  RSS — Resident Set Size              │  │
│  │  pages actually in RAM right now      │  │
│  │  ┌───────────────────────────────┐    │  │
│  │  │  Heap — live objects          │    │  │
│  │  │  (contacts, hash buckets,     │    │  │
│  │  │   sorted index)               │    │  │
│  │  └───────────────────────────────┘    │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

- **Heap** — memory your program explicitly allocated for data (the phonebook). Measured by language-specific profilers (Massif, tracemalloc, pprof, V8, JFR). This is the *data cost*.
- **RSS (Resident Set Size)** — total RAM the OS has actually mapped for the process: heap + stacks + code + runtime overhead (interpreter, JVM, V8). Measured by the **kernel** via `wait4().ru_maxrss`. This is the *whole-process cost*.
- **Virtual size** — address space reserved (may be swapped out, not in RAM). The harness does **not** report this — it's misleading.

### Why two panels?

At `n=50` the heap is tiny (0.1 MB in C++) but RSS is 12–55 MB — the runtime baseline dominates. At `n=1M` the heap dominates. You need both to answer "how much does the data cost?" vs "how much does the whole process cost?" The report's `plot-memory.png` shows both panels for this reason.

### Kernel RSS — `ru_maxrss`

On Linux the kernel tracks each process's peak RSS in `struct rusage.ru_maxrss` (kilobytes). The harness forks a child, runs the batch program, and calls `os.wait4(pid, 0)` in the parent — the kernel returns the child's peak RSS without any instrumentation inside the child. This is the **uniform yardstick**: same kernel measurement for all five languages, no profiler bias.

```python
# benchmark/mem_profile.py — measure_rss()
pid = os.fork()
if pid == 0:
    os.execvp(cmd[0], cmd)   # child: run the batch program
_, status, usage = os.wait4(pid, 0)
rss_kib = usage.ru_maxrss    # KiB on Linux
```

Evidence is written to `benchmark/mem/rss-<lang>-<n>.txt`.

> **Common confusion:** `ru_maxrss` is KiB on Linux but bytes on macOS. The harness assumes Linux (the measurement machine is Arch Linux).

## 5. Profiler heap — per-language tools

Each language has its own way to measure "how much heap did the phonebook actually use?":

| Lang | Tool | What it measures | Evidence file |
|------|------|------------------|---------------|
| C++ | **Valgrind/Massif** | `mem_heap_B + mem_heap_extra_B + mem_stacks_B` peak over snapshots | `massif-<n>.out` + `massif-<n>-msprint.txt` |
| Python | **tracemalloc** | peak of Python-side allocations (`get_traced_memory`) | `tracemalloc-<n>.txt` |
| Go | **pprof** (`HeapAlloc` live after GC, GC on) | live heap after load | `go-<n>.pprof` + `go-<n>-pprof-top.txt` |
| JS | **V8 `heapUsed`** after forced GC + `--heap-prof` | live V8 heap | `js-<n>.heapprofile` + `js-<n>.txt` |
| Java | **JFR + polled `used heap`** | peak used heap sampled during load | `java-<n>.jfr` + `java-<n>-jfr-summary.txt` |

All are invoked headlessly by `benchmark/mem_profile.py` on the **same load path** (`CSV → contacts + hash + sorted index`) so numbers are comparable. See [06 — Memory Profiling](../benchmark/06-memory-profiling.md) for details.

## 6. JIT, GC, and warmup

- **JIT (Just-In-Time compilation)** — Java (C1/C2) and JS (V8 TurboFan) start interpreted, then compile hot code after ~10k iterations. Run 1 is slow (interpreted + class loading), runs 2–5 are fast (compiled). C++ and Go are AOT-compiled (`-O2` / `go build`) so run 1 is already fast.
- **GC (Garbage Collection)** — Python, Go, Java, JS automatically reclaim unreachable memory. GC pauses add jitter; `best-of-5` mitigates it. The JS profiler forces `global.gc()` before measuring so `heapUsed` reflects live data, not garbage.
- **Warmup** — the guide says warmup runs must not be counted for JIT languages. The current harness keeps all 5 runs (no discarded warmup) and reports `best-of-5`; the report notes you should compare `min(best)` or `mean(runs 2–5)` for Java/JS until the planned 3 discarded warmup runs are added.

## 7. Complexity vs measured time

Complexity (`O(n)`, `O(1)`, `O(log n)`) predicts *growth* with `n`, not absolute milliseconds. The harness verifies this by running `n = 50, 10k, 100k, 200k, 500k, 1M` and plotting `ms vs n` on log-log axes — linear should be a diagonal, hash/binary should be flat. See [08 — Interpreting Results](../benchmark/08-interpreting-results.md).

---

Next: [02 — Datasets](02-datasets.md) — how the input data is built.
