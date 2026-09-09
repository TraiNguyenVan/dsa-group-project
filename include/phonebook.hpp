#ifndef PHONEBOOK_HPP
#define PHONEBOOK_HPP
#include <fstream>
#include <vector>
#include "../include/contact.hpp"


class PhoneBook {
private:
    std::vector<Contact> contacts;

    static std::string toLower(const std::string &s);
    static bool isAllDigits(const std::string &s);
    int hashFuntion(const std::string) const;

    static bool isPrime(std::size_t n);
    static std::size_t nextPrime(std::size_t n);
    
    std::vector<HashNode*> buckets;
    std::size_t numBuckets;
    std::size_t numElements;

public:
    static constexpr std::size_t DEFAULT_TABLE_SIZE = 101;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    explicit PhoneBook(std::size_t initialCapacity = DEFAULT_TABLE_SIZE);
    ~PhoneBook();

    PhoneBook(const PhoneBook&) = delete;
    PhoneBook& operator = (const PhoneBook&) = delete;
    PhoneBook(PhoneBook&&) = delete;
    PhoneBook& operator = (PhoneBook&) = delete;

    void hashInsert(const std::string& name, const std::string& phone);
    
    bool insertContact(const std::string& name, const std::string& phone);

    int loadfromCSV(const std::string& path);

    bool savetoCSV(const std::string& path) const;
    


    
};
#endif