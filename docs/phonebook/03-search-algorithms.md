# 03 — Search Algorithms

The three ways to find a contact by phone number, plus the name search and prefix search. All code is the C++ reference (`src/phonebook.cpp`).

## 1. Linear search — `searchLinearByPhone`

```cpp
int PhoneBook::searchLinearByPhone(const std::string& phone) const {
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].phone == phone) return static_cast<int>(i);
    }
    return -1;
}
```

- Scans `contacts[0..n)` until the phone matches.
- **Position matters:** `first` = 1 compare (best), `last`/`miss` = n compares (worst).
- No index, no order — works on the raw vector.

| Case | Compares | Complexity |
|------|----------|------------|
| `first` | 1 | O(1) |
| `middle` | n/2 | O(n/2) |
| `last` | n | O(n) |
| `miss` | n (full scan) | O(n) |

## 2. Hash search — `searchHashByPhone`

```cpp
int PhoneBook::searchHashByPhone(const std::string& phone) const {
    return hashtable.hashSearch(phone);
}
```

- `hash(phone) → bucket → walk chain` (see [02 — Data Structures](02-data-structures.md)).
- **Position-independent** — `first` and `last` cost the same.
- Expected O(1) because the load factor is capped at 0.75 (rehash keeps chains short).
- Worst case O(n) only if every phone collides into one chain (pathological).

## 3. Binary search — `searchBinaryByPhone`

```cpp
int PhoneBook::searchBinaryByPhone(const std::string& phone) const {
    std::size_t lo = 0, hi = sortedPhones.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (sortedPhones[mid] == phone) {
            return hashtable.hashSearch(phone);  // map phone → contact index
        }
        if (sortedPhones[mid] < phone) lo = mid + 1;
        else hi = mid;
    }
    return -1;
}
```

- Binary search on the **sorted index** (`sortedPhones`).
- **Position-independent** — always ~`log2(n)` probes.
- On hit, it uses the hash table to map the phone back to the contact index (the sorted index stores phones, not indices).
- The price: the sorted index must be maintained (O(n) insert shift, O(n log n) rebuild on delete).

| n | Max probes |
|---|-----------|
| 50 | 6 |
| 10k | 14 |
| 100k | 17 |
| 1M | 20 |

## 4. Name search — `searchLinearByName`

```cpp
int PhoneBook::searchLinearByName(const std::string& name) const {
    std::string target = toLower(name);
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (toLower(contacts[i].name) == target) return static_cast<int>(i);
    }
    return -1;
}
```

- Linear scan over names, **case-insensitive** (`toLower` on both sides).
- O(n) — there's no name index.

## 5. Prefix search — `searchPrefixByPhone`

```cpp
std::vector<std::size_t> PhoneBook::searchPrefixByPhone(const std::string& phone, std::size_t k) const {
    std::vector<std::size_t> result;
    std::size_t pos = lowerBound(sortedPhones, phone);   // first phone >= prefix
    for (std::size_t i = pos; (i < pos + k) && (i < sortedPhones.size()); ++i) {
        if (sortedPhones[i].compare(0, phone.size(), phone) != 0) break;  // no longer starts with prefix
        int index = hashtable.hashSearch(sortedPhones[i]);
        if (index != -1) result.push_back(static_cast<std::size_t>(index));
    }
    return result;
}
```

- `lowerBound` jumps to the first phone ≥ the prefix (O(log n)).
- Then walks forward collecting up to `k` phones that **start with** the prefix (O(k)).
- Stops early as soon as a phone no longer starts with the prefix (the sorted order guarantees all matches are contiguous).
- Total: **O(log n + k)**.

## Complexity summary

| Search | Best | Avg | Worst | Needs |
|--------|------|-----|-------|-------|
| `linear` | O(1) | O(n/2) | O(n) | nothing |
| `hash` | O(1) | O(1) | O(n) | hash table |
| `binary` | O(1) | O(log n) | O(log n) | sorted index |
| `name` (linear) | O(1) | O(n/2) | O(n) | nothing |
| `prefix` | O(log n) | O(log n + k) | O(log n + k) | sorted index |

The benchmark harness measures the first three on identical data across five languages — see [Benchmark Modes](../benchmark/04-benchmark-modes.md) and [Language Comparison](../languages/01-language-comparison.md).

---

Next: [04 — Implementation Notes](04-implementation-notes.md) — CSV parsing, normalization, and the design decisions.