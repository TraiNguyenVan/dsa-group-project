#include <cstddef>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

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
    const char* labels[4] = {"first (linear best case)", "middle (linear avg / binary best)",
                              "last (linear worst case)", "miss (linear worst / hash & binary worst)"};
    long long indices[4];
    indices[0] = 0;
    indices[1] = static_cast<long long>(n / 2);
    indices[2] = static_cast<long long>(n - 1);
    indices[3] = -1;

    cout << "\nBenchmarking phone search (linear: first=best / middle / last,miss=worst; "
            "hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary)"
         << " (" << n << " contacts, 5 runs each, best reported).\n";

    for (int k = 0; k < 4; ++k) {
        long long targetIdx = indices[k];
        string phone;
        if (k == 3) {
            phone = "0000000000";
        } else {
            phone = phonebook.getPhoneAt(static_cast<std::size_t>(targetIdx));
            if (phone.empty()) {
                cout << "[" << labels[k] << "] index " << targetIdx << ": cannot pick target.\n";
                continue;
            }
        }

        int idxLin = -1;
        int idxHash = -1;
        int idxBin = -1;
        double linBest = benchmark([&]() { idxLin = phonebook.searchLinearByPhone(phone); });
        double hashBest = benchmark([&]() { idxHash = phonebook.searchHashByPhone(phone); });
        double binBest = benchmark([&]() { idxBin = phonebook.searchBinaryByPhone(phone); });

        cout << "[" << labels[k] << " index " << targetIdx << " phone " << phone << "]\n";
        cout << "  Linear best of 5: " << linBest << "ms. (index " << idxLin << ")\n";
        cout << "  Hash best of 5: " << hashBest << "ms. (index " << idxHash << ", position-independent)\n";
        cout << "  Binary best of 5: " << binBest << "ms. (index " << idxBin << ", sorted index, position-independent)\n";
    }
    cout << "Linear: first=best, last/miss=worst. Hash/binary: ~constant; miss is worst (full chain / log n probes).\n";
}

static string utcTimestamp() {
    auto now = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm, &now);
#else
    gmtime_r(&now, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return string(buf);
}

// Batch benchmark: first/middle/last/miss x linear/hash/binary targets,
// writes EVERY run (4 cases x 3 algos x 5 = 60 rows) to CSV. `miss` uses
// phone "0000000000" (not in dataset) to capture true worst case for
// hash/binary (full chain / log n probes).
int runSearchBenchmarkBatch(PhoneBook& phonebook, const string& csvInput,
                            const string& outCsv, bool append) {
    int loaded = phonebook.loadfromCSV(csvInput);
    if (loaded == -1) {
        cerr << "Cannot open file: " << csvInput << "\n";
        return 1;
    }
    if (phonebook.size() == 0) {
        cerr << "No contacts to benchmark.\n";
        return 1;
    }

    std::size_t n = phonebook.size();
    long long indices[4];
    const char* cases[4] = {"first", "middle", "last", "miss"};
    indices[0] = 0;
    indices[1] = static_cast<long long>(n / 2);
    indices[2] = static_cast<long long>(n - 1);
    indices[3] = -1;

    bool needHeader = true;
    if (append) {
        ifstream probe(outCsv);
        if (probe.good()) {
            string line;
            if (std::getline(probe, line) && !line.empty()) {
                needHeader = false;
            }
        }
    }
    ofstream out;
    out.open(outCsv, append ? (ios::out | ios::app) : (ios::out | ios::trunc));
    if (!out.is_open()) {
        cerr << "Cannot open output file: " << outCsv << "\n";
        return 1;
    }
    if (needHeader) {
        out << "language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone\n";
    }
    string timestamp = utcTimestamp();
    string toolchain = string("g++ ") + __VERSION__;
    const int repeats = 5;
    for (int k = 0; k < 4; ++k) {
        long long targetIdx = indices[k];
        string phone;
        if (k == 3) {
            phone = "0000000000";
        } else {
            phone = phonebook.getPhoneAt(static_cast<std::size_t>(targetIdx));
            if (phone.empty()) {
                continue;
            }
        }
        for (int a = 0; a < 3; ++a) {
            const char* algo = (a == 0) ? "linear" : (a == 1) ? "hash" : "binary";
            for (int r = 1; r <= repeats; ++r) {
                double t;
                if (a == 0) {
                    t = timeIt([&]() { phonebook.searchLinearByPhone(phone); });
                } else if (a == 1) {
                    t = timeIt([&]() { phonebook.searchHashByPhone(phone); });
                } else {
                    t = timeIt([&]() { phonebook.searchBinaryByPhone(phone); });
                }
                out << "cpp," << csvInput << "," << n << "," << cases[k] << "," << algo
                    << "," << r << "," << std::setprecision(17) << t << "," << timestamp
                    << ",\"" << toolchain << "\"," << targetIdx << "," << phone << "\n";
            }
        }
    }
    out.close();
    cout << "Wrote 75 rows -> " << outCsv << " (" << n << " contacts).\n";
    return 0;
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
    cout << "8. Delete contact by phone\n";
    cout << "9. Exit\n";

    cout << "========================================\n";
    cout << "Enter your choice: ";
}

