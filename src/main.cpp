#include <cstddef>
#include <iostream>
#include <random>
#include <string>

#include "../include/phonebook.hpp"
#include "../include/timer.hpp"

using namespace std;

void runSearchBenchmark(PhoneBook& phonebook, const string& csvInput) {
    if (phonebook.size() == 0) {
        cout << "Phonebook empty, auto-loading from: " << csvInput << "\n";
        int loaded = phonebook.loadfromCSV(csvInput);
        if (loaded == -1) {
            cout << "Cannot open file.\n";
            return;
        }
        if (phonebook.size() == 0) {
            cout << "No contacts to benchmark. Load contacts first (option 1).\n";
            return;
        }
        cout << "Auto-loaded " << loaded << " contacts.\n";
    }

    std::size_t n = phonebook.size();
    std::size_t indices[3];
    const char* labels[3] = {"first (linear best case)", "random (linear average case)",
                             "last (linear worst case)"};
    indices[0] = 0;
    indices[2] = n - 1;
    if (n <= 2) {
        indices[1] = n - 1;
    } else {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::size_t> dist(0, n - 1);
        indices[1] = dist(gen);
    }

    cout << "\nBenchmarking phone search (linear cases: first=best / random=average / last=worst; "
            "hash is ~O(1) in all cases)"
         << " (" << n << " contacts, 5 runs each, best reported).\n";

    for (int k = 0; k < 3; ++k) {
        std::size_t targetIdx = indices[k];
        string phone = phonebook.getPhoneAt(targetIdx);
        if (phone.empty()) {
            cout << "[" << labels[k] << "] index " << targetIdx << ": cannot pick target.\n";
            continue;
        }

        int idxLin = -1;
        int idxHash = -1;
        double linBest = benchmark([&]() { idxLin = phonebook.searchLinearByPhone(phone); });
        double hashBest = benchmark([&]() { idxHash = phonebook.searchHashByPhone(phone); });

        cout << "[" << labels[k] << " index " << targetIdx << " phone " << phone << "]\n";
        cout << "  Linear best of 5: " << linBest << "ms. (index " << idxLin << ")\n";
        cout << "  Hash best of 5: " << hashBest << "ms. (index " << idxHash << ", position-independent)\n";
    }
    cout << "Linear: first=best, last=worst. Hash: ~constant regardless of position.\n";
}

void printMenu() {
    cout << "\n";
    cout << "========================================\n";
    cout << "             PHONE BOOK CLI\n";
    cout << "========================================\n";

    cout << "0. Benchmark phone search (5 runs, best)\n";
    cout << "1. Load contacts from CSV\n";
    cout << "2. Save contacts to CSV\n";
    cout << "3. Insert contact\n";
    cout << "4. Search contact\n";
    cout << "5. Print all contacts\n";
    cout << "6. Print contact by index\n";
    cout << "7. Show number of contacts\n";
    cout << "8. Exit\n";

    cout << "========================================\n";
    cout << "Enter your choice: ";
}

