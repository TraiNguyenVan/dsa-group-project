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
8. Exit
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

## Perf claims — method + template (no invented numbers)

Option `0` reports, per target (`first/random/last`):

```text
[role index N phone P]
  Linear best of 5: Xms. (index N)
  Hash best of 5: Yms. (index N, position-independent)
```

Per `CONTRIBUTING.md`, a timing without conditions is not a measurement.
Do not compare ms across languages (different runtimes/timers). The only
portable claim is the intra-run trend: linear grows first→last, hash stays
~constant.

Copy this table and fill with real runs:

| Run | Dataset | CPU/RAM, OS | Toolchain | Commit | n | target | Linear best 5 (ms) | Hash best 5 (ms) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| C++ ex | `contacts_50` | | `g++ --version`, `-O2` | | 50 | first/random/last | / / | / / |
| py ex | `contacts_50` | | `python3 --version` | | 50 | first/random/last | / / | / / |
| go ex | `contacts_50` | | `go version` | | 50 | first/random/last | / / | / / |
| js ex | `contacts_50` | | `node -v` | | 50 | first/random/last | / / | / / |
| java ex | `contacts_50` | | `javac 17` | | 50 | first/random/last | / / | / / |

Reproduce:

```sh
printf '0\n8\n' | ./build/cpp/demo data/contacts_50.csv
printf '0\n8\n' | python3 python/phonebook/main.py data/contacts_50.csv
printf '0\n8\n' | go run . ../../data/contacts_50.csv
printf '0\n8\n' | node src/main.js ../../data/contacts_50.csv
printf '0\n8\n' | java -cp out com.phonebook.Main ../../data/contacts_50.csv
```

## Contributing

See `CONTRIBUTING.md` (branches, small commits, `Fixes #N`, run before
review, cite sources, declare AI use).
