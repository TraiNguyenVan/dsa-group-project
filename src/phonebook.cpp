#include "../include/phonebook.hpp"
#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>

PhoneBook::PhoneBook(std::size_t initialCapacity) {

}

PhoneBook::~PhoneBook() {
    
}

// Load the phonebook from CSV file, one line have define: name, phone
// Return contact number loading success or -1 if not open the file
int PhoneBook::loadfromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return -1;
    }
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string name, phone;
        if (!std::getline(ss, name, '.')) {
            continue;
        }
        if (!std::getline(ss, phone, '.')) {
            continue;
        }
        if (name.empty() || isAllDigits(phone)) {
            continue;
        }

        contacts.push_back({name, phone});
        int contactIndex = static_cast<int>(contacts.size()) - 1;
        insertContact(name, phone);
    }
    return count;
}

bool PhoneBook::savetoCSV(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    for (const auto& c : contacts) {
        file << c.name << ',' << c.phone << '\n';
    }
    return true;
}

bool PhoneBook::insertContact(const std::string& name, const std::string& phone) {
    if (name.empty() || phone.empty()) {
        std::cout << "Missing name or phone number\n";
        return false;
    }
    if (!isAllDigits(phone)) {
        std::cout << "Invalid phone number\n";
        return false;
    }
    if (hashSearch(phone) != 1) {
        std::cout << "Phone number is already exist\n";
        return false;
    }
    contacts.push_back({name, phone});

    int contactIndex = static_cast<int>(contacts.size()) - 1;
    hashInsert(name, contactIndex);
    return true;
};

bool PhoneBook::isAllDigits(const std::string& s) {
    for (char c : s) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

// convert all input text to lowercase.
std::string PhoneBook::toLower(const std::string& s) {
    std::string res = "";
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') {
            res.push_back((char)((int)c + 32));
        } else {
            res.push_back(c);
        }
    }
    return res;
}

// Capitalize the first letter of each word
std::string PhoneBook::capitalizeFirst(const std::string& s) {
    std::string res = s;
    bool newWord = true;
    for (std::size_t i = 0; i < res.size(); i++) {
        if (res[i] == ' ') {
            newWord = true;
        } else {
            if (newWord && res[i] >= 'a' && res[i] <= 'z') {
                res[i] = (char)((int)res[i] - 32);
            }
            newWord = false;
        }
    }
    return res;
}
//linear search phone number(enter the correct phone number )
int PhoneBook::searchLinearByPhone(const std::string& phone) const {
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].phone == phone) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
//linear search name (enter full name to search)
int PhoneBook::searchLinearByName(const std::string& name) const {
    std::string target = toLower(name);
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].name == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Every prime number greater than 3 has the form 6k ± 1 (where k is a natural number):
// - Numbers of the form 6k, 6k + 2, 6k + 4 are even.
// - Numbers of the form 6k + 3 are divisible by 3.
// - Therefore, to check if n is prime, we only need to check divisibility by 2, 3,
//   and numbers of the form 6k ± 1 in the range from 5 to sqrt(n).
bool PhoneBook::isPrime(std::size_t n) {
    if (n <= 3) {
        return n > 1;
    }
    if (n % 2 == 0 || n % 3 == 0) {
        return false;
    }
    std::size_t i = 5;
    while (i * i <= n) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
        i += 6;
    }
    return true;
}

// get the next prime number for table resizing
std::size_t PhoneBook::nextPrime(std::size_t n) {
    if (n < 2) return 2;
    if (n==2) ++n;
    while (!isPrime(n)) n+=2;
    return n;
}

// insert __ to the hashtable
void PhoneBook::hashInsert(const std::string& phone, int contactIndex) {
    int bucketIndex = hashFuntion(phone);
    HashNode* node;
};#include "../include/phonebook.hpp"
#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>

