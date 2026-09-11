#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <cstddef>
#include <string>
#include <vector>

struct HashNode {
    std::string phone;
    int contactIndex;
    HashNode* next;
};

class HashTable {
public:
    // constexpr for complile time computed constant
    static constexpr std::size_t DEFAULT_TABLE_SIZE = 101;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    HashTable(std::size_t initialCapacity);
    // destructor for memory safety
    ~HashTable();
    // write in public to call directly
    void hashInsert(const std::string& phone, int contactIndex);
    int hashSearch(const std::string& phone) const;
    // O(1) average chained delete: unlink node for phone, return true if found
    bool hashDelete(const std::string& phone);
    // drop all entries (keeps bucket count); used to rebuild index after O(n) vector erase
    void clear();
    double loadFactor() const;

private:
    //  hashing related
    static bool isPrime(std::size_t n);
    static std::size_t nextPrime(std::size_t n);

    static constexpr std::size_t HASH_BASE = 31;
    std::size_t hashFunction(const std::string& phone) const;
    static std::size_t HashForSize(const std::string& phone, const std::size_t mod);

    // rehash when Load factor is too high
    void maybeRehash();
    void rehash(std::size_t newCapacity);
    std::vector<HashNode*> buckets;

    // ban doing create new phonebook by copy the older phonebook
    HashTable(const HashTable&) = delete;             // ban copy constructor
    HashTable& operator=(const HashTable&) = delete;  // ban copy assigment
    HashTable(HashTable&&) = delete;                  // ban move constructor
    HashTable& operator=(HashTable&&) = delete;       // ban move assignment

    std::size_t numBuckets;
    std::size_t numElements;
};

#endif