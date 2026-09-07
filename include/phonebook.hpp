#pragma once

#include "contact.hpp"
#include <vector>
#include <string>

class PhoneBook {
private:
    std::vector<Contact> contacts;
    HashNode* buckets[TABLE_SIZE];

    static std::string toLower(const std::string& s);
    static bool isAllDigits(const std::string& s);
    int hashFunction(const std::string& phone) const;

public:
    PhoneBook();
    ~PhoneBook();

    void hashInsert(const std::string& phone, int contactIndex);
    int hashSearch(const std::string& phone) const;
    std::vector<int> linearSearch(const std::string& query) const;

    bool insertContact(const std::string& name, const std::string& phone);
    void printAll() const;

    const Contact& getContact(int index) const;
    int size() const;

    static std::string trim(const std::string& s);
    static bool parseCsvLine(const std::string& line, std::string& name, std::string& phone);
    int loadFromCSV(const std::string& path);
    bool saveToCSV(const std::string& path) const;
};
