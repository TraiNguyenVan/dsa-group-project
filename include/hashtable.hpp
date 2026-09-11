#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Separate-chaining hash table mapping a phone number (std::string) to a
// contact index (int). Extracted from PhoneBook so the hash engine is a
// self-contained, reusable component.
class HashTable {
private:
    struct HashNode {
        std::string phone;
        int contactIndex;
        HashNode* next;
    };

    std::vector<HashNode*> buckets;
    std::size_t tableSize;
    std::size_t numElements;

    // Base of the polynomial hash (small odd prime -> good distribution).
    static constexpr std::size_t HASH_BASE = 31;

    static std::size_t hashForSize(const std::string& phone, std::size_t mod);
    int hashFunction(const std::string& phone) const;

    void rehash(std::size_t newSize);
    void maybeRehash();
    static bool isPrime(std::size_t n);
    static std::size_t nextPrime(std::size_t n);

public:
    static constexpr std::size_t DEFAULT_TABLE_SIZE = 101;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    explicit HashTable(std::size_t initialCapacity = DEFAULT_TABLE_SIZE);
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;
    HashTable(HashTable&&) = delete;
    HashTable& operator=(HashTable&&) = delete;

    void hashInsert(const std::string& phone, int contactIndex);
    int hashSearch(const std::string& phone) const;
    bool remove(const std::string& phone);

    std::size_t size() const;
    std::size_t bucketCount() const;
    double loadFactor() const;
};