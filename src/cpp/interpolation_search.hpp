// interpolation_search.hpp — interpolation search on a sorted int array
// Author:  @RolandZimmer
// Source:  n/a
// AI use:  n/a
#ifndef INTERPOLATION_SEARCH_HPP
#define INTERPOLATION_SEARCH_HPP

// inline: definition lives in this header, so more than one .cpp may include it
inline int interpolation_search(int arr[], int n, int x) {
    int low = 0;
    int high = n - 1;
    while (x >= arr[low] && x <= arr[high] && low <= high) {
        if (low == high) {
            if (x == arr[low]) {
                return low;
            }
            return -1;
        }
        int pos = low + (long long)(x - arr[low]) * (high - low) / (arr[high] - arr[low]);
        if (arr[pos] == x) {
            return pos;
        } else if (arr[pos] < x) {
            low = pos + 1;
        } else {
            high = pos - 1;
        }
    }
    // missing a semi-colon here. i put a ";"
    return -1;
}

#endif  // INTERPOLATION_SEARCH_HPP
