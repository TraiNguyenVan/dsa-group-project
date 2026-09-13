# 01 — Phonebook Logic: Overview

This is the **logic** of the phonebook — the algorithms and data structures that make it work. The benchmark harness (how we measure it) lives in [`../benchmark/`](../benchmark/); this folder is about *what the code does*.

## What the phonebook is

A contact list that supports:

| Operation | Method | Complexity |
|-----------|--------|------------|
| Insert | `insertContact(name, phone)` | O(1) hash + O(n) sorted-index shift |
| Delete | `deleteContactByPhone(phone)` | O(n) vector erase + full hash rebuild |
| Search by phone | `searchLinearByPhone` / `searchHashByPhone` / `searchBinaryByPhone` | O(n) / O(1) avg / O(log n) |
| Search by name | `searchLinearByName` | O(n) |
| Prefix search | `searchPrefixByPhone(prefix, k)` | O(log n + k) |
| Load / save CSV | `loadfromCSV` / `savetoCSV` | O(n log n) load |

## The three data structures

The phonebook keeps **three** structures in sync:

```
contacts:  vector<Contact>          ← the source of truth (insertion order)
hashtable: HashTable (chained)      ← phone → contact index, O(1) avg
sortedPhones: vector<string>        ← sorted copy of phones, for binary search
```

| Structure | Purpose | Why |
|-----------|---------|-----|
| `contacts` | The actual data, in insertion order | Printing, name search, CSV round-trip |
| `hashtable` | Phone → index lookup | O(1) average search, duplicate check |
| `sortedPhones` | Sorted phone index | O(log n) binary search + prefix search |

The sorted index is the price you pay for binary search: every insert must keep it sorted (O(n) shift), and a delete rebuilds it (O(n log n)).

## The three search algorithms

The whole point of the project is comparing these three ways to find a phone number:

| Algo | Idea | Best | Avg | Worst | Space |
|------|------|------|-----|-------|-------|
| `linear` | scan `contacts[0..n)` until `phone == target` | O(1) (first) | O(n/2) | O(n) (last/miss) | O(1) |
| `hash` | `hash(phone) → bucket → walk chain` | O(1) | O(1) | O(n) (all collide) | O(n) buckets + chains |
| `binary` | binary search on `sortedPhones` | O(1) | O(log n) | O(log n) | O(n) index |

- **`linear`** — no index, no order. Position matters: `first` is 1 compare, `last` is n compares.
- **`hash`** — position-independent. Expected O(1) because the load factor stays ≤ 0.75 (rehash).
- **`binary`** — position-independent, but needs the sorted index. `lowerBound` finds the spot; the index is kept sorted incrementally.

## File map (C++ reference implementation)

```
include/contact.hpp       Contact struct (name, phone)
include/hashtable.hpp     HashTable interface (chained, prime sizing, rehash)
include/phonebook.hpp     PhoneBook interface (search, insert, delete, CSV)
src/hashtable.cpp         hash function, insert/search/delete, rehash
src/phonebook.cpp         CSV, insert/delete, 3 searches, prefix, sorted index
src/main.cpp              CLI menu (options 0–9)
```

Every other language (`python/`, `go/`, `javascript/`, `java/`) is a **faithful port** of this C++ core — see [Language Comparison](../languages/01-language-comparison.md).

---

Next: [02 — Data Structures](02-data-structures.md) — the hash table and sorted index in detail.