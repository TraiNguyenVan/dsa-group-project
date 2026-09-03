// Singly LinkedList
#include "node.hpp" j
class LinkedList {
private:
    Node* head;
    int size;

public:
    LinkedList();
    ~LinkedList();
    void insert(Node* node);
    Node* search(int key);
    bool remove(int key);
    int get_size();
};
