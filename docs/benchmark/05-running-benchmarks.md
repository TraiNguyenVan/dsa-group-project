# 05 — Running Benchmarks

This page is the **hands-on manual**: prerequisites, every `make` target, manual commands, and what files you get.

## Prerequisites

| Language | Needed for normal run | Needed for benchmark | Needed for plots |
|----------|----------------------|---------------------|-----------------|
| C++ | `g++` (C++17) + `make` **or** CMake 3.16+ | same | — |
| Python | `python3` (stdlib only) | same | `matplotlib` (`pip install matplotlib`) |
| Go | `go 1.21+` | same | — |
| JavaScript | `node >=18` (no `npm install`) | same | — |
| Java | JDK 17+ (`javac` + `java`) | same | — |
| Memory profiling | — | `valgrind` + `ms_print`, `go`, `jfr` (JDK) | `matplotlib` |

Without `matplotlib` the pipeline still works — the plot step prints `plot skipped: pip install matplotlib` and CSVs are produced normally.

Check yours:

```sh
g++ --version; python3 --version; go version; node --version; javac -version; valgrind --version
```

> **Platform support:** the harness was **tested on Linux only** — Windows/macOS are not yet tested. The `make` targets assume a POSIX shell; on Windows use the CMake presets (`cmake --preset mingw-release`) for C++ or the manual commands below. Full notice: `README.md`.

## Make targets (the easy way)

All targets are idempotent — re-running overwrites the CSVs.

### `make` — build only

```sh
make
./build/cpp/demo data/contacts_50.csv   # run the CLI
```

Compiles `src/*.cpp` with `-std=c++17 -Wall -O2 -Iinclude` → `build/cpp/demo`.

No `make`/CMake? Just the compiler:

```sh
g++ -std=c++17 -Wall -O2 -Iinclude src/main.cpp src/phonebook.cpp src/hashtable.cpp -o build/cpp/demo
```

### `make run-benchmark` — single size (100k)

```sh
make run-benchmark
```

Runs **all 5 languages** on `data/contacts_100k.csv` (60 rows each → 300 rows) → `benchmark/results.csv`, then `benchmark/plot.py` → `benchmark/plot.png` + 12 per-case PNGs.

```
benchmark/results.csv   300 rows (5 langs × 4 cases × 3 algos × 5 runs)
benchmark/plot.png      3-panel overview (linear | hash | binary)
benchmark/plot-first-linear.png … plot-miss-binary.png  (12 splits)
```

### `make run-benchmark-sizes` — scaling (6 sizes)

```sh
make run-benchmark-sizes
```

Regenerates datasets (`python3 data/generate.py --seed 42`), then loops `50, 10k, 100k, 200k, 500k, 1M` × 5 langs → one appended `benchmark/results.csv` (1800 rows), then three plot families:

```
benchmark/results.csv              1800 rows (6 sizes × 300)
benchmark/plot-runtime-vs-n.png    line chart: ms vs n, log-log, 3 panels (one per algo)
benchmark/plot-unified-*.png       5 PNGs (one per lang, 12 series each: 3 algos × 4 cases)
benchmark/plot-per-algo-*.png      5 PNGs (one per lang, best/avg/worst per algo)
```

The line chart defaults to `case=last` (index `n-1`, same position every lang, worst for linear) so the `O(n)` slope is visible. Override: `python3 benchmark/plot.py --line --case first benchmark/results.csv benchmark/plot-runtime-vs-n.png`.

### `make run-memory` — memory profiling

```sh
make run-memory
```

Runs `benchmark/mem_profile.py` across all 6 sizes × 5 langs → `benchmark/mem/results.csv` (60 rows: 5 langs × 6 sizes × 2 metrics), then `benchmark/plot.py --mem` → `benchmark/plot-memory.png`.

```
benchmark/mem/results.csv          60 rows (profiler_heap + peak_rss per lang/size)
benchmark/plot-memory.png          two panels: profiler heap (top) + kernel RSS (bottom)
benchmark/mem/massif-*.out         Valgrind raw + ms_print text
benchmark/mem/tracemalloc-*.txt    Python top sites
benchmark/mem/go-*.pprof           Go heap profiles + pprof top
benchmark/mem/js-*.heapprofile     V8 heap snapshots (open in Chrome DevTools)
benchmark/mem/java-*.jfr           JFR recordings + jfr summary
benchmark/mem/rss-*-*.txt          kernel ru_maxrss per run
```

Takes ~5–15 minutes (Massif is slow). See [06 — Memory Profiling](06-memory-profiling.md).

### Other targets

```sh
make run-50; make run-100k; make run-200k; make run-1m  # C++ CLI on one dataset
make pdf                                                # report/report.md → report/report.pdf
make clean                                              # rm -rf build/cpp
```

## Manual commands (without `make`)

Batch mode is just a CLI flag — you can run any language alone:

```sh
# C++ (build first)
make
./build/cpp/demo --benchmark-csv benchmark/results.csv data/contacts_100k.csv
./build/cpp/demo --benchmark-csv benchmark/results.csv --append data/contacts_200k.csv  # append

# Python
python3 python/phonebook/main.py --benchmark-csv benchmark/results.csv data/contacts_100k.csv
python3 python/phonebook/main.py --benchmark-csv benchmark/results.csv --append data/contacts_100k.csv

# Go
cd go/phonebook && go run . --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv

# JavaScript
cd javascript/phonebook && node src/main.js --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv

# Java
cd java/phonebook && javac -d out src/com/phonebook/*.java
java -cp out com.phonebook.Main --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv
```

Interactive mode (no CSV, prints best-of-5):

```sh
printf '0\n9\n' | ./build/cpp/demo data/contacts_50.csv
printf '0\n9\n' | python3 python/phonebook/main.py data/contacts_50.csv
printf '0\n9\n' | (cd go/phonebook && go run . ../../data/contacts_50.csv)
printf '0\n9\n' | (cd javascript/phonebook && node src/main.js ../../data/contacts_50.csv)
printf '0\n9\n' | (cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main ../../data/contacts_50.csv)
```

## Output files at a glance

| File | Produced by | Rows | Purpose |
|------|-------------|------|---------|
| `benchmark/results.csv` | `run-benchmark` / `run-benchmark-sizes` | 300 / 1800 | runtime: every run |
| `benchmark/mem/results.csv` | `run-memory` | 60 | memory: profiler + RSS |
| `benchmark/plot.png` | `plot.py` (default) | — | 3-panel overview |
| `benchmark/plot-runtime-vs-n.png` | `plot.py --line` | — | scaling line chart |
| `benchmark/plot-unified-*.png` | `plot.py --unified` | — | per-lang all-cases |
| `benchmark/plot-per-algo-*.png` | `plot.py --per-algo` | — | per-lang best/avg/worst |
| `benchmark/plot-memory.png` | `plot.py --mem` | — | memory two-panel |
| `benchmark/gallery.html` | static | — | interactive browser |

## Tips

- **Start small:** `make run-benchmark` (100k, ~30s) before `run-benchmark-sizes` (1M, ~5 min) or `run-memory` (~10 min).
- **Reproducibility:** batch mode records `target_index`+`phone` so you can audit that every language searched the same deterministic position.
- **Without matplotlib:** CSVs are still produced; only PNGs are skipped.
- **Datasets missing?** `python3 data/generate.py --seed 42` recreates all six.

---

Next: [06 — Memory Profiling](06-memory-profiling.md) — how `mem_profile.py` measures heap and RSS.
