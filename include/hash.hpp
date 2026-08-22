#ifndef HASH_HPP
#define HASH_HPP
#include <iostream>

#include <vector>
using namespace std;

namespace HashTable {

class DivisionMethod {
   private:
    static const int SIZE = 10;
    int table[SIZE];

   public:
    int HashTable() {
        for (int i = 0; i < SIZE; ++i) {
            table[i] = -1;
        }
    }

    int hashFunction(int key) {
        return key % SIZE;
    }
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
        for (int i = 0; i < SIZE; i++) {
            cout << i << ": " << table[i] << '\n';
        }
    }
};

// like iam looking ughh like how we do on our project🗿🗿❓
// igig, i think only function in hpp and how it work in cpp? ughhh
// sir my lapdih is about to die so imma push da code and bro yesyesyes pipilabu
// poop1
// like only me and bro how to git bro what now
// can bro commit things
// 🐧 can we do it here
// yes bro have source control tab and a terminal
// how did it just ask me if I want to see 114 possibilities? Am I Doctor Strange?
// bro like when bro tab in empty space its gonna ask🐧 
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