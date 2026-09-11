#!/usr/bin/env python3
"""Generate phonebook benchmark datasets (reproducible, seeded).

Datasets are nested prefixes of a single 1M-row master stream so scaling
benchmarks are comparable: contacts_50 ⊂ contacts_100k ⊂ contacts_200k ⊂ 1m.

Usage:
    python3 data/generate.py [--seed 42] [--out-dir data]

Output:
    data/contacts_50.csv      (50 rows)
    data/contacts_10k.csv     (10,000 rows)
    data/contacts_100k.csv    (100,000 rows)
    data/contacts_200k.csv    (200,000 rows)
    data/contacts_500k.csv    (500,000 rows)
    data/contacts_1m.csv      (1,000,000 rows)

Format: Name,Phone per line. Phones are unique 10-digit strings (digits only) starting with one of the
Vietnamese mobile prefixes (prefix + 7 random digits, e.g. 0981234567).
Names are Vietnamese Family Middle Given (e.g. Nguyen Van An).
"""

import argparse
import csv
import random
from pathlib import Path

PREFIXES = [
    "096", "097", "098", "086",
    "032", "033", "034", "035", "036", "037", "038", "039",
    "091", "094", "088",
    "081", "082", "083", "084", "085",
    "090", "093", "089",
    "070", "076", "077", "078", "079",
]

# Vietnamese name pools: full name = Family Middle Given (e.g. Nguyen Van An).
FAMILY_NAMES = [
    "Nguyen", "Tran", "Le", "Pham", "Hoang", "Huynh", "Phan", "Vu",
    "Vo", "Dang", "Bui", "Do", "Ho", "Ngo", "Duong", "Ly",
]

MIDDLE_NAMES = [
    "Van", "Thi", "Huu", "Duc", "Thanh", "Ngoc", "Quang", "Minh",
]

GIVEN_NAMES = [
    "An", "Anh", "Bao", "Binh", "Chi", "Cuong", "Dung", "Duc",
    "Hai", "Hanh", "Hieu", "Hoa", "Hung", "Huong", "Khanh", "Lan",
    "Linh", "Long", "Mai", "Minh", "Nam", "Ngoc", "Phong", "Phuc",
    "Quang", "Son", "Thanh", "Thao", "Trang", "Tuan", "Viet", "Yen",
    "Khoa", "Phuong", "Tung", "Dat", "Kien", "Huy", "Thuy", "Nga",
]

# (filename, rows) — kept in ascending order so each file is a prefix of the next.
DATASETS = [
    ("contacts_50.csv", 50),
    ("contacts_10k.csv", 10_000),
    ("contacts_100k.csv", 100_000),
    ("contacts_200k.csv", 200_000),
    ("contacts_500k.csv", 500_000),
    ("contacts_1m.csv", 1_000_000),
]


def main() -> None:
    ap = argparse.ArgumentParser(description="Generate phonebook datasets.")
    ap.add_argument("--seed", type=int, default=42, help="RNG seed (default: 42)")
    ap.add_argument("--out-dir", default="data", help="Output directory (default: data)")
    args = ap.parse_args()

    out_dir = Path(args.out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    rng = random.Random(args.seed)
    max_n = max(n for _, n in DATASETS)

    # Unique 10-digit phones: uniform-random VN prefix + 7 random digits.
    # Address space is 28 * 10^7 = 280M, so 1M unique rows resolve fast.
    # NOTE: append in RNG order (not via set iteration) to keep output
    # deterministic for a given seed across runs / Python versions.
    phone_list: list[str] = []
    seen: set[str] = set()
    while len(phone_list) < max_n:
        p = rng.choice(PREFIXES) + f"{rng.randrange(10_000_000):07d}"
        if p not in seen:
            seen.add(p)
            phone_list.append(p)

    rows: list[tuple[str, str]] = []
    for i in range(max_n):
        family = rng.choice(FAMILY_NAMES)
        middle = rng.choice(MIDDLE_NAMES)
        given = rng.choice(GIVEN_NAMES)
        name = f"{family} {middle} {given}"
        rows.append((name, phone_list[i]))

    for filename, n in DATASETS:
        path = out_dir / filename
        with open(path, "w", newline="") as f:
            writer = csv.writer(f, quoting=csv.QUOTE_MINIMAL, lineterminator="\n")
            writer.writerows(rows[:n])
        print(f"Wrote {n:>8,} rows -> {path} (seed={args.seed})")


if __name__ == "__main__":
    main()
