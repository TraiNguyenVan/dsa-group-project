// Build and Run with:
// g++ -std=c++17 src/cpp/main.cpp -I include -o build/cpp/main
// build/cpp/main

// Part C — C++ demo skeleton (PTIT INT1306_CLC group project)
// Topic: TBD. Replace this skeleton with your real-world demo.
// Rules: implement the structure yourself (STL comparison OK, substitution NOT).
// Must compile with: g++ -std=c++17 -Wall
// Must run on n >= 100 000, print timings, handle edge cases.

#include <iostream>
#include <vector>

#include "binarySearch.hpp"
#include "linear_search.hpp"
#include "largest_element.hpp"
#include "interpolation_search.hpp"

// when your algorithm found the target's index, use this to print it out
void print(int index) {
    if (index == -1) {
        cout << "Value not found!\n";
    } else {
        cout << "Value found at index " << index << "!\n";
    }
}

int main(int argc, char* argv[]) {
    /*
    ____________________________Data here______________________________________
    */
    // random data
    const int n = 10;
    int arr1[n] = {12, 4, 9, 22, 7, 15, 3, 18, 11, 6};
    // sorted data
    int arr2[n] = {2, 5, 8, 11, 14, 17, 20, 23, 26, 29};

    // we are searching this value in the array
    int val = 67;

    // print the dataset out
    cout << "Array for searching demo(Randomized): ";
    for (int i = 0; i < n; ++i) {
        cout << arr1[i] << (i + 1 < n ? " " : "");
    }
    cout << endl;
    cout << "Array for searching demo(Sorted): ";
    for (int i = 0; i < n; ++i) {
        cout << arr2[i] << (i + 1 < n ? " " : "");
    }
    cout << endl;
    /*
    ____________________________Algorithms here________________________________
    */
    // Linear Search
    cout << "Searching with Linear Search: Target(" << val << ")\n";
    int index = linear_search(arr1, n, val);
    print(index);

    // Binary Search

    // Need Sorted array
    cout << "Searching with Binary Search: Target(" << val << ")\n";
    int res = binarySearch(arr2, n, val);
    print(res);

    // Interpolation search

    // Interpolation search need Sorted array
    // where the values in a sorted array are uniformly distributed
    cout << "Searching with Interpolation Search: Target(" << val << ")\n";
    int ans = interpolation_search(arr2, n, val);
    print(ans);

    // cout << largestRescusion(arr1, n) << "\n";

    return 0;
}
