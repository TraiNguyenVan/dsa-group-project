# Phonebook Search Benchmark

A PTIT Data Structures and Algorithms group project comparing **linear, hash,
and binary search** on the same phonebook data in C++, Python, Go, JavaScript,
and Java. The demo can load and save contact lists and add, delete, and
search contacts. The benchmark records runtime and memory measurements.

| Looking for | Start here |
| --- | --- |
| Assignment write-up and results | [Report (PDF)](report.pdf) · [Markdown source](report/report.md) |
| Presentation | [Slide deck and notes](slides/README.md) |
| Algorithms and benchmark methodology | [Documentation](docs/README.md) |
| Existing measurements | [Runtime CSV](benchmark/results.csv) · [Memory CSV](benchmark/mem/results.csv) · [Chart gallery](benchmark/gallery.html) |

## Quick start

Use a C++17 compiler and Make (or CMake 3.16+). The 50-contact sample is
included, so no dataset download or dependencies are needed for the C++ demo.

```sh
make run-50
```

Choose **1** to load contacts, **4** to search, **0** to run the interactive
benchmark, and **9** to exit. The search submenu also supports names and phone
prefixes. Scripts can pass the input and output paths explicitly:

```sh
./build/cpp/demo data/contacts_50.csv build/my-contacts.csv
```

The output defaults to the input path if omitted. Pass a separate output path
if you want to use the save option without changing the sample.

### CMake and Windows

```sh
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake --target run-50
```

On Windows with MSYS2 MinGW and CMake 3.21+, use
`cmake --preset mingw-release` then `cmake --build --preset mingw-release`.
Run `./build/demo.exe data/contacts_50.csv` from the project root, or add
`--target run-50` to the build command.

Make and CMake also provide `run-100k`, `run-200k`, and `run-1m`. These
targets use Python 3 to generate the requested dataset on first use. See the
[dataset guide](data/README.md) for size, schema, and seed details.

## Other implementations

Run these commands from the repository root. The ports expose the same menu
and accept the same input and output arguments.

| Language | Requirement | Run |
| --- | --- | --- |
| Python | Python 3 | `python3 python/phonebook/main.py data/contacts_50.csv` |
| Go | Go 1.21+ | `(cd go/phonebook && go run . ../../data/contacts_50.csv)` |
| JavaScript | Node.js 18+; no npm dependencies | `node javascript/phonebook/src/main.js data/contacts_50.csv` |
| Java | JDK 17+ | See below |

```sh
javac -d build/java java/phonebook/src/com/phonebook/*.java
java -cp build/java com.phonebook.Main data/contacts_50.csv
```

## Verify the project

```sh
make test                           # all five implementations
make test LANGUAGES="cpp python js"  # only the toolchains you have
```

The smoke test runs each real CLI: it loads the sample, checks the three
searches and a miss, inserts and deletes a contact, verifies the saved CSV, and
checks that the batch benchmark can write and append valid rows. It writes to a
temporary directory and does not modify the checked-in dataset or results.
The separate `tests/sec2/` and `tests/sec3/` folders hold input/answer fixtures
for the assignment grading scripts; they are not phonebook smoke tests.

## Reproduce the benchmarks

For a quick single-language run, enter menu option **0** or write a fresh batch CSV:

```sh
printf '0\n9\n' | make run-50
./build/cpp/demo --benchmark-csv build/results-cpp.csv data/contacts_50.csv
```

For the full five-language comparison, use `make run-benchmark` (100k contacts)
or `make run-benchmark-sizes` (50 through 1M). Use `make run-memory` for
profiling. These research targets are tested on Linux and can **overwrite
checked-in benchmark results and charts**. Run them in a separate worktree or
copy the published artifacts first if you want to keep both sets of numbers.
See the [benchmark runbook](docs/benchmark/05-running-benchmarks.md) for
dependencies and outputs.

To browse the charts locally, run `python3 -m http.server --directory benchmark 8000`
and open [the gallery](http://localhost:8000/gallery.html).

## Published measurement environment

> **Platform support:** the benchmark harness (`make run-benchmark` /
> `run-benchmark-sizes` / `run-memory`) was **tested on Linux only** —
> Windows/macOS have not been tested for benchmark comparability. The C++ program is verified on Windows
> via `cmake --preset mingw-release` (MSYS2 MinGW, manual build flags and
> CMake preset both checked); the Python/Go/JS/Java ports are expected to
> work on any OS with the runtime installed. Numbers from an untested
> platform are not comparable to the tables below.

| Component | Value |
| --- | --- |
| CPU | Intel Core i5-1135G7 @ 2.40 GHz (4 cores / 8 threads) |
| RAM | 15 GiB |
| OS | Omarchy (Arch Linux), kernel 7.2.3-arch1-3 |
| C++ | g++ 16.2.1 (GCC), `-O2` (see Makefile) |
| Python | Python 3.14.7 |
| Go | go1.27.1 linux/amd64 |
| Node | v26.7.0 |
| Java | javac 27 (JDK 27) |
| Dataset | `data/contacts_100k.csv` (n=100000), generated with seed 42 |
| Commit | `f673f1e7157fd09133d9a76d152e4a20f1c8f12b` |

The full measurement method, charts, and comparison tables are in
[`report.pdf`](report.pdf) (Part D). For the benchmark harness
and workflow walkthrough, see the docs guide in
[`docs/README.md`](docs/README.md).

## Repository map

- `src/` and `include/`: C++17 entry point (`main.cpp`), menus (`cli.cpp`),
  benchmark (`benchmark.cpp`), contact CSV I/O, and search algorithms.
- `python/`, `go/`, `javascript/`, and `java/`: equivalent ports.
- `data/`, `benchmark/`, and `tests/`: fixtures, generators, measurements, and checks.
- `docs/`, `report/`, `slides/`, and `resources/`: methodology and assignment deliverables.

See [CONTRIBUTING.md](CONTRIBUTING.md) for the team review, citation, and
AI-disclosure rules.
