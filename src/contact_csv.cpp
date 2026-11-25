#include "csv.hpp"
#include "phonebook.hpp"

#include <cctype>
#include <fstream>
#include <string>

std::string csv::escapeField(const std::string& field) {
    if (field.find_first_of(",\"\r\n") == std::string::npos) {
        return field;
    }
    std::string quoted = "\"";
    for (char ch : field) {
        if (ch == '"') {
            quoted += '"';
        }
        quoted += ch;
    }
    return quoted + '"';
}

namespace {

std::string trimCsv(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }
    return s.substr(start, end - start);
}

// Split one CSV line into (name, phone), respecting a quoted name field.
// e.g. Alice Nguyen,0123456789  or  "Do, Thanh Tuan",0939149732
// Handles ""-escaped quotes inside the quoted name.
bool parseCsvLine(const std::string& line, std::string& name, std::string& phone) {
    std::string l = trimCsv(line);
    if (l.empty()) {
        return false;
    }

    if (l[0] == '"') {
        // Quoted name: scan for closing quote, honouring "" escapes
        std::string parsed;
        std::size_t i = 1;
        bool closed = false;
        while (i < l.size()) {
            if (l[i] == '"') {
                if (i + 1 < l.size() && l[i + 1] == '"') {
                    parsed.push_back('"');
                    i += 2;
                } else {
                    closed = true;
                    i++;  // skip closing quote
                    break;
                }
            } else {
                parsed.push_back(l[i]);
                i++;
            }
        }
        if (!closed) {
            return false;
        }
        name = parsed;
        std::size_t comma = l.find(',', i);
        if (comma == std::string::npos) {
            return false;
        }
        phone = l.substr(comma + 1);
    } else {
        // Unquoted: split on LAST comma
        std::size_t comma = l.rfind(',');
        if (comma == std::string::npos) {
            return false;
        }
        name = l.substr(0, comma);
        phone = l.substr(comma + 1);
    }

    name = trimCsv(name);
    phone = trimCsv(phone);

    // Strip surrounding quotes from phone too, just in case
    if (phone.size() >= 2 && phone.front() == '"' && phone.back() == '"') {
        phone = trimCsv(phone.substr(1, phone.size() - 2));
    }
    return !(name.empty() && phone.empty());
}

}  // namespace

int PhoneBook::loadfromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return -1;
    }
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
        // Strip trailing \r for Windows-style files
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::string name, phone;
        if (!parseCsvLine(line, name, phone)) {
            continue;
        }
        // Reject malformed and duplicate records, keeping first-seen order.
        if (name.empty() || !isAllDigits(phone) || searchHashByPhone(phone) != -1) {
            continue;
        }
        // Push directly (no per-row index maintenance — that would be O(n^2));
        // the sorted index is built once below.
        appendContact(capitalizeFirst(toLower(name)), phone);
        ++count;
    }
    buildSortedIndex();
    return count;
}

bool PhoneBook::savetoCSV(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    for (const auto& contact : contacts) {
        file << csv::escapeField(contact.name) << ',' << contact.phone << '\n';
    }
    file.close();
    return static_cast<bool>(file);
}
