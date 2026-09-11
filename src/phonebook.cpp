#include "../include/phonebook.hpp"
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

//--------- Constructor----------
PhoneBook::PhoneBook(std::size_t initialCapacity) : hashtable(initialCapacity) {}

// --------- CSV ------------

static std::string trimCsv(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) {
        start++;
    }
    std::size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) {
        end--;
    }
    return s.substr(start, end - start);
}

// Split one CSV line into (name, phone), respecting a quoted name field.
// e.g. Alice Nguyen,0123456789  or  "Do, Thanh Tuan",0939149732
// Handles ""-escaped quotes inside the quoted name.
static bool parseCsvLine(const std::string& line, std::string& name, std::string& phone) {
    std::string l = trimCsv(line);
    if (l.empty()) {
        return false;
    }

    if (l[0] == '"') {
        // Quoted name: scan for closing quote, honouring "" escapes
        std::string parsed;
        std::size_t i = 1;
        bool closed = false;
        while (i < l.size()) {
            if (l[i] == '"') {
                if (i + 1 < l.size() && l[i + 1] == '"') {
                    parsed.push_back('"');
                    i += 2;
                } else {
                    closed = true;
                    i++;  // skip closing quote
                    break;
                }
            } else {
                parsed.push_back(l[i]);
                i++;
            }
        }
        if (!closed) {
            return false;
        }
        name = parsed;
        std::size_t comma = l.find(',', i);
        if (comma == std::string::npos) {
            return false;
        }
        phone = l.substr(comma + 1);
    } else {
        // Unquoted: split on LAST comma
        std::size_t comma = l.rfind(',');
        if (comma == std::string::npos) {
            return false;
        }
        name = l.substr(0, comma);
        phone = l.substr(comma + 1);
    }

    name = trimCsv(name);
    phone = trimCsv(phone);

    // Strip surrounding quotes from phone too, just in case
    if (phone.size() >= 2 && phone.front() == '"' && phone.back() == '"') {
        phone = trimCsv(phone.substr(1, phone.size() - 2));
    }
    return !(name.empty() && phone.empty());
}

int PhoneBook::loadfromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return -1;
    }
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        // Strip trailing \r for Windows-style files
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        // Ignore empty / whitespace-only lines
        if (trimCsv(line).empty()) {
            continue;
        }

        std::string name, phone;
        if (!parseCsvLine(line, name, phone)) {
            continue;
        }
        // Remove invalid data
        if (name.empty() || !isAllDigits(phone)) {
            continue;
        }
        if (insertContact(name, phone)) {
            ++count;
        }
    }
    return count;
}

bool PhoneBook::savetoCSV(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    for (const auto& c : contacts) {
        bool needsQuote =
            c.name.find(',') != std::string::npos || c.name.find('"') != std::string::npos;
        if (needsQuote) {
            file << '"';
            for (char ch : c.name) {
                if (ch == '"') {
                    file << '"' << '"';  // escape by doubling
                } else {
                    file << ch;
                }
            }
            file << '"';
        } else {
            file << c.name;
        }
        file << ',' << c.phone << '\n';
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
    if (searchHashByPhone(phone) != -1) {
        std::cout << "Phone number is already exist\n";
        return false;
    }
    // Normalized Name
    std::string normalizedName = capitalizeFirst(toLower(name));
    // add contact to vector
    contacts.push_back({normalizedName, phone});
    // add contact to hashtable
    int contactIndex = static_cast<int>(contacts.size()) - 1;
    hashtable.hashInsert(phone, contactIndex);
    return true;
};

//------------------ Utility function --------------------
// Check if the string consists entirely of digits (used for phone number validation);
// returns false if the string is empty or contains any character other than '0'-'9'
bool PhoneBook::isAllDigits(const std::string& s) {
    if (s.empty()) {
        return false;
    }
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

//------------------------------ Search -------------------------------
// Linear search Phone
int PhoneBook::searchLinearByPhone(const std::string& phone) const {
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (contacts[i].phone == phone) {
            return static_cast<int>(i);
        }
    }
    return -1;
}
// hash search phone
int PhoneBook::searchHashByPhone(const std::string& phone) const {
    return hashtable.hashSearch(phone);
}
// linear search name (enter full name to search)
int PhoneBook::searchLinearByName(const std::string& name) const {
    std::string target = toLower(name);
    for (std::size_t i = 0; i < contacts.size(); i++) {
        if (toLower(contacts[i].name) == target) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

//-------------Print one contact------------
bool PhoneBook::printContact(std::size_t index) const {
    if (index >= contacts.size()) {
        return false;
    }
    std::cout << "Name: " << contacts[index].name << "\n";

    std::cout << "Phone: " << contacts[index].phone << "\n";

    return true;
}

//-------------Print All Contact----------------
void PhoneBook::printAll() const {
    for (std::size_t i = 0; i < contacts.size(); ++i) {
        std::cout << i << ". " << contacts[i].name << " - " << contacts[i].phone << "\n";
    }
}

//----------------Get phone--------------
std::string PhoneBook::getPhoneAt(std::size_t index) const {
    if (index >= contacts.size()) {
        return "";
    }
    return contacts[index].phone;
}