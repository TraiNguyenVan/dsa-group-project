# 01 — Language Comparison

The same phonebook — same chained hash table, same linear scan, same binary search on a sorted index — is ported **line-by-line** to five languages. This page is the single place to compare them: what differs, why, and how to read the numbers fairly.

> **Scope:** this is a *language + runtime* comparison of equivalent code. It is **not** "C++ vs Python's built-in `dict`" — no port uses `dict` / `map` / `HashMap`. Every port implements the same chained hash table. See [09 — Architecture](../benchmark/09-architecture.md) for the file map.

## 1. Be fair — what is actually being compared

Same algorithm, same input, same machine:

| | C++ | Python | Go | JavaScript (Node) | Java |
|---|---|---|---|---|---|
| Main | `src/main.cpp` | `python/phonebook/main.py` | `go/phonebook/main.go` | `javascript/phonebook/src/main.js` | `java/phonebook/src/com/phonebook/Main.java` |
| PhoneBook | `src/phonebook.cpp` + `.hpp` | `phonebook.py` | `phonebook.go` | `src/phonebook.js` | `PhoneBook.java` |
| HashTable | `src/hashtable.cpp` + `.hpp` | `hashtable.py` | `hashtable.go` | `src/hashtable.js` | `HashTable.java` |
| Timer | `include/timer.hpp` | `timer.py` | `timer.go` | `src/timer.js` | `Timer.java` |
| Contact | `include/contact.hpp` | `contact.py` | `contact.go` | `src/contact.js` | `Contact.java` |

All five are **faithful ports** of `src/main.cpp` — same menu, same `option 0` interactive benchmark, same `--benchmark-csv [--append]` batch mode, same seed 42, same `miss=0000000000`. Differences are only runtime-forced: 64-bit wrap, stdin handling, timing API.

**What this comparison is NOT:** C++ vs Python's built-in `dict`, or C++ vs Java's `HashMap`. Those are different questions (implemented vs stdlib). Here every port implements the same chained hash table — so the gap you measure is the *language + runtime*, not the algorithm.

## 2. Explain the gap — why the numbers differ

"Python was 40× slower" is an observation. This section is the answer.

### 2.1 Compilation & execution model

| Lang | Model | Run 1 cost | Warm-up needed? |
|---|---|---|---|
| C++ | AOT — `g++ -O2` | ~1.1× best (cache/branch) | No |
| Go | AOT — `go build` | ~1.1× best (cache/branch) | No |
| Python | Interpreted (CPython), no JIT | stable, no JIT spike | No |
| Java | JIT — JVM interpreted → C1 → C2 | 5–80× best (class-load + G1 + compile) | **Yes** |
| JS | JIT — V8 Ignition → TurboFan | 5–100× best (cold V8) | **Yes** |

- **AOT (C++, Go):** compiled once at build time. Run 1 is already fast; the small excess is cold cache + branch prediction.
- **JIT (Java, JS):** start interpreted, then compile hot code after ~10k iterations. One 100k linear scan triggers compilation *mid-run*:

```
Java middle-linear 100k: run1 39ms → run5 17ms  (JIT kicked in)
JS   first-linear 1M:    run1 101× best          (cold V8)
C++/Go:                  run1 ~1.1× best         (AOT, already compiled)
```

- **Python:** no JIT — the interpreter runs every bytecode every time. No warm-up spike, but the interpreter + boxed objects are the reason linear search is slowest here.

### 2.2 Memory model — why Python/Java/JS pay more

| Lang | Model | Why the gap |
|---|---|---|
| C++ | Manual RAII, no GC | `vector<Contact>` is **contiguous** — one allocation, cache-friendly, no per-element pointer chase. SSO for short strings. |
| Python | Refcount + generational GC | every `Contact`/`HashNode`/`str` is a boxed `PyObject` + header, reached through a pointer. `buckets = [None] * n` is a list of pointers. |
| Go | Tracing GC (concurrent) | `[]Contact` values are contiguous, but each `HashNode` is a separate heap alloc → pointer chase per probe. |
| JS | V8 generational GC | `BigInt` for the 64-bit hash wrap (`HASH_BASE = 31n`) — BigInt ops are slower than native ints. `Array` of pointers. |
| Java | Tracing G1, concurrent | `HashNode[]` of references; every `HashNode` is an object with header + fields. `remainderUnsigned` for the wrap. |

The hash loop is the same algorithm in all five — but in C++ it's a few native int ops on contiguous memory, in Python it's a bytecode loop dereferencing boxed objects, in JS it's BigInt math. That's the mechanism behind the gap, not "Python is slow".

