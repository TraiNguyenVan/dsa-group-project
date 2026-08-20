// Build and run with:
// g++ -std=c++17 src/cpp/test_2.cpp -I include -o build/cpp/test_2
// build/cpp/test_2

// Option 2: Linear Probing - Same as Option 1 but with collision handling, only use an array to hold the table
// and a simple hash function that use mod operator to calculate key's hash(aka index)

// How collision was handled:

// - for insert operation: we use a while loop to scan for nearest
//  empty slot (aka table[index] == -1) then save key to that table[index]

// - for search operation: since hash function may return the same index ()
// on 2 different keys, therefore we also use a while loop to iterate through
// indexes until it found the right bukket (aka table[index] == key)

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
    // tìm chỗ còn trống để nhét phần tử dư vào và chạy tới lúc ko còn thì nó tự hủy đúng nghĩa =)))

    void insert(int key) {
        int index = hashFunction(key);

        while (table[index] != -1) {
            index = (index + 1) % SIZE;
        }

        table[index] = key;
    }
    // search này là tìm chỗ trống để đẩy vào
    bool search(int key) {
        int index = hashFunction(key);

        while (table[index] != -1) {
            if (table[index] == key) return true;

            index = (index + 1) % SIZE;
        }

        return false;
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
    // add more dulicated keys to expose collision
    H.insert(16);
    H.insert(16);
    H.insert(16);
    H.insert(16);
    H.insert(16);
    H.print();
    return 0;
}