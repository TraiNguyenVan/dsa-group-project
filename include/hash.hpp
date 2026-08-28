#ifndef HASH_HPP
#define HASH_HPP
#include <iostream>

using namespace std;

namespace HashTable {

class LinearProbing {
private:
    static const int SIZE = 10;
    int table[SIZE];

public:
    LinearProbing() {
        for (int i = 0; i < SIZE; ++i) {
            table[i] = -1;
        }
    }
    int hashFunction(int key) {
        return key % SIZE;
    }
    void insert(int key) {
        int index = hashFunction(key);

        while (table[index] != -1) {
            index = (index + 1) % SIZE;
        }

        table[index] = key;
    }
    bool search(int key) {
        int index = hashFunction(key);

        while (table[index] != -1) {
            if (table[index] == key) {
                return true;
            }

            index = (index + 1) % SIZE;
        }

        return false;
    }
    void print() {
        for (int i = 0; i < SIZE; i++) {
            cout << i << ": " << table[i] << '\n';
        }
    }
};

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
