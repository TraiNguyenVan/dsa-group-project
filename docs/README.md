# Documentation

Three folders, three questions:

| Folder | Answers | Start here |
|--------|---------|------------|
| [`phonebook/`](phonebook/01-overview.md) | **What does the phonebook do?** — the logic: data structures, search algorithms, implementation | [01 — Overview](phonebook/01-overview.md) |
| [`benchmark/`](benchmark/01-concepts.md) | **How do we measure it?** — the harness: timing, memory, plotting, interpreting | [01 — Concepts](benchmark/01-concepts.md) |
| [`languages/`](languages/01-language-comparison.md) | **How do the 5 languages compare?** — ports, JIT/GC, trade-offs, recommendation | [01 — Language Comparison](languages/01-language-comparison.md) |

## The phonebook logic — `docs/phonebook/`

The algorithms and data structures behind the contact list.

| Doc | What you learn |
|-----|----------------|
| [01 — Overview](phonebook/01-overview.md) | The three structures, the three searches, complexity table |
| [02 — Data Structures](phonebook/02-data-structures.md) | Chained hash table, prime sizing, rehash, sorted index |
| [03 — Search Algorithms](phonebook/03-search-algorithms.md) | Linear / hash / binary / name / prefix search |
| [04 — Implementation Notes](phonebook/04-implementation-notes.md) | CSV parsing, normalization, insert/delete costs, memory safety |

## The benchmark harness — `docs/benchmark/`

How the phonebook is measured — same algorithms, same data, five languages.

| Doc | What you learn |
|-----|----------------|
| [01 — Concepts](benchmark/01-concepts.md) | Wall-clock, RSS, heap, profiler vs kernel, best-of-5, JIT — plain English |
| [02 — Datasets](benchmark/02-datasets.md) | How `data/generate.py` builds nested-prefix CSVs, schema, reproducibility |
| [03 — Timing Harness](benchmark/03-timing-harness.md) | `timer.hpp` → `timeIt` / `benchmark`, per-language ports, why 5 runs |
| [04 — Benchmark Modes](benchmark/04-benchmark-modes.md) | Interactive option `0` vs batch `--benchmark-csv`, cases × algos, CSV schema |
| [05 — Running Benchmarks](benchmark/05-running-benchmarks.md) | `make` targets, manual commands, prerequisites, output files |
| [06 — Memory Profiling](benchmark/06-memory-profiling.md) | `mem_profile.py` dual-panel design, 5 profilers + kernel RSS (`wait4`) |
| [07 — Plotting & Gallery](benchmark/07-plotting-and-gallery.md) | `plot.py` modes, `gallery.html`, PNG catalogue |
| [08 — Interpreting Results](benchmark/08-interpreting-results.md) | How to read `results.csv` / `mem/results.csv`, charts, pitfalls |
| [09 — Architecture](benchmark/09-architecture.md) | File map, data-flow diagram, where benchmark code lives |
| [10 — FAQ](benchmark/10-faq.md) | Troubleshooting, common mistakes, quick recipes |

## The language comparison — `docs/languages/`

| Doc | What you learn |
|-----|----------------|
| [01 — Language Comparison](languages/01-language-comparison.md) | Ports, clocks, JIT/GC, trade-offs, recommendation |

## 30-second quickstart

```sh
# 1. Build + single-size benchmark (100k) → benchmark/results.csv + plot
make run-benchmark

# 2. Scaling run (50 → 1M, 6 sizes × 5 langs) → runtime-vs-n chart
make run-benchmark-sizes

# 3. Memory profiling (5 profilers + kernel RSS) → benchmark/mem/results.csv
make run-memory

# 4. Open the interactive gallery
cd benchmark && python -m http.server 8000
# → http://localhost:8000/gallery.html
```

> All three `make` targets are idempotent. Re-running overwrites the CSVs. Without `matplotlib` the plot step prints `plot skipped` but CSVs are still produced.

> **Platform support:** the harness is tested on **Linux only** (Windows/macOS untested) — see [`README.md`](../README.md) and [05 — Running Benchmarks](benchmark/05-running-benchmarks.md).

## Where things live

```
data/generate.py              dataset generator (seed 42)
include/timer.hpp             timing primitive (ported to 4 langs)
src/main.cpp                  C++ CLI + benchmark (option 0 + --benchmark-csv)
python/phonebook/             Python port — same menu, same benchmark
go/phonebook/                 Go port — same menu, same benchmark
javascript/phonebook/         JavaScript port — same menu, same benchmark
java/phonebook/               Java port — same menu, same benchmark
benchmark/
  results.csv                 runtime results (every run, 5 per cell)
  mem/results.csv             memory results (profiler_heap + peak_rss)
  mem_profile.py              memory orchestrator
  mem/profile_*.py|js         per-language profiler wrappers
  plot.py                     all plotting modes
  gallery.html                interactive browser for results.csv
  mem/*.out|.pprof|.jfr|…     raw evidence kept per run
```

Next: [phonebook/01 — Overview](phonebook/01-overview.md) — start with what the phonebook does.
