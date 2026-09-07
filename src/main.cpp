#include "phonebook.hpp"
#include <cctype>
#include <chrono>
#include <iostream>
#include <string>

using namespace std;
using namespace std::chrono;

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

int main(int argc, char* argv[]) {
    PhoneBook book;

    string csvPath = (argc > 1) ? argv[1] : "data/contacts.csv";
    int loaded = book.loadFromCSV(csvPath);
    if (loaded >= 0) {
        cout << "Loaded " << loaded << " contact(s) from " << csvPath << ".\n";
    } else {
        cout << "Note: could not open " << csvPath
             << " (starting with an empty phonebook).\n";
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

            if (book.insertContact(name, phone))
                cout << "Contact added.\n";
        } else if (choice == 3) {
            book.printAll();
        } else if (choice == 4) {
            if (book.saveToCSV(csvPath))
                cout << "Saved " << book.size()
                     << " contact(s) to " << csvPath << ".\n";
            else
                cout << "Error: could not write to " << csvPath << ".\n";
        } else if (choice == 5) {
            cout << "Goodbye!\n";
            break;
        } else {
            cout << "Invalid choice, try again.\n";
        }
    }

    return 0;
}
