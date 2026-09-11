# Phonebook Lookup — C++ (source of truth) + Python/Go/JS/Java for comparison

CLI phonebook that compares **linear search vs hash search** on phone numbers.
The C++ program is the graded deliverable. The four ports exist only so the
teacher can compare the same logic across languages.

Source of truth: `src/main.cpp`, `src/phonebook.cpp`, `src/hashtable.cpp`,
`include/phonebook.hpp`, `include/hashtable.hpp`, `include/timer.hpp`,
`include/contact.hpp`.

## Repo layout

```text
src/main.cpp             CLI: menu 0-8, benchmark, search sub-menu
src/phonebook.cpp        CSV, insert/validate, linear + hash search, print
src/hashtable.cpp        chained hash table, prime sizing, rehash
include/timer.hpp        timeIt / benchmark(best-of-5) / printTaskDuration
data/                    datasets + generator (see data/README.md)

python/phonebook/        main.py, phonebook.py, hashtable.py, timer.py, contact.py
go/phonebook/            main.go, phonebook.go, hashtable.go, timer.go, contact.go, go.mod
javascript/phonebook/    src/main.js, src/phonebook.js, src/hashtable.js, src/timer.js, src/contact.js
java/phonebook/          src/com/phonebook/Main.java, PhoneBook.java, HashTable.java, Timer.java, Contact.java
```

## C++ — build and run

Prereqs: `g++` with C++17, or CMake 3.16+.

```sh
# Makefile (mirrors CMakeLists.txt)
make
./build/cpp/demo data/contacts_50.csv
make run-50      # 50 rows
make run-100k    # 100k rows
make run-200k
make run-1m

# CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/demo data/contacts_100k.csv
cmake --build build --target run-50
```

CLI args: `demo [csvInput] [csvOutput]`.
Default `csvInput = data/contacts_100k.csv`, `csvOutput = csvInput`.

Menu (`src/main.cpp:65-83`):

```text
0. Benchmark phone search (5 runs, best)
1. Load contacts from CSV
2. Save contacts to CSV
3. Insert contact
4. Search contact (1=phone linear, 2=phone hash, 3=name linear, 4=back)
5. Print all contacts
6. Print contact by index
7. Show number of contacts
8. Delete contact by phone (O(n) erase + rebuild, preserves order)
9. Exit
```

Bad input prints `Invalid input.` and reprompts. EOF prints `Goodbye`.

## C++ behavior (ports copy this)

* **CSV** (`src/phonebook.cpp:30-83`): quoted `"Do, Thanh Tuan"` with `""`
  escapes, unquoted split on **last** comma, trim spaces, strip wrapping
  quotes on phone, drop trailing `\r`, skip blank lines, skip rows with
  empty name or non-digit phone.
* **Insert** (`src/phonebook.cpp:143-164`): reject empty, reject
  non-digit phone (`Invalid phone number`), reject duplicate phone via
  hash (`Phone number is already exist`), else store
  `CapitalizeFirst(ToLower(name))` (ASCII-only) + index in hash table.
* **Delete** (`deleteContactByPhone`, option `8`): O(n) — vector erase
  shifts the tail + full hash rebuild so order is preserved (usual
  DSA-course version). Missing phone prints `Phone number not found`.
* **Search:** `searchLinearByPhone` exact scan, `searchHashByPhone` chained
  lookup, `searchLinearByName` case-insensitive scan via `toLower`.
* **Hash** (`src/hashtable.cpp`): `hash = hash*31 + (unsigned char)(c-'0')`
  with 64-bit wrap, `% numBuckets`; start `101 → nextPrime`; rehash at
  `load > 0.75` to `nextPrime(2x)`; `isPrime` uses `6k±1`.
* **Timer** (`include/timer.hpp`): `timeIt` ms, `benchmark(work, 5)`
  best-of-5, `printTaskDuration` prints `\nTook: Xms.`
* **Benchmark** (`src/main.cpp:11-63`, option `0`): auto-loads CSV if empty,
  picks `first / random / last` phones, times linear vs hash 5 runs each.

## Ports — per-language detail (comparison only)

Same prompts, same messages, same file split as C++. Differences are only
what the runtime forces (64-bit wrap, stdin, timing API).

