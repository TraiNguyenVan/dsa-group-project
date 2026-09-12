# 06 — Memory Profiling

Runtime tells you *how fast*; memory tells you *how much*. This harness measures both, with two independent yardsticks so you can answer "how much does the data cost?" vs "how much does the whole process cost?"

## The two yardsticks

| Panel | What it measures | How | Answers |
|-------|-----------------|-----|---------|
| **Profiler heap** (`profiler_heap`) | Live heap objects: contacts + hash buckets + sorted index | Language-specific profiler (Massif, tracemalloc, pprof, V8, JFR) | Data cost — comparable across langs but each tool has bias |
| **Peak RSS** (`peak_rss`) | Whole process RAM actually in physical memory | Kernel `wait4().ru_maxrss` — same for every lang | Whole-process cost — uniform, no profiler bias |

At `n=50` the heap is tiny (0.1 MB C++) but RSS is 12–55 MB — the runtime baseline dominates. At `n=1M` the heap dominates. You need both panels; `benchmark/plot-memory.png` shows them side-by-side.

```
n=50:   [ baseline ████████████████ ] [ heap ░ ]
n=1M:   [ baseline ████ ] [ heap ████████████████████████ ]
```

## Orchestrator: `benchmark/mem_profile.py`

One script runs everything headlessly:

```sh
python3 benchmark/mem_profile.py                          # all 6 sizes, both panels
python3 benchmark/mem_profile.py --sizes 50,100k,1m        # subset
python3 benchmark/mem_profile.py --skip-rss               # only profiler heap
make run-memory                                            # same, plus plot
```

What it does, in order:

1. **Regenerate datasets** — `python3 data/generate.py --seed 42` (so scaling is comparable).
2. **Build** — `make` (C++), `go build -o benchmark/mem/phonebook-go` (Go), `javac -d java/phonebook/out` (Java).
3. **For each size** (`50, 10k, 100k, 200k, 500k, 1M`):
   - Run 5 profiler-heap measurements (one per lang) on the **same load path**: `CSV → PhoneBook.load + hash + sorted index`.
   - Run 5 kernel-RSS measurements (one per lang) on the **same batch program** (`--benchmark-csv` with a scratch CSV).
4. **Write** `benchmark/mem/results.csv` (60 rows: 5 langs × 6 sizes × 2 metrics) + evidence files.
5. `make run-memory` then runs `python3 benchmark/plot.py --mem benchmark/mem/results.csv benchmark/plot-memory.png`.

Stdlib only — no extra pip packages.

### Output: `benchmark/mem/results.csv`

```csv
language,dataset,n,metric,mb,tool
cpp,data/contacts_50.csv,50,profiler_heap,0.093,valgrind/massif
cpp,data/contacts_50.csv,50,peak_rss,12.598,kernel wait4/ru_maxrss
python,data/contacts_50.csv,50,profiler_heap,0.159,tracemalloc
...
java,data/contacts_1m.csv,1000000,peak_rss,330.4,kernel wait4/ru_maxrss
```

| Column | Meaning |
|--------|---------|
| `language` | `cpp` / `python` / `go` / `js` / `java` |
| `dataset` | CSV path |
| `n` | rows |
| `metric` | `profiler_heap` or `peak_rss` |
| `mb` | megabytes |
| `tool` | which profiler / `kernel wait4/ru_maxrss` |

## Per-language profilers (profiler heap)

All run on the **same load path** — no search, just `loadfromCSV` — so the heap number is the cost of holding the phonebook in memory.

### C++ — Valgrind/Massif

```sh
valgrind --tool=massif --massif-out-file=benchmark/mem/massif-100k.out \
  build/cpp/demo --benchmark-csv /tmp/scratch.csv data/contacts_100k.csv
ms_print benchmark/mem/massif-100k.out > benchmark/mem/massif-100k-msprint.txt
```

- Massif samples heap over time; peak = `max(snapshot.heap + heap_extra + stacks)`.
- `massif-*.out` is the raw binary; `massif-*-msprint.txt` is the human-readable graph (kept as evidence).
- Measures the C++ heap only — no GC, no runtime overhead. This is the floor.

### Python — tracemalloc

`benchmark/mem/profile_python.py` (stdlib):

```python
tracemalloc.start()
pb = PhoneBook()
pb.loadfrom_csv(csv_path)
current, peak = tracemalloc.get_traced_memory()  # peak is the answer
```

- Tracks Python-side allocations only (every `Contact`/`str` is a `PyObject`).
- Evidence: `tracemalloc-<n>.txt` — top 10 allocation sites inside `python/phonebook/`.

```sh
python3 benchmark/mem/profile_python.py data/contacts_100k.csv benchmark/mem/tracemalloc-100k.txt
# stdout: peak_tracemalloc_bytes 35840000
```

