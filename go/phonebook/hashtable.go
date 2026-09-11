package main

// Faithful port of include/hashtable.hpp + src/hashtable.cpp.
// Chained hash table with polynomial rolling hash, prime sizing, rehash.

const (
	DefaultTableSize = 101
	MaxLoadFactor    = 0.75
	HashBase         = 31
)

// HashNode is one entry in a bucket chain.
type HashNode struct {
	Phone        string
	ContactIndex int
	Next         *HashNode
}

// HashTable holds the buckets.
type HashTable struct {
	buckets     []*HashNode
	numBuckets  int
	numElements int
}

// NewHashTable creates a table sized to the next prime >= initialCapacity.
func NewHashTable(initialCapacity int) *HashTable {
	if initialCapacity == 0 {
		initialCapacity = DefaultTableSize
	}
	nb := NextPrime(initialCapacity)
	return &HashTable{
		buckets:     make([]*HashNode, nb),
		numBuckets:  nb,
		numElements: 0,
	}
}

// IsPrime mirrors HashTable::isPrime (6k +- 1 check).
func IsPrime(n int) bool {
	if n <= 3 {
		return n > 1
	}
	if n%2 == 0 || n%3 == 0 {
		return false
	}
	for i := 5; i*i <= n; i += 6 {
		if n%i == 0 || n%(i+2) == 0 {
			return false
		}
	}
	return true
}

// NextPrime mirrors HashTable::nextPrime.
func NextPrime(n int) int {
	if n <= 2 {
		return 2
	}
	if n%2 == 0 {
		n++
	}
	if n == 2 {
		n++
	}
	for !IsPrime(n) {
		n += 2
	}
	return n
}

// HashForSize mirrors HashTable::HashForSize.
// uint64 arithmetic wraps like C++ size_t on 64-bit platforms.
func HashForSize(phone string, mod int) int {
	var h uint64
	for i := 0; i < len(phone); i++ {
		diff := phone[i] - '0' // byte wrap == (unsigned char)(c - '0')
		h = h*HashBase + uint64(diff)
	}
	return int(h % uint64(mod))
}

func (t *HashTable) hashFunction(phone string) int {
	return HashForSize(phone, t.numBuckets)
}

// HashInsert prepends a node and rehashes if needed.
func (t *HashTable) HashInsert(phone string, contactIndex int) {
	idx := t.hashFunction(phone)
	node := &HashNode{Phone: phone, ContactIndex: contactIndex, Next: t.buckets[idx]}
	t.buckets[idx] = node
	t.numElements++
	t.maybeRehash()
}

// HashSearch returns the contact index or -1.
func (t *HashTable) HashSearch(phone string) int {
	idx := t.hashFunction(phone)
	for node := t.buckets[idx]; node != nil; node = node.Next {
		if node.Phone == phone {
			return node.ContactIndex
		}
	}
	return -1
}

// HashDelete unlinks the node for phone; true if found. O(1) average.
func (t *HashTable) HashDelete(phone string) bool {
	idx := t.hashFunction(phone)
	node := t.buckets[idx]
	var prev *HashNode
	for node != nil {
		if node.Phone == phone {
			if prev != nil {
				prev.Next = node.Next
			} else {
				t.buckets[idx] = node.Next
			}
			t.numElements--
			return true
		}
		prev = node
		node = node.Next
	}
	return false
}

// HashUpdate repoints an existing phone entry at a new index.
// Kept for teaching chained-hash update; PhoneBook delete (O(n) rebuild)
// does not use it.
func (t *HashTable) HashUpdate(phone string, newContactIndex int) bool {
	idx := t.hashFunction(phone)
	for node := t.buckets[idx]; node != nil; node = node.Next {
		if node.Phone == phone {
			node.ContactIndex = newContactIndex
			return true
		}
	}
	return false
}

// Clear drops all entries but keeps the bucket count.
func (t *HashTable) Clear() {
	for i := range t.buckets {
		t.buckets[i] = nil
	}
	t.numElements = 0
}

// LoadFactor returns numElements / numBuckets.
func (t *HashTable) LoadFactor() float64 {
	return float64(t.numElements) / float64(t.numBuckets)
}

func (t *HashTable) maybeRehash() {
	if t.LoadFactor() > MaxLoadFactor {
		t.Rehash(t.numBuckets * 2)
	}
}

// Rehash moves every node into a bigger prime-sized table.
func (t *HashTable) Rehash(newCapacity int) {
	newBucketCount := NextPrime(newCapacity)
	newBuckets := make([]*HashNode, newBucketCount)
	for i := 0; i < t.numBuckets; i++ {
		current := t.buckets[i]
		for current != nil {
			nextNode := current.Next
			newIndex := HashForSize(current.Phone, newBucketCount)
			current.Next = newBuckets[newIndex]
			newBuckets[newIndex] = current
			current = nextNode
		}
	}
	t.buckets = newBuckets
	t.numBuckets = newBucketCount
}
