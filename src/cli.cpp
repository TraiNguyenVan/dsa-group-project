#include "cli.hpp"
#include "benchmark.hpp"
#include "phonebook.hpp"
#include "timer.hpp"

#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

enum class Input { ok, invalid, end };

template <typename Integer>
Input readInteger(Integer& value) {
    if (!(std::cin >> value)) {
        if (std::cin.eof()) {
            return Input::end;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return Input::invalid;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Input::ok;
}

bool readLine(const char* prompt, std::string& value) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, value));
}

void printMenu() {
    std::cout << "\n========================================\n"
                 "             PHONE BOOK CLI\n"
                 "========================================\n"
                 "0. Benchmark phone search (5 runs, best)\n"
                 "1. Load contacts from CSV\n"
                 "2. Save contacts to CSV\n"
                 "3. Insert contact\n"
                 "4. Search contact\n"
                 "5. Print all contacts\n"
                 "6. Print contact by index\n"
                 "7. Show number of contacts\n"
                 "8. Delete contact by phone\n"
                 "9. Exit\n"
                 "========================================\n"
                 "Enter your choice: ";
}

void loadContacts(PhoneBook& book, const std::string& path) {
    std::cout << "Loading contacts from: " << path << '\n';
    int count = -1;
    printTaskDuration([&] { count = book.loadfromCSV(path); });
    if (count == -1) {
        std::cout << "Cannot open file.\n";
    } else {
        std::cout << "Successfully loaded " << count << " contacts.\n";
    }
}

void saveContacts(const PhoneBook& book, const std::string& path) {
    std::cout << "\nSaving contacts to: " << path << '\n';
    bool saved = false;
    printTaskDuration([&] { saved = book.savetoCSV(path); });
    std::cout << (saved ? "Contacts saved successfully.\n" : "Cannot open output file.\n");
}

bool insertContact(PhoneBook& book) {
    std::string name, phone;
    if (!readLine("\nEnter name: ", name) || !readLine("Enter phone: ", phone)) {
        return false;
    }
    bool inserted = false;
    printTaskDuration([&] { inserted = book.insertContact(name, phone); });
    std::cout << (inserted ? "Contact inserted successfully.\n" : "Failed to insert contact.\n");
    return true;
}

enum class Search { linear, hash, binary, name };

int findContact(const PhoneBook& book, Search search, const std::string& query) {
    switch (search) {
        case Search::linear: return book.searchLinearByPhone(query);
        case Search::hash: return book.searchHashByPhone(query);
        case Search::binary: return book.searchBinaryByPhone(query);
        case Search::name: return book.searchLinearByName(query);
    }
    return -1;
}

bool searchSingle(const PhoneBook& book, Search search, const char* prompt,
                  const char* found, const char* missing) {
    std::string query;
    if (!readLine(prompt, query)) {
        return false;
    }
    int index = -1;
    printTaskDuration([&] { index = findContact(book, search, query); });
    if (index == -1) {
        std::cout << missing << '\n';
    } else {
        std::cout << found << '\n' << "Contact index: " << index << '\n';
        book.printContact(static_cast<std::size_t>(index));
    }
    return true;
}

bool searchPrefix(const PhoneBook& book) {
    std::string prefix;
    if (!readLine("Enter phone number: ", prefix)) {
        return false;
    }
    constexpr std::size_t kMaxResults = 10;
    std::vector<std::size_t> matches;
    printTaskDuration([&] { matches = book.searchPrefixByPhone(prefix, kMaxResults); });
    if (matches.empty()) {
        std::cout << "This does not match any phone prefix.\n";
    } else {
        std::cout << "Found, here is the first " << matches.size() << " results (sorted index). \n";
        for (const auto index : matches) {
            std::cout << "Contact index: " << index << '\n';
            book.printContact(index);
        }
    }
    return true;
}

bool searchContacts(const PhoneBook& book) {
    std::cout << "\n========== Search ==========\n"
                 "1. Search phone - Linear Search\n"
                 "2. Search phone - Hash Search\n"
                 "3. Search phone - Binary Search (sorted index)\n"
                 "4. Search name - Linear Search\n"
                 "5. Prefix phone search\n"
                 "6. Back\n"
                 "============================\n"
                 "Enter your choice: ";
    int choice;
    const auto input = readInteger(choice);
    if (input == Input::end) {
        return false;
    }
    if (input == Input::ok) {
        switch (choice) {
            case 1:
                return searchSingle(book, Search::linear,
                                    "Enter phone number: ", "Phone number found.",
                                    "Phone number not found.");
            case 2:
                return searchSingle(book, Search::hash,
                                    "Enter phone number: ", "Phone number found using Hash Table.",
                                    "Phone number not found.");
            case 3:
                return searchSingle(book, Search::binary,
                                    "Enter phone number: ",
                                    "Phone number found using Binary Search (sorted index).",
                                    "Phone number not found.");
            case 4:
                return searchSingle(book, Search::name, "Enter name: ",
                                    "Name found.", "Name not found.");
            case 5:
                return searchPrefix(book);
            case 6:
                std::cout << "Back to main menu.\n";
                return true;
        }
    }
    std::cout << "Invalid search choice.\n";
    return true;
}

bool printByIndex(const PhoneBook& book) {
    std::cout << "\nEnter contact index: ";
    std::size_t index;
    switch (readInteger(index)) {
        case Input::end:
            return false;
        case Input::invalid:
            std::cout << "Invalid index.\n";
            return true;
        case Input::ok:
            break;
    }
    bool printed = false;
    printTaskDuration([&] { printed = book.printContact(index); });
    if (!printed) {
        std::cout << "Invalid contact index.\n";
    }
    return true;
}

bool deleteByPhone(PhoneBook& book) {
    std::string phone;
    if (!readLine("\nEnter phone number to delete: ", phone)) {
        return false;
    }
    bool deleted = false;
    printTaskDuration([&] { deleted = book.deleteContactByPhone(phone); });
    std::cout << (deleted ? "Contact deleted successfully.\n" : "Failed to delete contact.\n");
    return true;
}

bool command(int choice, PhoneBook& book, const std::string& inputPath,
             const std::string& outputPath) {
    switch (choice) {
        case 0:
            runSearchBenchmark(book, inputPath);
            break;
        case 1:
            loadContacts(book, inputPath);
            break;
        case 2:
            saveContacts(book, outputPath);
            break;
        case 3:
            return insertContact(book);
        case 4:
            return searchContacts(book);
        case 5:
            std::cout << "\n========== Contacts ==========\n";
            printTaskDuration([&] { book.printAll(); });
            std::cout << "==============================\n";
            break;
        case 6:
            return printByIndex(book);
        case 7:
            std::cout << "\nNumber of contacts: " << book.size() << '\n';
            break;
        case 8:
            return deleteByPhone(book);
        default:
            std::cout << "Invalid choice. Please choose from 0 to 9.\n";
    }
    return true;
}

}  // namespace

int runPhoneBookCli(const std::string& inputPath, const std::string& outputPath) {
    PhoneBook book;
    while (true) {
        printMenu();
        int choice;
        const auto input = readInteger(choice);
        if (input == Input::end) {
            break;
        }
        if (input == Input::invalid) {
            std::cout << "Invalid input.\n";
            continue;
        }
        if (choice == 9) {
            std::cout << "Goodbye\n";
            return 0;
        }
        if (!command(choice, book, inputPath, outputPath)) {
            break;
        }
    }
    std::cout << "\nGoodbye\n";
    return 0;
}
