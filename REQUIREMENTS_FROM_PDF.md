# DSA Group Project — All Requirements from the PDF

> **Source:** `C0B_Group_Project_Guide.pdf` (21 pages)
> **Course:** Data Structures & Algorithms (INT1306_CLC), Faculty of Information Technology 2, PTIT HCMC Campus
> **Term:** HK I 2026-2027 · **Proposed by:** Dr. HieuLN
> **Weight:** Mid-term project · 20% of the final mark
> **Extracted:** 2026-08-17, verified against full PDF text from session history

---

## 1. Overview (pp. 1–2)

- **6 groups**, 1 topic per group, 45 min per presentation, 20% of final mark.
- Each group owns one topic deeply instead of skimming all.
- Write code that works in front of the class.
- Find out what the structure costs — in C++ and in four other languages.
- Practise technical English: slides, code comments, report, and Q&A.
- Grade classmates' work (defines what "correct" means).

## 2. Topic Allocation (p. 4)

Six topics, one per group. No two groups may take the same topic. Topics confirmed by the lecturer.

| Group | Topic | Syllabus | Structures |
|-------|-------|----------|------------|
| G1 | Linked Lists | §4.1 | Singly · doubly · circular · sorted lists |
| G2 | Stacks & the Queue family | §4.2–4.3 | Stack · queue · circular queue · deque · priority queue |
| G3 | Sorting techniques | §2.1 | Selection · Insertion · Bubble · Quick · Merge · Radix |
| G4 | Searching & Hash tables | §2.2 | Linear · Binary · Interpolation search · hashing & collisions |
| G5 | Trees & Heaps | §4.4 | Binary tree · BST · AVL · min/max heap |
| G6 | Graphs | §4.5 | Representations · DFS/BFS · Dijkstra, Bellman-Ford, Floyd · Kruskal, Prim |

Groups formed in week 1; topics confirmed by the lecturer.

## 3. Part A — Position & Purpose (p. 5) · 10 pts

Answer these explicitly, per structure:
1. Where in the syllabus does each structure appear — chapter, section, page?
2. Which learning outcome (LO1 / LO2 / LO3) does it serve?
3. What problem does it exist to solve? What was unacceptable before it?
4. Which structures does it build on, and which are built on top of it?
5. When is it the wrong choice? Name a case where you would not use it.

**Deliverables:**
- A map slide: structures placed on the syllabus outline.
- One paragraph of motivation per structure.
- Cited sources — textbook chapter or documentation URL.

## 4. Part B — Algorithms & Complexity (p. 6) · 20 pts

**Required table.** For every structure in your topic, tabulate every operation with:
- Best complexity
- Average complexity
- Worst complexity
- Space complexity
- **Why** — a number without a justification scores nothing.

Example provided for a singly linked list (Group 1): Search/find, Insert at head, Insert at position k, Delete by value, Traverse all — each with best/avg/worst/space and justification.

One table per structure in the topic.

## 5. Part C — Real-World C++ Demo (pp. 7–9) · 25 pts

### 5.1 Rules (p. 7)
1. Must solve a problem a **real user** would have — not "insert 5 numbers into a list".
2. **Implement the structure yourself.** You may compare against the STL, but not substitute it.
3. Must compile with `g++ -std=c++17 -Wall` and run live in class.
4. Must handle at least one dataset large enough to make the complexity visible (**n ≥ 100 000**).
5. Must print timing so the class can see the cost, not just be told about it.
6. Include the failure cases: **empty input, duplicate keys, out-of-range access**.

### 5.2 Suggested Demo Problems (p. 7)
| Group | Suggested Problem |
|-------|-------------------|
| G1 | Browser history / undo-redo with a doubly linked list |
| G2 | Hospital triage queue or print spooler with a priority queue |
| G3 | Sorting 1M transaction records, comparing six sorts |
| G4 | Autocomplete / phone-book lookup with a hash table |
| G5 | Leaderboard with a heap, or a dictionary index with an AVL tree |
| G6 | Shortest metro route in Hanoi with Dijkstra |

You may propose your own problem — get it approved before building.

