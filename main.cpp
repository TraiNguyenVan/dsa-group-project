// =========================================================
// Made by QTUAN with claude
// Phonebook Look-up CLI
// Skeleton implementing 2 search algorithms:
//   1) Linear Search  - O(n), partial match on name or phone
//   2) Hash Table     - O(1) average, exact full-phone lookup
// =========================================================

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <cctype>

using namespace std;
using namespace std::chrono;

// ---------------------------------------------------------
// Data model
// ---------------------------------------------------------
struct Contact {
    string name;
    string phone; // digits only
};

// Hash table node (separate chaining for collisions)
struct HashNode {
    string phone;
    int contactIndex;   // index into the contacts vector
    HashNode* next;
};

const int TABLE_SIZE = 101; // prime size for fewer collisions

class PhoneBook {
private:
    vector<Contact> contacts;
    HashNode* buckets[TABLE_SIZE];

    // ---- helpers ----
    static string toLower(const string& s) {
        string out = s;
        transform(out.begin(), out.end(), out.begin(), ::tolower);
        return out;
    }

    static bool isAllDigits(const string& s) {
        if (s.empty()) return false;
        for (char c : s)
            if (!isdigit((unsigned char)c)) return false;
        return true;
    }

    // djb2-style hash function over the phone digits
    int hashFunction(const string& phone) const {
        unsigned long hash = 0;
        for (char c : phone)
            hash = (hash * 31 + (c - '0')) % TABLE_SIZE;
        return (int)hash;
    }

public:
    PhoneBook() {
        for (int i = 0; i < TABLE_SIZE; i++) buckets[i] = nullptr;
    }

    ~PhoneBook() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* node = buckets[i];
            while (node) {
                HashNode* toDelete = node;
                node = node->next;
                delete toDelete;
            }
        }
    }

    // -----------------------------------------------------
    // Hash table operations
    // -----------------------------------------------------
    void hashInsert(const string& phone, int contactIndex) {
        int idx = hashFunction(phone);
        HashNode* node = new HashNode{phone, contactIndex, buckets[idx]};
        buckets[idx] = node; // insert at head of chain
    }

    // Exact match only — returns index into contacts, or -1 if not found
    int hashSearch(const string& phone) const {
        int idx = hashFunction(phone);
        HashNode* node = buckets[idx];
        while (node) {
            if (node->phone == phone) return node->contactIndex;
            node = node->next;
        }
        return -1;
    }

    // -----------------------------------------------------
    // Linear search — partial match on name OR phone
    // -----------------------------------------------------
    vector<int> linearSearch(const string& query) const {
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

    // -----------------------------------------------------
    // Insert (Function 2)
    // -----------------------------------------------------
    bool insertContact(const string& name, const string& phone) {
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

    // -----------------------------------------------------
    // Print (Function 3)
    // -----------------------------------------------------
    void printAll() const {
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

    // Expose contacts for the search-result printer in main()
    const Contact& getContact(int index) const { return contacts[index]; }
};

// ---------------------------------------------------------
// Menu-driven search — user explicitly picks the algorithm
// so the two can be benchmarked side by side
// ---------------------------------------------------------
void doSearch(const PhoneBook& book) {
    cout << "1. Linear search (name / partial phone)\n";
    cout << "2. Hash search (exact full phone number)\n";
    cout << "Choose search method: ";
    int method;
    cin >> method;
    cin.ignore();

    cout << "Enter search value: ";
    string query;
    getline(cin, query);

    if (method == 1) {
        auto start = high_resolution_clock::now();
        vector<int> results = book.linearSearch(query);
        auto end = high_resolution_clock::now();

        if (results.empty()) {
            cout << "No matching contact found.\n";
        } else {
            for (int idx : results) {
                const Contact& c = book.getContact(idx);
                cout << c.name << " - " << c.phone << "\n";
            }
        }
        cout << "[Linear] elapsed: "
             << duration_cast<microseconds>(end - start).count() << " us\n";

    } else if (method == 2) {
        for (char c : query) {
            if (!isdigit((unsigned char)c)) {
                cout << "Hash search requires a full, exact phone number.\n";
                return;
            }
        }
        auto start = high_resolution_clock::now();
        int idx = book.hashSearch(query);
        auto end = high_resolution_clock::now();

        if (idx == -1) {
            cout << "No matching contact found.\n";
        } else {
            const Contact& c = book.getContact(idx);
            cout << c.name << " - " << c.phone << "\n";
        }
        cout << "[Hash] elapsed: "
             << duration_cast<microseconds>(end - start).count() << " us\n";

    } else {
        cout << "Invalid method.\n";
    }
}

// ---------------------------------------------------------
// Main CLI loop
// ---------------------------------------------------------
int main() {
    PhoneBook book;

    while (true) {
        cout << "\n===== Phonebook CLI =====\n";
        cout << "1. Search\n";
        cout << "2. Insert\n";
        cout << "3. Print all\n";
        cout << "4. Exit\n";
        cout << "Choose an option: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            doSearch(book);
        } else if (choice == 2) {
            cout << "Enter name: ";
            string name;
            getline(cin, name);
            cout << "Enter phone: ";
            string phone;
            getline(cin, phone);

            if (book.insertContact(name, phone))
                cout << "Contact added.\n";
        } else if (choice == 3) {
            book.printAll();
        } else if (choice == 4) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice, try again.\n";
        }
    }

    return 0;
}
