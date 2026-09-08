#!/usr/bin/env python3
"""Generate phonebook benchmark datasets (reproducible, seeded).

Datasets are nested prefixes of a single 1M-row master stream so scaling
benchmarks are comparable: contacts_50 ⊂ contacts_100k ⊂ contacts_200k ⊂ 1m.

Usage:
    python3 data/generate.py [--seed 42] [--out-dir data]

Output:
    data/contacts_50.csv      (50 rows)
    data/contacts_100k.csv    (100,000 rows)
    data/contacts_200k.csv    (200,000 rows)
    data/contacts_1m.csv      (1,000,000 rows)

Format: Name,Phone per line. Names with a comma are quoted ("Last, First").
Phones are unique zero-padded 10-digit strings (digits only).
"""

import argparse
import csv
import random
from pathlib import Path

FIRST_NAMES = [
    "Alice", "Amir", "Beth", "Bob", "Carl", "Charlie", "David", "Diana",
    "Eli", "Eva", "Fiona", "Frank", "George", "Grace", "Hannah", "Henry",
    "Ivan", "Ivy", "Jack", "Jade", "Kate", "Kevin", "Leo", "Lily",
    "Marco", "Mia", "Nina", "Noah", "Olivia", "Oscar", "Paul", "Penny",
    "Quinn", "Ray", "Rosa", "Sam", "Sophia", "Tina", "Tom", "Uma",
    "Una", "Victor", "Vince", "Wendy", "Xavier", "Xena", "Yara", "Yusuf",
    "Zane", "Zoe",
]

LAST_NAMES = [
    "Allen", "Anderson", "Brown", "Bui", "Clark", "Dang", "Davis", "Do",
    "Flores", "Garcia", "Gonzalez", "Harris", "Hernandez", "Hill", "Hoang",
    "Jackson", "Johnson", "Jones", "King", "Le", "Lee", "Lewis", "Lopez",
    "Martin", "Martinez", "Miller", "Moore", "Nguyen", "Perez", "Pham",
    "Phan", "Ramirez", "Robinson", "Rodriguez", "Sanchez", "Scott", "Smith",
    "Taylor", "Thomas", "Thompson", "Torres", "Tran", "Vu", "Walker",
    "White", "Williams", "Wilson", "Wright", "Young",
]

# (filename, rows) — kept in ascending order so each file is a prefix of the next.
DATASETS = [
    ("contacts_50.csv", 50),
    ("contacts_100k.csv", 100_000),
    ("contacts_200k.csv", 200_000),
    ("contacts_1m.csv", 1_000_000),
]

QUOTED_FRACTION = 0.10  # ~10% "Last, First" rows, exercises the CSV quote path


def main() -> None:
    ap = argparse.ArgumentParser(description="Generate phonebook datasets.")
    ap.add_argument("--seed", type=int, default=42, help="RNG seed (default: 42)")
    ap.add_argument("--out-dir", default="data", help="Output directory (default: data)")
    args = ap.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rng = random.Random(args.seed)
    max_n = max(n for _, n in DATASETS)

    # Unique 10-digit phones (zero-padded, e.g. 0174292433 is valid).
    phones = rng.sample(range(10_000_000_000), max_n)

    rows: list[tuple[str, str]] = []
    for i in range(max_n):
        first = rng.choice(FIRST_NAMES)
        last = rng.choice(LAST_NAMES)
        if rng.random() < QUOTED_FRACTION:
            name = f"{last}, {first}"
        else:
            name = f"{first} {last}"
        rows.append((name, f"{phones[i]:010d}"))

    for filename, n in DATASETS:
        path = out_dir / filename
        with open(path, "w", newline="") as f:
            writer = csv.writer(f, quoting=csv.QUOTE_MINIMAL)
            writer.writerows(rows[:n])
        print(f"Wrote {n:>8,} rows -> {path} (seed={args.seed})")


if __name__ == "__main__":
    main()
