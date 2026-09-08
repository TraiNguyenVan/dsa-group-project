#pragma once

#include "contact.hpp"
#include <cstddef>
#include <string>
#include <vector>

class PhoneBook {
private:
    std::vector<Contact> contacts;
    std::vector<HashNode*> buckets;
    std::size_t tableSize;
    std::size_t numElements;

    static std::string toLower(const std::string& s);
    static bool isAllDigits(const std::string& s);
    int hashFunction(const std::string& phone) const;
    static std::size_t hashForSize(const std::string& phone, std::size_t mod);

    void rehash(std::size_t newSize);
    void maybeRehash();
    static bool isPrime(std::size_t n);
    static std::size_t nextPrime(std::size_t n);

public:
    static constexpr std::size_t DEFAULT_TABLE_SIZE = 101;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    explicit PhoneBook(std::size_t initialCapacity = DEFAULT_TABLE_SIZE);
    ~PhoneBook();

    PhoneBook(const PhoneBook&) = delete;
    PhoneBook& operator=(const PhoneBook&) = delete;
    PhoneBook(PhoneBook&&) = delete;
    PhoneBook& operator=(PhoneBook&&) = delete;

    void hashInsert(const std::string& phone, int contactIndex);
    int hashSearch(const std::string& phone) const;
    std::vector<int> linearSearch(const std::string& query) const;

    bool insertContact(const std::string& name, const std::string& phone);
    void printAll() const;

    const Contact& getContact(int index) const;
    int size() const;
    std::size_t bucketCount() const;
    double loadFactor() const;

    static std::string trim(const std::string& s);
    static bool parseCsvLine(const std::string& line, std::string& name, std::string& phone);
    int loadFromCSV(const std::string& path);
    bool saveToCSV(const std::string& path) const;
};
