#pragma once

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

const int TABLE_SIZE = 101;
