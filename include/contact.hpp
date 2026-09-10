#ifndef CONTACT_HPP
#define CONTACT_HPP

#include <string>

struct Contact {
    std::string name;
    std::string phone;
};

struct HashNode {
    std::string phone;
    int contactIndex;
    HashNode* next;
};

#endif#ifndef CONTACT_HPP
#define CONTACT_HPP

#include <string>

struct Contact {
    std::string name;
    std::string phone;
};

struct HashNode {
    std::string phone;
    int contactIndex;
    HashNode* next;
};

#endif