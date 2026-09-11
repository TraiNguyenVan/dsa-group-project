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

    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
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


def main():
    ap = argparse.ArgumentParser(description="Plot benchmark/results.csv (best of 5).")
    ap.add_argument("input", help="input CSV (benchmark/results.csv)")
    ap.add_argument("output", help="output PNG (benchmark/plot.png)")
    ap.add_argument("--line", action="store_true",
                    help="line chart of best-of-5 ms vs n (multi-size results.csv)")
    ap.add_argument("--case", choices=CASE_ORDER, default="last",
                    help="target case for the line chart (default: last)")
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

    # Two linear-scale panels (linear algo | hash algo): ratios within a
    # panel are honest, and the us-scale hash bars get their own axis
    # instead of being flattened by the ms-scale linear bars.
    # Missing combos -> NaN (gap, no bar).
    width = 0.8 / len(LANG_ORDER)
    panels = [
        ("Linear scan ms (best of 5)", [(c, "linear") for c in CASE_ORDER]),
        ("Hash lookup ms (best of 5)", [(c, "hash") for c in CASE_ORDER]),
    ]

    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
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
    fig.suptitle(f"Phone search: linear vs hash ({dataset}, n={n})")
    fig.tight_layout()
    fig.savefig(args.output, dpi=120)
    print(f"Wrote {args.output} ({len(best)} cells).")

    # 6 per-group charts: one PNG per (case, algo), horizontal bars sorted
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
    split image, so the 6 charts always land in the right place.
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
