#pragma once

#include "contact.hpp"
#include "hashtable.hpp"
#include <cstddef>
#include <string>
#include <vector>

class PhoneBook {
private:
    std::vector<Contact> contacts;
    HashTable table;

    static std::string toLower(const std::string& s);
    static bool isAllDigits(const std::string& s);

public:
    explicit PhoneBook(std::size_t initialCapacity = HashTable::DEFAULT_TABLE_SIZE);

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
