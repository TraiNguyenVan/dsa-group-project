# Datasets

Benchmark datasets for the phonebook lookup CLI. All files share the same
schema and are nested prefixes of one seeded 1M-row stream, so scaling
results are comparable (`contacts_50` ⊂ `contacts_100k` ⊂ `contacts_200k` ⊂ `contacts_1m`).

| File               | Rows      | Approx. size |
| ------------------ | --------- | ------------ |
| `contacts_50.csv`  | 50        | ~1 KB        |
| `contacts_100k.csv`| 100,000   | ~2.6 MB      |
| `contacts_200k.csv`| 200,000   | ~5.1 MB      |
| `contacts_1m.csv`  | 1,000,000 | ~26 MB       |

Format: `Name,Phone` per line (no header, LF line endings). `Phone` is a
unique 10-digit string (digits only): one of the Vietnamese mobile prefixes
below plus 7 random digits (e.g. `0981234567`). Names are plain Vietnamese
`Family Middle Given` (e.g. `Nguyen Van An`), no commas or quotes.

Valid prefixes: `096, 097, 098, 086, 032, 033, 034, 035, 036, 037, 038,
039, 091, 094, 088, 081, 082, 083, 084, 085, 090, 093, 089, 070, 076,
077, 078, 079`.

## Regenerate

```sh
python3 data/generate.py --seed 42
```

`--seed` controls the RNG (default `42`); `--out-dir` controls the output
directory (default `data`). Re-running with the same seed reproduces
identical files.
