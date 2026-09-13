# 06 — Memory Profiling

Runtime tells you *how fast*; memory tells you *how much*. This page is self-contained — if you haven't read [01 — Concepts](01-concepts.md) yet, the primer below gives you the three ideas you need.

## Basic concepts (30-second primer)

### Heap vs RSS — the two numbers

Picture a running program's memory as nested boxes:

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

- **Heap** — the memory your program explicitly asked for to hold its data (the phonebook: contacts, hash buckets, sorted index). This is the *data cost*.
- **RSS (Resident Set Size)** — all the RAM the OS has actually mapped for the whole process: heap + stacks + code + runtime overhead (interpreter, JVM, V8). This is the *whole-process cost*.

Why do you need both? At `n=50` the heap is tiny (0.1 MB in C++) but RSS is 12–55 MB — the runtime baseline dominates. At `n=1M` the heap dominates. One number alone can't answer both "how much does the data cost?" and "how much does the whole process cost?".

### Peak vs current

Memory use changes over time: a program allocates, frees, allocates more. The number that matters for a benchmark is the **peak** — the most memory used at any one moment — because that decides whether the program fits in RAM. Every profiler below reports a peak (or a post-GC "live" number that approximates it).

### Profiler bias — why the five numbers don't match exactly

Each language's profiler measures a slightly different thing:

- **Massif** counts every byte the C++ allocator hands out.
- **tracemalloc** counts Python objects (every `str`/`Contact` is a `PyObject` with overhead).
- **pprof** counts Go's live heap (`HeapAlloc` after explicit GC, GC on — same rule as V8/JFR).
- **V8** counts the JS heap after a forced garbage collection.
- **JFR** samples the JVM heap over time.

So the *shape* of the comparison is meaningful (C++ is the floor, Python boxes objects, etc.), but don't expect the tools to agree byte-for-byte. That's exactly why the harness also measures **kernel RSS** — one uniform measurement, taken by the OS itself, for all five languages.

## The two yardsticks

| Panel | What it measures | How | Answers |
|-------|-----------------|-----|---------|
| **Profiler heap** (`profiler_heap`) | Live heap objects: contacts + hash buckets + sorted index | Language-specific profiler (Massif, tracemalloc, pprof, V8, JFR) | Data cost — comparable across langs but each tool has bias |
| **Peak RSS** (`peak_rss`) | Whole process RAM actually in physical memory | Kernel `wait4().ru_maxrss` — same for every lang | Whole-process cost — uniform, no profiler bias |

`benchmark/plot-memory.png` shows both panels side-by-side.

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

| Flag | Meaning |
|------|---------|
| *(none)* | Run all 6 sizes (`50, 10k, 100k, 200k, 500k, 1M`) × both panels. |
| `--sizes 50,100k,1m` | Only run the dataset sizes you list (comma-separated). Use this for a quick run — Massif is slow. |
| `--skip-rss` | Skip the kernel-RSS measurements; only run the 5 language profilers. |

What it does, in order:

1. **Regenerate datasets** — `python3 data/generate.py --seed 42` (so scaling is comparable).
   - `--seed 42` — a fixed random seed, so the generated CSVs are **identical every run**. Without it, every run would get different data and the numbers wouldn't be comparable.
2. **Build** — `make` (C++), `go build -o benchmark/mem/phonebook-go` (Go), `javac -d java/phonebook/out` (Java).
   - `go build -o benchmark/mem/phonebook-go` — `-o` = **output**: write the compiled binary to this exact path (instead of the default `phonebook` in the current folder).
   - `javac -d java/phonebook/out` — `-d` = **directory**: put the compiled `.class` files in this folder (instead of next to the sources).
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

**What it does:** Massif is a debugger-style tool that intercepts every `malloc`/`free` and takes snapshots of the heap over time. The peak snapshot is the answer. It's slow (≈30 s per size) but exact — no GC, no runtime overhead — so this is the floor every other language is compared against.

```sh
valgrind --tool=massif --massif-out-file=benchmark/mem/massif-100k.out \
  build/cpp/demo --benchmark-csv benchmark/mem/scratch.csv data/contacts_100k.csv
ms_print benchmark/mem/massif-100k.out > benchmark/mem/massif-100k-msprint.txt
```

Flag by flag:

| Piece | Meaning |
|-------|---------|
| `valgrind` | The tool that runs your program in a sandbox and watches its memory. |
| `--tool=massif` | Which Valgrind tool to use: `massif` = the heap profiler (Valgrind has others, e.g. `memcheck` for bugs). |
| `--massif-out-file=benchmark/mem/massif-100k.out` | Where to save the raw profile. This file is binary — you read it with `ms_print` below. |
| `build/cpp/demo` | The program being profiled (the compiled C++ phonebook). |
| `--benchmark-csv benchmark/mem/scratch.csv` | The app's own flag: run in **batch mode** and append results to this CSV. `scratch.csv` is a throwaway file — we only care about the memory, not the timing rows. |
| `data/contacts_100k.csv` | The dataset to load (positional argument). |
| `ms_print` | Massif's companion tool: turns the raw binary profile into a readable graph. |
| `> benchmark/mem/massif-100k-msprint.txt` | Shell redirect: save the printed graph to a file instead of dumping it on your screen. |