### Go — pprof (HeapInuse, GC off)

`go/phonebook/memprofile_test.go` — env-gated test:

```sh
MEMPROFILE_CSV=data/contacts_100k.csv MEMPROFILE_OUT=benchmark/mem/go-100k.pprof \
  go test -v -run TestMemProfile ./...
go tool pprof -top -nodecount=10 benchmark/mem/go-100k.pprof > benchmark/mem/go-100k-pprof-top.txt
```

- `debug.SetGCPercent(-1)` disables GC during load, so `HeapInuse` after load == peak live heap.
- `go-*.pprof` is the binary profile (open with `go tool pprof`); `go-*-pprof-top.txt` is the text summary.

### JavaScript — V8 heapUsed + --heap-prof

`benchmark/mem/profile_js.js`:

```sh
node --heap-prof --heap-prof-dir=benchmark/mem --heap-prof-name=js-100k.heapprofile \
  benchmark/mem/profile_js.js data/contacts_100k.csv benchmark/mem/js-100k.txt
```

- Forces `global.gc()` before `process.memoryUsage().heapUsed` so the number is live heap, not garbage.
- `js-*.heapprofile` is a Chrome DevTools snapshot (open in `chrome://inspect` → Memory).
- `js-*.txt` records `heapUsed / heapTotal / rss / external`.

### Java — JFR + polled heap

`java/phonebook/src/com/phonebook/MemProfile.java`:

```sh
javac -d java/phonebook/out java/phonebook/src/com/phonebook/*.java
java -XX:StartFlightRecording=filename=benchmark/mem/java-100k.jfr,dumponexit=true \
  -cp java/phonebook/out com.phonebook.MemProfile data/contacts_100k.csv
jfr summary benchmark/mem/java-100k.jfr > benchmark/mem/java-100k-jfr-summary.txt
```

- A daemon thread samples `Runtime.totalMemory() - freeMemory()` every 5 ms during load → peak.
- After load: `System.gc()` twice → live heap after GC.
- `java-*.jfr` is a VisualVM-readable flight recording; `java-*-jfr-summary.txt` is the text summary.

## Kernel RSS — `wait4` / `ru_maxrss`

The uniform yardstick. No instrumentation inside the child — the kernel measures it.

```python
# benchmark/mem_profile.py:measure_rss()
pid = os.fork()
if pid == 0:
    os.execvp(cmd[0], cmd)          # child: run the batch program
_, status, usage = os.wait4(pid, 0) # parent: kernel returns peak RSS
rss_kib = usage.ru_maxrss           # KiB on Linux
```

| Lang | Batch command measured |
|------|----------------------|
| C++ | `build/cpp/demo --benchmark-csv /tmp/scratch.csv <csv>` |
| Python | `python3 python/phonebook/main.py --benchmark-csv /tmp/scratch.csv <csv>` |
| Go | `benchmark/mem/phonebook-go --benchmark-csv /tmp/scratch.csv <csv>` |
| JS | `node javascript/phonebook/src/main.js --benchmark-csv /tmp/scratch.csv <csv>` |
| Java | `java -cp java/phonebook/out com.phonebook.Main --benchmark-csv /tmp/scratch.csv <csv>` |

Evidence: `benchmark/mem/rss-<lang>-<n>.txt` (e.g. `rss-cpp-100k.txt`).

> **Linux vs macOS:** `ru_maxrss` is KiB on Linux, bytes on macOS. The harness assumes Linux. On macOS divide by 1024 again.

## Reading the chart

`benchmark/plot-memory.png` — two panels, log y, x = dataset size, 5 grouped bars per size:

- **Left panel — profiler heap:** data cost. C++ is the floor (~173 MB at 1M); Python ~2× (boxed objects); Go ~367 MB HeapInuse; JS live ~206 MB; Java ~255 MB.
- **Right panel — peak RSS:** whole-process cost. Baselines dominate at small n (JS ~55 MB, Java ~53 MB, C++/Go ~12 MB at n=50).

Both panels are needed for an honest claim. See [08 — Interpreting Results](08-interpreting-results.md).

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `valgrind: command not found` | Valgrind not installed | `sudo pacman -S valgrind` (Arch) / `sudo apt install valgrind` |
| `massif failed` warning | C++ demo not built | `make` first |
| `go memprofile failed` | `go` not in PATH | `go version` to check |
| `jfr: command not found` | JDK < 17 or JRE only | Install JDK 17+ (`javac -version`) |
| `run-memory` takes long | Massif is slow (30s per size) | Use `--sizes 50,100k` for a quick run |
| `ru_maxrss` looks wrong on macOS | Units are bytes, not KiB | Divide by 1024 |

---

Next: [07 — Plotting & Gallery](07-plotting-and-gallery.md) — turning CSVs into charts.