### 5.3 Timing Pattern (p. 8)

The PDF provides this exact `timeIt` template:

```cpp
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;

template <typename F>
double timeIt(F work, int repeats = 5) {
    double best = 1e18;
    for (int r = 0; r < repeats; ++r) {
        auto t0 = high_resolution_clock::now();
        work();
        auto t1 = high_resolution_clock::now();
        double ms = duration<double, milli>(t1 - t0).count();
        best = min(best, ms);
    }
    return best;   // report the best of 5 runs
}
```

**Report, for every n:**
- Input size n
- Operation count (if instrumented)
- Wall-clock time, best of 5 runs
- Peak memory, if relevant

### 5.4 Test Conditions (p. 8)
State your test conditions:
- CPU and RAM
- OS
- Compiler and version
- Optimisation flags
- Dataset and how it was generated

> "A measurement without its conditions is not a measurement."

### 5.5 Live Demo Script (p. 9)

Exact sequence — do not just print a final number:

1. **Data Verification (1 min):** Show the generated dataset (e.g., `head data.csv`). Prove input is real, randomized, and correctly sized (n ≥ 100 000).
2. **Edge Case Execution (2 mins):** Run with malicious/edge inputs first. Demo: empty file, duplicate keys, or out-of-bounds access. Show graceful handling or rejection.
3. **Core Algorithm Execution & Verification (3 mins):** Run on a smaller subset first (n = 50) and print output to visually prove correctness.
4. **Stress Test & Profiling (6 mins):** Run on the full dataset (n ≥ 100 000). Display live terminal output showing the structured timing format.

## 6. Part D — Cross-Language Comparison (pp. 10–12) · 20 pts

### 6.1 Comparison Table (p. 10)

Same problem, same input, five languages: C++, Python, Java, JavaScript, Go.
Fill **every cell** with your own measurements — not numbers copied from a blog.

| Criterion | C++ | Python | Java | JS | Go |
|-----------|-----|--------|------|----|----|
| Built-in structure used | | | | | |
| Manual implementation effort | | | | | |
| Runtime on your dataset | | | | | |
| Memory footprint | | | | | |
| Memory management | | | | | |
| Type safety | | | | | |
| Readability of the code | | | | | |
| Where you would use it | | | | | |

> The last row — where you would use it — is what earns the marks.

### 6.2 What a Good Comparison Says (p. 11)
1. **Be fair** — same algorithm, same input, same machine. Say whether you compare hand-written C++ vs hand-written X, or vs X's built-in.
2. **Explain the gap** — not just "Python was 40× slower" but "because each element is a boxed object reached through a pointer, and the loop runs in the interpreter".
3. **Cover trade-offs** — runtime, memory, lines of code, time spent writing, how easy the bug was to find. Speed is not the only axis.
4. **Name the right tool** — end with a recommendation: which language for this problem in production, and why. Defend in Q&A.

**Angles worth covering:**
- Manual pointers vs. garbage collection
- Static vs. dynamic typing
- Compiled vs. interpreted vs. JIT
- Standard-library coverage of your structure
- Concurrency support, if your topic needs it
- Tooling: debugger, profiler, package manager

### 6.3 Empirical Evidence (p. 12)

Filling the table is not enough. You must prove findings with:

1. **Line chart** — runtime growth curves of all 5 languages overlaid, across multiple dataset sizes (e.g., n = 10k, 50k, 100k, 500k, 1M). Use Excel, matplotlib, etc. Save to `report/`.
2. **Bar chart** — peak memory in MB across the 5 implementations, measured with real profilers:
   - C++: Valgrind/Massif
   - Python: memory_profiler / tracemalloc
   - Java: VisualVM
   - JS: Chrome DevTools
   - Go: pprof
3. **Statistical fairness** — run each test at least 5 times. Warm-up runs do **not** count for Java/JS JIT compilers. **Report the average AND the variance.** If Java is slow on run 1 but fast on run 5, explain why (JIT compilation).

## 7. Part E — Exercises for the Class (p. 13) · 10 pts

