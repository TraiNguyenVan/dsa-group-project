// =============================================================================
//  test_hash.cpp  —  Lecture-time smoke test for HashTableChaining
//                    and HashTableLinearProbing
// =============================================================================
//
//  Build:
//      g++ -std=c++17 -Wall -Wextra src/cpp/test_hash.cpp -o build/cpp/test_hash
//
//  Run:
//      ./build/cpp/test_hash
//
//  Returns exit code 0 on success, non-zero on any failed assertion.
// =============================================================================

#include <cassert>
#include <iostream>
using std::cout;

#include "hashing.hpp"
#include "linear_probing.hpp"

int main() {
    cout << "== HashTableChaining ==\n";
    HashTableChaining ht;
    ht.insert(12);
    ht.insert(13);
    ht.insert(22);        // collides with 12 (12 % 10 == 22 % 10)
    ht.insert(42);        // collides with 2
    assert(ht.search(13));
    assert(ht.search(42));
    assert(!ht.search(99));
    assert(ht.remove(13));
    assert(!ht.search(13));
    ht.display();

    cout << "\n  size        = " << ht.size()
         << "\n  load factor = " << ht.loadFactor() << "\n";

    cout << "\n== HashTableLinearProbing ==\n";
    HashTableLinearProbing lp;
    lp.insert(12);
    lp.insert(13);
    lp.insert(22);        // collides with 12 -> next free slot
    lp.insert(42);
    assert(lp.search(12));
    assert(lp.search(22));
    assert(!lp.search(99));
    assert(lp.remove(13));
    assert(!lp.search(13));
    lp.display();

    cout << "\n  count       = " << lp.count()
         << "\n  load factor = " << lp.loadFactor() << "\n";

    cout << "\nAll assertions passed.\n";
    return 0;
}
