#ifndef PHONEBOOK_HPP
#define PHONEBOOK_HPP
#include <fstream>
#include <vector>
#include "../include/contact.hpp"


class PhoneBook {
private:
    std::vector<Contact> contacts;

    // string processing
    static std::string toLower(const std::string &s);
    static std::string capitalizeFirst(const std::string &s);
    static bool isAllDigits(const std::string &s);

    //  hashing related 
    int hashFuntion(const std::string) const;
    static bool isPrime(std::size_t n);
    static std::size_t nextPrime(std::size_t n);
    std::vector<HashNode*> buckets;
    
    std::size_t numBuckets;
    std::size_t numElements;

public:
    static constexpr std::size_t DEFAULT_TABLE_SIZE = 101;
    static constexpr double MAX_LOAD_FACTOR = 0.75;

    // explicit so compilier ain't do weird type conversion on object construction
    explicit PhoneBook(std::size_t initialCapacity = DEFAULT_TABLE_SIZE);
    // destructor for memory safety
    ~PhoneBook();

    // bro please chatgpt and comment this for me
    PhoneBook(const PhoneBook&) = delete;
    PhoneBook& operator = (const PhoneBook&) = delete;
    PhoneBook(PhoneBook&&) = delete;
    PhoneBook& operator = (PhoneBook&) = delete;

    
    bool insertContact(const std::string& name, const std::string& phone);

    void hashInsert(const std::string& phone, int contactIndex);
    int hashSearch(const std::string& phone) const;
    
    //linear search phone number(enter the correct phone number )
    int searchLinearByPhone(const std::string& phone) const;
    //linear search name (enter full name to search)
    int searchLinearByName(const std::string& name) const;


    // well.. size
    std::size_t size() const {
        return contacts.size();
    }

    // Load the phonebook from CSV file, one line have define: name, phone
    // Return contact number loading success or -1 if not open the file
    int loadfromCSV(const std::string& path);
    bool savetoCSV(const std::string& path) const;
};
#endif#ifndef PHONEBOOK_HPP
#define PHONEBOOK_HPP
#include <fstream>
#include <vector>
#include "../include/contact.hpp"


class PhoneBook {
private:
    std::vector<Contact> contacts;

    static std::string toLower(const std::string &s);
    static std::string capitalizeFirst(const std::string &s);
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

// explicit so compilier ain't do weird type conversion on object construction
    explicit PhoneBook(std::size_t initialCapacity = DEFAULT_TABLE_SIZE);
    ~PhoneBook();

    PhoneBook(const PhoneBook&) = delete;
    PhoneBook& operator = (const PhoneBook&) = delete;
    PhoneBook(PhoneBook&&) = delete;
    PhoneBook& operator = (PhoneBook&) = delete;

    void hashInsert(const std::string& phone, int contactIndex);
    int hashSearch(const std::string& phone) const;
    
    bool insertContact(const std::string& name, const std::string& phone);

    int searchLinearByPhone(const std::string& phone) const;

    int searchLinearByName(const std::string& name) const;


    std::size_t size() const {
        return contacts.size();
    }
    int loadfromCSV(const std::string& path);

    bool savetoCSV(const std::string& path) const;
    


    
};
#endif