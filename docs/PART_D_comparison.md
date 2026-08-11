# Part D — Cross-Language Comparison

> Same problem, same input, five languages. Port the demo (or its core loop) into `src/<lang>/`.
> Fill **every cell** with your own measurements — never numbers copied from a blog.

## Comparison table

| Criterion | C++ | Python | Java | JavaScript | Go |
|-----------|-----|--------|------|------------|-----|
| Built-in structure used | — | — | — | — | — |
| Manual implementation effort | — | — | — | — | — |
| Runtime on your dataset | — | — | — | — | — |
| Memory footprint | — | — | — | — | — |
| Memory management | — | — | — | — | — |
| Type safety | — | — | — | — | — |
| Readability of the code | — | — | — | — | — |
| Where you would use it | — | — | — | — | — |

## What a good comparison says

1. **Be fair** — same algorithm, same input, same machine. Say whether you compare hand-written C++ vs hand-written X, or vs X's built-in.
2. **Explain the gap** — "Python was 40× slower because each element is a boxed object reached through a pointer, and the loop runs in the interpreter" — not just "Python was 40× slower".
3. **Cover trade-offs** — runtime, memory, lines of code, time spent writing, ease of finding bugs.
4. **Name the right tool** — which language would you pick in production for this problem, and why (defend in Q&A).

Angles: manual pointers vs GC · static vs dynamic typing · compiled vs interpreted vs JIT · stdlib coverage · concurrency (if relevant) · tooling (debugger, profiler, package manager).

## Empirical evidence (required — filling the table is not enough)

1. **Line chart** — runtime growth curves of all 5 languages overlaid, at multiple dataset sizes (e.g. n = 10k, 50k, 100k, 500k, 1M). Excel or matplotlib → `report/`.
2. **Bar chart** — peak memory in MB across the 5 implementations, measured with real profilers:
   - C++: Valgrind/Massif · Python: memory_profiler / tracemalloc · Java: VisualVM · JS: Chrome DevTools · Go: pprof
3. **Statistical fairness** — ≥ 5 runs each (warm-up runs don't count for Java/JS JIT). Report average AND variance; explain JIT effects (e.g. Java slow on run 1, fast on run 5).
