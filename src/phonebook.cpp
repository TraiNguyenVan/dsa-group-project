#include "../include/phonebook.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <vector>

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

// convert the first 
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
    return false;
}

int PhoneBook::searchLinearByName(const std::string& name) const {
    std::string target = toLower(name);
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].name == target) {
            return static_cast<int>(i);
        }
    }
    return false;
}

void PhoneBook::hashInsert(const std::string& phone, int contactIndex) {
    int bucketIndex = hashFuntion(phone);
    HashNode* node = new HashNode{phone, contactIndex, buckets[bucketIndex]};
};