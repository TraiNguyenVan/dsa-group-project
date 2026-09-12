# Benchmark Harness — Documentation

> **Audience:** anyone who has never profiled code before. Every concept is introduced from first principles, then mapped to the exact file and command in this repo.

This harness answers one question: **how much does a phonebook lookup cost?** It measures the same three algorithms (`linear` / `hash` / `binary`) on the same datasets in five languages (`C++`, `Python`, `Go`, `JavaScript`, `Java`) and produces comparable numbers for time and memory.

## Map

| Doc | What you learn |
|-----|----------------|
| [01 — Concepts](01-concepts.md) | Wall-clock, RSS, heap, profiler vs kernel, best-of-5, JIT — plain English |
| [02 — Datasets](02-datasets.md) | How `data/generate.py` builds nested-prefix CSVs, schema, reproducibility |
| [03 — Timing Harness](03-timing-harness.md) | `timer.hpp` → `timeIt` / `benchmark`, per-language ports, why 5 runs |
| [04 — Benchmark Modes](04-benchmark-modes.md) | Interactive option `0` vs batch `--benchmark-csv`, cases × algos, CSV schema |
| [05 — Running Benchmarks](05-running-benchmarks.md) | `make` targets, manual commands, prerequisites, output files |
| [06 — Memory Profiling](06-memory-profiling.md) | `mem_profile.py` dual-panel design, 5 profilers + kernel RSS (`wait4`) |
| [07 — Plotting & Gallery](07-plotting-and-gallery.md) | `plot.py` modes, `gallery.html`, PNG catalogue |
| [08 — Interpreting Results](08-interpreting-results.md) | How to read `results.csv` / `mem/results.csv`, charts, pitfalls |
| [09 — Architecture](09-architecture.md) | File map, data-flow diagram, where benchmark code lives |
| [10 — FAQ](10-faq.md) | Troubleshooting, common mistakes, quick recipes |

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

## Where things live

```
data/generate.py              dataset generator (seed 42)
include/timer.hpp             timing primitive (ported to 4 langs)
src/main.cpp                  C++ CLI + benchmark (option 0 + --benchmark-csv)
python|go|javascript|java/    faithful ports — same menu, same benchmark
benchmark/
  results.csv                 runtime results (every run, 5 per cell)
  mem/results.csv             memory results (profiler_heap + peak_rss)
  mem_profile.py              memory orchestrator
  mem/profile_*.py|js         per-language profiler wrappers
  plot.py                     all plotting modes
  gallery.html                interactive browser for results.csv
  mem/*.out|.pprof|.jfr|…     raw evidence kept per run
```

Next: [01 — Concepts](01-concepts.md) — start from zero.
