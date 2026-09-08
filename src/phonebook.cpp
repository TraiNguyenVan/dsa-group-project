#include "../include/phonebook.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

using namespace std;

string PhoneBook::toLower(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

bool PhoneBook::isAllDigits(const string& s) {
    if (s.empty()) return false;
    for (char c : s)
        if (!isdigit((unsigned char)c)) return false;
    return true;
}

int PhoneBook::hashFunction(const string& phone) const {
    return (int)hashForSize(phone, tableSize);
}

size_t PhoneBook::hashForSize(const string& phone, size_t mod) {
    unsigned long hash = 0;
    for (char c : phone)
        hash = hash * 31 + (unsigned char)(c - '0');
    return (size_t)(hash % mod);
}

bool PhoneBook::isPrime(size_t n) {
    if (n < 2) return false;
    if (n % 2 == 0) return n == 2;
    for (size_t i = 3; i * i <= n; i += 2)
        if (n % i == 0) return false;
    return true;
}

size_t PhoneBook::nextPrime(size_t n) {
    if (n <= 2) return 2;
    if (n % 2 == 0) ++n;
    while (!isPrime(n)) n += 2;
    return n;
}

PhoneBook::PhoneBook(size_t initialCapacity) {
    if (initialCapacity == 0) initialCapacity = DEFAULT_TABLE_SIZE;
    tableSize = nextPrime(initialCapacity);
    numElements = 0;
    buckets.assign(tableSize, nullptr);
}

PhoneBook::~PhoneBook() {
    for (size_t i = 0; i < tableSize; i++) {
        HashNode* node = buckets[i];
        while (node) {
            HashNode* toDelete = node;
            node = node->next;
            delete toDelete;
        }
    }
}

void PhoneBook::rehash(size_t newSize) {
    newSize = nextPrime(newSize);
    if (newSize <= tableSize) return;
    vector<HashNode*> newBuckets(newSize, nullptr);
    for (size_t i = 0; i < tableSize; i++) {
        HashNode* node = buckets[i];
        while (node) {
            HashNode* next = node->next;
            size_t idx = hashForSize(node->phone, newSize);
            node->next = newBuckets[idx];
            newBuckets[idx] = node;
            node = next;
        }
    }
    buckets.swap(newBuckets);
    tableSize = newSize;
}

void PhoneBook::maybeRehash() {
    if ((double)numElements / (double)tableSize > MAX_LOAD_FACTOR)
        rehash(tableSize * 2);
}

void PhoneBook::hashInsert(const string& phone, int contactIndex) {
    size_t idx = (size_t)hashFunction(phone);
    HashNode* node = new HashNode{phone, contactIndex, buckets[idx]};
    buckets[idx] = node;
    ++numElements;
    maybeRehash();
}

int PhoneBook::hashSearch(const string& phone) const {
    size_t idx = (size_t)hashFunction(phone);
    HashNode* node = buckets[idx];
    while (node) {
        if (node->phone == phone) return node->contactIndex;
        node = node->next;
    }
    return -1;
}

vector<int> PhoneBook::linearSearch(const string& query) const {
    vector<int> results;
    string q = toLower(query);

    for (int i = 0; i < (int)contacts.size(); i++) {
        string nameLower = toLower(contacts[i].name);
        if (nameLower.find(q) != string::npos ||
            contacts[i].phone.find(query) != string::npos) {
            results.push_back(i);
        }
    }
    return results;
}

bool PhoneBook::insertContact(const string& name, const string& phone) {
    if (name.empty() || phone.empty()) {
        cout << "Invalid input: name and phone cannot be empty.\n";
        return false;
    }
    if (!isAllDigits(phone)) {
        cout << "Invalid input: phone must contain digits only.\n";
        return false;
    }
    if (hashSearch(phone) != -1) {
        cout << "Duplicate: this phone number already exists.\n";
        return false;
    }

    contacts.push_back(Contact{name, phone});
    int newIndex = (int)contacts.size() - 1;
    hashInsert(phone, newIndex);
    return true;
}

void PhoneBook::printAll() const {
    if (contacts.empty()) {
        cout << "Phonebook is empty.\n";
        return;
    }
    cout << "No.\tName\t\tPhone\n";
    for (int i = 0; i < (int)contacts.size(); i++) {
        cout << (i + 1) << "\t" << contacts[i].name
             << "\t\t" << contacts[i].phone << "\n";
    }
}

const Contact& PhoneBook::getContact(int index) const { return contacts[index]; }

int PhoneBook::size() const { return (int)contacts.size(); }

size_t PhoneBook::bucketCount() const { return tableSize; }

double PhoneBook::loadFactor() const {
    return tableSize == 0 ? 0.0 : (double)numElements / (double)tableSize;
}

string PhoneBook::trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) start++;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

bool PhoneBook::parseCsvLine(const string& line, string& name, string& phone) {
    string l = trim(line);
    if (l.empty()) return false;

    if (l[0] == '"') {
        size_t close = l.find('"', 1);
        if (close == string::npos) return false;
        name = l.substr(1, close - 1);
        size_t comma = l.find(',', close + 1);
        if (comma == string::npos) return false;
        phone = l.substr(comma + 1);
    } else {
        size_t comma = l.rfind(',');
        if (comma == string::npos) return false;
        name = l.substr(0, comma);
        phone = l.substr(comma + 1);
    }

    name = trim(name);
    phone = trim(phone);

    if (phone.size() >= 2 && phone.front() == '"' && phone.back() == '"')
        phone = phone.substr(1, phone.size() - 2);
    phone = trim(phone);
    return !(name.empty() && phone.empty());
}

int PhoneBook::loadFromCSV(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return -1;

    string line;
    int loaded = 0, skipped = 0;
    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (trim(line).empty()) continue;

        string name, phone;
        if (!parseCsvLine(line, name, phone)) { skipped++; continue; }
        if (name.empty() || phone.empty()) { skipped++; continue; }
        if (!isAllDigits(phone)) { skipped++; continue; }
        if (hashSearch(phone) != -1) { skipped++; continue; }

        contacts.push_back(Contact{name, phone});
        hashInsert(phone, (int)contacts.size() - 1);
        loaded++;
    }
    if (skipped > 0)
        cout << "Skipped " << skipped << " invalid/duplicate row(s) from "
             << path << ".\n";
    return loaded;
}

bool PhoneBook::saveToCSV(const string& path) const {
    ofstream file(path);
    if (!file.is_open()) return false;

    for (size_t i = 0; i < contacts.size(); i++) {
        const string& name = contacts[i].name;
        const string& phone = contacts[i].phone;

        bool needsQuote = name.find(',') != string::npos ||
                          name.find('"') != string::npos;
        if (needsQuote) {
            file << '"';
            for (char c : name) {
                if (c == '"') file << '"' << '"';
                else file << c;
            }
            file << '"';
        } else {
            file << name;
        }
        file << ',' << phone;
        if (i + 1 < contacts.size()) file << '\n';
    }
    return true;
}
