#ifndef HASH_HPP
#define HASH_HPP
#include <iostream>
#include "linkedlist.hpp"

using namespace std;

namespace HashTable {

class LinearProbing {
private:
    int size;
    int* table;

public:
    LinearProbing(int size) : size(size), table(new int[size]) {
        for (int i = 0; i < size; ++i) {
            table[i] = -1;
        }
    }

    ~LinearProbing() {
        delete[] table;
    }

    LinearProbing(const LinearProbing&) = delete;
    LinearProbing& operator=(const LinearProbing&) = delete;

    int hashFunction(int key) {
        return key % size;
    }

    void insert(int key) {
        int index = hashFunction(key);

        while (table[index] != -1) {
            index = (index + 1) % size;
        }

        table[index] = key;
    }

    bool search(int key) {
        int index = hashFunction(key);

        while (table[index] != -1) {
            if (table[index] == key) {
                return true;
            }

            index = (index + 1) % size;
        }

        return false;
    }

    void print() {
        for (int i = 0; i < size; i++) {
            cout << i << ": " << table[i] << '\n';
        }
    }
};

class SeperatedChaining {
private:
    int size;
    LinkedList* table;

public:
    SeperatedChaining(int size) : size(size), table(new LinkedList[size]) {}
    ~SeperatedChaining() {

    };
    int hashFunction(int key);
    void insert(int key);
    bool search(int key);
    bool remove(int key);
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
