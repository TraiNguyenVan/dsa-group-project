# Datasets

Benchmark datasets for the phonebook lookup CLI. All files share the same
schema and are nested prefixes of one seeded 1M-row stream, so scaling
results are comparable (`contacts_50` ⊂ `contacts_100k` ⊂ `contacts_200k` ⊂ `contacts_1m`).

| File               | Rows      | Approx. size |
| ------------------ | --------- | ------------ |
| `contacts_50.csv`  | 50        | ~1 KB        |
| `contacts_100k.csv`| 100,000   | ~2.4 MB      |
| `contacts_200k.csv`| 200,000   | ~4.7 MB      |
| `contacts_1m.csv`  | 1,000,000 | ~24 MB       |

Format: `Name,Phone` per line (no header). `Phone` is a unique zero-padded
10-digit string (digits only, e.g. `0174292433`). Names are `First Last`,
with ~10% in quoted `"Last, First"` form to exercise the CSV quote path.

## Regenerate

```sh
python3 data/generate.py --seed 42
```

`--seed` controls the RNG (default `42`); `--out-dir` controls the output
directory (default `data`). Re-running with the same seed reproduces
identical files.
