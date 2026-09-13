# 04 — Benchmark Modes

The same phonebook code can be benchmarked two ways. Both measure the same 12 cells, but they serve different purposes.

## The 12 cells

Every benchmark — interactive or batch — covers:

```
4 cases × 3 algos = 12 cells
```

| Case | Phone picked | Meaning for `linear` | Meaning for `hash` / `binary` |
|------|--------------|----------------------|-------------------------------|
| `first` | `contacts[0]` | **best** — 1 compare | position-independent (~constant) |
| `middle` | `contacts[n/2]` | **avg** — n/2 compares | position-independent |
| `last` | `contacts[n-1]` | **worst** — n compares | position-independent |
| `miss` | `"0000000000"` (not in dataset) | **worst** — n compares (full scan) | **worst** — full chain / log n probes |

- `miss` is the honest worst case for hash/binary: hash must walk the entire chain, binary must do all `log n` probes before concluding "not found".
- Each cell is timed **5 times** → 60 rows per language per dataset.

```
4 cases × 3 algos × 5 runs = 60 rows
6 sizes × 5 langs × 60 = 1800 rows (full scaling run)
```

## Mode 1 — Interactive (option `0`)

What you see when you run the program normally and press `0`:

```sh
./build/cpp/demo data/contacts_50.csv
# → menu → press 0
```

```
Benchmarking phone search (linear: first=best / middle / last,miss=worst;
 hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary)
 (50 contacts, 5 runs each, best reported).

[first (linear best case) index 0 phone 0901867825]
  Linear best of 5: 0.000026ms. (index 0)
  Hash best of 5: 0.00005ms. (index 0, position-independent)
  Binary best of 5: 0.000077ms. (index 0, sorted index, position-independent)
[middle (linear avg / binary best) index 25 phone 0707707870]
  ...
```

- Calls `benchmark()` (best-of-5) per cell and **prints only the best**.
- Good for: **live demos, quick sanity checks, showing the class the cost**.

Source: `src/main.cpp:runSearchBenchmark()` and its ports (`python/phonebook/main.py:run_search_benchmark`, `go/phonebook/main.go:runSearchBenchmark`, etc.).

## Mode 2 — Batch (`--benchmark-csv`)

Headless, machine-readable, for plotting and analysis:

```sh
./build/cpp/demo --benchmark-csv benchmark/results.csv data/contacts_100k.csv
python3 python/phonebook/main.py --benchmark-csv benchmark/results.csv --append data/contacts_100k.csv
(cd go/phonebook && go run . --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv)
(cd javascript/phonebook && node src/main.js --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv)
(cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv)
```

- Calls `timeIt()` 5 times per cell and **writes every run** as a separate CSV row (60 rows). `plot.py` then takes `min()` per cell — same result as interactive, but auditable.
- `--append` — append to existing CSV (first writer creates header, rest append). Without it, the file is truncated.
- Good for: **reproducible measurements, plotting, statistics**.

Source: `src/main.cpp:runSearchBenchmarkBatch()` and ports.

### CSV schema (`benchmark/results.csv`)

```csv
language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone
cpp,data/contacts_100k.csv,100000,first,linear,1,0.000116,2026-09-11T16:04:40Z,"g++ 16.2.1 20260810",0,0901867825
cpp,data/contacts_100k.csv,100000,first,linear,2,5.1e-05,2026-09-11T16:04:40Z,"g++ 16.2.1 20260810",0,0901867825
...
```

| Column | Meaning |
|--------|---------|
| `language` | `cpp` / `python` / `go` / `js` / `java` |
| `dataset` | CSV path used |
| `n` | number of contacts |
| `case` | `first` / `middle` / `last` / `miss` |
| `algo` | `linear` / `hash` / `binary` |
| `run` | `1`–`5` |
| `ms` | wall-clock milliseconds (float, per-language format) |
| `timestamp` | UTC when the batch started |
| `toolchain` | compiler/interpreter version |
| `target_index` | index of the target phone (`-1` for `miss`) |
| `phone` | the 10-digit phone searched for |

> **Auditable:** `target_index` + `phone` let you verify that every language searched the same position (for `first`/`middle`/`last`) and that `miss` really is absent.

## Which mode to use?

| Goal | Mode | Command |
|------|------|---------|
| Show the class live | Interactive `0` | `./build/cpp/demo` → `0` |
| One-size measurement | Batch single | `make run-benchmark` |
| Scaling chart (runtime vs n) | Batch multi-size | `make run-benchmark-sizes` |
| Custom dataset / n | Batch manual | `--benchmark-csv` by hand |

---

Next: [05 — Running Benchmarks](05-running-benchmarks.md) — the actual commands.
