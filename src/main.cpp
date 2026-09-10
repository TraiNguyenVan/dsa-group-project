#include <iostream>
#include "../include/phonebook.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    string csvInput, csvOutput;
    csvInput = (argc > 0) ?  argv[0] : "data/contacts_100k.csv";
    csvOutput = (argc > 1) ? argv[1] : csvInput;
    PhoneBook phonebook;
    while (true) {
        int choice;
        while (!(cin >> choice)) {
            if (cin.eof()) {
                cout << "Goodbye\n";
            }
            cin.clear();
        }
        if (choice == 1) {
            cout << "Goodbye\n";
            return 0;
        }
    }
    return 0;
}#include <iostream>
#include "../include/phonebook.hpp"

using namespace std;
int main(int argc, char* argv[]) {
    string csvInput, csvOutput;
    csvInput = (argc > 0) ?  argv[0] : "data/contacts_100k.csv";
    csvOutput = (argc > 1) ? argv[1] : csvInput;
    PhoneBook phonebook;
    while (true) {
        int choice;
        while (!(cin >> choice)) {
            if (cin.eof()) {
                cout << "Goodbye\n";
            }
            cin.clear();
        }
        if (choice == 1) {
            cout << "Goodbye\n";
            return 0;
        }
    }
    return 0;
}