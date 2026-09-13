# 02 — Datasets

All benchmarks run on the same CSV files so results are comparable across languages and sizes.

## Schema

One contact per line, **no header**, LF line endings:

```
Name,Phone
Nguyen Van An,0981234567
Tran Thi Binh,0912345678
```

| Field | Format | Example |
|-------|--------|---------|
| `Name` | `Family Middle Given` — Vietnamese pools, no commas/quotes | `Nguyen Van An` |
| `Phone` | 10 digits, unique, `prefix + 7 random digits` | `0981234567` |

Valid prefixes (28 Vietnamese mobile prefixes):

```
096 097 098 086 032 033 034 035 036 037 038 039
091 094 088 081 082 083 084 085 090 093 089
070 076 077 078 079
```

Phones are unique — address space is `28 × 10⁷ = 280M`, so 1M unique rows generate quickly.

## Files

| File | Rows | Approx. size |
|------|------|-------------|
| `data/contacts_50.csv` | 50 | ~1 KB |
| `data/contacts_10k.csv` | 10,000 | ~250 KB |
| `data/contacts_100k.csv` | 100,000 | ~2.6 MB |
| `data/contacts_200k.csv` | 200,000 | ~5.1 MB |
| `data/contacts_500k.csv` | 500,000 | ~13 MB |
| `data/contacts_1m.csv` | 1,000,000 | ~26 MB |

All six are **nested prefixes** of a single 1M-row master stream:

```
contacts_50 ⊂ contacts_10k ⊂ contacts_100k ⊂ contacts_200k ⊂ contacts_500k ⊂ contacts_1m
```

This means scaling benchmarks are apples-to-apples — `contacts_100k` is literally the first 100k rows of `contacts_1m`.

## Generator

Source: `data/generate.py` (stdlib only).

```sh
python3 data/generate.py --seed 42            # default: writes to data/
python3 data/generate.py --seed 42 --out-dir data
python3 data/generate.py --seed 123 --out-dir benchmark/output
```

- `--seed` controls the RNG (default `42`). Same seed → identical files, byte-for-byte, across runs and Python versions (phones are appended in RNG order, not via `set` iteration).
- `--out-dir` controls the output directory (default `data`).
- The generator is called automatically by `make run-benchmark-sizes` and `benchmark/mem_profile.py`, so you rarely need to run it by hand.

### How it works (simplified)

```python
rng = random.Random(seed)
# 1. Generate 1M unique phones: random prefix + 7 random digits
phone_list = []
seen = set()
while len(phone_list) < 1_000_000:
    p = rng.choice(PREFIXES) + f"{rng.randrange(10_000_000):07d}"
    if p not in seen:
        seen.add(p); phone_list.append(p)
# 2. Generate 1M names: Family + Middle + Given
rows = [(f"{rng.choice(FAMILY)} {rng.choice(MIDDLE)} {rng.choice(GIVEN)}", phone_list[i])
        for i in range(1_000_000)]
# 3. Write prefixes: rows[:50], rows[:10_000], …, rows[:1_000_000]
```

## Using datasets

```sh
# Quick demo (50 rows, instant)
./build/cpp/demo data/contacts_50.csv
python3 python/phonebook/main.py data/contacts_50.csv

# Scaling (100k — the default if you omit the arg)
./build/cpp/demo data/contacts_100k.csv
./build/cpp/demo                          # same as above

# Custom output file (second positional arg)
./build/cpp/demo data/contacts_100k.csv benchmark/results.csv
```

> **Tip:** use `contacts_50.csv` for demos and correctness checks (you can print all 50). Use `100k`+ for timing — small `n` is dominated by overhead and timer noise.

## Verifying a dataset

```sh
head -5 data/contacts_100k.csv
wc -l data/contacts_100k.csv          # should match the file's n
cut -d, -f2 data/contacts_100k.csv | sort | uniq -d | head  # should be empty (unique phones)
```

---

Next: [03 — Timing Harness](03-timing-harness.md) — how `timeIt` / `benchmark` work.