int main(int argc, char* argv[]) {
    string csvInput;
    string csvOutput;
    csvInput = (argc > 1) ? argv[1] : "data/contacts_100k.csv";
    csvOutput = (argc > 2) ? argv[2] : csvInput;
    PhoneBook phonebook;

    while (true) {
        printMenu();
        int choice;
        if (!(cin >> choice)) {
            if (cin.eof()) {
                cout << "\nGoodbye\n";
                return 0;
            }
            cin.clear();               // reset failbit so cin can be used again
            cin.ignore(200000, '\n');  // discard the bad input remaining in the buffer
            cout << "Invalid input.\n";
            continue;
        }
        cin.clear();
        cin.ignore(200000, '\n');
        if (choice == 0) {
            runSearchBenchmark(phonebook, csvInput);
        } else if (choice == 1) {
            cout << "Loading contacts from: " << csvInput << "\n";

            int result;
            printTaskDuration([&]() { result = phonebook.loadfromCSV(csvInput); });

            if (result == -1) {
                cout << "Cannot open file.\n";
            } else {
                cout << "Successfully loaded " << result << " contacts.\n";
            }

        } else if (choice == 2) {
            cout << "\nSaving contacts to: " << csvOutput << "\n";
            bool result;
            printTaskDuration([&]() { result = phonebook.savetoCSV(csvOutput); });
            if (result) {
                cout << "Contacts saved successfully.\n";
            } else {
                cout << "Cannot open output file.\n";
            }
        } else if (choice == 3) {
            string name;
            string phone;

            cout << "\nEnter name: ";
            getline(cin, name);

            cout << "Enter phone: ";
            getline(cin, phone);

            bool result;
            printTaskDuration([&]() { result = phonebook.insertContact(name, phone); });
            if (result) {
                cout << "Contact inserted successfully.\n";
            } else {
                cout << "Failed to insert contact.\n";
            }
        } else if (choice == 4) {
            int searchChoice;

            cout << "\n";
            cout << "========== Search ==========\n";
            cout << "1. Search phone - Linear Search\n";
            cout << "2. Search phone - Hash Search\n";
            cout << "3. Search name - Linear Search\n";
            cout << "4. Back\n";
            cout << "============================\n";

            cout << "Enter your choice: ";

            if (!(cin >> searchChoice)) {
                cin.clear();
                cin.ignore(200000, '\n');
                cout << "Invalid search choice.\n";
                continue;
            }
            cin.ignore(200000, '\n');
            if (searchChoice == 1) {
                string phone;
                cout << "Enter phone number: ";
                getline(cin, phone);
                int index;
                printTaskDuration([&]() { index = phonebook.searchLinearByPhone(phone); });
                if (index == -1) {
                    cout << "Phone number not found.\n";
                } else {
                    cout << "Phone number found.\n";
                    cout << "Contact index: " << index << "\n";
                    phonebook.printContact(index);
                }
            } else if (searchChoice == 2) {
                string phone;
                cout << "Enter phone number: ";
                getline(cin, phone);
                int index;
                printTaskDuration([&]() { index = phonebook.searchHashByPhone(phone); });
                if (index == -1) {
                    cout << "Phone number not found.\n";
                } else {
                    cout << "Phone number found using Hash Table.\n";
                    cout << "Contact index: " << index << "\n";
                    phonebook.printContact(index);
                }
            } else if (searchChoice == 3) {
                string name;
                cout << "Enter name: ";
                getline(cin, name);
                int index;
                printTaskDuration([&]() { index = phonebook.searchLinearByName(name); });
                if (index == -1) {
                    cout << "Name not found.\n";

                } else {
                    cout << "Name found.\n";

                    cout << "Contact index: " << index << "\n";

                    phonebook.printContact(index);
                }
            } else if (searchChoice == 4) {
                cout << "Back to main menu.\n";
            } else {
                cout << "Invalid search choice.\n";
            }
        } else if (choice == 5) {
            cout << "\n";
            cout << "========== Contacts ==========\n";
            printTaskDuration([&]() { phonebook.printAll(); });
            cout << "==============================\n";
        } else if (choice == 6) {
            std::size_t index;
            cout << "\nEnter contact index: ";
            if (!(cin >> index)) {
                cin.clear();
                cin.ignore(200000, '\n');
                cout << "Invalid index.\n";
                continue;
            }
            bool result;
            printTaskDuration([&]() { result = phonebook.printContact(index); });
            if (!result) {
                cout << "Invalid contact index.\n";
            }
        } else if (choice == 7) {
            cout << "\nNumber of contacts: " << phonebook.size()
                 << "\n";
        } else if (choice == 8) {
            cout << "Goodbye\n";
            return 0;
        } else {
            cout << "Invalid choice. "
                 << "Please choose from 0 to 8.\n";
        }
    }
    return 0;
}