// Build and run with:
// g++ -std=c++17 src/cpp/test_1.cpp -I include -o build/cpp/test_1
// build/cpp/test_1

// Option 1: Division Method - No collision handling, only use an array to hold the table
// and a simple hash function that use mod operator to calculate key's hash(index)

#include <iostream>
using namespace std;

class HashTable {
   private:
    static const int SIZE = 10;
    int table[SIZE];

   public:
    HashTable() {
        for (int i = 0; i < SIZE; ++i) {
            table[i] = -1;
        }
    }

    int hashFunction(int key) { return key % SIZE; }
    // phần insert này thì là đề trực tiếp dữ liệu lên cái đã có
    //
    void insert(int key) {
        int index = hashFunction(key);
        table[index] = key;
    }
    // collison occur when hashFunction return the same index () on different keys
    // Tinh xac suat the 2 so ngau nhien sau khi qua hashFunction tra ve 2 index bang nhau
    bool search(int key) {
        int index = hashFunction(key);
        return (index != -1) ? index : 0;
    }
    void print() {
        for (int i = 0; i < SIZE; i++) cout << i << ": " << table[i] << '\n';
    }
};

int main() {
    HashTable H;
    H.insert(12);
    H.insert(13);
    H.insert(14);
    H.insert(15);
    H.insert(16);
    H.insert(16);
    H.print();
    return 0;
}