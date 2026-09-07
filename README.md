<<<<<<< HEAD
=======
# DSA Group Project — PTIT INT1306_CLC

Mid-term group project · 20% of final mark · HK I 2026-2027
Faculty of Information Technology 2, PTIT HCMC Campus

**Topic: G4 — Searching & Hash Tables** (syllabus §2.2)
Linear · Binary · Interpolation search · hashing & collisions.

**Status: early development.** The C++ side has started — the three search algorithms
and three hash-table collision strategies compile and run. Everything else in this repo
is still the template scaffold. Nothing here is presentation-ready yet; see
[Progress against the self-check](#progress-against-the-self-check-pdf-13).

**Demo problem:** not yet chosen. The guide suggests *autocomplete / phone-book lookup
with a hash table* for G4 (§5.2). Any problem must be approved by the lecturer before
we build it.

**Everyone: read [CONTRIBUTING.md](CONTRIBUTING.md) before your first commit.**

## Deliverables (all six)

Points from the assessment breakdown (§10). Status reflects what is in the repo today,
not what is planned.

| Part | Pts | What | Where | Status |
|------|-----|------|-------|--------|
| A | 10 | Position & purpose of the data structures | `docs/PART_A_position.md` | Not started — placeholder |
| B | 20 | Algorithms & complexity table (best/avg/worst/space, justified) | `docs/PART_B_complexity.md` | Not started — placeholder |
| C | 25 | Real-world C++ demo — must compile, run on n ≥ 100 000, print timings, handle edge cases | `src/cpp/` | **In progress** — searches + hash variants compile; no dataset, timing or edge cases yet |
| D | 20 | Cross-language comparison (Python, Java, JavaScript, Go) | `src/<lang>/` + `docs/PART_D_comparison.md` | Not started — all four dirs are empty |
| E | 10 | Exercises for the class (3–5, with marking scheme) | `exercises/` | Not started — template only |
| F | 15 | Peer grading with evidence pack | `grading/` | Scaffolded — marking sheet template exists |

## Layout

```
├── README.md                  ← this file: build & run, expected output, machine/compiler
├── CONTRIBUTING.md            ← branching, commits, PRs, definition of done
├── REQUIREMENTS_FROM_PDF.md   ← every requirement extracted from the guide
├── C0B_Group_Project_Guide.pdf ← the assignment brief (source of truth)
├── docs/                      ← write-ups for parts A, B, D
├── src/                       ← C++ demo + ports to Python, Java, JavaScript, Go
├── data/                      ← dataset or generator script (n ≥ 100 000) — planned, empty
├── slides/                    ← presentation (English, sent 24 h in advance)
├── report/                    ← report.pdf (6–10 pages: parts A, B, D + charts)
├── exercises/                 ← published after the presentation (deadline ≥ 1 week)
├── grading/                   ← evidence pack, added after exercises are marked
└── build/                     ← compiler output, gitignored
```

### `data/` — planned, not yet in use

`data/` is still where the real dataset and its generator will live; the guide requires
the repo to ship either the dataset or the script that produces it (§11), at n ≥ 100 000
(§5.1). That intent has not changed.

It is just not wired up yet. Today the directory holds only its own `README.md` — no
generator, no dataset, and no code path in `src/cpp` reads from it. `.gitignore` already
anticipates the eventual contents: it ignores `data/*.csv`, `*.bin` and `*.dat` while
whitelisting `data/README.md` and a `data/generate.py` that does not exist yet.

Until that lands, the demo runs on hardcoded arrays and self-check item 6 below stays
unticked.

## Build & run (C++ demo)

> **Provisional.** There is no official build command yet — the project is in early
> development and the entry point will change once the real demo problem is chosen.
> What follows is what works today.

```bash
cd src/cpp
make run        # g++ -std=c++17 -Wall -O2 → ../../build/cpp/demo, then runs it
```

`make` alone only builds; the binary lands in `build/cpp/demo`, not in `src/cpp`, so
there is no `./demo` to run from here. Use `make run`, or `make clean` to remove the
build directory.

The guide requires `g++ -std=c++17 -Wall` (§5.1). We also pass `-O2`, which must be
declared under optimisation flags below.

`make edge` exists but does nothing useful yet — it passes `--empty`, `--dups` and
`--oob`, and `main.cpp` currently ignores all three.

### Expected output

`main.cpp` searches two hardcoded 10-element arrays for the value `67`, which is in
neither, so all three searches report a miss:

```
Array for searching demo(Randomized): 12 4 9 22 7 15 3 18 11 6
Array for searching demo(Sorted): 2 5 8 11 14 17 20 23 26 29
Searching with Linear Search: Target(67)
Value not found!
Searching with Binary Search: Target(67)
Value not found!
Searching with Interpolation Search: Target(67)
Value not found!
```

For Part C this output still has to grow into the reporting format the guide asks for
(§5.3): input size n, operation count, wall-clock time as best of 5 runs, and peak
memory where relevant. None of that is emitted yet.

### Hash tables (scratch)

The three collision strategies live in `src/cpp/hashing/`, each with its own `main()`.
`make all` builds them alongside the demo, or compile one directly:

```bash
g++ -std=c++17 -Wall -I src/cpp src/cpp/hashing/division_method.cpp    -o build/cpp/division_method     # no collision handling
g++ -std=c++17 -Wall -I src/cpp src/cpp/hashing/linear_probing.cpp     -o build/cpp/linear_probing      # open addressing
g++ -std=c++17 -Wall -I src/cpp src/cpp/hashing/separate_chaining.cpp  -o build/cpp/separate_chaining   # separate chaining
```

These are exploratory. They are not wired into the demo and are not edge-case hardened.

Deliberately blank until we have real measurements to report — §5.4 requires these, and
§12 treats fabricated timings as fabricated data.

**Test conditions to record here (required — a measurement without its conditions is not a measurement):**
- CPU & RAM:
- OS:
- Compiler & version:
- Optimisation flags:
- Dataset & how it was generated:

## Progress against the self-check (PDF §13)

The guide's own pre-presentation checklist, used here as the status report. A box is
ticked only when something in this repo proves it.

- [ ] Every structure in our topic is placed in the syllabus, with section numbers — `docs/PART_A_position.md` is still the template
- [ ] We can say what problem each structure solves, and when not to use it — same file, not written
- [ ] The operation table is complete: best / average / worst / space, all justified — `docs/PART_B_complexity.md` is still the template
- [ ] The C++ demo compiles clean with `-Wall` on two different machines — compiles clean, but verified on one machine only
- [ ] The demo solves a problem a real user would recognise — no problem chosen; two hardcoded arrays
- [ ] The demo runs on n ≥ 100 000 and prints its own timings — runs on n = 10, no timing code
- [ ] Edge cases are demonstrated, not just handled — `make edge` passes flags that `main.cpp` ignores
- [ ] The same problem runs in Python, Java, JavaScript and Go, on our machine — no source files in any of the four
- [ ] The comparison table is filled with our own numbers and our test conditions — every cell still `—`
- [ ] 3–5 exercises are written, with a marking scheme and our reference solutions — template only
- [ ] Two markers are nominated, and the marking sheet template exists — template exists; markers not nominated
- [ ] Every member has a speaking part and can answer questions on any part — not assigned yet

**0 of 12 complete.**

### What Part C needs next

Part C is 25 points, the largest single component, and everything below blocks it:

1. Get a real demo problem approved (§5.2) — nothing else can be finalised first.
2. Write `data/generate.py` and produce a dataset at n ≥ 100 000 (§5.1).
3. Add the `timeIt()` harness from §5.3 and print n, operation count and best-of-5 timing.
4. Make `main.cpp` read the dataset instead of its hardcoded arrays.
5. Wire up `--empty`, `--dups` and `--oob` so `make edge` demonstrates the three required
   failure cases (§5.1).

## Rules this repo enforces

1. **Cite everything** — textbook chapter/page or URL with access date; adapted code cited in a comment at the point of use.
2. **AI is a tool, not an author** — declare where AI was used; be able to explain every line on request.
3. **No shared code between groups** — ideas yes, source files no.
4. **Measurements must be real** — fabricated timings = zero for parts C and D.
5. **Grade honestly** — peer marks are audited.
6. **Everyone contributes** — contribution statement required; free riders marked individually.

## Timeline

| When | Milestone |
|------|-----------|
| Week 1 | Groups formed, topic confirmed |
| Week 2 | One-page plan: demo problem + datasets |
| −7 days | Draft slides reviewed with lecturer |
| −24 h | Final slides + code submitted |
| Day 0 | Presentation (45 min, everyone speaks, English); exercises published |
| +7 days | Class submissions due |
| +14 days | Evidence pack + marks to lecturer |

Late submission: −10% per day. Nothing accepted after +3 days.
>>>>>>> origin/feature/basic-algorithm-implementation
