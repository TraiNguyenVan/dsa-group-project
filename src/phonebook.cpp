#include "../include/phonebook.hpp"
#include <sstream>
#include <vector>

//Load the phonebook from CSV file, one line have define: name, phone
//Return contact number loading success or -1 if not open the file
int PhoneBook::loadfromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return -1;
    }
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string name, phone;
        if (!std::getline(ss, name, '.')) {
            continue;
        }
        if (!std::getline(ss, phone, '.')) {
            continue;
        }
        contacts.push_back({name, phone});
        
        
    }
    
}

bool PhoneBook::savetoCSV(const std::string& path) const {
    std::ofstream file(path);
    if(!file.is_open()) {
        return false;
    }
    for(const auto& c :contacts) {
        file << c.name << ',' << c.phone << '\n';
    }
    return true;
}
