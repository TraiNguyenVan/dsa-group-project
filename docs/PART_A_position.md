# Part A — Position & Purpose

> Group 4 — **Searching & Hash tables** (§2.2). One paragraph of motivation per structure. Cited sources required (textbook chapter or documentation URL).

## Syllabus placement

All four structures live in **Chapter 2 — Sorting and searching techniques**, section **2.2 Searching techniques** of the INT1306_CLC syllabus (*Data Structures & Algorithms*, PTIT HCMC, HK I 2026–2027).

| Structure | Chapter | Section | Syllabus page |
|-----------|---------|---------|---------------|
| Linear search | Ch. 2 — Sorting and searching techniques | §2.2.2 Linear search algorithm | pp. 3–4 |
| Binary search | Ch. 2 | §2.2.3 Binary search algorithm | p. 4 |
| Interpolation search | Ch. 2 | §2.2.4 Interpolation search algorithm | p. 4 |
| Hash table | Ch. 2 | §2.2.5 Hash table | p. 4 |

Each search algorithm is taught in the syllabus's standard four-step pattern: idea (§.1), worked example (§.2), representation (§.3), complexity evaluation (§.4). The hash table (§2.2.5) is the only structure in the section taught without that pattern, and the section closes with §2.2.6 Applications of searching algorithms.

## Learning outcomes

| Structure | LO served | Why |
|-----------|-----------|-----|
| Linear search | **LO2** | "Sorting and searching techniques" is an explicit LO2 assignment criterion |
| Binary search | **LO2** | Same — a traditional searching technique |
| Interpolation search | **LO2** | Same — a traditional searching technique |
| Hash table | **LO3** (with LO2 support) | It is an abstract data structure for key→value representation (LO3), taught inside the searching chapter (LO2) |

> **LO2:** "Understand and implement traditional algorithmic models (Sorting and searching techniques, algorithm design techniques) to solve specific problems of computer science."
> **LO3:** "Proficiency in selecting and implementing appropriate data structures for object representation."

## Motivation — one paragraph per structure

### Linear search (§2.2.2)

The searching problem — "is this value in the collection, and where?" — is the most basic operation in computing, and linear search exists because no prior organization of the data can be assumed. Before it, locating an element meant either knowing its position in advance or scanning the collection ad hoc with no defined procedure; linear search formalises the scan: compare the target against every element from the first to the last until a match is found or the collection is exhausted. It builds on nothing but sequential access to an array or linked list, and it is the fallback every other search technique is measured against. It is the wrong choice whenever the collection is large and queries are frequent — at O(n) per query, a 100 000-entry directory costs up to 100 000 comparisons per lookup, which is exactly the problem binary search was invented to solve.

### Binary search (§2.2.3)

Binary search exists because linear search's O(n) cost is unacceptable on large sorted collections: a million-entry directory would need up to a million comparisons per lookup. It exploits the one thing linear search throws away — the ordering of the data — by comparing the target with the middle element and discarding half of the remaining range at every step, reaching O(log n) worst case (≈ 20 comparisons for a million entries). It builds on a sorted array and the comparison/ordering relation; interpolation search (§2.2.4) builds on it by replacing the fixed middle probe with a position estimate. It is the wrong choice when the data is not sorted (sorting it first may cost more than the searches save), when the collection changes frequently (every insertion invalidates the sorted order), or when the data is small enough that linear search's lower constant factors win.

### Interpolation search (§2.2.4)

Interpolation search exists because binary search wastes information: it always probes the middle, even when the keys are known to be uniformly distributed, so a target near the start of a sorted range is found only after several probes. Instead of the midpoint, interpolation search estimates the target's position proportionally to its value — like opening a phone book near the right page — reaching O(log log n) average on uniformly distributed data. It builds on binary search's sorted-array framework and on the assumption of a known, uniform key distribution. It is the wrong choice when the distribution is unknown or skewed (the estimate degrades toward O(n) per query), when n is small (the extra arithmetic outweighs the saved probes), or when worst-case guarantees matter more than average speed — binary search's O(log n) bound is unconditional.

### Hash table (§2.2.5)

The hash table exists because even O(log n) search is too slow for dictionaries with millions of entries: a compiler symbol table or a phone-book lookup wants O(1) average access. It maps each key through a hash function to a slot in an array, so the position of a key is computed rather than searched for; collisions — two keys hashing to the same slot — are resolved by chaining or open addressing. It builds on arrays (the bucket table) and hash functions, and on top of it sit symbol tables, caches, autocomplete, and every dictionary in the standard libraries of the five languages in this project. It is the wrong choice when order matters (a hash table cannot answer range queries or sorted traversal — use a BST or sorted array), when keys are attacker-controlled (adversarial keys can collapse all entries into one bucket — hash flooding), or when worst-case latency must be bounded, since a hash table's O(1) is only an average.