### Requirements
- Publish **3–5 exercises** immediately after your presentation.
- Cover a range: **one recall question, two implementation tasks, one analysis task, one open/design task**.
- Every exercise must be answerable from what you taught — no material you did not present.
- Publish a **marking scheme** with the exercises: total points and what each point is for.
- State the **deadline** (minimum one week) and the submission channel.
- Prepare your own **reference solution** before you publish.

### Example set (Group 3, Sorting)
| # | Type | Task | Pts |
|---|------|------|-----|
| 1 | Recall | State the worst-case complexity of Quick sort and the input that triggers it. | 1 |
| 2 | Implement | Write Merge sort in C++ and sort the supplied 100k-line file. | 3 |
| 3 | Implement | Modify Bubble sort to stop early on a sorted array; show the new best case. | 2 |
| 4 | Analyse | Count the comparisons of Insertion sort on the supplied dataset; compare to n²/2. | 2 |
| 5 | Design | You must sort 10 GB that does not fit in RAM. Which algorithm, and why? | 2 |

## 8. Part F — Peer Grading with Evidence (pp. 14–15) · 15 pts

> The marks your group awards feed directly into classmates' attendance and participation score — 10% of their final grade. Treat it with the seriousness that implies.

### 8.1 Workflow (p. 14)
1. **Publish the scheme first** — points per exercise and per criterion, before anyone submits.
2. **Mark independently** — at least two members of your group mark every submission separately.
3. **Reconcile** — where the two marks differ by more than 20%, discuss and record the agreed mark.
4. **Write the feedback** — two or three sentences per submission: what was right, what was wrong, what to fix.
5. **Submit the evidence pack** — marks, feedback, and the artefacts you based them on — to the lecturer.
6. **Face the appeal** — any student may contest a mark. Your group must justify it with the evidence.

### 8.2 Evidence Pack Contents (p. 15)
Every mark must be traceable:
- The submitted file or repository link, with its timestamp.
- The marking sheet: one row per student, one column per criterion.
- A screenshot or terminal log showing the submitted code compiling and running.
- The written feedback you sent back.
- Names of the two markers, and the reconciled mark where they disagreed.
- An attendance record for the session your exercises were set in.

### 8.3 Marking Sheet — Required Columns (p. 15)
```
student_id | name | ex1 | ex2 | ex3 | ex4 | ex5 | total | marker_A | marker_B | agreed | note
```

### 8.4 Zero Tolerance (p. 15)
- Marks awarded without evidence are **void** — the lecturer re-marks the whole set and the grading component of your group's project score is **forfeited**.
- Inflating marks for friends, or deflating them for rivals, is an **academic integrity matter**.

## 9. Presentation Day (p. 16)

### 9.1 45-Minute Allocation
| Time | Part | Content |
|------|------|---------|
| 8 min | Part A | Position & purpose — where structures live and why |
| 10 min | Part B | Algorithms & complexity — the operation table, defended |
| 12 min | Part C | Live C++ demo — compile and run in front of the class |
| 8 min | Part D | Cross-language comparison — measurements and recommendation |
| 3 min | Part E | Hand out exercises — publish tasks, scheme, deadline |
| 4 min | Q&A | Anyone may ask; every member must be able to answer |

### 9.2 Non-Negotiable Rules
- **Everyone speaks.** A silent member scores zero.
- **English throughout**, including Q&A.
- **No reading from the slides.**
- **Bring the demo on two machines** — no excuses.
- **Send slides + code 24 h in advance.**

## 10. Assessment Breakdown (p. 17)

| Part | Description | Points |
|------|-------------|--------|
| A | Position & purpose — correct syllabus placement, real motivation, cited | 10 |
| B | Algorithms & complexity — complete table, correct bounds, justified | 20 |
| C | C++ demo — compiles, runs live, solves real problem, measured, handles edge cases | 25 |
| D | Language comparison — fair method, own measurements, explained gaps, clear recommendation | 20 |
| E | Exercises set — range of difficulty, answerable, published with marking scheme | 10 |
| F | Grading with evidence — complete evidence pack, independent double marking, useful feedback | 15 |
| **Total** | | **100** |