- Massif samples heap over time; peak = `max(snapshot.heap + heap_extra + stacks)`.
- `massif-*.out` is the raw binary; `massif-*-msprint.txt` is the human-readable graph (kept as evidence).
- Measures the C++ heap only — no GC, no runtime overhead. This is the floor.

### Python — tracemalloc

**What it does:** `tracemalloc` is a Python stdlib module that records every allocation the interpreter makes. `get_traced_memory()` returns `(current, peak)`; we take the peak. It only sees Python-side objects — every `Contact` and `str` is a `PyObject` with overhead, which is why Python's number is roughly 2× C++'s.

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

Flag by flag:

| Piece | Meaning |
|-------|---------|
| `python3 benchmark/mem/profile_python.py` | Run the tracemalloc wrapper script. |
| `data/contacts_100k.csv` | First positional argument: the dataset to load. |
| `benchmark/mem/tracemalloc-100k.txt` | Second positional argument: where to write the top-10 allocation sites (the evidence file). |
| `# stdout: peak_tracemalloc_bytes 35840000` | The script also prints the peak on screen — this is the number that goes into `results.csv`. |

### Go — pprof (HeapAlloc live, GC on)

**What it does:** pprof is Go's built-in profiler. GC stays enabled (default `GOGC`, same rule as JS `heapUsed` after `gc()` and Java G1 polled heap) so the number is the live footprint under normal runtime. Two explicit `runtime.GC()` calls settle floating garbage, then `HeapAlloc` is read; `runtime.KeepAlive(pb)` keeps the phonebook live across the GCs.

`go/phonebook/memprofile_test.go` — env-gated test:

```sh
MEMPROFILE_CSV=data/contacts_100k.csv MEMPROFILE_OUT=benchmark/mem/go-100k.pprof \
  go test -v -run TestMemProfile ./...
go tool pprof -top -nodecount=10 benchmark/mem/go-100k.pprof > benchmark/mem/go-100k-pprof-top.txt
```

Flag by flag:

| Piece | Meaning |
|-------|---------|
| `MEMPROFILE_CSV=data/contacts_100k.csv` | Environment variable: tells the test **which dataset** to load. |
| `MEMPROFILE_OUT=benchmark/mem/go-100k.pprof` | Environment variable: tells the test **where to write** the pprof file. |
| `go test` | Run the Go test suite. |
| `-v` | **Verbose**: print each test's output (this is how you see the `peak_go_heap_bytes` line). |
| `-run TestMemProfile` | Only run tests whose name contains `TestMemProfile` — skips everything else so the run is fast. |
| `./...` | Test all packages in the current module (the `...` is a wildcard). |
| `go tool pprof` | Go's built-in profile viewer. |
| `-top` | Show the biggest allocation sites, biggest first. |
| `-nodecount=10` | Only show the top 10 entries. |
| `benchmark/mem/go-100k.pprof` | The profile file to read (the one the test wrote). |
| `> benchmark/mem/go-100k-pprof-top.txt` | Shell redirect: save the text summary to a file. |

- `runtime.GC()` twice settles floating garbage, then `HeapAlloc` == live footprint (always `< RSS`); `runtime.KeepAlive(pb)` prevents the GCs from collecting the phonebook itself.
- `go-*.pprof` is the binary profile (open with `go tool pprof`); `go-*-pprof-top.txt` is the text summary.

### JavaScript — V8 heapUsed + --heap-prof

**What it does:** V8 is Node's JavaScript engine. We force a full garbage collection (`global.gc()`) right before reading `process.memoryUsage().heapUsed`, so the number is live data, not garbage waiting to be collected. `--heap-prof` additionally writes a snapshot you can open in Chrome DevTools.

`benchmark/mem/profile_js.js`:

```sh
node --heap-prof --heap-prof-dir=benchmark/mem --heap-prof-name=js-100k.heapprofile \
  benchmark/mem/profile_js.js data/contacts_100k.csv benchmark/mem/js-100k.txt
```

Flag by flag:

| Piece | Meaning |
|-------|---------|
| `node` | The Node.js runtime. |
| `--heap-prof` | Turn on V8's heap-snapshot writer — this is what produces the `.heapprofile` file. |
| `--heap-prof-dir=benchmark/mem` | **Directory** where the snapshot goes. |
| `--heap-prof-name=js-100k.heapprofile` | **Filename** for the snapshot (otherwise Node auto-generates one). |
| `benchmark/mem/profile_js.js` | The script to run (loads the CSV, forces GC, prints heap numbers). |
| `data/contacts_100k.csv` | First positional argument: the dataset to load. |
| `benchmark/mem/js-100k.txt` | Second positional argument: where to write the `heapUsed / heapTotal / rss / external` numbers. |

