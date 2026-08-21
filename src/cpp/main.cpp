// Build and Run with:
//   make -C src/cpp           # builds demo
//   ./build/cpp/demo          # runs all demos
//
// =============================================================================
//  Part C  —  Searching + Hashing demo (PTIT INT1306_CLC group project)
// =============================================================================
//  Demonstrates four data structures side-by-side:
//     1. Linear search          (linear_search.hpp)
//     2. Binary search          (binarySearch.hpp)
//     3. Interpolation search   (interpolation_search.hpp)
//     4. Hash table (chaining)  (hashing.hpp)
//
//  Topic for the larger project: TBD.  Replace the placeholder datasets with
//  your real-world demo once the group picks a topic.
//
//  Rules:
//    - Implement the structure yourself (STL comparison OK, substitution NOT).
//    - Must compile with:  g++ -std=c++17 -Wall -Wextra
//    - Must run on n >= 100 000 (we use small datasets here for teaching).
// =============================================================================

#include <iostream>
#include <vector>

#include "binarySearch.hpp"
#include "hashing.hpp"
#include "interpolation_search.hpp"
#include "linear_search.hpp"

using std::cout;
using std::endl;

// -----------------------------------------------------------------------------
//  Pretty-printing helpers
// -----------------------------------------------------------------------------
void print(int index) {
    if (index == -1) {
        cout << "Value not found!" << endl;
    } else {
        cout << "Value found at index " << index << "!" << endl;
    }
}

void section(const char* title) {
    cout << "\n==================================================\n"
         << "  " << title << "\n"
         << "==================================================\n";
}

// -----------------------------------------------------------------------------
//  Hashing demo — separate chaining
// -----------------------------------------------------------------------------
void demoHashChaining() {
    section("DEMO 4 — HASH TABLE (SEPARATE CHAINING)");

    HashTableChaining ht;

    int keys[] = {12, 22, 42, 17, 27, 37, 7, 19, 29, 9};
    for (int k : keys) ht.insert(k);

    cout << "\n  Inserted " << ht.size() << " keys into the table.\n";
    ht.display();

    cout << "\n  -- Search tests --\n";
    for (int q : {22, 27, 99}) {
        cout << "  search(" << q << ") -> "
             << (ht.search(q) ? "FOUND" : "NOT FOUND") << "\n";
    }

    cout << "\n  -- Delete test (remove key 22) --\n";
    cout << "  remove(22) -> " << (ht.remove(22) ? "DELETED" : "MISS") << "\n";
    ht.display();

    cout << "\n  load factor = " << ht.loadFactor() << "\n";
}

// -----------------------------------------------------------------------------
//  main
// -----------------------------------------------------------------------------
int main(int /*argc*/, char* /*argv*/[]) {
    // ----- Datasets ----------------------------------------------------------
    const int n = 10;
    int arr1[n] = {12, 4, 9, 22, 7, 15, 3, 18, 11, 6};   // unsorted
    int arr2[n] = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29};  // sorted
    int val = 67;                                        // not present -> -1

    cout << "Array for searching demo (random): ";
    for (int i = 0; i < n; ++i) cout << arr1[i] << (i + 1 < n ? " " : "");
    cout << endl;
    cout << "Array for searching demo (sorted): ";
    for (int i = 0; i < n; ++i) cout << arr2[i] << (i + 1 < n ? " " : "");
    cout << endl;

    // ----- DEMO 1: Linear search ---------------------------------------------
    section("DEMO 1 — LINEAR SEARCH");
    cout << "Searching with Linear Search: Target(" << val << ")\n";
    print(linear_search(arr1, n, val));

    // ----- DEMO 2: Binary search ---------------------------------------------
    section("DEMO 2 — BINARY SEARCH");
    cout << "Searching with Binary Search: Target(" << val << ")\n";
    print(binarySearch(arr2, n, val));

    // ----- DEMO 3: Interpolation search --------------------------------------
    section("DEMO 3 — INTERPOLATION SEARCH");
    cout << "Searching with Interpolation Search: Target(" << val << ")\n";
    print(interpolation_search(arr2, n, val));

    // ----- DEMO 4: Hash table — separate chaining ---------------------------
    demoHashChaining();

    return 0;
}
