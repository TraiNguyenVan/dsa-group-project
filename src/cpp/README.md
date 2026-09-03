### 1. In Open Hashing - Direct Chaining (NoiKetTrucTiep)

  In this method, Node is a standard singly linked list node where each bucket in the hash table points to a
  linked list:

    struct Node {
        int key;       // The actual value being stored
        Node* next;    // Pointer to the next node in the chain
    };

  And Hash is a struct containing an array of node pointers:

    struct Hash {
        Node* bucket[M]; // Array of linked lists
        // ...
    };

    

  ──────
  ### 2. In Open Hashing - Coalesced Chaining (NoiKetHopNhat)

  In Coalesced Chaining, all elements are stored inside the table array. Therefore, next is not a pointer but
  an integer index referencing another slot in the same array:

    struct Node {
        int key;       // The stored value (or NULLKEY/state indicator)
        int next;      // Array index of the next node in the chain (or NULLKEY)
    };
  ──────
  ### 3. In Closed Hashing / Open Addressing (Linear Probing, Quadratic Probing, Double Hashing)

  For open addressing methods, each slot in the table contains the actual value and a status flag (key is used
  as a state indicator, where NULLKEY means empty/free, and 0 means occupied):

    struct Node {
        int key;       // Status flag (e.g., NULLKEY/nil, or 0 if occupied)
        int value;     // The actual value stored
    };

