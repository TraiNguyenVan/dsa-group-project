#ifndef PHONEBOOK_HPP
#define PHONEBOOK_HPP
#include <cstddef>
#include <vector>
#include <string>
#include "../include/contact.hpp"
#include "../include/hashtable.hpp"

class PhoneBook {
private:
    std::vector<Contact> contacts;
    HashTable hashtable;
    // Sorted copy of phone numbers (kept sorted incrementally) so binary
    // search is O(log n). The main vector + hash table are untouched.
    std::vector<std::string> sortedPhones;
    // string processing
    static std::string toLower(const std::string& s);
    static std::string capitalizeFirst(const std::string& s);
    static bool isAllDigits(const std::string& s);
    // Hand-written binary search: first index where v[i] >= target.
    static std::size_t lowerBound(const std::vector<std::string>& v,
                                  const std::string& target);

public:
    // explicit so compilier ain't do weird type conversion on object construction
    explicit PhoneBook(std::size_t initialCapacity = HashTable::DEFAULT_TABLE_SIZE);

    // ban doing create new phonebook by copy the older phonebook
    PhoneBook(const PhoneBook&) = delete;             // ban copy constructor
    PhoneBook& operator=(const PhoneBook&) = delete;  // ban copy assigment
    PhoneBook(PhoneBook&&) = delete;                  // ban move constructor
    PhoneBook& operator=(PhoneBook&&) = delete;       // ban move assignment

    bool insertContact(const std::string& name, const std::string& phone);
    // O(n): vector erase shifts tail + hash index rebuilt to preserve order
    bool deleteContactByPhone(const std::string& phone);

    // linear search phone number(enter the correct phone number )
    int searchLinearByPhone(const std::string& phone) const;
    // hash search by phone number
    int searchHashByPhone(const std::string& phone) const;
    // binary search by phone on the sorted index (O(log n))
    int searchBinaryByPhone(const std::string& phone) const;
    // linear search name (enter full name to search)
    int searchLinearByName(const std::string& name) const;
    // Prefix search using lower_bow + linear for first k results
    std::vector<std::size_t>searchPrefixByPhone(const std::string& phone, std::size_t k) const;
    // print all the contacts
    void printAll() const;
    // print one contact in one a specific location
    bool printContact(std::size_t index) const;

    // Returns the number of contacts currently in the address book
    std::size_t size() const {
        return contacts.size();
    }
    // Retrieve the contact's phone number at the specified index; return "" if the index is invalid
    std::string getPhoneAt(std::size_t index) const;

    // Load the phonebook from CSV file, one line have define: name, phone
    int loadfromCSV(const std::string& path);
    // Return contact number loading success or -1 if not open the file
    bool savetoCSV(const std::string& path) const;
    // Rebuild the sorted index from scratch (O(n log n)); called once after load.
    void buildSortedIndex();
};
#endif