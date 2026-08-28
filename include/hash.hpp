#ifndef HASH_HPP
#define HASH_HPP
#include <iostream>

using namespace std;

namespace HashTable {

class DivisionMethod {
   private:
    static const int SIZE = 10;
    int table[SIZE];
   public:
    DivisionMethod() {
        for (int i = 0; i < SIZE; ++i) {
            table[i] = -1;
        }
    }
    int hashFunction(int key) {
        return key % SIZE;
    }
    void insert(int key) {
        int index = hashFunction(key);
        table[index] = key;
    }
    bool search(int key) {
        int index = hashFunction(key);
        return (index != -1) ? index : 0;
    }
    void print() {
        for (int i = 0; i < SIZE; i++) {
            cout << i << ": " << table[i] << '\n';
        }
    }
};

class LinearProbing {};

class MidSquare {
    private:
        string square(string s);
        int hashReturn(string s);
    public:
        string hash(string s);
    //~~~~~~~~~~~~~~~~~~~~~~~~~
};
}  // namespace HashTable

#endif
