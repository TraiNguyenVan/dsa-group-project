# data/

Dataset(s) for the demo, or the generator script that produces them.

- Must be **real, randomized, correctly sized** — n ≥ 100 000 (visible at demo time via `head data.csv`).
- Generator should be reproducible (fixed seed) so timings are comparable across the 5 languages.
- Keep generated files out of git (see `.gitignore`); commit the generator instead.

## Dataset plan (fill in)

- Problem dataset: `data/generate.py` → `data/dataset.csv` (n = ___, seed = ___)
- Small subset for correctness demo (n = 50): generated at runtime or checked in
- Edge cases: empty file, duplicate keys, out-of-range access (scripts under `src/` or here)
