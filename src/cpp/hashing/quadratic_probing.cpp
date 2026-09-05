#include <iostream>
#include <vector>
using namespace std;

// Function to compute the next 
// power of 2 greater than or equal to m
int nextPowerOf2(int m) {
    m-- ;
    m |= m >> 1 ;
    m |= m >> 2 ;
    m |= m >> 4 ;
    m |= m >> 8 ;
    m |= m >> 16 ;
    m |= m >> 32 ;
    m++ ;
    return m ;
}

void quadProbing(vector<int>& table, int tsize, vector<int>& arr) {
    
    // Get number of elements to insert
    int n = arr.size();

    for (int i = 0; i < n; i++) {
        
        // Hash function
        int hv = arr[i] % tsize;

        // Place directly if slot is empty
        if (table[hv] == -1) {
            table[hv] = arr[i];
        }
        else {
            int m = nextPowerOf2(tsize) ;

            // Perform modified quadratic probing
            for (int j = 1; j <= m; j++) {
                
                // Compute new index using the
                // custom probing formula
                int t = (hv + (j + j * j) / 2) % m;

                // Skip if out of bounds for original table size
                if (t >= tsize)
                    continue ;

                // Place element if slot is empty
                if (table[t] == -1) {
                    table[t] = arr[i];
                    break ;
                }
            }
        }
    }
}

int main() {
    vector<int> arr = { 21, 10, 32, 43, 54, 65, 87, 76 };
    int tsize = 11;
    vector<int> table(tsize, -1);
    quadProbing(table, tsize, arr);

    for (int i = 0; i < table.size(); i++) {
        cout << table[i] << " ";
    }
    cout << endl;

    return 0;
}