# Hash Table Codebase Design Draft

> This is a planning draft. Nothing here is final — pick the parts that make sense for the project and ignore the rest.

## 1. Big picture

The project currently mixes two different ideas under one namespace:

- **Hash functions** — how a key becomes an index (`MidSquare`, division method, etc.).
- **Collision-resolution strategies** — what happens when two keys map to the same index (`LinearProbing`, `SeparateChaining`, etc.).

A cleaner structure separates these two concerns and gives every hash-table technique a clear home.

## 2. Proposed directory tree

```
dsa-group-project/
├── include/
│   └── hash/
│       ├── hash_table_base.hpp          # Common interface all tables share
│       ├── hash_functions.hpp           # Division, Mid-Square, etc.
│       │
│       ├── open_addressing/
│       │   ├── open_addressing_base.hpp # Shared probing helpers (state enum, tombstone logic)
│       │   ├── linear_probing.hpp
│       │   ├── quadratic_probing.hpp
│       │   └── double_hashing.hpp
│       │
│       └── separate_chaining/
│           ├── separate_chaining.hpp    # Main table class
│           └── chain_storage/
│               ├── chain_base.hpp       # Common chain interface
│               ├── linked_list_chain.hpp
│               ├── balanced_tree_chain.hpp
│               └── dynamic_array_chain.hpp
│
├── src/                                 # Optional: mostly empty for header-only templates
│   └── hash/
│
├── tests/
│   └── hash/
│       ├── test_hash_functions.cpp
│       ├── test_linear_probing.cpp
│       ├── test_quadratic_probing.cpp
│       ├── test_double_hashing.cpp
│       └── test_separate_chaining.cpp
│
├── examples/
│   └── hash_demo.cpp
│
├── CMakeLists.txt
└── README.md
```

## 3. What each piece is for

| File / folder | Responsibility |
|---|---|
| `hash_table_base.hpp` | Defines the contract every hash table must satisfy: `insert`, `search`, `remove`, `load_factor`, etc. |
| `hash_functions.hpp` | Small, reusable callables that map a key → index. `DivisionHash`, `MidSquareHash`, etc. |
| `open_addressing_base.hpp` | Holds things all probing tables need: an enum for `EMPTY` / `OCCUPIED` / `DELETED`, maybe a slot struct. |
| `linear_probing.hpp`, `quadratic_probing.hpp`, `double_hashing.hpp` | Concrete collision-resolution strategies. They inherit or compose the base. |
| `separate_chaining.hpp` | The table array itself. |
| `chain_storage/` | The different bucket implementations you can plug into separate chaining. |
| `tests/` | One focused test file per technique. |
| `examples/` | A small `main()` demo showing how to swap techniques. |

## 4. Concept map

```
Hashing / Hash Tables
│
├── 1. HASH FUNCTIONS        ← "How do I turn a key into an index?"
│   ├── Division method      (key % table_size)
│   ├── Mid-Square method    (square key, take middle digits)
│   ├── Folding method
│   ├── Multiplicative method
│   └── Universal hashing
│
└── 2. COLLISION RESOLUTION  ← "What if two keys land in the same slot?"
    │
    ├── A. OPEN HASHING
    │   └── a.k.a. SEPARATE CHAINING
    │       └── The chain/bucket outside the table can be implemented as:
    │           ├── Linked list
    │           ├── Balanced tree (e.g. AVL, Red-Black)
    │           └── Dynamic array / vector
    │
    └── B. CLOSED HASHING
        └── a.k.a. OPEN ADDRESSING
            └── All records stay inside the table array.
                If slot h(key) is taken, probe for another empty slot:
                ├── Linear Probing
                │   └── (index + 1) % m, then (index + 2) % m, ...
                ├── Quadratic Probing
                │   └── (index + 1²) % m, (index + 2²) % m, ...
                └── Double Hashing
                    └── (index + i * h₂(key)) % m
```

## 5. Naming note

| Informal name | Formal name | Meaning |
|---|---|---|
| **Closed hashing** | Open addressing | Everything stays *closed* inside the table array. |
| **Open hashing** | Separate chaining | Data can spill *outside* the table into chains/buckets. |

Suggested class names to avoid confusion:

- `LinearProbing`
- `QuadraticProbing`
- `DoubleHashing`
- `SeparateChaining`
- `MidSquareHash` (as a hash-function class, not a table)

## 6. Two possible design flavors

### Flavor A: Inheritance (runtime swap)

```cpp
// hash_table_base.hpp
template <typename K, typename V>
class HashTable {
public:
    virtual void insert(const K& key, const V& value) = 0;
    virtual bool search(const K& key) const = 0;
    virtual bool remove(const K& key) = 0;
    virtual ~HashTable() = default;
};
```

Then:

```cpp
class LinearProbing : public HashTable<int, std::string> { ... };
class SeparateChaining : public HashTable<int, std::string> { ... };
```

**Pros:** easy to swap implementations at runtime (`HashTable* t = new LinearProbing(...)`).  
**Cons:** virtual dispatch overhead.

### Flavor B: Policy / template composition (compile-time swap)

```cpp
template <typename K, typename V, typename ProbeStrategy>
class OpenAddressingTable { ... };
```

Then:

```cpp
using LinearTable    = OpenAddressingTable<int, std::string, LinearProbing>;
using QuadraticTable = OpenAddressingTable<int, std::string, QuadraticProbing>;
```

**Pros:** zero runtime overhead, modern C++ style.  
**Cons:** harder to swap at runtime.

## 7. Where the current file fits

Current `include/hash.hpp`:

```
HashTable namespace
├── LinearProbing        → Closed Hashing / Open Addressing
├── SeperatedChaining    → Open Hashing / Separate Chaining  (empty skeleton, typo in name)
└── MidSquare            → Hash Function  (not a table!)
```

## 8. Multi-language constraint

Since the same techniques must be implemented in **C++, Java, Python, JavaScript, and Go**, the C++ design should be easy to translate.

- Keep the public API simple and consistent across languages.
- Avoid C++-only tricks (complex templates, `std::optional`, move semantics) in the baseline.
- Define the cross-language contract in [`hash_api_spec.md`](hash_api_spec.md).

## 9. Recommended baseline design

For easy porting, start with:

```
namespace HashTable {

class LinearProbing {
public:
    LinearProbing(int size);
    void insert(int key, int value);
    bool search(int key) const;
    bool remove(int key);
    int size() const;
    int count() const;
    double loadFactor() const;
    void print() const;
private:
    // table array + state tracking
};

class SeparateChaining { /* same API */ };
class QuadraticProbing { /* same API */ };
class DoubleHashing { /* same API */ };

} // namespace HashTable
```

This is plain enough to port line-by-line to the other languages while still being clean C++.

## 10. Open decisions

- [x] Keep `HashTable` as a **namespace** for the baseline (yes, for portability).
- [ ] Confirm the API in [`hash_api_spec.md`](hash_api_spec.md) works for all target languages.
- [ ] Choose the sentinel/state representation for open-addressing tables.
- [ ] Decide whether separate chaining uses the existing `linkedlist.hpp` or a built-in list.
- [ ] Implement **deletion** and **tombstones** now, or defer them?
- [ ] Add **automatic resizing** later as an optional extension.
