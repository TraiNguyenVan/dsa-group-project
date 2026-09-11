package com.phonebook;

// Faithful port of include/hashtable.hpp + src/hashtable.cpp.
// Chained hash table with polynomial rolling hash, prime sizing, rehash.
public class HashTable {
    public static final int DEFAULT_TABLE_SIZE = 101;
    public static final double MAX_LOAD_FACTOR = 0.75;
    private static final long HASH_BASE = 31L;

    public static class HashNode {
        String phone;
        int contactIndex;
        HashNode next;

        HashNode(String phone, int contactIndex, HashNode next) {
            this.phone = phone;
            this.contactIndex = contactIndex;
            this.next = next;
        }
    }

    private HashNode[] buckets;
    private int numBuckets;
    private int numElements;

    public HashTable(int initialCapacity) {
        if (initialCapacity == 0) {
            initialCapacity = DEFAULT_TABLE_SIZE;
        }
        this.numBuckets = nextPrime(initialCapacity);
        this.numElements = 0;
        this.buckets = new HashNode[this.numBuckets];
    }

    public HashTable() {
        this(DEFAULT_TABLE_SIZE);
    }

    public static boolean isPrime(int n) {
        if (n <= 3) {
            return n > 1;
        }
        if (n % 2 == 0 || n % 3 == 0) {
            return false;
        }
        for (int i = 5; (long) i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) {
                return false;
            }
        }
        return true;
    }

    public static int nextPrime(int n) {
        if (n <= 2) {
            return 2;
        }
        if (n % 2 == 0) {
            n++;
        }
        if (n == 2) {
            n++;
        }
        while (!isPrime(n)) {
            n += 2;
        }
        return n;
    }

    // long arithmetic wraps like C++ size_t on 64-bit; unsigned remainder matches.
    static int hashForSize(String phone, int mod) {
        long h = 0L;
        for (int i = 0; i < phone.length(); i++) {
            int diff = (phone.charAt(i) - '0') & 0xFF; // (unsigned char)(c - '0')
            h = h * HASH_BASE + diff;
        }
        return (int) Long.remainderUnsigned(h, mod);
    }

    private int hashFunction(String phone) {
        return hashForSize(phone, numBuckets);
    }

    public void hashInsert(String phone, int contactIndex) {
        int idx = hashFunction(phone);
        HashNode node = new HashNode(phone, contactIndex, buckets[idx]);
        buckets[idx] = node;
        numElements++;
        maybeRehash();
    }

    public int hashSearch(String phone) {
        int idx = hashFunction(phone);
        HashNode node = buckets[idx];
        while (node != null) {
            if (node.phone.equals(phone)) {
                return node.contactIndex;
            }
            node = node.next;
        }
        return -1;
    }

    public boolean hashDelete(String phone) {
        int idx = hashFunction(phone);
        HashNode node = buckets[idx];
        HashNode prev = null;
        while (node != null) {
            if (node.phone.equals(phone)) {
                if (prev != null) {
                    prev.next = node.next;
                } else {
                    buckets[idx] = node.next;
                }
                numElements--;
                return true;
            }
            prev = node;
            node = node.next;
        }
        return false;
    }

    public boolean hashUpdate(String phone, int newContactIndex) {
        int idx = hashFunction(phone);
        HashNode node = buckets[idx];
        while (node != null) {
            if (node.phone.equals(phone)) {
                node.contactIndex = newContactIndex;
                return true;
            }
            node = node.next;
        }
        return false;
    }

    public void clear() {
        for (int i = 0; i < numBuckets; i++) {
            buckets[i] = null;
        }
        numElements = 0;
    }

    public double loadFactor() {
        return (double) numElements / (double) numBuckets;
    }

    private void maybeRehash() {
        if (loadFactor() > MAX_LOAD_FACTOR) {
            rehash(numBuckets * 2);
        }
    }

    private void rehash(int newCapacity) {
        int newBucketCount = nextPrime(newCapacity);
        HashNode[] newBuckets = new HashNode[newBucketCount];
        for (int i = 0; i < numBuckets; i++) {
            HashNode current = buckets[i];
            while (current != null) {
                HashNode nextNode = current.next;
                int newIndex = hashForSize(current.phone, newBucketCount);
                current.next = newBuckets[newIndex];
                newBuckets[newIndex] = current;
                current = nextNode;
            }
        }
        buckets = newBuckets;
        numBuckets = newBucketCount;
    }
}
