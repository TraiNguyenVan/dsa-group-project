#include "../include/hashtable.hpp"
#include <cstddef>

// ----------- Constructor/Destructor ---------
HashTable::HashTable(std::size_t initialCapacity) {
    if (initialCapacity == 0) {
        initialCapacity = DEFAULT_TABLE_SIZE;
    }
    numBuckets = nextPrime(initialCapacity);
    // basically run a for loop and fill every slot with nullptr
    numElements = 0;
    buckets.assign(numBuckets, nullptr);
}

HashTable::~HashTable() {
    // iterate throught hashnodes and vaporize them from da memory
    for (std::size_t i = 0; i < numBuckets; ++i) {
        HashNode* current = buckets[i];
        while (current != nullptr) {
            HashNode* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
}

// Every prime number greater than 3 has the form 6k ± 1 (where k is a natural number):
// - Numbers of the form 6k, 6k + 2, 6k + 4 are even.
// - Numbers of the form 6k + 3 are divisible by 3.
// - Therefore, to check if n is prime, we only need to check divisibility by 2, 3,
//   and numbers of the form 6k ± 1 in the range from 5 to sqrt(n).
bool HashTable::isPrime(std::size_t n) {
    if (n <= 3) {
        return n > 1;
    }
    if (n % 2 == 0 || n % 3 == 0) {
        return false;
    }
    std::size_t i = 5;
    // can use n/i <= n to prevent overflow 
    while (i * i <= n) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
        i += 6;
    }
    return true;
}

// get the next prime number for table resizing
std::size_t HashTable::nextPrime(std::size_t n) {
    if (n <= 2) {
        return 2;
    }
    if (n % 2 == 0) {
        ++n;
    }
    if (n == 2) {
        ++n;
    }
    while (!isPrime(n)) {
        n += 2;
    }
    return n;
}

//----------------------Hash Function------------------
// Polynomial rolling hash, applying modulo numBuckets at each step to prevent overflow with long
// phone numbers (10–11 digits).

std::size_t HashTable::HashForSize(const std::string& phone, const std::size_t mod) {
    std::size_t hash = 0;
    for (char c : phone) {
        hash = (hash * HASH_BASE + (unsigned char)(c - '0'));
    }
    return hash % mod;
}

std::size_t HashTable::hashFunction(const std::string& phone) const {
    return HashForSize(phone, numBuckets);
}

//------------- Insert ------------------------
void HashTable::hashInsert(const std::string& phone, int contactIndex) {
    std::size_t idx = (hashFunction(phone));
    HashNode* node = new HashNode{phone, contactIndex, buckets[idx]};
    buckets[idx] = node;
    ++numElements;
    maybeRehash();
}

//--------------- Search ------------------------
int HashTable::hashSearch(const std::string& phone) const {
    std::size_t idx = (hashFunction(phone));
    HashNode* node = buckets[idx];
    while (node != nullptr) {
        if (node->phone == phone) {
            return node->contactIndex;
        }
        node = node->next;
    }
    return -1;
}

//------------------Load Factor---------------
double HashTable::loadFactor() const {
    return static_cast<double>(numElements) / static_cast<double>(numBuckets);
}

//------------------mybeRehash--------------
void HashTable::maybeRehash() {
    if (loadFactor() > MAX_LOAD_FACTOR) {
        rehash(numBuckets * 2);
    }
}

//------------------Rehash------------------
void HashTable::rehash(std::size_t newCapacity) {
    // Get a prime number for the new table size
    std::size_t newBucketCount = nextPrime(newCapacity);
    // Create new bucket array
    std::vector<HashNode*> newBuckets(newBucketCount, nullptr);
    // Move every old node into the new table
    for (std::size_t i = 0; i < numBuckets; i++) {
        HashNode* current = buckets[i];
        while (current != nullptr) {
            HashNode* nextNode = current->next;
            // Calculate new hash
            std::size_t newIndex =
                static_cast<std::size_t>(HashForSize(current->phone, newBucketCount));
            // Insert node into new bucket
            current->next = newBuckets[newIndex];
            newBuckets[newIndex] = current;
            current = nextNode;
        }
    }
    // Replace old table with new table
    buckets.swap(newBuckets);
    numBuckets = newBucketCount;
}
