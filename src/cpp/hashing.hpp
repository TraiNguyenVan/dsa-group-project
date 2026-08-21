// =============================================================================
//  hashing.hpp  —  Hash Table with Separate Chaining (Open Hashing)
// =============================================================================
//
//  LECTURE NOTE  —  PTIT INT1306_CLC  /  Data Structures & Algorithms
//
//  -----------------------------------------------------------------------------
//  WHAT IS A HASH TABLE?
//  -----------------------------------------------------------------------------
//  A hash table provides *average-case O(1)* key-to-value mapping by using a
//  hash function h(k) that converts a key k into an index in [0, M-1].
//
//  Direct addressing   : key k  ->  bucket h(k)     (one array slot per key)
//  Indirect addressing : key k  ->  bucket h(k) % M (shared buckets = collision)
//
//  The Pigeonhole Principle guarantees that if N > M (more keys than buckets),
//  at least two distinct keys MUST share the same bucket.  This event is
//  called a COLLISION, and resolving it efficiently is the entire problem
//  of hashing.
//
//  -----------------------------------------------------------------------------
//  COLLISION RESOLUTION — STRATEGY A: SEPARATE CHAINING  (this file)
//  -----------------------------------------------------------------------------
//  Each bucket holds the HEAD of a linked list.  Colliding keys live in the
//  same bucket but on different nodes of its list.
//
//        table[0]            table[5]            table[7]
//       +--------+          +--------+          +--------+
//       |  20 -> | ----->   |  15 -> | ----->   |  42 -> |  NULL
//       |  10 -> |  NULL    |  25 -> |  NULL    |        |
//       +--------+          +--------+          +--------+
//
//  - Load factor  alpha = N / M          (number of keys / number of buckets)
//  - Average chain length is alpha       (independent of the hashing quality)
//  - Worst case O(n) when EVERY key hashes to the same bucket
//  - Average case O(1 + alpha)          for insert / search / delete
//
//  PROPERTIES:
//    (+) Simple to implement; works even when alpha > 1.
//    (+) Deletion is trivial: unlink the node, free its memory.
//    (-) Pointer overhead per node; poorer cache locality than open addressing.
//
//  -----------------------------------------------------------------------------
//  HASH FUNCTION USED HERE — DIVISION METHOD
//  -----------------------------------------------------------------------------
//      h(k) = k mod M
//
//  Cheap, deterministic, and "good enough" for small integer keys.
//  (For production code you'd avoid even M, use a prime, or use the
//  multiplication method h(k) = floor( M * ( k*A mod 1 ) ).)
//
// =============================================================================

#pragma once

#include <iostream>
using std::cout;

// -----------------------------------------------------------------------------
//  Node — a single element in a bucket's chain
// -----------------------------------------------------------------------------
struct Node {
    int   key;       // The stored value
    Node* next;      // Pointer to the next node in the same bucket (or nullptr)

    explicit Node(int k) : key(k), next(nullptr) {}
};

// -----------------------------------------------------------------------------
//  HashTableChaining — Open Hashing via Separate Chaining
// -----------------------------------------------------------------------------
class HashTableChaining {
   private:
    static const int SIZE = 10;     // Number of buckets — M
    Node* table[SIZE];              // table[i] = head of chain at bucket i

    // h(k) = k mod M   — the Division Method
    int hashFunction(int key) const { return key % SIZE; }

   public:
    // ---- ctor / dtor ---------------------------------------------------------
    HashTableChaining() {
        for (int i = 0; i < SIZE; ++i) table[i] = nullptr;
    }

    ~HashTableChaining() { clear(); }

    // Disable copy (we own raw pointers) — students can add copy/ move later.
    HashTableChaining(const HashTableChaining&) = delete;
    HashTableChaining& operator=(const HashTableChaining&) = delete;

    // ---- INSERT --------------------------------------------------------------
    //  Prepend new node at head of chain — O(1).
    //  Duplicates are kept (each insert() creates a new node).
    void insert(int key) {
        int   index = hashFunction(key);
        Node* node  = new Node(key);
        node->next  = table[index];
        table[index] = node;
    }

    // ---- SEARCH --------------------------------------------------------------
    //  Walk the chain at bucket h(key).  Returns true if found.
    bool search(int key) const {
        Node* current = table[hashFunction(key)];
        while (current != nullptr) {
            if (current->key == key) return true;
            current = current->next;
        }
        return false;
    }

    // ---- DELETE --------------------------------------------------------------
    //  Removes the FIRST node matching `key` in its bucket.
    //  Returns true if a node was removed.
    bool remove(int key) {
        int   index  = hashFunction(key);
        Node* cur    = table[index];
        Node* prev   = nullptr;

        while (cur != nullptr) {
            if (cur->key == key) {
                if (prev == nullptr) {
                    table[index] = cur->next;     // head of chain
                } else {
                    prev->next = cur->next;        // middle / tail
                }
                delete cur;
                return true;
            }
            prev = cur;
            cur  = cur->next;
        }
        return false;
    }

    // ---- DISPLAY -------------------------------------------------------------
    //  Prints every bucket and its chain.  Useful for lecture-time tracing.
    void display() const {
        cout << "\n  Hash Table (Separate Chaining)  —  M = " << SIZE << "\n";
        cout << "  ---------------------------------\n";
        for (int i = 0; i < SIZE; ++i) {
            cout << "  Bucket " << i << ": ";
            Node* cur = table[i];
            if (cur == nullptr) {
                cout << "NULL";
            } else {
                while (cur != nullptr) {
                    cout << cur->key;
                    if (cur->next != nullptr) cout << " -> ";
                    cur = cur->next;
                }
                cout << " -> NULL";
            }
            cout << "\n";
        }
    }

    // ---- METRICS -------------------------------------------------------------
    int  size() const {
        int n = 0;
        for (int i = 0; i < SIZE; ++i)
            for (Node* c = table[i]; c != nullptr; c = c->next) ++n;
        return n;
    }

    double loadFactor() const { return static_cast<double>(size()) / SIZE; }

    // ---- CLEAR ---------------------------------------------------------------
    void clear() {
        for (int i = 0; i < SIZE; ++i) {
            Node* cur = table[i];
            while (cur != nullptr) {
                Node* tmp = cur;
                cur       = cur->next;
                delete tmp;
            }
            table[i] = nullptr;
        }
    }
};
