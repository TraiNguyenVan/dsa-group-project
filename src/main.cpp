#include "../include/phonebook.hpp"
#include "../include/timer.hpp"
#include <cctype>
#include <iostream>
#include <string>

using namespace std;
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

int main(int argc, char* argv[]) {
    PhoneBook book;

    string csvPath = (argc > 1) ? argv[1] : "data/contacts_200k.csv";
    auto load = timing::measure([&] { return book.loadFromCSV(csvPath); });
    if (load.value >= 0) {
        cout << "Loaded " << load.value << " contact(s) from " << csvPath << ".\n";
    } else {
        cout << "Note: could not open " << csvPath
             << " (starting with an empty phonebook).\n";
    }
    timing::printElapsed("Load", load.microseconds);

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