int main(int argc, char* argv[]) {
    string csvInput;
    string csvOutput;
    string benchCsv;
    bool benchAppend = false;
    vector<string> positionals;
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "--benchmark-csv" && i + 1 < argc) {
            benchCsv = argv[++i];
        } else if (a == "--append") {
            benchAppend = true;
        } else {
            positionals.push_back(a);
        }
    }
    csvInput = (!positionals.empty()) ? positionals[0] : "data/contacts_100k.csv";
    csvOutput = (positionals.size() > 1) ? positionals[1] : csvInput;
    PhoneBook phonebook;

    if (!benchCsv.empty()) {
        return runSearchBenchmarkBatch(phonebook, csvInput, benchCsv, benchAppend);
    }

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
            cout << "3. Search phone - Binary Search (sorted index)\n";
            cout << "4. Search name - Linear Search\n";
            cout << "5. Prefix phone search\n";
            cout << "6. Back\n";
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
                string phone;
                cout << "Enter phone number: ";
                getline(cin, phone);
                int index;
                printTaskDuration([&]() { index = phonebook.searchBinaryByPhone(phone); });
                if (index == -1) {
                    cout << "Phone number not found.\n";
                } else {
                    cout << "Phone number found using Binary Search (sorted index).\n";
                    cout << "Contact index: " << index << "\n";
                    phonebook.printContact(index);
                }
            } else if (searchChoice == 4) {
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
            } 
            else if (searchChoice == 5) {
                string phone;
                cout << "Enter phone number: ";
                getline(cin, phone);
                std::vector<std::size_t> results;
                // we can develop to make it prints a certain number of results
                // or make it prints results by pages
                std::size_t numResults = 10;
                printTaskDuration([&]() { results = phonebook.searchPrefixByPhone(phone, numResults); });
                if (results.empty()) {
                    cout << "This does not match any phone prefix.\n";
                } else {
                    cout << "Found, here is the first " << numResults << " results (sorted index). \n";
                    for (size_t i = 0; i < results.size(); ++i) {
                        cout << "Contact index: " << results[i] << "\n";
                        phonebook.printContact(results[i]);

                    }
                }
            }            
            else if (searchChoice == 6) {
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
            string phone;
            cout << "\nEnter phone number to delete: ";
            getline(cin, phone);
            bool result;
            printTaskDuration([&]() { result = phonebook.deleteContactByPhone(phone); });
            if (result) {
                cout << "Contact deleted successfully.\n";
            } else {
                cout << "Failed to delete contact.\n";
            }
        } else if (choice == 9) {
            cout << "Goodbye\n";
            return 0;
        } else {
            cout << "Invalid choice. "
                 << "Please choose from 0 to 9.\n";
        }
    }
    return 0;
}