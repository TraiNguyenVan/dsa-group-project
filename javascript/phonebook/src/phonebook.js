'use strict';

// Faithful port of include/phonebook.hpp + src/phonebook.cpp.
const fs = require('node:fs');
const { Contact } = require('./contact');
const { HashTable, DEFAULT_TABLE_SIZE } = require('./hashtable');
const { timeIt } = require('./timer');

function trimCsv(s) {
  return s.replace(/^[ \t\n\r\x0b\x0c]+|[ \t\n\r\x0b\x0c]+$/g, '');
}

// Returns [ok, name, phone]. Mirrors C++ parseCsvLine.
function parseCsvLine(line) {
  const l = trimCsv(line);
  if (l === '') return [false, '', ''];
  let name = '';
  let phone = '';
  if (l[0] === '"') {
    let parsed = '';
    let i = 1;
    let closed = false;
    while (i < l.length) {
      if (l[i] === '"') {
        if (i + 1 < l.length && l[i + 1] === '"') {
          parsed += '"';
          i += 2;
        } else {
          closed = true;
          i += 1;
          break;
        }
      } else {
        parsed += l[i];
        i += 1;
      }
    }
    if (!closed) return [false, '', ''];
    name = parsed;
    const comma = l.indexOf(',', i);
    if (comma === -1) return [false, '', ''];
    phone = l.slice(comma + 1);
  } else {
    const comma = l.lastIndexOf(',');
    if (comma === -1) return [false, '', ''];
    name = l.slice(0, comma);
    phone = l.slice(comma + 1);
  }
  name = trimCsv(name);
  phone = trimCsv(phone);
  if (phone.length >= 2 && phone[0] === '"' && phone[phone.length - 1] === '"') {
    phone = trimCsv(phone.slice(1, -1));
  }
  if (name === '' && phone === '') return [false, '', ''];
  return [true, name, phone];
}

// mix merges a[left..middle] and a[middle+1..right] into a sorted run.
function mix(a, left, middle, right) {
  const temp = new Array(right - left + 1);
  let i = left;
  let j = middle + 1;
  let k = 0;
  while (i <= middle && j <= right) {
    if (a[i] <= a[j]) {
      temp[k++] = a[i++];
    } else {
      temp[k++] = a[j++];
    }
  }
  while (i <= middle) {
    temp[k++] = a[i++];
  }
  while (j <= right) {
    temp[k++] = a[j++];
  }
  for (let x = left; x <= right; x++) {
    a[x] = temp[x - left];
  }
}

// split/divide
function divide(a, left, right) {
  const middle = Math.floor((left + right) / 2);
  if (left < right) {
    divide(a, left, middle);
    divide(a, middle + 1, right);
    mix(a, left, middle, right);
  }
}

class PhoneBook {
  constructor(initialCapacity = DEFAULT_TABLE_SIZE) {
    this.contacts = [];
    this.hashtable = new HashTable(initialCapacity);
    // Sorted copy of phone numbers (kept sorted incrementally) so binary
    // search is O(log n). The main array + hash table are untouched.
    this.sortedPhones = [];
  }

  // Hand-written binary search: first index where v[i] >= target.
  static lowerBound(v, target) {
    let lo = 0, hi = v.length;
    while (lo < hi) {
      const mid = lo + ((hi - lo) >> 1);
      if (v[mid] < target) lo = mid + 1;
      else hi = mid;
    }
    return lo;
  }

  // Rebuild the sorted index from scratch (O(n log n)); called once after load.
  buildSortedIndex() {
    this.sortedPhones = this.contacts.map((c) => c.phone);
    // Guard: mirrors C++ size_t underflow protection; 0/1 elements need no sort.
    if (this.sortedPhones.length >= 2) {
      divide(this.sortedPhones, 0, this.sortedPhones.length - 1);
    }
  }

