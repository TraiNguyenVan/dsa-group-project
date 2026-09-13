#!/usr/bin/env python3
"""Grouped bar chart from benchmark/results.csv (best of 5 runs per cell).

Each language writes `ms` in its own float format (C++ setprecision(17),
Python repr, Go %g, JS double, Java Double.toString) — all are parsed with
a defensive float() that strips whitespace and skips bad rows with a
warning instead of crashing. Units are milliseconds in every language.

Usage:
    python3 benchmark/plot.py benchmark/results.csv benchmark/plot.png
"""

import argparse
import csv
import os
import sys

LANG_ORDER = ["cpp", "python", "go", "js", "java"]
CASE_ORDER = ["first", "middle", "last", "miss"]
ALGO_ORDER = ["linear", "hash", "binary"]
GROUPS = [(c, a) for c in CASE_ORDER for a in ALGO_ORDER]
# For overview plots that still want the classic 3-case view
CASE_ORDER_CLASSIC = ["first", "last"]


def parse_ms(raw, rowno):
    """Parse one ms cell from any of the 5 languages. Returns None if bad."""
    if raw is None:
        print(f"warn: row {rowno}: empty ms, skipped", file=sys.stderr)
        return None
    s = raw.strip().strip('"').strip()
    if not s:
        print(f"warn: row {rowno}: empty ms, skipped", file=sys.stderr)
        return None
    try:
        v = float(s)  # handles 0.005, 9.7e-05, 5.7e-05, 0.000318 alike
    except ValueError:
        print(f"warn: row {rowno}: unparsable ms {raw!r}, skipped", file=sys.stderr)
        return None
    if v != v or v in (float("inf"), float("-inf")):  # NaN / Inf
        print(f"warn: row {rowno}: non-finite ms {raw!r}, skipped", file=sys.stderr)
        return None
    if v < 0:
        print(f"warn: row {rowno}: negative ms {raw!r}, skipped", file=sys.stderr)
        return None
    return v