### 2.3 Timing primitives

| Lang | File | Clock | Resolution | Returns |
|---|---|---|---|---|
| C++ | `include/timer.hpp` | `std::chrono::high_resolution_clock::now()` | ns | `double` ms |
| Python | `python/phonebook/timer.py` | `time.perf_counter()` | ns | `float` ms |
| Go | `go/phonebook/timer.go` | `time.Now()` | ns | `float64` ms |
| JS | `javascript/phonebook/src/timer.js` | `perf_hooks.performance.now()` | µs | `number` ms |
| Java | `java/phonebook/src/com/phonebook/Timer.java` | `System.nanoTime()` | ns | `double` ms |

All measure **wall-clock** and write `ms` to CSV. Each writes its own float format (`setprecision(17)`, `repr`, `%g`, `Double.toString`) — `benchmark/plot.py:parse_ms()` handles all of them with a defensive `float()`.

## 3. Cover the trade-offs — speed is not the only axis

| Criterion | C++ | Python | Java | JavaScript | Go |
|---|---|---|---|---|---|
| Built-in structures | `vector` (contacts, buckets, sorted index) | `list` (contacts, buckets, sorted index) | `ArrayList` (contacts, sorted index) + `HashNode[]` (buckets) | `Array` (contacts, buckets, sorted index) | slices `[]Contact`, `[]*HashNode`, `[]string` |
| Effort | highest: `new/delete`, header split, `=delete`, rehash, CSV state machine, `lowerBound`, CMake `-O2` | lowest: `dataclass+list`, `& MASK64`, `""`-CSV, `lowerBound`, `perf_counter` | high: `remainderUnsigned`, `HashNode[]`, NIO.2 CSV, `lowerBound`, `nanoTime`, `javac -d` | high: `BigInt & MASK64`, `perf_hooks`, `readline` CLI, `mulberry32(42)` | medium: `uint64` wraps free; chaining/CSV/`lowerBound`/`time.Now`, `go run` |
| Types | strong static `constexpr/const`, `template`, `-Wall` | dynamic + hints (`dataclass`, `List/Callable`) | static+generics, checked `IOException` | dynamic JS `'use strict'`, no TS | static concrete, no generics, errors as `-1/nil` |
| Readability | verbose/explicit; costs visible | most concise; `with open`, f-strings | verbose enterprise; ~2× Python LOC | small, C++-mirroring | explicit; `defer`/`copy`-shift noisy but clear |
| Use when | perf-critical / 1M / embedded: `-O2`, no GC | teaching/prototyping/glue; not hot loop (use `dict`) | large-team/enterprise/Android; not terse scripts | demo/web/zero-build CLI; not 100k+ hot path (`BigInt`) | backend/microservice: single binary, `pprof`, GC |

**Lines of code** (relative, from the report's D.2): Java is ~2× Python's LOC; C++ is verbose but explicit; JS mirrors C++ closely; Go is explicit with `defer`/`copy`-shift noise. The trade-off: C++/Go pay more effort up front for speed and no GC pauses; Python pays the least effort but the interpreter + boxed objects cap the hot loop.

## 4. Name the right tool — the recommendation

For **this problem** — a phonebook lookup at n ≥ 100k, read-heavy, single machine:

- **Pick C++** if the hot path is the point: `-O2`, no GC, contiguous `vector<Contact>`, whole-process floor (~218 MB RSS at 1M). The cost is manual memory management and the most code.
- **Pick Go** if you want ~C++ speed with less effort and a single static binary: `go build`, `pprof` built in, GC is concurrent. The `[]Contact` values are contiguous; per-`HashNode` allocs are the main overhead.
- **Pick Python** for teaching/prototyping/glue — but **not** for the hot loop. The interpreter + boxed objects make linear search ~10× slower than C++ at 1M; use the built-in `dict` if you need speed in Python.
- **Pick Java** for large-team/enterprise/Android — but the JVM baseline (~51 MB RSS at n=50) and JIT warm-up mean it's not the leanest choice for a small CLI.
- **Pick JS** for a demo/web/zero-build CLI — but `BigInt` for the 64-bit wrap and the V8 baseline (~53 MB RSS) make it a poor fit for a 100k+ hot path.

**Defend it:** the recommendation is *for this problem* — a read-heavy lookup at n ≥ 100k. If the workload were write-heavy or the team were Python-only, the answer changes. The DSA point (linear O(n) vs hash O(1) vs binary O(log n)) holds in **all five** languages — the language changes the constant, not the complexity.

---

Next: [10 — FAQ](../benchmark/10-faq.md) — troubleshooting and quick recipes.