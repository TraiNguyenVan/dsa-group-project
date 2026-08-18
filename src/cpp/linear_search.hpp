// Simple implementation of linear search 
// By @TraiNguyenVan

#include <iostream>

using namespace std;

int linear_search(int a[], int n, int val) {
    for (int i = 0; i < n; i++) {
        if (a[i] == val)
            return i;
    }
    return -1;
}