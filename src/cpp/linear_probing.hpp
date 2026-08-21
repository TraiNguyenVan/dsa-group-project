// =============================================================================
//  linear_probing.hpp  —  Hash Table with Linear Probing (Closed Hashing)
// =============================================================================
//
//  LECTURE NOTE  —  PTIT INT1306_CLC  /  Data Structures & Algorithms
//
//  -----------------------------------------------------------------------------
//  COLLISION RESOLUTION — STRATEGY B: OPEN ADDRESSING / LINEAR PROBING
//  -----------------------------------------------------------------------------
//  In Closed Hashing every key is stored INSIDE the table array.  When the
//  bucket at h(k) is already taken we probe (search) the next slots until we
//  find a free one.
//
//  Probing sequence (linear probing):
//        h(k, i) = ( h(k) + i ) mod M     for i = 0, 1, 2, ...
//
//  Visualisation of inserting 12, 13, 14, 15, 16, 17 into M = 10:
//  h(k) = k mod 10
//       index:  0  1  2  3  4  5  6  7  8  9
//      ----------------------------------------
//      insert 12 -> 12 lands at 2                 [2] = 12
//      insert 13 -> 13 lands at 3                 [3] = 13
//      insert 14 -> 14 lands at 4                 [4] = 14
//      insert 15 -> 15 lands at 5                 [5] = 15
//      insert 16 -> 16 lands at 6                 [6] = 16
//      insert 17 -> 17 lands at 7                 [7] = 17
//
//  Now collision in action — insert 27 with h(27) = 7:
//      bucket 7 is taken by 17
//      probe i=1 -> bucket 8 (free)              [8] = 27
//
//  -----------------------------------------------------------------------------
//  CLUSTERING  (the main weakness of linear probing)
//  -----------------------------------------------------------------------------
//  Linear probing creates "clusters" of occupied slots.  Once a cluster forms,
//  every new key that hashes anywhere inside it must walk past the whole cluster
//  before finding an empty slot.  Larger clusters -> slower.
//
//  - PRIMARY CLUSTERING  — caused by the probing rule itself
//                            (any key hashing into a cluster extends it)
//
//  PROPERTIES:
//    (+) Excellent cache locality — all data is in a single contiguous array.
//    (+) Lower memory overhead (no per-node pointers).
//    (-) Performance collapses as alpha approaches 1.
//    (-) Deletion is hard: cannot just clear the slot (breaks probing chains),
//        so we use a "tombstone" DELETED state and rehash when alpha gets large.
//
// =============================================================================

#pragma once

#include <iostream>
using std::cout;

// -----------------------------------------------------------------------------
//  Slot state — needed because integer keys alone cannot tell apart "empty",
//  "deleted", and "occupied".
// -----------------------------------------------------------------------------
enum SlotState { EMPTY = 0, OCCUPIED = 1, DELETED = 2 };

// -----------------------------------------------------------------------------
//  HashTableLinearProbing — Closed Hashing via Linear Probing
// -----------------------------------------------------------------------------
class HashTableLinearProbing {
   private:
    static const int SIZE = 10;        // Number of buckets — M
    int         keys[SIZE];            // stored key (valid only when state==OCCUPIED)
    SlotState   state[SIZE];           // EMPTY / OCCUPIED / DELETED

    int hashFunction(int key) const { return key % SIZE; }

   public:
    // ---- ctor ---------------------------------------------------------------
    HashTableLinearProbing() {
        for (int i = 0; i < SIZE; ++i) {
            keys[i]  = 0;
            state[i] = EMPTY;
        }
    }

    // ---- INSERT --------------------------------------------------------------
    //  Probes forward (i = 0..M-1) until an EMPTY or DELETED slot is found.
    //  Returns false if the table is full.
    bool insert(int key) {
        for (int i = 0; i < SIZE; ++i) {
            int index = (hashFunction(key) + i) % SIZE;

            if (state[index] == EMPTY || state[index] == DELETED) {
                keys[index]  = key;
                state[index] = OCCUPIED;
                return true;
            }
        }
        return false;        // table full
    }

    // ---- SEARCH --------------------------------------------------------------
    //  Same probing walk.  Stops at EMPTY (= key definitely not in table).
    bool search(int key) const {
        for (int i = 0; i < SIZE; ++i) {
            int index = (hashFunction(key) + i) % SIZE;

            if (state[index] == EMPTY) return false;
            if (state[index] == OCCUPIED && keys[index] == key) return true;
            // else DELETED or wrong OCCUPIED -> keep probing
        }
        return false;
    }

    // ---- DELETE --------------------------------------------------------------
    //  Marks the slot DELETED (a "tombstone").  Re-using the slot later is fine
    //  because insert() will accept a DELETED slot.
    bool remove(int key) {
        for (int i = 0; i < SIZE; ++i) {
            int index = (hashFunction(key) + i) % SIZE;

            if (state[index] == EMPTY) return false;
            if (state[index] == OCCUPIED && keys[index] == key) {
                state[index] = DELETED;
                return true;
            }
        }
        return false;
    }

    // ---- DISPLAY -------------------------------------------------------------
    //  Shows bucket, state, and key.  Educational view of clustering.
    void display() const {
        cout << "\n  Hash Table (Linear Probing)  —  M = " << SIZE << "\n";
        cout << "  ---------------------------------\n";
        cout << "  bucket | state     | key\n";
        cout << "  -------+-----------+-------\n";
        for (int i = 0; i < SIZE; ++i) {
            const char* s = (state[i] == EMPTY) ? "EMPTY"
                            : (state[i] == OCCUPIED) ? "OCCUPIED"
                                                     : "DELETED";
            cout << "    " << i << "    | " << s << "  | ";
            if (state[i] == OCCUPIED) cout << keys[i];
            cout << "\n";
        }
    }

    // ---- METRICS -------------------------------------------------------------
    int count() const {
        int n = 0;
        for (int i = 0; i < SIZE; ++i) if (state[i] == OCCUPIED) ++n;
        return n;
    }

    double loadFactor() const { return static_cast<double>(count()) / SIZE; }
};
