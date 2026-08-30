#include "../../../include/linkedlist.hpp"

LinkedList::LinkedList() : head(nullptr), size(0) {}

LinkedList::~LinkedList() {}

void LinkedList::insert(Node* node) {
    if (size == 0) {
    }
    node->next = head;
    head = node;
    ++size;
}

Node* LinkedList::search(int key) {
    Node* node = head;
    while (node != nullptr) {
        if (node->val == key) {
            return node;
        }
        node = node->next;
    }
    return nullptr;
}

// someone write this for me please
bool LinkedList::remove(int key) {
    Node* node = head;
    return false;
}

int LinkedList::get_size() {
    return size;
}
