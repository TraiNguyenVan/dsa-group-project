#include "phonebook.hpp"
#include "timer.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

//--------- Constructor----------
PhoneBook::PhoneBook(std::size_t initialCapacity) : hashtable(initialCapacity) {}

// Append to the primary list and hash lookup together. Callers decide when to
// maintain the sorted phone index (once per file, or immediately for one edit).
void PhoneBook::appendContact(const std::string& normalizedName, const std::string& phone) {
    const auto index = static_cast<int>(contacts.size());
    contacts.push_back({normalizedName, phone});
    hashtable.hashInsert(phone, index);
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

    // add contact to vector + hashtable (O(1))
    double tHash = timeIt([&]() {
        appendContact(normalizedName, phone);
    });

    // keep the sorted index sorted: binary-search the spot (O(log n)) + shift (O(n))
    double tIndex = timeIt([&]() {
        std::size_t pos = lowerBound(sortedPhones, phone);
        sortedPhones.insert(sortedPhones.begin() + pos, phone);
    });

    std::cout << "  hash insert: " << tHash << "ms, sorted-index insert: " << tIndex
              << "ms (the cost of keeping binary search possible)\n";
    return true;
};

// O(n): vector erase shifts the tail + full hash rebuild so order is preserved
// (usual DSA-course version; swap-with-last would be O(1) but reorders).
bool PhoneBook::deleteContactByPhone(const std::string& phone) {
    int idx = searchHashByPhone(phone);
    if (idx == -1) {
        std::cout << "Phone number not found\n";
        return false;
    }
    contacts.erase(contacts.begin() + idx);
    hashtable.clear();
    for (int i = 0; i < static_cast<int>(contacts.size()); ++i) {
        hashtable.hashInsert(contacts[static_cast<std::size_t>(i)].phone, i);
    }
    buildSortedIndex();  // delete is O(n) anyway; rebuild the sorted index
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
// first index where v[i] >= target.
std::size_t PhoneBook::lowerBound(const std::vector<std::string>& v,
                                  const std::string& target) {
    std::size_t lo = 0, hi = v.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (v[mid] < target) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}
// Binary search by phone on the sorted index (O(log n)).
// Returns the contact index via the hash table, or -1 if not found.
int PhoneBook::searchBinaryByPhone(const std::string& phone) const {
    std::size_t lo = 0, hi = sortedPhones.size();
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (sortedPhones[mid] == phone) {
            return hashtable.hashSearch(phone);
        }
        if (sortedPhones[mid] < phone) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return -1;
}
// Prefix search using lower_bound + linear for first k results
std::vector<std::size_t> PhoneBook::searchPrefixByPhone(const std::string& phone, std::size_t k) const {
    std::vector<std::size_t> result;
    std::size_t pos = lowerBound(sortedPhones, phone);
    for (std::size_t i = pos; (i < pos + k) && (i < sortedPhones.size()); ++i) {
        // stop as soon as the sorted phone no longer starts with the prefix
        if (sortedPhones[i].compare(0, phone.size(), phone) != 0) break;
        int index = hashtable.hashSearch(sortedPhones[i]);
        if (index != -1) result.push_back(static_cast<std::size_t>(index));
    }
    return result;
}

void mix(std::vector<std::string>& a, int left, int middle, int right) {
    std::vector<std::string> temp(right - left + 1);
    int i = left;
    int j = middle + 1;
    int k = 0;
    while (i <= middle && j <= right) {
        if (a[i] <= a[j]) {
            temp[k++] = a[i++];
        } else {
            temp[k++] = a[j++];
        }
    }
    while (i <= middle) {
        temp[k++] = a[i++];
    }
    while (j <= right) {
        temp[k++] = a[j++];
    }
    for (int x = left; x <= right; x++) {
        a[x] = temp[x - left];
    }
}

// split/divide
void divide(std::vector<std::string>& a, int left, int right) {
    int middle = (left + right) / 2;
    if (left < right) {
        divide(a, left, middle);
        divide(a, middle + 1, right);
        mix(a, left, middle, right);
    }
}


// Rebuild the sorted index from scratch (O(n log n)); called once after load.
void PhoneBook::buildSortedIndex() {
    sortedPhones.clear(); // drop old sorted phone numbers so the index starts fresh
    sortedPhones.reserve(contacts.size()); // pre-allocate for the current contact count
    for (const auto& c : contacts) {
        sortedPhones.push_back(c.phone);
    }
    // Guard: size_t underflows on an empty vector (0 - 1 wraps to a huge
    // number), which would make divide recurse forever. 0/1 elements need no sort.
    if (sortedPhones.size() >= 2) {
        divide(sortedPhones, 0, static_cast<int>(sortedPhones.size()) - 1);
    }
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