// separate_chaining.cpp — hash table experiment: separate chaining
// Author:  @TraiNguyenVan, @N25DECE074-truong-quang-tuan, @CapCapSever
// Source:  n/a
// AI use:  n/a

// Build and run with:
// g++ -std=c++17 src/cpp/hashing/separate_chaining.cpp -I src/cpp -o build/cpp/separate_chaining
// build/cpp/separate_chaining

// Option 3: Separate Chaining
#include <iostream>
using namespace std;

// ==========================
// Node của Linked List
// ==========================
struct Node {
    int key;
    Node* next;
    // constructor của node
    Node(int k) {
        key = k;
        next = nullptr;
    }
};

// ==========================
// Hash Table
// ==========================
class HashTable {
   private:
    static const int SIZE = 10;

    // Mỗi phần tử của table là đầu của một Linked List
    Node* table[SIZE];

    // Hàm băm
    // tìm một hằm băm cao cấp hơn (maybe or not)
    int hashFunction(int key) {
        return key % SIZE;
    }

   public:
    // ==========================
    // Constructor
    // ==========================
    HashTable() {
        for (int i = 0; i < SIZE; i++) {
            table[i] = nullptr;
        }
    }

    // ==========================
    // INSERT
    // ==========================
    void insert(int key) {
        int index = hashFunction(key);

        // Tạo Node mới
        Node* newNode = new Node(key);

        // Đưa Node mới vào đầu Linked List
        newNode->next = table[index];

        table[index] = newNode;
    }

    // ==========================
    // SEARCH
    // ==========================
    bool search(int key) {
        int index = hashFunction(key);

        // Bắt đầu từ Node đầu tiên
        Node* current = table[index];

        // Duyệt Linked List
        while (current != nullptr) {
            if (current->key == key) {
                return true;
            }

            current = current->next;
        }

        return false;
    }

    // ==========================
    // DELETE
    // ==========================
    bool remove(int key) {
        int index = hashFunction(key);

        Node* current = table[index];
        Node* prev = nullptr;

        while (current != nullptr) {
            // Tìm thấy key
            if (current->key == key) {
                // Nếu Node cần xóa là Node đầu tiên
                if (prev == nullptr) {
                    table[index] = current->next;
                }

                // Nếu Node nằm giữa hoặc cuối
                else {
                    prev->next = current->next;
                }

                delete current;

                return true;
            }

            // Di chuyển sang Node tiếp theo
            prev = current;
            current = current->next;
        }

        return false;
    }

    // ==========================
    // DISPLAY
    // ==========================
    void display() {
        for (int i = 0; i < SIZE; i++) {
            cout << "Bucket " << i << ": ";

            Node* current = table[i];

            while (current != nullptr) {
                cout << current->key << " -> ";

                current = current->next;
            }

            cout << "NULL\n";
        }
    }

    // ==========================
    // Destructor
    // ==========================
    ~HashTable() {
        for (int i = 0; i < SIZE; i++) {
            Node* current = table[i];

            while (current != nullptr) {
                Node* temp = current;

                current = current->next;

                delete temp;
            }
        }
    }
};

// ==========================
// MAIN
// ==========================
int main() {
    HashTable ht;

    // Thêm phần tử
    ht.insert(10);
    ht.insert(20);
    ht.insert(15);
    ht.insert(25);
    ht.insert(35);
    ht.insert(42);

    // Hiển thị
    cout << "Hash Table:\n";
    ht.display();

    // ==========================
    // SEARCH
    // ==========================

    cout << "\nSearch 25: ";

    if (ht.search(25)) {
        cout << "Found\n";
    } else {
        cout << "Not Found\n";
    }

    cout << "Search 100: ";

    if (ht.search(100)) {
        cout << "Found\n";
    } else {
        cout << "Not Found\n";
    }

    // ==========================
    // DELETE
    // ==========================

    cout << "\nDelete 25\n";

    if (ht.remove(25)) {
        cout << "Deleted successfully\n";
    } else {
        cout << "Not Found\n";
    }

    // Hiển thị lại
    cout << "\nHash Table after delete:\n";
    ht.display();

    return 0;
}