// binarySearch.hpp — iterative binary search on a sorted int array
// Author:  @Alex75762107
// Source:  https://www.geeksforgeeks.org/dsa/search-insert-position-of-k-in-a-sorted-array/
// AI use:  n/a
#ifndef BINARY_SEARCH_HPP
#define BINARY_SEARCH_HPP

#include <iostream>

// inline: definition lives in this header, so more than one .cpp may include it
inline int binarySearch(int arr[], int n, int x) {
    int low = 0;
    int high = n - 1;

    while (high >= low) {
        int mid = low + (high - low) / 2;

        // If found at mid, then return it
        if (x == arr[mid]) {
            return mid;
        }
        // Search the right half
        else if (x > arr[mid]) {
            low = mid + 1;
        } else {
            high = mid - 1;  // Search the left half
        }
    }
    return -1;
}

#endif  // BINARY_SEARCH_HPP