def load_best(path, n_filter=None):
    """Return (best[(lang, case, algo)], meta). best = min of runs.

    If n_filter is set, only rows with n == n_filter are considered.
    """
    best = {}
    skipped = 0
    dataset = "?"
    n = "?"
    with open(path, "r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for rowno, row in enumerate(reader, start=2):
            try:
                lang = (row.get("language") or "").strip()
                case = (row.get("case") or "").strip()
                algo = (row.get("algo") or "").strip()
            except AttributeError:
                skipped += 1
                continue
            if lang not in LANG_ORDER or case not in CASE_ORDER or algo not in ALGO_ORDER:
                skipped += 1
                continue
            if n_filter is not None:
                try:
                    row_n = int((row.get("n") or "").strip())
                except ValueError:
                    skipped += 1
                    continue
                if row_n != n_filter:
                    continue
            v = parse_ms(row.get("ms"), rowno)
            if v is None:
                skipped += 1
                continue
            key = (lang, case, algo)
            if key not in best or v < best[key]:
                best[key] = v
            if dataset == "?":
                dataset = (row.get("dataset") or "?").strip()
                n = (row.get("n") or "?").strip()
    if skipped:
        print(f"warn: skipped {skipped} row(s)", file=sys.stderr)
    return best, dataset, n


def plot_line(input_path, output_path, case="last"):
    """Line chart: best-of-5 ms vs n, one line per language, per algo panel.

    Reads a multi-size results.csv (produced by `make run-benchmark-sizes`),
    groups best-of-5 by (lang, algo, case, n), and plots the runtime growth
    curves of all 5 languages overlaid on one graph (log-log so 50..1M fits).
    Defaults to the `last` case (index n-1, same position in every language,
    worst case for linear search) so the O(n) slope is visible.
    """
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    best = {}  # (lang, algo, case, n) -> best ms
    with open(input_path, "r", encoding="utf-8", newline="") as f:
        for rowno, row in enumerate(csv.DictReader(f), start=2):
            lang = (row.get("language") or "").strip()
            algo = (row.get("algo") or "").strip()
            c = (row.get("case") or "").strip()
            try:
                n = int((row.get("n") or "").strip())
            except ValueError:
                continue
            if lang not in LANG_ORDER or algo not in ALGO_ORDER or c not in CASE_ORDER:
                continue
            v = parse_ms(row.get("ms"), rowno)
            if v is None:
                continue
            key = (lang, algo, c, n)
            if key not in best or v < best[key]:
                best[key] = v

    ns = sorted({n for (_, _, _, n) in best})
    if not ns:
        print("error: no plottable rows found", file=sys.stderr)
        return 1

    fig, axes = plt.subplots(1, 3, figsize=(18, 6))
    for ax, algo in zip(axes, ALGO_ORDER):
        for lang in LANG_ORDER:
            ys = [best.get((lang, algo, case, n), float("nan")) for n in ns]
            ax.plot(ns, ys, marker="o", label=lang)
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlabel("n (rows)")
        ax.set_ylabel("best-of-5 ms")
        ax.set_title(f"{algo} lookup ({case} position)")
        ax.grid(True, which="both", alpha=0.3)
    axes[0].legend()
    fig.suptitle("Runtime vs dataset size, all 5 languages")
    fig.tight_layout()
    fig.savefig(output_path, dpi=120)
    print(f"Wrote {output_path} ({len(ns)} sizes, {len(best)} cells).")
    return 0


def plot_unified(input_path, output_path):
    """Per-language line charts: one PNG per language, every algo x every case.

    5 charts (cpp/python/go/js/java), each with 12 series (3 algos x 4
    cases: first/middle/last/miss) of best-of-5 ms vs n on log-log axes, so each
    PNG shows every algo time on every case for that language. Color =
    algo, linestyle = case (first solid, middle dashdot, last dotted, miss solid+marker).
    Log-log is required, not cosmetic: n spans 50..1M and linear-scan ms
    vs hash/binary us differ ~1000x, so linear axes would hide both the
    growth slope and the gap. Reads the multi-size results.csv from
    `make run-benchmark-sizes`. `output_path` is the cpp chart; the rest
    are named `plot-unified-<lang>.png`.
    """
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    best = {}  # (lang, algo, case, nn) -> best ms
    with open(input_path, "r", encoding="utf-8", newline="") as f:
        for rowno, row in enumerate(csv.DictReader(f), start=2):
            lang = (row.get("language") or "").strip()
            algo = (row.get("algo") or "").strip()
            c = (row.get("case") or "").strip()
            if lang not in LANG_ORDER or algo not in ALGO_ORDER or c not in CASE_ORDER:
                continue
            try:
                nn = int((row.get("n") or "").strip())
            except ValueError:
                continue
            v = parse_ms(row.get("ms"), rowno)
            if v is None:
                continue
            key = (lang, algo, c, nn)
            if key not in best or v < best[key]:
                best[key] = v

    if not best:
        print("error: no plottable rows found", file=sys.stderr)
        return 1
    ns = sorted({nn for (_, _, _, nn) in best})

    algo_colors = {"linear": "tab:blue", "hash": "tab:orange", "binary": "tab:green"}
    case_styles = {"first": "-", "middle": "-.", "last": ":", "miss": "-"}
    out_dir = os.path.dirname(output_path) or "."
    stem = os.path.splitext(os.path.basename(output_path))[0]
    # `plot-unified-cpp.png` -> base `plot-unified`; bare `plot-unified`
    # stays as-is. One chart per language either way.
    base = stem[:-len("-cpp")] if stem.endswith("-cpp") else stem
    for lang in LANG_ORDER:
        fig, ax = plt.subplots(figsize=(10, 6))
        n_cells = 0
        for algo in ALGO_ORDER:
            for c in CASE_ORDER:
                ys = [best.get((lang, algo, c, n), float("nan")) for n in ns]
                # clamp 0 -> NaN for log scale (timer can return 0 on tiny n)
                ys = [float("nan") if y == 0 else y for y in ys]
                n_cells += sum(1 for y in ys if y == y)
                ax.plot(ns, ys, marker="o", markersize=4,
                        color=algo_colors[algo], linestyle=case_styles[c],
                        label=f"{algo}-{c}")
        if n_cells == 0:
            plt.close(fig)
            print(f"skip {lang}: no data", file=sys.stderr)
            continue
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlabel("n (rows)")
        ax.set_ylabel("best-of-5 ms (log scale)")
        ax.set_title(f"{lang}: every algo on every case vs n")
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(title="algo-case", fontsize=7, ncol=3)
        fig.tight_layout()
        lang_path = os.path.join(out_dir, f"{base}-{lang}.png")
        fig.savefig(lang_path, dpi=120)
        plt.close(fig)
        print(f"Wrote {lang_path} ({lang}, {len(ns)} sizes, {n_cells} cells).")
    return 0


def plot_per_algo_cases(input_path, output_path):
    """Per-language grouped bars: x=algo, 3 bars best/avg/worst per algo.

    For each (lang, algo) at the largest n in the CSV:
      hit_cases = first/middle/last (all hits)
      best  = min(hit_cases)          — linear: first, hash: ~any, binary: middle
      worst = max(hit_cases + [miss]) — linear: miss/last, hash/binary: miss
      avg   = mean(hit_cases)         — linear ~n/2, hash/binary ~constant
    Log y scale is required: linear worst (ms) vs hash best (us) differ ~1000x.
    One PNG per language (5 total), 9 bars each (3 algos x 3 stats).
    """
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import math

    best = {}  # (lang, algo, case, nn) -> best ms
    with open(input_path, "r", encoding="utf-8", newline="") as f:
        for rowno, row in enumerate(csv.DictReader(f), start=2):
            lang = (row.get("language") or "").strip()
            algo = (row.get("algo") or "").strip()
            c = (row.get("case") or "").strip()
            if lang not in LANG_ORDER or algo not in ALGO_ORDER or c not in CASE_ORDER:
                continue
            try:
                nn = int((row.get("n") or "").strip())
            except ValueError:
                continue
            v = parse_ms(row.get("ms"), rowno)
            if v is None:
                continue
            key = (lang, algo, c, nn)
            if key not in best or v < best[key]:
                best[key] = v

    if not best:
        print("error: no plottable rows found", file=sys.stderr)
        return 1
    ns = sorted({nn for (_, _, _, nn) in best})
    n_max = ns[-1]
    hit_cases = ["first", "middle", "last"]

    out_dir = os.path.dirname(output_path) or "."
    stem = os.path.splitext(os.path.basename(output_path))[0]
    base = stem[:-len("-cpp")] if stem.endswith("-cpp") else stem
    # also handle -python etc if user passes non-cpp
    for suffix in ["-python", "-go", "-js", "-java"]:
        if base.endswith(suffix):
            base = base[:-len(suffix)]
            break

    for lang in LANG_ORDER:
        # compute best/avg/worst per algo at n_max
        stats = {}  # algo -> (best, avg, worst)
        for algo in ALGO_ORDER:
            hit_vals = [best.get((lang, algo, c, n_max)) for c in hit_cases]
            hit_vals = [v for v in hit_vals if v is not None and v == v]
            miss_val = best.get((lang, algo, "miss", n_max))
            if not hit_vals:
                continue
            b = min(hit_vals)
            # worst includes miss if present
            worst_candidates = hit_vals + ([miss_val] if miss_val is not None and miss_val == miss_val else [])
            w = max(worst_candidates)
            a = sum(hit_vals) / len(hit_vals)
            stats[algo] = (b, a, w)

        if not stats:
            print(f"warn: no data for {lang} at n={n_max}, skipped", file=sys.stderr)
            continue

        # clamp 0 -> small epsilon for log scale
        for algo in list(stats.keys()):
            b, a, w = stats[algo]
            stats[algo] = (b if b > 0 else 1e-6, a if a > 0 else 1e-6, w if w > 0 else 1e-6)

        fig, ax = plt.subplots(figsize=(10, 6))
        x = range(len(ALGO_ORDER))
        width = 0.22
        colors = {"best": "tab:green", "avg": "tab:blue", "worst": "tab:red"}
        for i, label in enumerate(["best", "avg", "worst"]):
            vals = []
            for algo in ALGO_ORDER:
                if algo in stats:
                    vals.append(stats[algo][i])
                else:
                    vals.append(float("nan"))
            offs = [p + (i - 1) * width for p in x]
            bars = ax.bar(offs, vals, width=width, label=label, color=colors[label])
            for b, v in zip(bars, vals):
                if v == v and v > 0:
                    # place label slightly above bar; use log-aware offset
                    ax.text(b.get_x() + b.get_width() / 2, v * 1.08, f"{v:.2g}",
                            ha="center", va="bottom", fontsize=8, rotation=0)

        ax.set_xticks(list(x))
        ax.set_xticklabels(ALGO_ORDER)
        ax.set_yscale("log")
        ax.set_ylabel("best-of-5 ms (log scale)")
        ax.set_title(f"{lang}: best / avg / worst per algo (n={n_max})")
        ax.legend(title="case stat")
        ax.grid(True, which="both", axis="y", alpha=0.3)
        # annotate n and hit definition
        ax.text(0.02, 0.98, f"best=min(hit)  avg=mean(hit)  worst=max(hit,miss)\nhit=first/middle/last",
                transform=ax.transAxes, va="top", ha="left", fontsize=7,
                bbox=dict(boxstyle="round,pad=0.3", fc="white", alpha=0.7))
        fig.tight_layout()
        lang_path = os.path.join(out_dir, f"{base}-{lang}.png")
        fig.savefig(lang_path, dpi=120)
        plt.close(fig)
        print(f"Wrote {lang_path} ({lang}, n={n_max}, {len(stats)} algos).")
    return 0


def plot_mem(input_path, output_path):
    """Two-panel peak-memory bar chart from benchmark/mem/results.csv.

    Panel 1: profiler heap per language (Massif / tracemalloc / pprof /
    node --heap-prof / JFR-polled) — the guide's language-specific tools.
    Panel 2: kernel-measured peak process RSS (wait4/ru_maxrss) of the
    same batch program — one uniform yardstick across languages.
    x = the 6 dataset sizes (log), 5 grouped bars per size (lang colors),
    log y (Python's per-object overhead dwarfs C++ ~10-30x).
    """
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    data = {}  # (lang, metric, n) -> MB
    with open(input_path, "r", encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            lang = (row.get("language") or "").strip()
            metric = (row.get("metric") or "").strip()
            try:
                n = int((row.get("n") or "").strip())
                mb = float((row.get("mb") or "").strip())
            except ValueError:
                continue
            if lang not in LANG_ORDER or metric not in ("profiler_heap", "peak_rss"):
                continue
            data[(lang, metric, n)] = mb

    if not data:
        print("error: no plottable rows found", file=sys.stderr)
        return 1
    ns = sorted({n for (_, _, n) in data})

    cycle = plt.rcParams["axes.prop_cycle"].by_key()["color"]
    lang_color = {lang: cycle[i % len(cycle)] for i, lang in enumerate(LANG_ORDER)}
    width = 0.8 / len(LANG_ORDER)

    panels = [
        ("profiler_heap", "Peak heap per language profiler (MB)"),
        ("peak_rss", "Peak process RSS, kernel wait4/ru_maxrss (MB)"),
    ]
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    for ax, (metric, title) in zip(axes, panels):
        x = range(len(ns))
        for i, lang in enumerate(LANG_ORDER):
            vals = [data.get((lang, metric, n), float("nan")) for n in ns]
            offs = [p + (i - (len(LANG_ORDER) - 1) / 2) * width for p in x]
            bars = ax.bar(offs, vals, width=width, label=lang,
                          color=lang_color[lang])
            for b, v in zip(bars, vals):
                if v == v and v > 0:
                    ax.text(b.get_x() + b.get_width() / 2, v * 1.02, f"{v:.2g}",
                            ha="center", va="bottom", fontsize=7, rotation=45)
        ax.set_xticks(list(x))
        ax.set_xticklabels([str(n) for n in ns])
        ax.set_xlabel("n (rows)")
        ax.set_ylabel("MB (log scale)")
        ax.set_yscale("log")
        ax.set_title(title)
        ax.grid(True, which="both", axis="y", alpha=0.3)
    axes[0].legend()
    fig.suptitle("Peak memory across 5 implementations (all dataset sizes)")
    fig.tight_layout()
    fig.savefig(output_path, dpi=120)
    print(f"Wrote {output_path} ({len(ns)} sizes, {len(data)} cells).")
    return 0


def main():
    ap = argparse.ArgumentParser(description="Plot benchmark/results.csv (best of 5).")
    ap.add_argument("input", help="input CSV (benchmark/results.csv)")
    ap.add_argument("output", help="output PNG (benchmark/plot.png)")
    ap.add_argument("--line", action="store_true",
                    help="line chart of best-of-5 ms vs n (multi-size results.csv)")
    ap.add_argument("--case", choices=CASE_ORDER, default="last",
                    help="target case for the line chart (default: last)")
    ap.add_argument("--unified", action="store_true",
                    help="per-language line charts: one PNG per language, "
                    "every algo on every case vs n (log-log)")
    ap.add_argument("--per-algo", action="store_true",
                    help="per-language grouped bars: x=algo, 3 bars best/avg/worst per algo (log y)")
    ap.add_argument("--mem", action="store_true",
                    help="peak-memory bar chart from benchmark/mem/results.csv "
                    "(two panels: profiler heap + kernel peak RSS)")
    ap.add_argument("--n", type=int, default=None,
                    help="filter to this n for the overview plot (default: 100000; "
                    "use e.g. --n 50 to force the 50-row demo)")
    args = ap.parse_args()

    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("error: matplotlib not installed (pip install matplotlib)", file=sys.stderr)
        return 2

    if args.line:
        return plot_line(args.input, args.output, args.case)

    if args.unified:
        return plot_unified(args.input, args.output)

    if args.per_algo:
        return plot_per_algo_cases(args.input, args.output)

    if args.mem:
        return plot_mem(args.input, args.output)

    # Overview 3-panel plot: default to n=100k so a multi-size CSV
    # (from run-benchmark-sizes) does not collapse to n=50 (the min).
    n_filter = args.n if args.n is not None else 100000
    try:
        best, dataset, n = load_best(args.input, n_filter=n_filter)
    except FileNotFoundError:
        print(f"error: input not found: {args.input}", file=sys.stderr)
        return 1
    except OSError as e:
        print(f"error: cannot read {args.input}: {e}", file=sys.stderr)
        return 1

    if not best:
        # No rows for the requested n — fall back to unfiltered so the
        # command still produces something, but warn loudly.
        print(f"warn: no rows for n={n_filter}, falling back to all n", file=sys.stderr)
        try:
            best, dataset, n = load_best(args.input, n_filter=None)
        except (FileNotFoundError, OSError):
            pass
    if not best:
        print("error: no plottable rows found", file=sys.stderr)
        return 1

    # Three linear-scale panels (linear | hash | binary): ratios within a
    # panel are honest, and the us-scale hash/binary bars get their own axis
    # instead of being flattened by the ms-scale linear bars.
    # Missing combos -> NaN (gap, no bar). Use classic 3-case view for the
    # overview so it stays readable; full 5-case data is in unified/per-algo.
    width = 0.8 / len(LANG_ORDER)
    panels = [
        ("Linear scan ms (best of 5)", [(c, "linear") for c in CASE_ORDER_CLASSIC]),
        ("Hash lookup ms (best of 5)", [(c, "hash") for c in CASE_ORDER_CLASSIC]),
        ("Binary search ms (best of 5)", [(c, "binary") for c in CASE_ORDER_CLASSIC]),
    ]

    fig, axes = plt.subplots(1, 3, figsize=(18, 6))
    for ax, (ylabel, groups) in zip(axes, panels):
        x = range(len(groups))
        for i, lang in enumerate(LANG_ORDER):
            vals = [best.get((lang, c, a), float("nan")) for c, a in groups]
            offs = [p + (i - (len(LANG_ORDER) - 1) / 2) * width for p in x]
            bars = ax.bar(offs, vals, width=width, label=lang)
            for b, v in zip(bars, vals):
                if v == v and v > 0:  # not NaN
                    ax.text(b.get_x() + b.get_width() / 2, v * 1.02, f"{v:.2g}",
                            ha="center", va="bottom", fontsize=8, rotation=45)
        ax.set_xticks(list(x))
        ax.set_xticklabels([c for c, _ in groups])
        ax.set_ylabel(ylabel)
        ax.set_ylim(bottom=0)
    axes[0].legend()
    fig.suptitle(f"Phone search: linear vs hash vs binary ({dataset}, n={n})")
    fig.tight_layout()
    fig.savefig(args.output, dpi=120)
    print(f"Wrote {args.output} ({len(best)} cells).")

    # 9 per-group charts: one PNG per (case, algo), horizontal bars sorted
    # fastest-first. Same LANG_ORDER colors as the overview.
    cycle = plt.rcParams["axes.prop_cycle"].by_key()["color"]
    lang_color = {lang: cycle[i % len(cycle)] for i, lang in enumerate(LANG_ORDER)}
    out_dir = os.path.dirname(args.output) or "."
    stem = os.path.splitext(os.path.basename(args.output))[0]
    split_names = []
    for c, a in GROUPS:
        pairs = [(lang, best[(lang, c, a)])
                 for lang in LANG_ORDER if (lang, c, a) in best]
        pairs.sort(key=lambda p: p[1])  # fastest first
        fig1, ax1 = plt.subplots(figsize=(8, 5))
        if pairs:
            langs = [p[0] for p in pairs]
            vals = [p[1] for p in pairs]
            bars = ax1.barh(langs, vals,
                            color=[lang_color[lang] for lang in langs])
            for b, v in zip(bars, vals):
                ax1.text(v * 1.02, b.get_y() + b.get_height() / 2, f"{v:.4g}",
                         ha="left", va="center", fontsize=9)
            ax1.set_xlim(right=max(vals) * 1.25)
            ax1.invert_yaxis()  # fastest bar on top
        ax1.set_xlabel("ms (best of 5 runs)")
        ax1.set_title(f"{c}-{a} across languages, fastest first ({dataset}, n={n})")
        fig1.tight_layout()
        name = f"{stem}-{c}-{a}.png"
        split_path = os.path.join(out_dir, name)
        fig1.savefig(split_path, dpi=120)
        plt.close(fig1)
        split_names.append(name)
        print(f"Wrote {split_path}.")

    update_readme(out_dir, split_names)
    return 0


README_START = "<!-- PLOT-SPLITS:START -->"
README_END = "<!-- PLOT-SPLITS:END -->"


def update_readme(out_dir, split_names):
    """Rewrite the auto-generated image list in README.md (idempotent).

    Looks for README.md next to the benchmark/ dir and replaces everything
    between PLOT-SPLITS markers with one `![..](benchmark/...)` line per
    split image, so the 9 charts always land in the right place.
    """
    root = os.path.dirname(os.path.abspath(out_dir))
    readme = os.path.join(root, "README.md")
    try:
        with open(readme, "r", encoding="utf-8") as f:
            text = f.read()
    except OSError as e:
        print(f"warn: README update skipped (cannot read {readme}): {e}",
              file=sys.stderr)
        return
    if README_START not in text or README_END not in text:
        print("warn: README update skipped (markers not found)", file=sys.stderr)
        return
    bench_dir = os.path.basename(os.path.abspath(out_dir))
    lines = [f"![{os.path.splitext(n)[0].replace('plot-', '')}]"
             f"({bench_dir}/{n})" for n in split_names]
    block = README_START + "\n\n" + "\n".join(lines) + "\n\n" + README_END
    pre, _, rest = text.partition(README_START)
    _, _, post = rest.partition(README_END)
    with open(readme, "w", encoding="utf-8", newline="\n") as f:
        f.write(pre + block + post)
    print(f"Updated image list in {readme}.")


if __name__ == "__main__":
    raise SystemExit(main())