> Scaled to the 20% mid-term project component. Individual marks may be adjusted up or down by up to **20%** based on your own contribution.

## 11. Logistics — What to Submit (p. 18)

| Item | Description |
|------|-------------|
| `slides.pptx` / `.pdf` | Presentation, in English. |
| `report.pdf` | 6–10 pages: parts A, B and D written up. Must include plotted runtime charts, memory profiling screenshots, and the measurement tables. |
| `src/` | All C++ source, plus ports to the other languages. Must include a Makefile or build command. |
| `data/` | The dataset, or the generator script that produces it. |
| `README.md` | How to build and run, expected output, machine and compiler used. |
| `grading/` | The evidence pack from Part F — added after your exercises are marked. |

### Deadlines
| When | Milestone |
|------|-----------|
| Week 1 | Groups formed, topic confirmed |
| Week 2 | One-page plan: demo problem + datasets |
| −7 days | Draft slides reviewed with the lecturer |
| −24 h | Final slides + code submitted |
| Day 0 | Presentation; exercises published |
| +7 days | Class submissions due to your group |
| +14 days | Evidence pack + marks to the lecturer |

> **Late submission: −10% per day. Nothing accepted after +3 days.**

## 12. Academic Integrity & AI Use (p. 19)

1. **Cite everything** — book, chapter, page, or URL with access date. Code adapted from elsewhere is cited in a comment at the point of use.
2. **AI is a tool, not an author** — you may use AI assistants. You must declare where, and be able to explain every line you submit, line by line, on request.
3. **No shared code between groups** — discussing ideas is fine. Sharing source files is not. Both groups lose the marks.
4. **Measurements must be real** — fabricated timings are treated as fabricated data — zero for Parts C and D.
5. **Grade honestly** — peer marks are audited. Inflated or retaliatory marking is an integrity offence.
6. **Everyone contributes** — submit a short contribution statement: who did what. Free riders are marked individually.

## 13. Self-Check Before Presenting (p. 20)

Tick all twelve before presenting. If any box is unticked 24 hours before your slot, tell the lecturer — do not present unprepared.

- [ ] Every structure in our topic is placed in the syllabus, with section numbers.
- [ ] We can say what problem each structure solves, and when not to use it.
- [ ] The operation table is complete: best / average / worst / space, all justified.
- [ ] The C++ demo compiles clean with `-Wall` on two different machines.
- [ ] The demo solves a problem a real user would recognise.
- [ ] The demo runs on n ≥ 100 000 and prints its own timings.
- [ ] Edge cases are demonstrated, not just handled.
- [ ] The same problem runs in Python, Java, JavaScript and Go, on our machine.
- [ ] The comparison table is filled with our own numbers and our test conditions.
- [ ] 3–5 exercises are written, with a marking scheme and our reference solutions.
- [ ] Two markers are nominated, and the marking sheet template exists.
- [ ] Every member has a speaking part and can answer questions on any part.

## 14. Start This Week (p. 21)

- Form your group.
- Pick your topic.
- Find a real problem.

> "The demo is the hard part, and it is the part you cannot do the night before. Choose a problem you actually care about — you will be living with it all semester."

---

## Note on the Timing/Benchmark Contradiction

The PDF contains **two separate requirements** about benchmarking that appear to conflict:

1. **Page 8 (Part C)** — provides a `timeIt()` template that takes the **best of 5 runs** (`best = min(best, ms)`). This is for the C++ demo timing display.
2. **Page 12 (Part D)** — states: "Run each test at least 5 times (warm-up runs do not count for Java/JS JIT compilers). **Report the average and the variance.** If Java is slow on run 1 but fast on run 5, explain why (JIT compilation)."

**Interpretation:** Part C (the C++ demo) uses best-of-5 for the live timing display. Part D (cross-language comparison) requires average + variance across ≥5 runs for statistical fairness. The current `main.cpp` comment "The required timing pattern — report the BEST of 5 runs" is accurate for Part C but would be **wrong** if applied to Part D's comparison measurements.
