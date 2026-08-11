# src/ — five implementations of the same problem

Same problem, same input, same machine. Measure, don't guess.

| Language | Dir | Built-in available | Profiler for memory |
|----------|-----|--------------------|---------------------|
| C++ (demo) | `cpp/` | STL (compare, don't substitute) | Valgrind/Massif |
| Python | `python/` | stdlib | memory_profiler / tracemalloc |
| Java | `java/` | java.util | VisualVM |
| JavaScript | `javascript/` | Node.js | Chrome DevTools |
| Go | `go/` | stdlib | pprof |

Fill `docs/PART_D_comparison.md` with YOUR OWN numbers (runtime line chart n = 10k/50k/100k/500k/1M, memory bar chart, ≥ 5 runs each, report average + variance).
