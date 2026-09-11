"""Faithful port of include/phonebook.hpp + src/phonebook.cpp."""
from typing import List, Tuple

try:
    from contact import Contact
    from hashtable import HashTable
    from timer import time_it
except ImportError:  # package-style run: python -m python.phonebook.main
    from python.phonebook.contact import Contact
    from python.phonebook.hashtable import HashTable
    from python.phonebook.timer import time_it


def _trim_csv(s: str) -> str:
    return s.strip(" \t\n\r\x0b\x0c")


def _parse_csv_line(line: str) -> Tuple[bool, str, str]:
    l = _trim_csv(line)
    if not l:
        return False, "", ""
    name = ""
    phone = ""
    if l[0] == '"':
        parsed_chars = []
        i = 1
        closed = False
        while i < len(l):
            if l[i] == '"':
                if i + 1 < len(l) and l[i + 1] == '"':
                    parsed_chars.append('"')
                    i += 2
                else:
                    closed = True
                    i += 1
                    break
            else:
                parsed_chars.append(l[i])
                i += 1
        if not closed:
            return False, "", ""
        name = "".join(parsed_chars)
        comma = l.find(",", i)
        if comma == -1:
            return False, "", ""
        phone = l[comma + 1:]
    else:
        comma = l.rfind(",")
        if comma == -1:
            return False, "", ""
        name = l[:comma]
        phone = l[comma + 1:]

    name = _trim_csv(name)
    phone = _trim_csv(phone)
    if len(phone) >= 2 and phone[0] == '"' and phone[-1] == '"':
        phone = _trim_csv(phone[1:-1])
    if not name and not phone:
        return False, "", ""
    return True, name, phone


