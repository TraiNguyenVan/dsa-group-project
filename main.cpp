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
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include "timer.hpp"

using namespace std;

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

    int size() const { return (int)contacts.size(); }

    // -----------------------------------------------------
    // CSV loading — called once on startup
    // Handles: Name,Phone per line, with optional quotes
    // e.g. Alice Nguyen,0123456789
    //      "Doe, John",0111222333
    // -----------------------------------------------------
    static string trim(const string& s) {
        size_t start = 0;
        while (start < s.size() && isspace((unsigned char)s[start])) start++;
        size_t end = s.size();
        while (end > start && isspace((unsigned char)s[end - 1])) end--;
        return s.substr(start, end - start);
    }

    // Split one CSV line into (name, phone), respecting quoted name
    static bool parseCsvLine(const string& line, string& name, string& phone) {
        string l = trim(line);
        if (l.empty()) return false;

        if (l[0] == '"') {
            // Quoted name: find closing quote
            size_t close = l.find('"', 1);
            if (close == string::npos) return false;
            name = l.substr(1, close - 1);
            size_t comma = l.find(',', close + 1);
            if (comma == string::npos) return false;
            phone = l.substr(comma + 1);
        } else {
            // Unquoted: split on LAST comma so names with commas
            // (if unquoted) still work; normally just one comma
            size_t comma = l.rfind(',');
            if (comma == string::npos) return false;
            name = l.substr(0, comma);
            phone = l.substr(comma + 1);
        }

        name = trim(name);
        phone = trim(phone);

        // Strip surrounding quotes from phone too, just in case
        if (phone.size() >= 2 && phone.front() == '"' && phone.back() == '"')
            phone = phone.substr(1, phone.size() - 2);
        phone = trim(phone);
        return !(name.empty() && phone.empty());
    }

    // Load contacts from CSV. Returns number of contacts loaded.
    // Skips invalid/duplicate rows quietly (prints one summary line).
    int loadFromCSV(const string& path) {
        ifstream file(path);
        if (!file.is_open()) return -1;

        string line;
        int loaded = 0, skipped = 0;
        while (getline(file, line)) {
            // Strip trailing \r for Windows-style files
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

    // -----------------------------------------------------
    // CSV saving — writes the whole phonebook back to disk.
    // Names containing a comma or quote are re-quoted so the
    // file round-trips cleanly on the next load.
    // Returns true on success.
    // -----------------------------------------------------
    bool saveToCSV(const string& path) const {
        ofstream file(path);
        if (!file.is_open()) return false;

        for (size_t i = 0; i < contacts.size(); i++) {
            const string& name = contacts[i].name;
            const string& phone = contacts[i].phone;

            // Quote name if it contains a comma or quote
            bool needsQuote = name.find(',') != string::npos ||
                              name.find('"') != string::npos;
            if (needsQuote) {
                file << '"';
                for (char c : name) {
                    if (c == '"') file << '"' << '"'; // escape by doubling
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
        auto t = timing::measure([&] { return book.linearSearch(query); });

        if (t.value.empty()) {
            cout << "No matching contact found.\n";
        } else {
            for (int idx : t.value) {
                const Contact& c = book.getContact(idx);
                cout << c.name << " - " << c.phone << "\n";
            }
        }
        timing::printElapsed("Linear", t.microseconds);

    } else if (method == 2) {
        for (char c : query) {
            if (!isdigit((unsigned char)c)) {
                cout << "Hash search requires a full, exact phone number.\n";
                return;
            }
        }
        auto t = timing::measure([&] { return book.hashSearch(query); });

        if (t.value == -1) {
            cout << "No matching contact found.\n";
        } else {
            const Contact& c = book.getContact(t.value);
            cout << c.name << " - " << c.phone << "\n";
        }
        timing::printElapsed("Hash", t.microseconds);

    } else {
        cout << "Invalid method.\n";
    }
}

// ---------------------------------------------------------
// Main CLI loop
// ---------------------------------------------------------
int main(int argc, char* argv[]) {
    PhoneBook book;

    // Load initial data from CSV on startup.
    // Usage: ./demo [path/to/contacts.csv]  (default: data/contacts_200k.csv)
    string csvPath = (argc > 1) ? argv[1] : "data/contacts_200k.csv";
    auto loadStart = high_resolution_clock::now();
    int loaded = book.loadFromCSV(csvPath);
    auto loadEnd = high_resolution_clock::now();
    if (loaded >= 0) {
        cout << "Loaded " << loaded << " contact(s) from " << csvPath << ".\n";
        cout << "[Load] elapsed: "
             << duration_cast<microseconds>(loadEnd - loadStart).count() << " us\n";
    } else {
        cout << "Note: could not open " << csvPath
             << " (starting with an empty phonebook).\n";
        cout << "[Load] elapsed: "
             << duration_cast<microseconds>(loadEnd - loadStart).count() << " us\n";
    }

    while (true) {
        cout << "\n===== Phonebook CLI =====\n";
        cout << "1. Search\n";
        cout << "2. Insert\n";
        cout << "3. Print all\n";
        cout << "4. Save to CSV\n";
        cout << "5. Exit\n";
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

            auto t = timing::measure([&] { return book.insertContact(name, phone); });
            if (t.value)
                cout << "Contact added.\n";
            timing::printElapsed("Insert", t.microseconds);
        } else if (choice == 3) {
            auto t = timing::measure([&] { book.printAll(); });
            timing::printElapsed("Print", t.microseconds);
        } else if (choice == 4) {
            auto t = timing::measure([&] { return book.saveToCSV(csvPath); });
            if (t.value)
                cout << "Saved " << book.size()
                     << " contact(s) to " << csvPath << ".\n";
            else
                cout << "Error: could not write to " << csvPath << ".\n";
            timing::printElapsed("Save", t.microseconds);
        } else if (choice == 5) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice, try again.\n";
        }
    }

    return 0;
}
