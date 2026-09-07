# Hash Table API Specification

> Cross-language contract for all hash-table implementations in this project.
> Every language (C++, Java, Python, JavaScript, Go) should follow this API shape so implementations stay comparable.

## 1. Supported techniques

| Technique | Category | Notes |
|---|---|---|
| `LinearProbing` | Closed hashing / Open addressing | Probe sequence: `(h(k) + i) % m` |
| `QuadraticProbing` | Closed hashing / Open addressing | Probe sequence: `(h(k) + c1*i + c2*i²) % m` |
| `DoubleHashing` | Closed hashing / Open addressing | Probe sequence: `(h1(k) + i * h2(k)) % m` |
| `SeparateChaining` | Open hashing / Separate chaining | Buckets implemented as linked lists by default |

## 2. Common hash-table interface

Every technique must expose the following public methods.

### Core operations

| Method | Input | Output | Behavior |
|---|---|---|---|
| `insert(key, value)` | `key`, `value` | nothing | Insert or update the value for `key`. |
| `search(key)` | `key` | `bool` | Return `true` if `key` exists, otherwise `false`. |
| `remove(key)` | `key` | `bool` | Remove `key` if it exists. Return `true` if removed, `false` otherwise. |

### Utility / inspection

| Method | Output | Behavior |
|---|---|---|
| `size()` | `int` | Number of slots in the table array (`m`). |
| `count()` | `int` | Number of stored key/value pairs (`n`). |
| `loadFactor()` | `double` | Returns `n / m`. |
| `print()` | nothing | Print the internal state for debugging/demo. Format is language-dependent. |

## 3. Constructor

All tables are constructed with at least the table size `m`.

```cpp
// C++
auto table = LinearProbing<int, int>(size);
auto table = SeparateChaining<int, std::string>(size);
```

```java
// Java
var table = new LinearProbing<Integer, Integer>(size);
var table = new SeparateChaining<Integer, String>(size);
```

```python
# Python
table = LinearProbing(size)
table = SeparateChaining(size)
```

```javascript
// JavaScript
const table = new LinearProbing(size);
const table = new SeparateChaining(size);
```

```go
// Go
table := NewLinearProbing(size)
table := NewSeparateChaining(size)
```

## 4. Key/value types

- **C++:** templated as `HashTable<K, V>`.
- **Java:** generic as `HashTable<K, V>`.
- **Python:** keys and values can be any hashable / comparable objects; examples use `int`.
- **JavaScript:** keys and values are flexible; examples use primitives.
- **Go:** generics `HashTable[K comparable, V any]`.

For the baseline implementations, use `int` keys and `int` values unless the language makes other types more natural.

## 5. Hash function

Default hash function for all techniques:

```
h(key) = key % m
```

For string keys or custom objects, each language uses its own standard hash and reduces it modulo `m`.

## 6. Collision-resolution details

### Closed hashing (open addressing)

- Use a sentinel or state enum for `EMPTY`, `OCCUPIED`, and `DELETED` slots.
- `remove` must use a tombstone/deleted marker so probing chains remain intact.
- The table does **not** automatically resize in the baseline version.

### Separate chaining (open hashing)

- Each bucket is a linked list by default.
- `insert` adds to the head or tail of the list.
- `search` scans the list.
- `remove` unlinks the node if found.

## 7. Error handling

| Situation | Behavior |
|---|---|
| Insert into a full open-addressing table | Throw / return false / report error (language-dependent). |
| Search for a missing key | Return `false` / `null` / `None`. |
| Remove a missing key | Return `false`. |

## 8. Example usage

```cpp
HashTable::LinearProbing<int, int> table(10);
table.insert(5, 50);
table.insert(15, 150);   // collision if h(15) == h(5)
bool found = table.search(5);   // true
table.remove(5);
found = table.search(5);        // false
```

## 9. What is optional / extra

The following are **not** required in the baseline, but may be added later:

- [ ] Automatic resizing / rehashing when load factor exceeds a threshold.
- [ ] Pluggable hash-function objects.
- [ ] Balanced-tree or dynamic-array buckets for separate chaining.
- [ ] Iterator support.

## 10. Naming convention

Use the same class/struct/type names across all languages:

- `LinearProbing`
- `QuadraticProbing`
- `DoubleHashing`
- `SeparateChaining`

In C++, wrap them in the `HashTable` namespace to match the existing file.