  static isAllDigits(s) {
    if (!s) return false;
    for (let i = 0; i < s.length; i++) {
      if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
  }

  // ASCII-only, like C++.
  static toLower(s) {
    let res = '';
    for (let i = 0; i < s.length; i++) {
      const c = s[i];
      if (c >= 'A' && c <= 'Z') res += String.fromCharCode(c.charCodeAt(0) + 32);
      else res += c;
    }
    return res;
  }

  static capitalizeFirst(s) {
    const arr = s.split('');
    let newWord = true;
    for (let i = 0; i < arr.length; i++) {
      if (arr[i] === ' ') newWord = true;
      else {
        if (newWord && arr[i] >= 'a' && arr[i] <= 'z') {
          arr[i] = String.fromCharCode(arr[i].charCodeAt(0) - 32);
        }
        newWord = false;
      }
    }
    return arr.join('');
  }

  insertContact(name, phone) {
    if (!name || !phone) {
      console.log('Missing name or phone number');
      return false;
    }
    if (!PhoneBook.isAllDigits(phone)) {
      console.log('Invalid phone number');
      return false;
    }
    if (this.searchHashByPhone(phone) !== -1) {
      console.log('Phone number is already exist');
      return false;
    }
    const normalized = PhoneBook.capitalizeFirst(PhoneBook.toLower(name));
    // add contact to array + hashtable (O(1))
    const tHash = timeIt(() => {
      this.contacts.push(new Contact(normalized, phone));
      this.hashtable.hashInsert(phone, this.contacts.length - 1);
    });

    // keep the sorted index sorted: binary-search the spot (O(log n)) + shift (O(n))
    const tIndex = timeIt(() => {
      const pos = PhoneBook.lowerBound(this.sortedPhones, phone);
      this.sortedPhones.splice(pos, 0, phone);
    });

    console.log(`  hash insert: ${tHash}ms, sorted-index insert: ${tIndex}ms (the cost of keeping binary search possible)`);
    return true;
  }

  searchLinearByPhone(phone) {
    for (let i = 0; i < this.contacts.length; i++) {
      if (this.contacts[i].phone === phone) return i;
    }
    return -1;
  }

  searchHashByPhone(phone) {
    return this.hashtable.hashSearch(phone);
  }

  // Binary search by phone on the sorted index (O(log n)).
  // Returns the contact index via the hash table, or -1 if not found.
  searchBinaryByPhone(phone) {
    let lo = 0, hi = this.sortedPhones.length;
    while (lo < hi) {
      const mid = lo + ((hi - lo) >> 1);
      if (this.sortedPhones[mid] === phone) return this.hashtable.hashSearch(phone);
      if (this.sortedPhones[mid] < phone) lo = mid + 1;
      else hi = mid;
    }
    return -1;
  }

  deleteContactByPhone(phone) {
    // O(n): splice shifts tail + full hash rebuild preserves order.
    const idx = this.searchHashByPhone(phone);
    if (idx === -1) {
      console.log('Phone number not found');
      return false;
    }
    this.contacts.splice(idx, 1);
    this.hashtable.clear();
    for (let i = 0; i < this.contacts.length; i++) {
      this.hashtable.hashInsert(this.contacts[i].phone, i);
    }
    this.buildSortedIndex(); // delete is O(n) anyway; rebuild the sorted index
    return true;
  }

  searchLinearByName(name) {
    const target = PhoneBook.toLower(name);
    for (let i = 0; i < this.contacts.length; i++) {
      if (PhoneBook.toLower(this.contacts[i].name) === target) return i;
    }
    return -1;
  }

  // Prefix search using lower_bound + linear for first k results.
  searchPrefixByPhone(phone, k) {
    const result = [];
    const pos = PhoneBook.lowerBound(this.sortedPhones, phone);
    for (let i = pos; i < pos + k && i < this.sortedPhones.length; i++) {
      // stop as soon as the sorted phone no longer starts with the prefix
      if (!this.sortedPhones[i].startsWith(phone)) break;
      const index = this.hashtable.hashSearch(this.sortedPhones[i]);
      if (index !== -1) result.push(index);
    }
    return result;
  }

  printAll() {
    for (let i = 0; i < this.contacts.length; i++) {
      console.log(`${i}. ${this.contacts[i].name} - ${this.contacts[i].phone}`);
    }
  }

  printContact(index) {
    if (!Number.isInteger(index) || index < 0 || index >= this.contacts.length) return false;
    console.log(`Name: ${this.contacts[index].name}`);
    console.log(`Phone: ${this.contacts[index].phone}`);
    return true;
  }

  size() {
    return this.contacts.length;
  }

  getPhoneAt(index) {
    if (!Number.isInteger(index) || index < 0 || index >= this.contacts.length) return '';
    return this.contacts[index].phone;
  }

  loadfromCSV(path) {
    let content;
    try {
      content = fs.readFileSync(path, 'utf8');
    } catch (e) {
      return -1;
    }
    let count = 0;
    const lines = content.split('\n');
    for (let raw of lines) {
      let line = raw;
      if (line.endsWith('\r')) line = line.slice(0, -1);
      if (trimCsv(line) === '') continue;
      const [ok, name, phone] = parseCsvLine(line);
      if (!ok) continue;
      if (!name || !PhoneBook.isAllDigits(phone)) continue;
      if (this.searchHashByPhone(phone) !== -1) continue;
      // Push directly (no per-row index maintenance — that would be O(n^2));
      // the sorted index is built once below.
      const normalized = PhoneBook.capitalizeFirst(PhoneBook.toLower(name));
      this.contacts.push(new Contact(normalized, phone));
      this.hashtable.hashInsert(phone, this.contacts.length - 1);
      count++;
    }
    this.buildSortedIndex();
    return count;
  }

  savetoCSV(path) {
    try {
      let out = '';
      for (const c of this.contacts) {
        const needsQuote = c.name.includes(',') || c.name.includes('"');
        if (needsQuote) {
          out += '"';
          for (const ch of c.name) {
            if (ch === '"') out += '""';
            else out += ch;
          }
          out += '"';
        } else {
          out += c.name;
        }
        out += ',' + c.phone + '\n';
      }
      fs.writeFileSync(path, out, 'utf8');
      return true;
    } catch (e) {
      return false;
    }
  }

  // aliases matching C++ names
  loadfromCsv(p) { return this.loadfromCSV(p); }
  savetoCsv(p) { return this.savetoCSV(p); }
}

module.exports = { PhoneBook, trimCsv, parseCsvLine };