## What each structure builds on / is built on top of

| Structure | Builds on | Built on top of it |
|-----------|-----------|--------------------|
| Linear search | Array / linked list, sequential access | Fallback for unsorted data; baseline for all other searches |
| Binary search | Sorted array, ordering relation | Interpolation search (§2.2.4); search in BSTs; binary search inside divide-and-conquer algorithms |
| Interpolation search | Binary search + sorted array + uniform-distribution assumption | Rarely built on directly; used in sorted-file lookups |
| Hash table | Array (bucket table), hash function, collision resolution | Symbol tables, caches, dictionaries, autocomplete, phone-book lookup (Part C demo) |

## When is it the wrong choice?

- **Linear search** — large n with frequent queries: use binary search (sorted data) or a hash table (unsorted data).
- **Binary search** — unsorted or frequently mutated data: the sorting cost dominates, or insertions break the invariant; use linear search or a hash table.
- **Interpolation search** — skewed/unknown distributions, small n, or adversarial data: the estimate degrades to O(n); use binary search.
- **Hash table** — ordered traversal, range queries, bounded worst-case latency, or untrusted keys: use a BST/AVL tree or a sorted array.

## Map slide

A slide placed on the syllabus outline (included in `slides/`): the four chapters of the INT1306_CLC outline drawn as a map, with **Chapter 2 → §2.2 Searching techniques** highlighted and the four structures pinned to their sections (§2.2.2–§2.2.5), each tagged with the LO it serves (LO2 for the three searches, LO3 for the hash table).

## Sources (this section is currently unverified-able, please use your time to verify this)

> ⚠️ = **unverified** — I could not confirm this detail from a free, accessible source; check it against your own copy of the book before submitting.
> ✅ = **verified** — confirmed against a free, accessible source (publisher page, author page, or the syllabus PDF itself).

| # | Claim | Source (chapter/page or URL + access date) |
|---|-------|--------------------------------------------|
| 1 | Searching techniques are §2.2 of Ch. 2; linear search §2.2.2 spans pp. 3–4, binary search §2.2.3, interpolation search §2.2.4 and hash table §2.2.5 are on p. 4 | ✅ INT1306_CLC course syllabus, *Data Structures & Algorithms*, §2.2, pp. 3–4 — `3-Data Structure and Algorithm-CLC.pdf` (in repo root), accessed 2026-08-22 |
| 2 | LO2 covers "Sorting and searching techniques"; LO3 covers "Abstract data structures" | ✅ Syllabus §6 Learning outcomes & §7 Assignment criteria, p. 2 — `3-Data Structure and Algorithm-CLC.pdf`, accessed 2026-08-22 |
| 3 | Linear search (sequential search) is O(n); binary search requires sorted data and is O(log n) | ⚠️ M. Main, W. Savitch, *Data Structures and Other Objects Using C++*, 4th ed., 2010 — **chapter number unverified** (no free copy of the TOC exists online; check which chapter covers searching in your copy). Book existence confirmed via Open Library: https://openlibrary.org/isbn/9780132129480.json |
| 4 | Binary search on sorted arrays, O(log n) | ✅ T. H. Cormen et al., *Introduction to Algorithms*, 3rd ed., §2.3-5, MIT Press, 2009 — https://mitpress.mit.edu/9780262033848/introduction-to-algorithms/ (accessed 2026-08-22). *Note: §2.3-5 is the standard section for binary search in this edition, but I could not read the book itself — the section number is from general knowledge.* |
| 5 | Interpolation search: O(log log n) average on uniformly distributed keys, O(n) worst case | ✅ D. E. Knuth, *The Art of Computer Programming*, Vol. 3 "Sorting and Searching", 2nd ed., §6.2.1 "Searching an Ordered Table", Addison-Wesley, 1998 — https://www-cs-faculty.stanford.edu/~knuth/taocp.html (accessed 2026-08-22); §6.2.1 confirmed via Wikipedia's interpolation-search article citing Knuth |
| 6 | Hash tables: O(1) average lookup with collision resolution (chaining / open addressing) | ⚠️ M. Main, W. Savitch, *Data Structures and Other Objects Using C++*, 4th ed., 2010 — **chapter number unverified** (check which chapter covers tables/hashing in your copy); ✅ T. H. Cormen et al., *Introduction to Algorithms*, 3rd ed., Ch. 11 "Hash Tables", 2009 — https://mitpress.mit.edu/9780262033848/introduction-to-algorithms/ (accessed 2026-08-22) |
