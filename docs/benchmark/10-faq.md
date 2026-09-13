# 10 — FAQ & Recipes

Quick answers, troubleshooting, and copy-paste recipes.

## Troubleshooting

### Build & run

| Symptom | Cause | Fix |
|---------|-------|-----|
| `g++: command not found` | No compiler | `sudo pacman -S gcc` / `sudo apt install g++` |
| `make: command not found` | No make | `sudo pacman -S make` |
| `cmake: command not found` | No CMake | `sudo pacman -S cmake` (or just use `make`) |
| `Cannot open file: data/contacts_100k.csv` | Datasets not generated | `python3 data/generate.py --seed 42` |
| `Invalid input.` loop | Typed a letter at the menu | Type a number `0`–`9` |
| `Goodbye` immediately | EOF (piped empty input) | Run without piping, or `printf '0\n9\n' \| ./build/cpp/demo …` |

### Benchmark

| Symptom | Cause | Fix |
|---------|-------|-----|
| `plot skipped: pip install matplotlib` | No matplotlib | `pip install matplotlib` — CSVs are still produced |
| `benchmark/results.csv` has 0 rows | Wrong `n` filter | `plot.py` defaults to `n=100k`; use `--n 50` or check `wc -l benchmark/results.csv` |
| `warn: skipped N row(s)` | Bad `ms` values or unknown lang/case/algo | Check CSV header; `plot.py:parse_ms` skips non-finite/negative |
| Java/JS run 1 is 10–100× slower | JIT cold start (C1/C2, TurboFan) | Compare `best` or `mean(runs 2–5)`, not `run=1` |
| `run-benchmark-sizes` is slow | 6 sizes × 5 langs × 60 rows | Normal (~5 min); start with `make run-benchmark` (100k only, ~30s) |

### Memory profiling

| Symptom | Cause | Fix |
|---------|-------|-----|
| `valgrind: command not found` | Not installed | `sudo pacman -S valgrind` / `sudo apt install valgrind` |
| `massif failed` warning | C++ demo not built | `make` first |
| `go memprofile failed` | `go` not in PATH | `go version` to check |
| `jfr: command not found` | JDK < 17 or JRE only | Install JDK 17+ (`javac -version`) |
| `run-memory` takes 10+ min | Massif is slow (30s per size) | `python3 benchmark/mem_profile.py --sizes 50,100k` for a quick run |
| `ru_maxrss` looks 1024× too big on macOS | Units are bytes on macOS, KiB on Linux | Divide by 1024 |
| `rss-*.txt` shows 0 KiB | Child failed to exec | Check `benchmark/mem/scratch.csv` and the `warn: rss batch run failed` line |

### Gallery

| Symptom | Cause | Fix |
|---------|-------|-----|
| `gallery.html` shows `Loading…` forever | Opened via `file://` (blocks `fetch`) | `cd benchmark && python -m http.server 8000` → `http://localhost:8000/gallery.html` |
| Charts are empty | `results.csv` missing or wrong path | `ls benchmark/results.csv` and check browser console |
| CDN fails (no internet) | `Chart.js` / `PapaParse` CDN blocked | Gallery has a minimal CSV fallback; PNGs in `benchmark/plot*.png` still work |

## Recipes

### "I just want to see it work" (30 seconds)

```sh
make run-benchmark
ls -lh benchmark/results.csv benchmark/plot.png
```

### "Show me scaling" (5 minutes)

```sh
make run-benchmark-sizes
ls -lh benchmark/results.csv benchmark/plot-runtime-vs-n.png
python3 benchmark/plot.py --line --case last benchmark/results.csv benchmark/scaling.png
```

### "How much RAM does it use?" (10 minutes)

```sh
make run-memory
cat benchmark/mem/results.csv
ls -lh benchmark/plot-memory.png benchmark/mem/rss-*.txt
```

### "I changed the hash function — did it get faster?"

```sh
# 1. Rebuild
make
# 2. Benchmark one size, one lang (fastest feedback)
./build/cpp/demo --benchmark-csv benchmark/before.csv data/contacts_100k.csv
# ... edit src/hashtable.cpp ...
make
./build/cpp/demo --benchmark-csv benchmark/after.csv data/contacts_100k.csv
# 3. Compare best-of-5 for last-linear
python3 -c "
import csv
from collections import defaultdict
for path in ['benchmark/before.csv','benchmark/after.csv']:
    best={}
    for r in csv.DictReader(open(path)):
        k=(r['case'],r['algo']); v=float(r['ms'])
        if k not in best or v<best[k]: best[k]=v
    print(path, best[('last','linear')], best[('last','hash')])
"
```

### "Run one language only"

```sh
python3 python/phonebook/main.py --benchmark-csv benchmark/py.csv data/contacts_100k.csv
cat benchmark/py.csv
```

### "Compute mean ± stdev for a cell"

```sh
python3 -c "
import csv, statistics
from collections import defaultdict
rows=defaultdict(list)
for r in csv.DictReader(open('benchmark/results.csv')):
    rows[(r['language'],r['case'],r['algo'],r['n'])].append(float(r['ms']))
for k,v in sorted(rows.items()):
    if k==('cpp','last','linear','100000'):
        print(k, f\"mean={statistics.mean(v):.4f} stdev={statistics.stdev(v):.4f} best={min(v):.4f}\")
"
```

### "Open the interactive gallery"

```sh
cd benchmark && python -m http.server 8000
# → http://localhost:8000/gallery.html
# Try: n=100000, case=last, algo=linear → see which lang is fastest
# Then: Runtime vs n → see O(n) vs O(1) slopes
```

### "Regenerate datasets with a different seed"

```sh
python3 data/generate.py --seed 123
head -3 data/contacts_50.csv
# All six files are now seed 123; re-run benchmarks to measure on them
```

## Glossary (one-line)

| Term | One-line |
|------|----------|
| **Wall-clock** | Real elapsed time (what a wall clock shows) |
| **RSS** | Resident Set Size — RAM actually in physical memory for the process |
| **Heap** | Live objects (contacts, buckets, index) — subset of RSS |
| **ru_maxrss** | Kernel's peak RSS for a child process (`wait4` on Linux, KiB) |
| **Massif** | Valgrind heap profiler (C++) |
| **tracemalloc** | Python allocation tracker (stdlib) |
| **pprof** | Go heap profiler (`HeapInuse` with GC off) |
| **V8 heapUsed** | JS live heap after GC (`process.memoryUsage()`) |
| **JFR** | Java Flight Recorder (VisualVM-readable) |
| **JIT** | Just-In-Time compilation (Java C1/C2, JS TurboFan) — run 1 slow, runs 2–5 fast |
| **GC** | Garbage Collection — automatic memory reclaim (adds jitter) |
| **best-of-5** | `min(5 runs)` — discards outliers, less noisy than mean |
| **Nested prefixes** | `contacts_50 ⊂ 10k ⊂ 100k ⊂ … ⊂ 1m` — same RNG stream, comparable scaling |

## Still stuck?

1. Check `benchmark/results.csv` exists and has rows: `wc -l benchmark/results.csv; head benchmark/results.csv`
2. Check `benchmark/mem/results.csv` for memory: `cat benchmark/mem/results.csv`
3. Re-read the relevant doc: [05 — Running](05-running-benchmarks.md) for commands, [06 — Memory](06-memory-profiling.md) for profilers, [08 — Interpreting](08-interpreting-results.md) for numbers
4. Open an issue with: command you ran, full error output, `g++ --version; python3 --version; go version; node --version; javac -version`

---

Back to [README](../README.md) — the hub.
