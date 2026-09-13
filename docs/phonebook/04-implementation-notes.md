# 04 — Implementation Notes

Design decisions and edge cases in the C++ reference implementation (`src/` + `include/`). Every port mirrors these.

## 1. CSV parsing — `parseCsvLine`

The loader handles real-world CSV, not just the generated datasets:

```cpp
// src/phonebook.cpp
static bool parseCsvLine(const std::string& line, std::string& name, std::string& phone) {
    std::string l = trimCsv(line);
    if (l.empty()) return false;

    if (l[0] == '"') {
        // Quoted name: scan for closing quote, honouring "" escapes
        // e.g. "Do, Thanh Tuan",0939149732
        ...
    } else {
        // Unquoted: split on LAST comma
        std::size_t comma = l.rfind(',');
        ...
    }
    ...
}
```

- **Quoted names** (`"Do, Thanh Tuan",0939149732`) — a name containing a comma must be quoted; `""` inside is an escaped quote.
- **Unquoted** — split on the **last** comma (names can contain commas only if quoted, so the last comma is the name/phone separator).
- **Trims** whitespace, strips `\r` (Windows line endings), skips blank lines.
- **Validation:** drops rows with empty names or non-digit phones (`isAllDigits`).
- **Duplicates:** skips a phone already in the hash table.

## 2. Name normalization

```cpp
// Lowercase everything, then capitalize the first letter of each word
std::string PhoneBook::toLower(const std::string& s) { ... }
std::string PhoneBook::capitalizeFirst(const std::string& s) { ... }
```

- `toLower` — ASCII-only lowercase (A–Z → a–z).
- `capitalizeFirst` — capitalizes the first letter of each space-separated word.
- Applied on **load** and **insert**, so `"nguyen van an"` and `"Nguyen Van An"` store identically.
- Name search is case-insensitive (`toLower` on both sides).

## 3. Insert — the O(n) cost of binary search

```cpp
bool PhoneBook::insertContact(const std::string& name, const std::string& phone) {
    // validate: non-empty, all digits, not a duplicate
    ...
    // 1. push to vector + hash insert (O(1))
    contacts.push_back({normalizedName, phone});
    hashtable.hashInsert(phone, contacts.size() - 1);

    // 2. keep sortedPhones sorted: lowerBound (O(log n)) + shift (O(n))
    std::size_t pos = lowerBound(sortedPhones, phone);
    sortedPhones.insert(sortedPhones.begin() + pos, phone);
    ...
}
```

The insert prints the split timing:

```
hash insert: 0.001ms, sorted-index insert: 0.05ms (the cost of keeping binary search possible)
```

That's the honest trade-off: **binary search is only O(log n) because every insert pays O(n) to keep the index sorted.**

## 4. Delete — O(n) by design

```cpp
bool PhoneBook::deleteContactByPhone(const std::string& phone) {
    int idx = searchHashByPhone(phone);
    if (idx == -1) return false;
    contacts.erase(contacts.begin() + idx);   // O(n) shift
    hashtable.clear();                        // drop all chains
    for (int i = 0; i < contacts.size(); ++i)
        hashtable.hashInsert(contacts[i].phone, i);  // rebuild with new indices
    buildSortedIndex();                       // O(n log n) re-sort
    return true;
}
```

- `vector::erase` shifts the tail — O(n).
- The hash table stores **indices**, so after the shift every index changes → full rebuild.
- The sorted index is rebuilt from scratch.
- **Why not swap-with-last?** That would be O(1) but reorders `contacts`, breaking insertion order (printing, CSV round-trip). The DSA-course version keeps order at O(n) cost.

## 5. Sorted index build — merge sort

```cpp
void PhoneBook::buildSortedIndex() {
    sortedPhones.clear();
    sortedPhones.reserve(contacts.size());
    for (const auto& c : contacts) sortedPhones.push_back(c.phone);
    if (sortedPhones.size() >= 2) {
        divide(sortedPhones, 0, static_cast<int>(sortedPhones.size()) - 1);
    }
}
```

- `divide`/`mix` is a **merge sort** (O(n log n)) — no `std::sort`, so the algorithm is visible and portable.
- **Guard:** `size() >= 2` — an empty vector would underflow `0 - 1` to a huge `size_t` and recurse forever.

## 6. Memory safety — deleted copies

```cpp
// include/phonebook.hpp
PhoneBook(const PhoneBook&) = delete;             // ban copy constructor
PhoneBook& operator=(const PhoneBook&) = delete;  // ban copy assignment
PhoneBook(PhoneBook&&) = delete;                  // ban move constructor
PhoneBook& operator=(PhoneBook&&) = delete;       // ban move assignment
```

- The hash table owns `HashNode*` chains. Copying a `PhoneBook` would **double-free** the chains (two objects sharing the same nodes).
- Copy/move are deleted — the compiler rejects any accidental copy.
- `HashTable` does the same for the same reason.

## 7. Why the ports differ (only where forced)

The Python/Go/JS/Java ports are line-by-line faithful, except where the runtime forces it:

| Difference | Why |
|------------|-----|
| 64-bit wrap (`& MASK64`) | C++ `size_t` wraps on overflow; Python/JS need explicit masking |
| `BigInt` in JS | JS numbers can't hold 64-bit hash intermediates |
| `remainderUnsigned` in Java | Java `%` is signed; C++ `%` on `size_t` is unsigned |
| `perf_counter` / `nanoTime` / `performance.now` | per-language wall-clock (see [Timing Harness](../benchmark/03-timing-harness.md)) |

---

Next: [Language Comparison](../languages/01-language-comparison.md) — how the five ports compare.