class PhoneBook:
    def __init__(self, initial_capacity: int = HashTable.DEFAULT_TABLE_SIZE):
        self.contacts: List[Contact] = []
        self.hashtable = HashTable(initial_capacity)
        # Sorted copy of phone numbers (kept sorted incrementally) so binary
        # search is O(log n). The main list + hash table are untouched.
        self.sorted_phones: List[str] = []

    @staticmethod
    def lower_bound(v: List[str], target: str) -> int:
        """Hand-written binary search: first index where v[i] >= target."""
        lo, hi = 0, len(v)
        while lo < hi:
            mid = lo + (hi - lo) // 2
            if v[mid] < target:
                lo = mid + 1
            else:
                hi = mid
        return lo

    def build_sorted_index(self) -> None:
        """Rebuild the sorted index from scratch (O(n log n)); called once after load."""
        self.sorted_phones = sorted(c.phone for c in self.contacts)

    @staticmethod
    def is_all_digits(s: str) -> bool:
        if not s:
            return False
        for c in s:
            if c < "0" or c > "9":
                return False
        return True

    @staticmethod
    def to_lower(s: str) -> str:
        res = []
        for c in s:
            if "A" <= c <= "Z":
                res.append(chr(ord(c) + 32))
            else:
                res.append(c)
        return "".join(res)

    @staticmethod
    def capitalize_first(s: str) -> str:
        res = list(s)
        new_word = True
        for i in range(len(res)):
            if res[i] == " ":
                new_word = True
            else:
                if new_word and "a" <= res[i] <= "z":
                    res[i] = chr(ord(res[i]) - 32)
                new_word = False
        return "".join(res)

    def insert_contact(self, name: str, phone: str) -> bool:
        if not name or not phone:
            print("Missing name or phone number")
            return False
        if not self.is_all_digits(phone):
            print("Invalid phone number")
            return False
        if self.search_hash_by_phone(phone) != -1:
            print("Phone number is already exist")
            return False
        normalized = self.capitalize_first(self.to_lower(name))
        # add contact to list + hashtable (O(1))
        holder = {}

        def do_hash():
            self.contacts.append(Contact(normalized, phone))
            self.hashtable.hash_insert(phone, len(self.contacts) - 1)

        t_hash = time_it(do_hash)

        # keep the sorted index sorted: binary-search the spot (O(log n)) + shift (O(n))
        def do_index():
            pos = self.lower_bound(self.sorted_phones, phone)
            self.sorted_phones.insert(pos, phone)

        t_index = time_it(do_index)
        print(f"  hash insert: {t_hash}ms, sorted-index insert: {t_index}ms"
              " (the cost of keeping binary search possible)")
        return True

    def delete_contact_by_phone(self, phone: str) -> bool:
        # O(n): list erase shifts tail + full hash rebuild preserves order.
        idx = self.search_hash_by_phone(phone)
        if idx == -1:
            print("Phone number not found")
            return False
        del self.contacts[idx]
        self.hashtable.clear()
        for i, c in enumerate(self.contacts):
            self.hashtable.hash_insert(c.phone, i)
        self.build_sorted_index()  # delete is O(n) anyway; rebuild the sorted index
        return True

    def search_linear_by_phone(self, phone: str) -> int:
        for i, c in enumerate(self.contacts):
            if c.phone == phone:
                return i
        return -1

    def search_hash_by_phone(self, phone: str) -> int:
        return self.hashtable.hash_search(phone)

    def search_binary_by_phone(self, phone: str) -> int:
        """Binary search by phone on the sorted index (O(log n)).

        Returns the contact index via the hash table, or -1 if not found.
        """
        lo, hi = 0, len(self.sorted_phones)
        while lo < hi:
            mid = lo + (hi - lo) // 2
            if self.sorted_phones[mid] == phone:
                return self.hashtable.hash_search(phone)
            if self.sorted_phones[mid] < phone:
                lo = mid + 1
            else:
                hi = mid
        return -1

    def search_linear_by_name(self, name: str) -> int:
        target = self.to_lower(name)
        for i, c in enumerate(self.contacts):
            if self.to_lower(c.name) == target:
                return i
        return -1

    def print_all(self) -> None:
        for i, c in enumerate(self.contacts):
            print(f"{i}. {c.name} - {c.phone}")

    def print_contact(self, index: int) -> bool:
        if index < 0 or index >= len(self.contacts):
            return False
        print(f"Name: {self.contacts[index].name}")
        print(f"Phone: {self.contacts[index].phone}")
        return True

    def size(self) -> int:
        return len(self.contacts)

    def get_phone_at(self, index: int) -> str:
        if index < 0 or index >= len(self.contacts):
            return ""
        return self.contacts[index].phone

    def loadfrom_csv(self, path: str) -> int:
        try:
            f = open(path, "r", encoding="utf-8", newline="")
        except OSError:
            return -1
        count = 0
        with f:
            for raw in f:
                line = raw
                if line.endswith("\n"):
                    line = line[:-1]
                    if line.endswith("\r"):
                        line = line[:-1]
                elif line.endswith("\r"):
                    line = line[:-1]
                if not _trim_csv(line):
                    continue
                ok, name, phone = _parse_csv_line(line)
                if not ok:
                    continue
                if not name or not self.is_all_digits(phone):
                    continue
                if self.search_hash_by_phone(phone) != -1:
                    continue
                # Push directly (no per-row index maintenance — that would be O(n^2));
                # the sorted index is built once below.
                normalized = self.capitalize_first(self.to_lower(name))
                self.contacts.append(Contact(normalized, phone))
                self.hashtable.hash_insert(phone, len(self.contacts) - 1)
                count += 1
        self.build_sorted_index()
        return count

    def saveto_csv(self, path: str) -> bool:
        try:
            f = open(path, "w", encoding="utf-8", newline="\n")
        except OSError:
            return False
        with f:
            for c in self.contacts:
                needs_quote = ("," in c.name) or ('"' in c.name)
                if needs_quote:
                    f.write('"')
                    for ch in c.name:
                        if ch == '"':
                            f.write('""')
                        else:
                            f.write(ch)
                    f.write('"')
                else:
                    f.write(c.name)
                f.write("," + c.phone + "\n")
        return True

    # camelCase aliases matching C++ names (convenience)
    loadfromCSV = loadfrom_csv
    savetoCSV = saveto_csv
    insertContact = insert_contact
    deleteContactByPhone = delete_contact_by_phone
    searchLinearByPhone = search_linear_by_phone
    searchHashByPhone = search_hash_by_phone
    searchBinaryByPhone = search_binary_by_phone
    buildSortedIndex = build_sorted_index
    searchLinearByName = search_linear_by_name
    printAll = print_all
    printContact = print_contact
    getPhoneAt = get_phone_at
