// linear_search.hpp — sequential search over an unsorted int array
// Author:  @TraiNguyenVan
// Source:  n/a
// AI use:  n/a
#ifndef LINEAR_SEARCH_HPP
#define LINEAR_SEARCH_HPP

#include <iostream>

using namespace std;

// inline: definition lives in this header, so more than one .cpp may include it
inline int linear_search(int a[], int n, int val) {
    for (int i = 0; i < n; i++) {
        if (a[i] == val) {
            return i;
        }
    }
    return -1;
}

#endif  // LINEAR_SEARCH_HPP