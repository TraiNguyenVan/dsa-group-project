# DSA Group Project — PTIT INT1306_CLC

Mid-term group project · 20% of final mark · HK I 2026-2027
Faculty of Information Technology 2, PTIT HCMC Campus

**Topic:** *TBD — not yet chosen.* This repo is the template scaffold. Pick a topic
(G1 Linked Lists · G2 Stacks & Queues · G3 Sorting · G4 Searching & Hash Tables ·
G5 Trees & Heaps · G6 Graphs), confirm with the lecturer, then fill in the parts below.

**Everyone: read [CONTRIBUTING.md](CONTRIBUTING.md) before your first commit.**

## Deliverables (all six)

| Part | What | Where |
|------|------|-------|
| A | Position & purpose of the data structures | `docs/PART_A_position.md` |
| B | Algorithms & complexity table (best/avg/worst/space, justified) | `docs/PART_B_complexity.md` |
| C | Real-world C++ demo (compiles, timed, edge cases) | `src/cpp/` |
| D | Cross-language comparison (Python, Java, JavaScript, Go) | `src/<lang>/` + `docs/PART_D_comparison.md` |
| E | Exercises for the class (3–5, with marking scheme) | `exercises/` |
| F | Peer grading with evidence pack | `grading/` |

## Layout

```
├── README.md            ← this file: build & run, expected output, machine/compiler
├── docs/                ← write-ups for parts A, B, D
├── src/                 ← C++ demo + ports to Python, Java, JavaScript, Go
├── data/                ← dataset or generator script (n ≥ 100 000)
├── slides/              ← presentation (English, sent 24 h in advance)
├── report/              ← report.pdf (6–10 pages: parts A, B, D + charts)
├── exercises/           ← published after the presentation (deadline ≥ 1 week)
└── grading/             ← evidence pack, added after exercises are marked
```

## Build & run (C++ demo)

```bash
cd src/cpp
make            # g++ -std=c++17 -Wall
./demo          # runs on the dataset in ../../data
```

**Test conditions to record here (required — a measurement without its conditions is not a measurement):**
- CPU & RAM:
- OS:
- Compiler & version:
- Optimisation flags:
- Dataset & how it was generated:

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
