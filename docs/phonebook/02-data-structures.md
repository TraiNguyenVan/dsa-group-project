# 02 — Data Structures

The phonebook's three structures, in detail. All code below is the C++ reference (`src/` + `include/`); every port mirrors it.

## 1. `Contact` — the record

```cpp
// include/contact.hpp
struct Contact {
    std::string name;
    std::string phone;
};
```

Two fields, nothing else. The phone is the **key** (unique); the name is the payload.

## 2. `HashTable` — chained hash table

```cpp
// include/hashtable.hpp
struct HashNode {
    std::string phone;
    int contactIndex;   // index into PhoneBook::contacts
    HashNode* next;     // chain (separate chaining)
};
```

### Hash function — polynomial rolling hash

```cpp
// src/hashtable.cpp
std::size_t HashTable::HashForSize(const std::string& phone, const std::size_t mod) {
    std::size_t hash = 0;
    for (char c : phone) {
        hash = (hash * HASH_BASE + (unsigned char)(c - '0'));
    }
    return hash % mod;
}
```

- `HASH_BASE = 31` — polynomial rolling hash over the digits.
- Modulo applied **at the end** (`hash % mod`) — the comment says "at each step" but the code does it once; the `size_t` wrap handles overflow.
- `mod = numBuckets` — always prime (see below).

### Prime sizing & rehash

```cpp
static constexpr std::size_t DEFAULT_TABLE_SIZE = 101;
static constexpr double MAX_LOAD_FACTOR = 0.75;
```

- Table starts at `nextPrime(101)` buckets.
- **Load factor** = `numElements / numBuckets`. When it exceeds `0.75`, `maybeRehash()` doubles and rehashes to the next prime.
- `isPrime` uses the **6k ± 1** test: every prime > 3 is of the form `6k ± 1`, so only divisibility by 2, 3, and `6k ± 1` up to `√n` is checked.

```cpp
bool HashTable::isPrime(std::size_t n) {
    if (n <= 3) return n > 1;
    if (n % 2 == 0 || n % 3 == 0) return false;
    std::size_t i = 5;
    while (i * i <= n) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
        i += 6;
    }
    return true;
}
```

### Insert — head-of-chain

```cpp
void HashTable::hashInsert(const std::string& phone, int contactIndex) {
    std::size_t idx = hashFunction(phone);
    HashNode* node = new HashNode{phone, contactIndex, buckets[idx]};  // head insert
    buckets[idx] = node;
    ++numElements;
    maybeRehash();
}
```

New nodes go at the **head** of the chain — O(1), no tail walk.

### Search — walk the chain

```cpp
int HashTable::hashSearch(const std::string& phone) const {
    std::size_t idx = hashFunction(phone);
    HashNode* node = buckets[idx];
    while (node != nullptr) {
        if (node->phone == phone) return node->contactIndex;
        node = node->next;
    }
    return -1;  // not found
}
```

Expected O(1) because the load factor is capped at 0.75 — chains stay short.

### Delete — unlink from chain

```cpp
bool HashTable::hashDelete(const std::string& phone) {
    std::size_t idx = hashFunction(phone);
    HashNode* node = buckets[idx];
    HashNode* prev = nullptr;
    while (node != nullptr) {
        if (node->phone == phone) {
            if (prev != nullptr) prev->next = node->next;
            else buckets[idx] = node->next;
            delete node;
            --numElements;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}
```

O(1) average — unlink and free, no compaction.

### Memory safety

- `~HashTable()` walks every chain and `delete`s every node.
- Copy/move are **deleted** (`= delete`) — a `PhoneBook` can't be copied, which prevents double-free of the chains.

## 3. `sortedPhones` — the sorted index

```cpp
// include/phonebook.hpp
std::vector<std::string> sortedPhones;  // sorted copy of phones
```

A **separate** sorted vector of phone strings, kept in sync:

- **Load:** `buildSortedIndex()` copies all phones and **merge-sorts** them (`divide`/`mix` in `src/phonebook.cpp`) — O(n log n), once.
- **Insert:** `lowerBound` finds the insertion spot (O(log n)), then `vector::insert` shifts the tail (O(n)).
- **Delete:** rebuilds from scratch (O(n log n)) — delete is O(n) anyway.
- **Search:** `searchBinaryByPhone` does a classic binary search; on hit it returns `hashtable.hashSearch(phone)` to map the phone back to the contact index.

### `lowerBound` — the primitive

```cpp
std::size_t PhoneBook::lowerBound(const std::vector<std::string>& v,
                                  const std::string& target) {
    std::size_t lo = 0, hi = v.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (v[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;  // first index where v[i] >= target
}
```

Used by both binary search and prefix search.

## Why three structures?

| Structure | Insert | Search | Delete | Memory |
|-----------|--------|--------|--------|--------|
| `contacts` (vector) | O(1) amortized | O(n) | O(n) erase | dense, cache-friendly |
| `hashtable` (chained) | O(1) avg | O(1) avg | O(1) avg | buckets + nodes |
| `sortedPhones` (vector) | O(n) shift | O(log n) | O(n log n) rebuild | dense copy |

No single structure is best at everything — that's the DSA point. The hash table wins on search, the vector wins on iteration/printing, the sorted index enables binary + prefix search.

---

Next: [03 — Search Algorithms](03-search-algorithms.md) — the three searches and the prefix search.