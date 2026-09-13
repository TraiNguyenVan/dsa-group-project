# 08 — Interpreting Results

You have CSVs and PNGs. This page tells you how to read them without fooling yourself.

## Runtime: `benchmark/results.csv`

Each row is **one run** (not an average). A cell `(lang, case, algo, n)` has 5 rows (`run=1..5`).

```csv
language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone
cpp,data/contacts_100k.csv,100000,last,linear,1,0.365,2026-09-11T16:04:40Z,"g++ 16.2.1",99999,0912345678
```

### Best-of-5

The harness reports `best = min(5 runs)` per cell — same as the live CLI. To reproduce:

```python
import csv
from collections import defaultdict

best = {}
with open("benchmark/results.csv") as f:
    for r in csv.DictReader(f):
        key = (r["language"], r["case"], r["algo"], r["n"])
        v = float(r["ms"])
        if key not in best or v < best[key]:
            best[key] = v

print(best[("cpp", "last", "linear", "100000")])  # e.g. 0.365
```

### Mean / stdev / CV

The CSV keeps all 5 runs so you can compute statistics offline:

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
    print(f"{key}: mean={mean:.4f} stdev={stdev:.4f} CV={cv:.2f} best={min(vals):.4f}")
```

- **CV (coefficient of variation) = stdev / mean** — how noisy the cell is. CV < 0.1 is stable; CV > 0.5 is noisy (often tiny `n` or sub-µs hash/binary where timer resolution dominates).
- At `n=1M`, `last-linear` is the most stable (large work, timer noise negligible). At `n=50`, hash/binary CV is high (work is ~50 ns, timer jitter dominates).

### What to compare

| Comparison | Valid? | Why |
|------------|--------|-----|
| Same `case` + `algo` across langs (e.g. `last-linear` cpp vs python) | ✅ | Same position, same work — directly comparable |
| `first` vs `last` within one lang for `linear` | ✅ | Shows `O(n)` growth — the DSA point |
| `first` vs `last` within one lang for `hash` | ✅ | Should be flat — proves `O(1)` |
| `hash` vs `linear` absolute ms | ✅ with caveat | Different algos, but same dataset — the gap is the point; cite mechanism (boxed objects, interpreter, etc.) |
| Single run (`run=1`) across langs | ❌ | Run 1 includes cold start (JIT, cache) — use `best` or `mean(runs 2–5)` for Java/JS |

### JIT warmup caveat

Java (C1/C2) and JS (V8 TurboFan) start interpreted, then compile hot code after ~10k iterations. One 100k linear scan triggers compilation mid-run:

```
Java middle-linear 100k: run1 39ms → run5 17ms  (JIT kicked in)
JS   first-linear 1M:    run1 101× best          (cold V8)
C++/Go:                  run1 ~1.1× best         (AOT, already compiled)
```

The current harness keeps all 5 runs (no discarded warmup) and reports `best-of-5`. Until the planned 3 discarded warmup runs are added, compare `min(best)` or `mean(runs 2–5)` for Java/JS and state which you used.

### Reading the scaling chart (`plot-runtime-vs-n.png`)

Log-log axes: x = `n` (50 → 1M), y = best ms, one line per language, 3 panels (one per algo), `case=last` by default.

- **Linear panel:** lines should be diagonal (slope ≈ 1 on log-log → `O(n)`). If a line is flat, something is wrong (maybe measuring hash by mistake).
- **Hash / binary panels:** lines should be flat (slope ≈ 0 → `O(1)` / `O(log n)`). `O(log n)` looks almost flat on log-log because `log(1M) ≈ 20` vs `log(50) ≈ 6` — only 3× growth.
- **Gap between langs:** vertical distance at a given `n` is the language/runtime cost. C++ and Go should be lowest (compiled), Python highest for linear (interpreter + boxed objects).

## Memory: `benchmark/mem/results.csv`

```csv
language,dataset,n,metric,mb,tool
cpp,data/contacts_100k.csv,100000,profiler_heap,18.505,valgrind/massif
cpp,data/contacts_100k.csv,100000,peak_rss,22.926,kernel wait4/ru_maxrss
```

Two metrics per `(lang, n)`:

| Metric | What it is | Use it to answer |
|--------|------------|-----------------|
| `profiler_heap` | Live heap (contacts + hash + sorted index) via language profiler | How much does the data cost? |
| `peak_rss` | Whole process RAM via kernel `ru_maxrss` | How much does the whole process cost? |

### Reading the memory chart (`plot-memory.png`)

Two panels, log y, x = `n`, 5 grouped bars per `n`:

- **Left — profiler heap:** data cost. C++ is the peak-allocated floor (Massif peak, transients included). Python ~2× (every object is a `PyObject` + header). Go `HeapAlloc` is live after GC (GC on, same rule as JS/Java — transients freed, always `< RSS`). JS `heapUsed` is live V8 heap after GC. Java is polled `used heap`. Massif-peak vs live-after-GC definitions differ: rank shapes, verify against RSS.
- **Right — peak RSS:** whole-process cost. At `n=50` baselines dominate (JS ~53 MB, Java ~51 MB, C++/Go ~12.6 MB). At `n=1M` heap dominates. You need both panels for an honest claim.

### Common pitfalls

| Pitfall | Why it's wrong | What to do |
|---------|---------------|------------|
| Comparing `profiler_heap` across langs as "which lang uses less RAM" | Each profiler measures a different thing (Massif peak vs tracemalloc peak vs HeapAlloc live vs heapUsed live) | Compare trends within each lang, and use `peak_rss` for cross-lang whole-process comparison |
| Reporting only `n=100k` | One point doesn't show growth | Use `run-benchmark-sizes` (6 sizes) and the log-log chart |
| Reporting `mean` without `stdev` | Hides noise; sub-µs cells have high CV | Report `mean ± stdev (best)` and mention CV |
| Ignoring `target_index` | Cross-language misses may be mistaken for a different phone | Check `target_index` in CSV; for strict apples-to-apples use `first`/`middle`/`last` (same index every lang) |
| Forgetting test conditions | `0.36 ms` means nothing without CPU/compiler/flags/dataset | Always cite the machine spec (see `README.md` → Machine & toolchain) |

---

Next: [09 — Architecture](09-architecture.md) — how the pieces fit together.
