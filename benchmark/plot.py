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
import sys

LANG_ORDER = ["cpp", "python", "go", "js", "java"]
CASE_ORDER = ["first", "random", "last"]
ALGO_ORDER = ["linear", "hash"]
GROUPS = [(c, a) for c in CASE_ORDER for a in ALGO_ORDER]


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


def load_best(path):
    """Return (best[(lang, case, algo)], meta). best = min of runs."""
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


def main():
    ap = argparse.ArgumentParser(description="Plot benchmark/results.csv (best of 5).")
    ap.add_argument("input", help="input CSV (benchmark/results.csv)")
    ap.add_argument("output", help="output PNG (benchmark/plot.png)")
    args = ap.parse_args()

    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("error: matplotlib not installed (pip install matplotlib)", file=sys.stderr)
        return 2

    try:
        best, dataset, n = load_best(args.input)
    except FileNotFoundError:
        print(f"error: input not found: {args.input}", file=sys.stderr)
        return 1
    except OSError as e:
        print(f"error: cannot read {args.input}: {e}", file=sys.stderr)
        return 1

    if not best:
        print("error: no plottable rows found", file=sys.stderr)
        return 1

    xlabels = [f"{c}-{a}" for c, a in GROUPS]
    x = range(len(GROUPS))
    width = 0.8 / len(LANG_ORDER)

    # Log y-axis: linear cases (~ms) dwarf hash cases (~µs) on a linear
    # scale. Missing combos -> NaN (no bar) so log never sees 0.
    import math

    positives = [v for v in best.values() if v > 0]
    floor = min(positives) / 5 if positives else 1e-6

    fig, ax = plt.subplots(figsize=(12, 6))
    for i, lang in enumerate(LANG_ORDER):
        vals = [best.get((lang, c, a), float("nan")) for c, a in GROUPS]
        offs = [p + (i - (len(LANG_ORDER) - 1) / 2) * width for p in x]
        bars = ax.bar(offs, vals, width=width, label=lang, log=True)
        for b, v in zip(bars, vals):
            if v == v and v > 0:  # not NaN
                ax.text(b.get_x() + b.get_width() / 2, v * 1.15, f"{v:.2g}",
                        ha="center", va="bottom", fontsize=7, rotation=45)

    ax.set_xticks(list(x))
    ax.set_xticklabels(xlabels, rotation=15)
    ax.set_yscale("log")
    ax.set_ylim(bottom=floor)
    ax.set_ylabel("ms (best of 5 runs, log scale)")
    ax.set_title(f"Phone search: linear vs hash ({dataset}, n={n})")
    ax.legend()
    fig.tight_layout()
    fig.savefig(args.output, dpi=120)
    print(f"Wrote {args.output} ({len(best)} cells).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