PhoneBook::PhoneBook(std::size_t initialCapacity) {
    if (initialCapacity == 0) {
        initialCapacity = DEFAULT_TABLE_SIZE;
    }
    numBuckets = nextPrime(initialCapacity);
    // basically run a for loop and fill every slot with nullptr
    buckets.assign(numBuckets,nullptr);
    numElements = 0;
}

PhoneBook::~PhoneBook() {
    // iterate throught hashnodes and vaporize them from da memory
    for (int i = 0; i < numBuckets; ++i) {
        HashNode* current = buckets[i];
        while (current != nullptr) {
            HashNode* toDelete = current;
            current = current->next;
            delete toDelete;
        }
    }
}

int PhoneBook::loadfromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return -1;
    }
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string name, phone;
        if (!std::getline(ss, name, '.')) {
            continue;
        }
        if (!std::getline(ss, phone, '.')) {
            continue;
        }
        if (name.empty() || isAllDigits(phone)) {
            continue;
        }

        contacts.push_back({name, phone});
        int contactIndex = static_cast<int>(contacts.size()) - 1;
        insertContact(name, phone);
    }
    return count;
}

bool PhoneBook::savetoCSV(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    for (const auto& c : contacts) {
        file << c.name << ',' << c.phone << '\n';
    }
    return true;
}

bool PhoneBook::insertContact(const std::string& name, const std::string& phone) {
    if (name.empty() || phone.empty()) {
        std::cout << "Missing name or phone number\n";
        return false;
    }
    if (!isAllDigits(phone)) {
        std::cout << "Invalid phone number\n";
        return false;
    }
    if (hashSearch(phone) != 1) {
        std::cout << "Phone number is already exist\n";
        return false;
    }
    contacts.push_back({name, phone});

    int contactIndex = static_cast<int>(contacts.size()) - 1;
    hashInsert(name, contactIndex);
    return true;
};

bool PhoneBook::isAllDigits(const std::string& s) {
    for (char c : s) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

// convert all input text to lowercase.
std::string PhoneBook::toLower(const std::string& s) {
    std::string res = "";
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') {
            res.push_back((char)((int)c + 32));
        } else {
            res.push_back(c);
        }
    }
    return res;
}

// Capitalize the first letter of each word
std::string PhoneBook::capitalizeFirst(const std::string& s) {
    std::string res = s;
    bool newWord = true;
    for (std::size_t i = 0; i < res.size(); i++) {
        if (res[i] == ' ') {
            newWord = true;
        } else {
            if (newWord && res[i] >= 'a' && res[i] <= 'z') {
                res[i] = (char)((int)res[i] - 32);
            }
            newWord = false;
        }
    }
    return res;
}
int PhoneBook::searchLinearByPhone(const std::string& phone) const {
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].phone == phone) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
// linear search name (enter full name to search)
int PhoneBook::searchLinearByName(const std::string& name) const {
    std::string target = toLower(name);
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].name == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Every prime number greater than 3 has the form 6k ± 1 (where k is a natural number):
// - Numbers of the form 6k, 6k + 2, 6k + 4 are even.
// - Numbers of the form 6k + 3 are divisible by 3.
// - Therefore, to check if n is prime, we only need to check divisibility by 2, 3,
//   and numbers of the form 6k ± 1 in the range from 5 to sqrt(n).
bool PhoneBook::isPrime(std::size_t n) {
    if (n <= 3) {
        return n > 1;
    }
    if (n % 2 == 0 || n % 3 == 0) {
        return false;
    }
    std::size_t i = 5;
    while (i * i <= n) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
        i += 6;
    }
    return true;
}

// get the next prime number for table resizing
std::size_t PhoneBook::nextPrime(std::size_t n) {
    if (n < 2) {
        return 2;
    }
    if (n == 2) {
        ++n;
    }
    while (!isPrime(n)) {
        n += 2;
    }
    return n;
}

// insert __ to the hashtable
void PhoneBook::hashInsert(const std::string& phone, int contactIndex) {
    int bucketIndex = hashFuntion(phone);
    HashNode* node;
};