| C++ | Python | Go | JavaScript (Node) | Java |
| --- | --- | --- | --- | --- |
| `src/main.cpp` | `python/phonebook/main.py` | `go/phonebook/main.go` | `javascript/phonebook/src/main.js` | `java/phonebook/src/com/phonebook/Main.java` |
| `src/phonebook.cpp` + `include/phonebook.hpp` | `phonebook.py` | `phonebook.go` | `src/phonebook.js` | `PhoneBook.java` |
| `src/hashtable.cpp` + `include/hashtable.hpp` | `hashtable.py` | `hashtable.go` | `src/hashtable.js` | `HashTable.java` |
| `include/timer.hpp` | `timer.py` | `timer.go` | `src/timer.js` | `Timer.java` |
| `include/contact.hpp` | `contact.py` | `contact.go` | `src/contact.js` | `Contact.java` |

### Python

Prereq: `python3` (stdlib only, no deps).

```sh
python3 python/phonebook/main.py data/contacts_50.csv
python3 python/phonebook/main.py data/contacts_100k.csv [csvOutput]
```

Notes: `time.perf_counter`, `random.randint`, 64-bit wrap via
`& 0xFFFFFFFFFFFFFFFF`, `(ord(c)-48) & 0xFF` to match
`(unsigned char)(c-'0')`.

### Go

Prereq: `go 1.21+`.

```sh
cd go/phonebook
go vet ./...
go run . ../../data/contacts_50.csv
go run . ../../data/contacts_100k.csv ../../out.csv
```

Notes: `uint64` wraps like C++ `size_t`, `time.Now`, `math/rand`,
`bufio.Reader` line input.

### JavaScript (Node)

Prereq: `node >=18`, no `npm install` needed.

```sh
cd javascript/phonebook
node src/main.js ../../data/contacts_50.csv
node src/main.js ../../data/contacts_100k.csv ../../out.csv
```

Notes: `perf_hooks.performance.now`, `Math.random`,
`readline/promises` async CLI, `BigInt` with `& 0xFFFFFFFFFFFFFFFFn`
for hash wrap, `fs.readFileSync`.

### Java

Prereq: JDK 17. In this workspace: `nix-shell -p openjdk17`.

```sh
cd java/phonebook
javac -d out src/com/phonebook/*.java
java -cp out com.phonebook.Main ../../data/contacts_50.csv
java -cp out com.phonebook.Main ../../data/contacts_100k.csv ../../out.csv
```

Notes: `System.nanoTime()/1e6`, `java.util.Random`, `Scanner`
`hasNextInt/nextLine` mirroring `cin >> choice`, `long` wrap +
`Long.remainderUnsigned` for hash mod. `out/` is git-ignored.

## Datasets

See `data/README.md`. `Name,Phone` per line, no header. Regenerate with
`python3 data/generate.py --seed 42`. Use `contacts_50.csv` for demos,
`100k/200k/1m` for scaling.

## Perf claims — method + data (no invented numbers)

Option `0` is unchanged: per target (`first/random/last`) it prints best-of-5:

```text
[role index N phone P]
  Linear best of 5: Xms. (index N)
  Hash best of 5: Yms. (index N, position-independent)
```

For the report, don't hand-copy that output — run the full matrix:

```sh
make run-benchmark   # all 5 langs, every run -> benchmark/results.csv (overwrite)
```

Each program also supports batch mode directly (same 30 rows each):

```sh
./build/cpp/demo --benchmark-csv benchmark/results.csv data/contacts_100k.csv
python3 python/phonebook/main.py --benchmark-csv benchmark/results.csv --append data/contacts_100k.csv
cd go/phonebook && go run . --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv
cd javascript/phonebook && node src/main.js --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv
cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main --benchmark-csv ../../benchmark/results.csv --append ../../data/contacts_100k.csv
```

`benchmark/results.csv` columns:
`language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone`
— 150 rows (5 langs × first/random/last × linear/hash × 5 runs).
Batch mode seeds RNG with 42 so the `random` target is reproducible
*within* a language (each language's RNG differs, so targets differ
across languages — `target_index,phone` columns make that auditable).
Option `0` stays unseeded.

`make run-benchmark` also draws `benchmark/plot.png` at the end via
`benchmark/plot.py` (two linear-scale panels — linear scan | hash lookup —
best of 5 per cell with value labels; split axes so bar heights stay in
true ratio and the µs hash bars aren't flattened by the ms linear bars; needs matplotlib —
without it the plot step prints `plot skipped` and the CSV is still
produced). The script parses every language's `ms` float format
(C++ `setprecision(17)`, Python `repr`, Go `%g`, JS double, Java
`Double.toString`) defensively: bad rows are skipped with a warning,
never a crash.