- Forces `global.gc()` before `process.memoryUsage().heapUsed` so the number is live heap, not garbage.
- `js-*.heapprofile` is a Chrome DevTools snapshot (open in `chrome://inspect` → Memory).
- `js-*.txt` records `heapUsed / heapTotal / rss / external`.

### Java — JFR + polled heap

**What it does:** JFR (Java Flight Recorder) is a low-overhead recorder built into the JDK. A daemon thread samples the used heap every 5 ms during load, so we capture the peak as it happens; after load we call `System.gc()` twice to also get the live-heap-after-GC number.

`java/phonebook/src/com/phonebook/MemProfile.java`:

```sh
javac -d java/phonebook/out java/phonebook/src/com/phonebook/*.java
java -XX:StartFlightRecording=filename=benchmark/mem/java-100k.jfr,dumponexit=true \
  -cp java/phonebook/out com.phonebook.MemProfile data/contacts_100k.csv
jfr summary benchmark/mem/java-100k.jfr > benchmark/mem/java-100k-jfr-summary.txt
```

Flag by flag:

| Piece | Meaning |
|-------|---------|
| `javac` | The Java compiler. |
| `-d java/phonebook/out` | **Directory**: put the compiled `.class` files here. |
| `java/phonebook/src/com/phonebook/*.java` | Compile every `.java` file in that folder (the `*` is a wildcard). |
| `java` | The Java runtime. |
| `-XX:StartFlightRecording=...` | Start JFR at launch. The value is a comma-separated list of options: |
| &nbsp;&nbsp;`filename=benchmark/mem/java-100k.jfr` | Where to save the recording. |
| &nbsp;&nbsp;`dumponexit=true` | Write the file when the program exits (instead of only on demand). |
| `-cp java/phonebook/out` | **Classpath**: where to find the compiled classes. |
| `com.phonebook.MemProfile` | The main class to run (the memory-profiling program). |
| `data/contacts_100k.csv` | The dataset to load (positional argument). |
| `jfr` | The JDK's built-in tool for reading JFR recordings. |
| `summary` | Subcommand: print a text summary of the recording. |
| `> benchmark/mem/java-100k-jfr-summary.txt` | Shell redirect: save the summary to a file. |

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
| C++ | `build/cpp/demo --benchmark-csv benchmark/mem/scratch.csv <csv>` |
| Python | `python3 python/phonebook/main.py --benchmark-csv benchmark/mem/scratch.csv <csv>` |
| Go | `benchmark/mem/phonebook-go --benchmark-csv benchmark/mem/scratch.csv <csv>` |
| JS | `node javascript/phonebook/src/main.js --benchmark-csv benchmark/mem/scratch.csv <csv>` |
| Java | `java -cp java/phonebook/out com.phonebook.Main --benchmark-csv benchmark/mem/scratch.csv <csv>` |

Every command uses the same two pieces: `--benchmark-csv benchmark/mem/scratch.csv` (run in batch mode, append to this throwaway CSV — we only want the memory, not the timing rows) and `<csv>` (the dataset to load, substituted per size). The Java one additionally uses `-cp java/phonebook/out` (classpath: where the compiled classes live) and `com.phonebook.Main` (the main class to run).

Evidence: `benchmark/mem/rss-<lang>-<n>.txt` (e.g. `rss-cpp-100k.txt`).

> **Linux vs macOS:** `ru_maxrss` is KiB on Linux, bytes on macOS. The harness assumes Linux. On macOS divide by 1024 again.

## Reading the chart

`benchmark/plot-memory.png` — two panels, log y, x = dataset size, 5 grouped bars per size:

- **Left panel — profiler heap:** data cost. C++ is the peak-allocated floor (~195 MB Massif peak at 1M, transients included); Python ~343 MB (boxed objects); Go ~136 MB live `HeapAlloc` (GC on, transients freed — lowest live number but peak-vs-live definitions differ, so rank shapes and check RSS); JS live ~227 MB; Java ~343 MB polled peak.
- **Right panel — peak RSS:** whole-process cost. Baselines dominate at small n (JS ~55 MB, Java ~53 MB, C++/Go ~12 MB at n=50).

Both panels are needed for an honest claim. See [08 — Interpreting Results](08-interpreting-results.md).

## Which number should I use?

| Question you're answering | Use |
|---------------------------|-----|
| "How much does the phonebook data itself cost?" | `profiler_heap` (left panel) |
| "How much RAM does the whole program need?" | `peak_rss` (right panel) |
| "Is language X's data structure more memory-hungry than Y's?" | `profiler_heap`, same size, same tool |
| "Will this fit in my machine / container memory limit?" | `peak_rss` |

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
