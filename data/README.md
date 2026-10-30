# Benchmark datasets

Only `contacts_50.csv` is checked into Git for quick demos and examples. The
larger benchmark datasets are generated locally when needed and are ignored by
Git, so they do not add weight to clones or commits.

## Generate datasets

```sh
python3 data/generate.py --seed 42
```

This writes nested-prefix CSVs for 50, 10k, 100k, 200k, 500k, and 1m contacts.
The 50 row file is tracked; the other CSVs are generated on demand. Use
`--out-dir` to write them elsewhere. Re-running with the same seed reproduces
the same rows.

The benchmark and memory profiling targets run the generator when needed. For
manual runs on datasets larger than 50 rows, generate them first.

## Format

Each row is `Name,Phone` with no header and LF line endings. Phone numbers are
unique 10 digit values using Vietnamese mobile prefixes; names use the form
`Family Middle Given`.