![Phone search benchmark: linear vs hash across 5 languages](benchmark/plot.png)

![first-linear](benchmark/plot-first-linear.png)
![first-hash](benchmark/plot-first-hash.png)
![random-linear](benchmark/plot-random-linear.png)
![random-hash](benchmark/plot-random-hash.png)
![last-linear](benchmark/plot-last-linear.png)
![last-hash](benchmark/plot-last-hash.png)

### Reading the chart fairly

**What is being compared.** Same handwritten algorithm, same input, same
machine: a hand-rolled chained hash table and a hand-rolled linear scan
over `contacts_100k.csv` (n=100000) in all five languages — no `dict`,
`map`, or `HashMap` anywhere. This is a *language + runtime* comparison
of equivalent code. It is deliberately **not** hand-written C++ vs
Python's built-in `dict`: that would compare a teaching implementation
against a production hash table (open addressing, optimized C) and would
answer a different question.

**Same position, except `random`.** `first` (index 0) and `last`
(index n−1) hit the identical phone in every language, so those bars
are directly comparable. The `random` target is seeded (42) but each
language has its own RNG, so the index differs per language — check
`target_index` in the CSV before comparing `random-*` bars across
languages.

**Explaining the gap, not just observing it.** "Python is slower" is an
observation; the mechanism is the answer:

* **C++ (`-O2`)** is the baseline: `vector<Contact>` stores structs
  contiguously, short strings sit inside the object (SSO), and the scan
  compiles to a tight machine-code loop with no per-element overhead.
* **Python** pays per element: every `Contact`, `str`, and `int` is a
  heap-boxed `PyObject` reached through a pointer, and each loop
  iteration runs CPython bytecode dispatch plus attribute-dict lookups.
  That is why `last-linear` is ~10× the others — 100k interpreter steps
  vs 100k machine instructions. The hash bars stay small because O(1)
  steps mean the per-op tax applies ~once, not 100k times.
* **Go** is compiled with value-type slices, so its scan is a real
  machine loop (bounds checks are nearly free here) — expect it closest
  to C++. Its gap is runtime checks + GC write barriers, not
  interpretation.
* **JavaScript (V8)** stores an array of heap objects (pointer chasing +
  hidden-class checks per access) and must JIT-warm: best-of-5 lets the
  optimizing compiler settle, but run-to-run spread is JIT tier-up, not
  noise in your code.
* **Java** is the same story on the JVM: `ArrayList<Contact>` holds
  references, the C2 compiler kicks in after ~10k loop iterations (one
  100k scan trips it), and G1 may pause — startup is *not* measured,
  only the timed search, which flatters Java fairly since all languages
  get the same treatment.

**What you may claim.** The portable, algorithmic claim is the *trend
within each language*: linear grows first→last, hash stays ~constant —
it holds in all five, which is the DSA point. Cross-language magnitude
differences are runtime properties; cite them only with the mechanism
above, plus your machine spec (CPU/RAM, OS, toolchains, commit).

Per `CONTRIBUTING.md`, a timing without conditions is not a measurement.
Do not compare ms across languages (different runtimes/timers). The only
portable claim is the intra-run trend: linear grows first→last, hash stays
~constant.

Copy this table and fill from `benchmark/results.csv` + your machine spec:

| Run | Dataset | CPU/RAM, OS | Toolchain | Commit | n | target | Linear runs (ms) | Hash runs (ms) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C++ ex | `contacts_100k` | | `g++ --version`, `-O2` | | 100000 | first/random/last | | |
| py ex | `contacts_100k` | | `python3 --version` | | 100000 | first/random/last | | |
| go ex | `contacts_100k` | | `go version` | | 100000 | first/random/last | | |
| js ex | `contacts_100k` | | `node -v` | | 100000 | first/random/last | | |
| java ex | `contacts_100k` | | `javac 17` | | 100000 | first/random/last | | |

Interactive reproduce (option 0):

```sh
printf '0\n9\n' | ./build/cpp/demo data/contacts_50.csv
printf '0\n9\n' | python3 python/phonebook/main.py data/contacts_50.csv
printf '0\n9\n' | (cd go/phonebook && go run . ../../data/contacts_50.csv)
printf '0\n9\n' | (cd javascript/phonebook && node src/main.js ../../data/contacts_50.csv)
printf '0\n9\n' | (cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main ../../data/contacts_50.csv)
```

## Contributing

See `CONTRIBUTING.md` (branches, small commits, `Fixes #N`, run before
review, cite sources, declare AI use).
