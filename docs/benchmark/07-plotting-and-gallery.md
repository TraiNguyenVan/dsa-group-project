# 07 — Plotting & Gallery

CSV files are hard to read. `benchmark/plot.py` turns them into PNGs; `benchmark/gallery.html` lets you explore them interactively.

## `benchmark/plot.py` — one script, five modes

All modes read `benchmark/results.csv` (or `benchmark/mem/results.csv` for `--mem`) and write PNGs with `matplotlib` (Agg backend, no display needed). Without `matplotlib` they print `plot skipped` and exit 2 — CSVs are still valid.

```sh
python3 benchmark/plot.py <input.csv> <output.png> [flags]
```

| Mode | Flag | Input | Output | What it draws |
|------|------|-------|--------|---------------|
| **Overview** | *(none)* | `results.csv` | `plot.png` + 12 splits | 3 panels (linear / hash / binary) at `n=100k`, bars = langs, x = case |
| **Scaling** | `--line [--case last]` | multi-size `results.csv` | `plot-runtime-vs-n.png` | 3 panels (one per algo), lines = langs, x = n (log-log), y = best ms |
| **Unified** | `--unified` | multi-size `results.csv` | `plot-unified-<lang>.png` ×5 | Per-lang: 12 series (3 algos × 4 cases) vs n, log-log |
| **Per-algo** | `--per-algo` | multi-size `results.csv` | `plot-per-algo-<lang>.png` ×5 | Per-lang: x=algo, 3 bars best/avg/worst at largest n, log y |
| **Memory** | `--mem` | `mem/results.csv` | `plot-memory.png` | 2 panels: profiler heap (left) + kernel RSS (right), x=n, bars=langs, log y |

### Overview (default)

```sh
python3 benchmark/plot.py benchmark/results.csv benchmark/plot.png
python3 benchmark/plot.py --n 50 benchmark/results.csv benchmark/plot.png  # force n=50
```

- Filters to `n=100k` by default (so a multi-size CSV doesn't collapse to `n=50`). Override with `--n`.
- Takes `best = min(5 runs)` per `(lang, case, algo)` — same as the live CLI.
- 3 linear-scale panels: `Linear scan ms` / `Hash lookup ms` / `Binary search ms`. Separate panels so hash/binary µs bars aren't flattened by linear ms bars.
- Also writes 12 per-case PNGs: `plot-first-linear.png` … `plot-miss-binary.png` (one per `case × algo`, horizontal bars sorted fastest-first).

```
benchmark/plot.png
benchmark/plot-first-linear.png   benchmark/plot-first-hash.png   benchmark/plot-first-binary.png
benchmark/plot-middle-linear.png  benchmark/plot-middle-hash.png  benchmark/plot-middle-binary.png
benchmark/plot-last-linear.png    benchmark/plot-last-hash.png    benchmark/plot-last-binary.png
benchmark/plot-miss-linear.png    benchmark/plot-miss-hash.png    benchmark/plot-miss-binary.png
```

### Scaling — `--line`

```sh
python3 benchmark/plot.py --line benchmark/results.csv benchmark/plot-runtime-vs-n.png
python3 benchmark/plot.py --line --case first benchmark/results.csv benchmark/first.png
```

- Reads a **multi-size** CSV (from `make run-benchmark-sizes`: 6 sizes × 5 langs × 60 rows).
- Groups `best = min(5 runs)` by `(lang, algo, case, n)` and plots `ms vs n` on **log-log** axes (required: `n` spans 50..1M and linear ms vs hash µs differ ~1000×).
- 3 panels (one per algo), one line per language, `case` defaults to `last` (index `n-1`, same position every lang, worst for linear — so the `O(n)` slope is visible).

### Unified — `--unified`

```sh
python3 benchmark/plot.py --unified benchmark/results.csv benchmark/plot-unified-cpp.png
# writes plot-unified-cpp.png, plot-unified-python.png, … (5 files)
```

- One PNG per language, each with **12 series** (3 algos × 4 cases) vs `n` on log-log.
- Color = algo (`linear` blue, `hash` orange, `binary` green), linestyle = case (`first` solid, `middle` dashdot, `last` dotted, `miss` solid+marker).
- Shows every algo on every case for that language — the most complete view.

### Per-algo — `--per-algo`

```sh
python3 benchmark/plot.py --per-algo benchmark/results.csv benchmark/plot-per-algo-cpp.png
# writes plot-per-algo-cpp.png, plot-per-algo-python.png, … (5 files)
```

- One PNG per language, at the **largest `n`** in the CSV.
- For each `(lang, algo)`: `hit = first/middle/last` (all hits), `best = min(hit)`, `worst = max(hit + [miss])`, `avg = mean(hit)`.
- Log y (linear worst ms vs hash best µs differ ~1000×). 9 bars per PNG (3 algos × 3 stats).

### Memory — `--mem`

```sh
python3 benchmark/plot.py --mem benchmark/mem/results.csv benchmark/plot-memory.png
```

- Two panels, log y, x = dataset size, 5 grouped bars per size (one per lang).
- Left: `profiler_heap` (Massif / tracemalloc / pprof / V8 / JFR) — data cost.
- Right: `peak_rss` (kernel `wait4/ru_maxrss`) — whole-process cost.
- See [06 — Memory Profiling](06-memory-profiling.md) for what each bar means.

### Defensive parsing

`plot.py:parse_ms()` handles every language's float format:

```python
def parse_ms(raw, rowno):
    s = raw.strip().strip('"').strip()
    v = float(s)  # handles 0.005, 9.7e-05, 5.7e-05, 0.000318 alike
    if v != v or v in (inf, -inf) or v < 0: skip
    return v
```

Bad rows are skipped with a `warn:` instead of crashing. `load_best()` also skips rows with unknown `language`/`case`/`algo`.

## `benchmark/gallery.html` — interactive browser

A static HTML file that reads `benchmark/results.csv` in the browser and draws 3 interactive charts + a best-of-5 table. No build step — just serve over HTTP (`file://` blocks `fetch()`).

```sh
cd benchmark && python -m http.server 8000
# open http://localhost:8000/gallery.html
```

What you get:

| Section | Controls | Shows |
|---------|----------|-------|
| **1. Overview** | `n` / `case` / `algo` + lang toggles | One bar per enabled language (best ms) |
| **2. Cases × algos** | `n` + `language` | Grouped bars: x=case, series=algo |
| **3. Runtime vs n** | `case` + `algo` + lang toggles | Lines: x=n, series=language (log y) |
| **Best-of-5 table** | `n` | Full matrix: rows=langs, cols=case/algo |

- Uses `Chart.js` + `PapaParse` from CDN, with a minimal CSV fallback if offline.
- Same `best = min(5 runs)` logic as `plot.py`.
- Static PNG fallback at the bottom (the 12 overview splits) if JS is disabled.

> **Tip:** use the gallery to pick the interesting `n`/`case`/`algo` before running `plot.py` for a publication PNG.

## Which chart to use?

| Question | Chart |
|----------|-------|
| Which lang is fastest for `last-linear` at 100k? | Overview (`plot.png`) or gallery §1 |
| Does linear really grow with `n`? | Scaling (`--line`, log-log) or gallery §3 |
| How does hash behave on `miss` vs `first`? | Unified (`--unified`) or per-algo (`--per-algo`) |
| How much RAM does the data cost vs the process? | Memory (`--mem`) |
| I want to explore freely | Gallery (`gallery.html`) |

---

Next: [08 — Interpreting Results](08-interpreting-results.md) — how to read the numbers without fooling yourself.
