#include "../include/hashtable.hpp"

using namespace std;

size_t HashTable::hashForSize(const string& phone, size_t mod) {
    size_t hash = 0;
    for (char c : phone)
        hash = hash * HASH_BASE + (unsigned char)(c - '0');
    return hash % mod;
}

int HashTable::hashFunction(const string& phone) const {
    return (int)hashForSize(phone, tableSize);
}

bool HashTable::isPrime(size_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (size_t i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

size_t HashTable::nextPrime(size_t n) {
    if (n <= 2) return 2;
    if (n % 2 == 0) ++n;
    while (!isPrime(n)) n += 2;
    return n;
}

HashTable::HashTable(size_t initialCapacity) {
    if (initialCapacity == 0) initialCapacity = DEFAULT_TABLE_SIZE;
    tableSize = nextPrime(initialCapacity);
    numElements = 0;
    buckets.assign(tableSize, nullptr);
}

HashTable::~HashTable() {
    for (size_t i = 0; i < tableSize; i++) {
        HashNode* node = buckets[i];
        while (node) {
            HashNode* toDelete = node;
            node = node->next;
            delete toDelete;
        }
    }
}

void HashTable::rehash(size_t newSize) {
    newSize = nextPrime(newSize);
    if (newSize <= tableSize) return;
    vector<HashNode*> newBuckets(newSize, nullptr);
    for (size_t i = 0; i < tableSize; i++) {
        HashNode* node = buckets[i];
        while (node) {
            HashNode* next = node->next;
            size_t idx = hashForSize(node->phone, newSize);
            node->next = newBuckets[idx];
            newBuckets[idx] = node;
            node = next;
        }
    }
    buckets.swap(newBuckets);
    tableSize = newSize;
}

void HashTable::maybeRehash() {
    if ((double)numElements / (double)tableSize > MAX_LOAD_FACTOR)
        rehash(tableSize * 2);
}

void HashTable::hashInsert(const string& phone, int contactIndex) {
    size_t idx = (size_t)hashFunction(phone);
    HashNode* node = new HashNode{phone, contactIndex, buckets[idx]};
    buckets[idx] = node;
    ++numElements;
    maybeRehash();
}

int HashTable::hashSearch(const string& phone) const {
    size_t idx = (size_t)hashFunction(phone);
    HashNode* node = buckets[idx];
    while (node) {
        if (node->phone == phone) return node->contactIndex;
        node = node->next;
    }
    return -1;
}

bool HashTable::remove(const string& phone) {
    size_t idx = (size_t)hashFunction(phone);
    HashNode* node = buckets[idx];
    HashNode* prev = nullptr;
    while (node) {
        if (node->phone == phone) {
            if (prev)
                prev->next = node->next;
            else
                buckets[idx] = node->next;
            delete node;
            --numElements;
            return true;
        }
        prev = node;
        node = node->next;
    }
    return false;
}

size_t HashTable::size() const { return numElements; }

size_t HashTable::bucketCount() const { return tableSize; }

double HashTable::loadFactor() const {
    return tableSize == 0 ? 0.0 : (double)numElements / (double)tableSize;
}