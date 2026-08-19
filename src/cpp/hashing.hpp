// =========================================================================
// Algorithm: Open Hashing - Direct Chaining (Nối kết trực tiếp)
// Source: Table 42 of Chương 3 - Hashing.docx
// =========================================================================

struct Node {
    int key;     // The actual value being stored
    Node* next;  // Pointer to the next node in the chain
};

void add(Node*& L, int x) {
    Node* p = L;
    if (L == NULL) {
        addTail(L, x);
        return;
    }
    if (x < L->key) {
        addHead(L, x);
        return;
    }
    while (p != NULL) {
        if (x <= p->key) {
            addMid(L, p, x);
            return;
        }
        p = p->next;
    }
    addTail(L, x);
}
void NoiKetTrucTiep(Hash& H, int x) {
    int i = getIndex(H, x);
    add(H.bucket[i], x);
}

// =========================================================================
// Algorithm: Open Hashing - Coalesced Chaining (Nối kết hợp nhất)
// Source: Table 43 of Chương 3 - Hashing.docx
// =========================================================================

void NoiKetHopNhat(Hash& H, int x) {
    if (isFull(H) == 1) {
        cout << "\nkhong the them " << x << " vi bang bam da day!";
        return;
    }
    int i = getIndex(H, x);
    if (H.bucket[i].key == NULLKEY) {
        H.bucket[i].key = x;
        H.N++;
    } else {
        while (H.bucket[i].key != NULLKEY) {
            while (H.bucket[i].next != NULLKEY) {
                i = H.bucket[i].next;
            }
            for (int j = H.m - 1; j >= 0; j--)
                if (H.bucket[j].key == NULLKEY) {
                    H.bucket[i].next = j;
                    H.bucket[j].key = x;
                    H.N++;
                    return;
                }
        }
    }
}

// =========================================================================
// Algorithm: Closed Hashing - Linear Probing (Dò tuyến tính)
// Source: Table 44 of Chương 3 - Hashing.docx
// =========================================================================

void DoTuyenTinh(Hash& H, int x) {
    if (isFull(H) == 1) {
        cout << "\nBang bam bi day, khong them duoc";
        return;
    }
    int i = getIndex(x);
    while (true) {
        if (H.bucket[i].key == NULLKEY) {
            H.bucket[i].value = x;
            H.bucket[i].key = 0;
            H.N++;
            return;
        }
        i++;
        if (i >= M) i -= M;
    }
}

// =========================================================================
// Algorithm: Closed Hashing - Quadratic Probing (Dò bậc hai)
// Source: Table 45 of Chương 3 - Hashing.docx
// =========================================================================

void DoBacHai(Hash& H, int x) {
    if (isFull(H) == 1) {
        cout << "\nBang bam bi day, khong them duoc";
        return;
    }
    int index = getIndex(x);
    int i = 0;
    while (H.bucket[index].key != NULLKEY) {
        index = (getIndex(x) + i * i) % M;
        i++;
        if (index > M) index = index - M;
    }
    H.bucket[index].value = x;
    H.bucket[index].key = 0;
    H.N++;
}

// =========================================================================
// Algorithm: Closed Hashing - Double Hashing (Băm kép)
// Source: Table 46 of Chương 3 - Hashing.docx
// =========================================================================

void BamKep(Hash& H, int x, int m) {
    if (isFull(H, m) == 1) {
        cout << "\nBang bam bi day, khong them duoc" << x << endl;
        return;
    }
    int i = getIndex(x, m);
    int index = i;
    int j = abs((m - x) % m);
    int so = 1;
    while (H.bucket[index].key != NULLKEY) {
        index = (i + so * j) % m;
        so++;
    }
    H.bucket[index].value = x;
    H.bucket[index].key = 0;
    H.N++;
}
