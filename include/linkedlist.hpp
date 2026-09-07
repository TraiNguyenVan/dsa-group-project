// Singly LinkedList
#include "node.hpp"
class LinkedList {
private:
    Node* head;
    int size;

public:
    LinkedList();
    ~LinkedList();
    void insert(Node* node);
    Node* search(int key);
    bool remove(int key);  // not implemented yet:D
    int get_size();
};
