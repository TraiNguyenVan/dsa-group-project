// By @Alex75762107

#include <iostream>

int binarySearch(int arr[], int n, int x) {
    int low = 0; 
    int high = n-1;
    
    while (high >= low) {
        int mid = low + (high - low) / 2;

        // If found at mid, then return it
        if (x == arr[mid]){
            return mid;
        }
        // Search the right half
        else if (x > arr[mid]){
            low = mid + 1;
        } 
        else high = mid - 1; //Search the left half        
    }
    return -1;
}