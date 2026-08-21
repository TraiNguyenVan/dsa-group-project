# C++ Lecture — Hash Tables

> **PTIT INT1306_CLC — Data Structures & Algorithms**
> Mid-term group project — Part C (Hashing chapter)

This module demonstrates two canonical collision-resolution strategies for the
abstract data type **Hash Table**.

---

## 1. The Problem Hashing Solves

| Operation            | Array (sorted) | Binary search | Linear search | Hash table (avg) |
|----------------------|----------------|---------------|---------------|------------------|
| Look up by key       | $O(\log n)$    | $O(\log n)$   | $O(n)$        | $O(1)$           |
| Insert by key        | $O(n)$         | $O(n)$        | $O(1)$ tail   | $O(1)$           |
| Delete by key        | $O(n)$         | $O(n)$        | $O(n)$        | $O(1)$           |

A hash table gives us **average-case constant-time** access — at the cost of
losing any ordering of the keys.

## 2. The Idea in One Picture

```
  +---- key k ---->  h(k) = k mod M  ---->  bucket index  ---->  table[index]
```

`h` is the *hash function*. `M` is the number of buckets (slots).

## 3. Collisions — Why They Are Inevitable

By the pigeonhole principle, if the number of keys `N` exceeds the number of
buckets `M`, **at least two distinct keys share the same bucket index**.
This event is a **collision** and every hashing scheme must define how to
resolve it.

## 4. Strategy A — Separate Chaining (`hashing.hpp`)

Each bucket holds the head of a **linked list**.  Colliding keys just live on
the same chain.

```
    table[0]         table[5]         table[7]
   +---------+      +---------+      +---------+
   | 20 -----+--->  | 15 -----+--->  | 42      |  NULL
   | 10      |  NUL | 25      |  NUL |         |
   +---------+      +---------+      +---------+
```

- Load factor  $\alpha = N/M$
- Avg case     $O(1+\alpha)$ for insert / search / delete
- Worst case   $O(n)$ when every key hashes to the same bucket
- Deletion     trivial — unlink the node
- Memory       one node per key + 1 pointer per node

## 5. Strategy B — Open Addressing / Linear Probing (`linear_probing.hpp`)

All keys live **inside the table array**.  On collision we probe forward:

$$
h(k,i) = \big( h(k) + i \big) \bmod M \quad \text{for } i = 0, 1, 2, \dots
$$

```
  bucket:  0  1  2  3  4  5  6  7  8  9
         +--+--+--+--+--+--+--+--+--+--+
  keys:   |  |  |12|13|14|15|16|17|27|  |
         +--+--+--+--+--+--+--+--+--+--+
                    ^           ^   ^
                    12      collision on 27
                                probes to bucket 8
```

- Avg case  $O\!\left(\dfrac{1}{2}\!\left(1+\dfrac{1}{1-\alpha}\right)\right)$
- Worst     $O(n)$ — collapses as $\alpha \to 1$
- Deletion  needs a tombstone (`DELETED`) because we cannot blank the slot

## 6. Comparison

| Property              | Separate Chaining    | Linear Probing          |
|-----------------------|----------------------|-------------------------|
| Cache locality        | Poor (pointer chase) | Excellent (one array)   |
| Pointer overhead      | Yes (per node)       | No                      |
| Tolerates $\alpha>1$  | Yes                  | No                      |
| Deletion              | Easy                 | Tombstones / rehash     |
| Clustering            | None at this level   | Primary clustering      |

## 7. Hash Function — Division Method

Both implementations use the cheapest one:

$$
h(k) = k \bmod M
$$

For production code you'd avoid powers of two for `M`, prefer a prime, or
switch to the multiplication method:
$$
h(k) = \lfloor M \cdot (k \cdot A \bmod 1) \rfloor
\quad \text{with } A \in (0,1), \text{ e.g. } A = (\sqrt{5}-1)/2
$$

## 8. Build & Run

```bash
make -C src/cpp            # builds demo
./build/cpp/demo           # runs linear, binary, interpolation, hash tables
```

`make clean` removes the `build/` directory.

## 9. Where To Read The Code First

1. `hashing.hpp` — Separate Chaining (the easier of the two)
2. `linear_probing.hpp` — Open Addressing
3. `main.cpp` — How both are exercised at runtime

---

*"A hash table is the only data structure that lets you cheat the laws of
asymptotic complexity — at the price of worst-case behaviour you must respect
in your analysis."*
