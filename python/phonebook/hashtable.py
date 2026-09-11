"""Chained hash table, faithful port of include/hashtable.hpp + src/hashtable.cpp."""


class HashNode:
    def __init__(self, phone: str, contact_index: int, next_node=None):
        self.phone = phone
        self.contact_index = contact_index
        self.next = next_node


class HashTable:
    DEFAULT_TABLE_SIZE = 101
    MAX_LOAD_FACTOR = 0.75
    HASH_BASE = 31
    _MASK64 = 0xFFFFFFFFFFFFFFFF

    def __init__(self, initial_capacity: int = DEFAULT_TABLE_SIZE):
        if initial_capacity == 0:
            initial_capacity = self.DEFAULT_TABLE_SIZE
        self.num_buckets = self.next_prime(initial_capacity)
        self.num_elements = 0
        self.buckets = [None] * self.num_buckets

    @staticmethod
    def is_prime(n: int) -> bool:
        if n <= 3:
            return n > 1
        if n % 2 == 0 or n % 3 == 0:
            return False
        i = 5
        while i * i <= n:
            if n % i == 0 or n % (i + 2) == 0:
                return False
            i += 6
        return True

    @staticmethod
    def next_prime(n: int) -> int:
        if n <= 2:
            return 2
        if n % 2 == 0:
            n += 1
        if n == 2:
            n += 1
        while not HashTable.is_prime(n):
            n += 2
        return n

    @staticmethod
    def hash_for_size(phone: str, mod: int) -> int:
        h = 0
        for c in phone:
            # (unsigned char)(c - '0') to match C++ wrap on non-digits
            diff = (ord(c) - ord('0')) & 0xFF
            h = ((h * HashTable.HASH_BASE + diff) & HashTable._MASK64)
        return h % mod

    def hash_function(self, phone: str) -> int:
        return self.hash_for_size(phone, self.num_buckets)

    def hash_insert(self, phone: str, contact_index: int) -> None:
        idx = self.hash_function(phone)
        node = HashNode(phone, contact_index, self.buckets[idx])
        self.buckets[idx] = node
        self.num_elements += 1
        self.maybe_rehash()

    def hash_search(self, phone: str) -> int:
        idx = self.hash_function(phone)
        node = self.buckets[idx]
        while node is not None:
            if node.phone == phone:
                return node.contact_index
            node = node.next
        return -1

    def hash_delete(self, phone: str) -> bool:
        idx = self.hash_function(phone)
        node = self.buckets[idx]
        prev = None
        while node is not None:
            if node.phone == phone:
                if prev is None:
                    self.buckets[idx] = node.next
                else:
                    prev.next = node.next
                self.num_elements -= 1
                return True
            prev = node
            node = node.next
        return False

    def hash_update(self, phone: str, new_contact_index: int) -> bool:
        idx = self.hash_function(phone)
        node = self.buckets[idx]
        while node is not None:
            if node.phone == phone:
                node.contact_index = new_contact_index
                return True
            node = node.next
        return False

    def clear(self) -> None:
        self.buckets = [None] * self.num_buckets
        self.num_elements = 0

    def load_factor(self) -> float:
        return self.num_elements / float(self.num_buckets)

    def maybe_rehash(self) -> None:
        if self.load_factor() > self.MAX_LOAD_FACTOR:
            self.rehash(self.num_buckets * 2)

    def rehash(self, new_capacity: int) -> None:
        new_bucket_count = self.next_prime(new_capacity)
        new_buckets = [None] * new_bucket_count
        for i in range(self.num_buckets):
            current = self.buckets[i]
            while current is not None:
                next_node = current.next
                new_index = self.hash_for_size(current.phone, new_bucket_count)
                current.next = new_buckets[new_index]
                new_buckets[new_index] = current
                current = next_node
        self.buckets = new_buckets
        self.num_buckets = new_bucket_count
