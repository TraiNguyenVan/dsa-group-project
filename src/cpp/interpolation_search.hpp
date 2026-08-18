#include <iostream>
#include <vector>
using namespace std;
int interpolation_search(vector<int> &arr, int x) {
  int low = 0;
  int high = size(arr) - 1;
  while (x >= arr[low] && x <= arr[high] && low <= high) {
    if (low == high) {
      if (x == arr[low])
        return low;
      return -1;
    }
    int pos =
        low + (long long)(x - arr[low]) * (high - low) / (arr[high] - arr[low]);
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
