'use strict';

// Faithful port of include/hashtable.hpp + src/hashtable.cpp.
// Chained hash table with polynomial rolling hash, prime sizing, rehash.

const DEFAULT_TABLE_SIZE = 101;
const MAX_LOAD_FACTOR = 0.75;
const HASH_BASE = 31n;
const MASK64 = 0xffffffffffffffffn;

class HashNode {
  constructor(phone, contactIndex, next = null) {
    this.phone = phone;
    this.contactIndex = contactIndex;
    this.next = next;
  }
}

function isPrime(n) {
  if (n <= 3) return n > 1;
  if (n % 2 === 0 || n % 3 === 0) return false;
  for (let i = 5; i * i <= n; i += 6) {
    if (n % i === 0 || n % (i + 2) === 0) return false;
  }
  return true;
}

function nextPrime(n) {
  if (n <= 2) return 2;
  if (n % 2 === 0) n += 1;
  if (n === 2) n += 1;
  while (!isPrime(n)) n += 2;
  return n;
}

// BigInt math with 64-bit mask to match C++ size_t wrap on 64-bit.
function hashForSize(phone, mod) {
  let h = 0n;
  for (let i = 0; i < phone.length; i++) {
    const diff = (phone.charCodeAt(i) - 48) & 0xff;
    h = (h * HASH_BASE + BigInt(diff)) & MASK64;
  }
  return Number(h % BigInt(mod));
}

class HashTable {
  constructor(initialCapacity = DEFAULT_TABLE_SIZE) {
    if (initialCapacity === 0) initialCapacity = DEFAULT_TABLE_SIZE;
    this.numBuckets = nextPrime(initialCapacity);
    this.numElements = 0;
    this.buckets = new Array(this.numBuckets).fill(null);
  }

  hashFunction(phone) {
    return hashForSize(phone, this.numBuckets);
  }

  hashInsert(phone, contactIndex) {
    const idx = this.hashFunction(phone);
    const node = new HashNode(phone, contactIndex, this.buckets[idx]);
    this.buckets[idx] = node;
    this.numElements += 1;
    this.maybeRehash();
  }

  hashSearch(phone) {
    const idx = this.hashFunction(phone);
    let node = this.buckets[idx];
    while (node !== null) {
      if (node.phone === phone) return node.contactIndex;
      node = node.next;
    }
    return -1;
  }

  hashDelete(phone) {
    const idx = this.hashFunction(phone);
    let node = this.buckets[idx];
    let prev = null;
    while (node !== null) {
      if (node.phone === phone) {
        if (prev === null) this.buckets[idx] = node.next;
        else prev.next = node.next;
        this.numElements -= 1;
        return true;
      }
      prev = node;
      node = node.next;
    }
    return false;
  }

  hashUpdate(phone, newContactIndex) {
    const idx = this.hashFunction(phone);
    let node = this.buckets[idx];
    while (node !== null) {
      if (node.phone === phone) {
        node.contactIndex = newContactIndex;
        return true;
      }
      node = node.next;
    }
    return false;
  }

  clear() {
    this.buckets = new Array(this.numBuckets).fill(null);
    this.numElements = 0;
  }

  loadFactor() {
    return this.numElements / this.numBuckets;
  }

  maybeRehash() {
    if (this.loadFactor() > MAX_LOAD_FACTOR) {
      this.rehash(this.numBuckets * 2);
    }
  }

  rehash(newCapacity) {
    const newBucketCount = nextPrime(newCapacity);
    const newBuckets = new Array(newBucketCount).fill(null);
    for (let i = 0; i < this.numBuckets; i++) {
      let current = this.buckets[i];
      while (current !== null) {
        const nextNode = current.next;
        const newIndex = hashForSize(current.phone, newBucketCount);
        current.next = newBuckets[newIndex];
        newBuckets[newIndex] = current;
        current = nextNode;
      }
    }
    this.buckets = newBuckets;
    this.numBuckets = newBucketCount;
  }
}

module.exports = {
  HashTable,
  HashNode,
  isPrime,
  nextPrime,
  hashForSize,
  DEFAULT_TABLE_SIZE,
  MAX_LOAD_FACTOR,
};
