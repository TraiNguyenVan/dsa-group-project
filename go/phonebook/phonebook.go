package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

// Faithful port of include/phonebook.hpp + src/phonebook.cpp.

// PhoneBook stores contacts plus a hash index on phone.
type PhoneBook struct {
	contacts  []Contact
	hashtable *HashTable
	// Sorted copy of phone numbers (kept sorted incrementally) so binary
	// search is O(log n). The main slice + hash table are untouched.
	sortedPhones []string
}

// lowerBound is a hand-written binary search: first index where v[i] >= target.
func lowerBound(v []string, target string) int {
	lo, hi := 0, len(v)
	for lo < hi {
		mid := lo + (hi-lo)/2
		if v[mid] < target {
			lo = mid + 1
		} else {
			hi = mid
		}
	}
	return lo
}

// mix merges a[left..middle] and a[middle+1..right] into a sorted run.
func mix(a []string, left, middle, right int) {
	temp := make([]string, right-left+1)
	i := left
	j := middle + 1
	k := 0
	for i <= middle && j <= right {
		if a[i] <= a[j] {
			temp[k] = a[i]
			k++
			i++
		} else {
			temp[k] = a[j]
			k++
			j++
		}
	}
	for i <= middle {
		temp[k] = a[i]
		k++
		i++
	}
	for j <= right {
		temp[k] = a[j]
		k++
		j++
	}
	for x := left; x <= right; x++ {
		a[x] = temp[x-left]
	}
}

// split/divide
func divide(a []string, left, right int) {
	middle := (left + right) / 2
	if left < right {
		divide(a, left, middle)
		divide(a, middle+1, right)
		mix(a, left, middle, right)
	}
}

// BuildSortedIndex rebuilds the sorted index from scratch (O(n log n)); called once after load.
func (p *PhoneBook) BuildSortedIndex() {
	p.sortedPhones = p.sortedPhones[:0]
	for _, c := range p.contacts {
		p.sortedPhones = append(p.sortedPhones, c.Phone)
	}
	// Guard: mirrors C++ size_t underflow protection; 0/1 elements need no sort.
	if len(p.sortedPhones) >= 2 {
		divide(p.sortedPhones, 0, len(p.sortedPhones)-1)
	}
}

// NewPhoneBook creates an empty phonebook.
func NewPhoneBook(initialCapacity ...int) *PhoneBook {
	cap := DefaultTableSize
	if len(initialCapacity) > 0 {
		cap = initialCapacity[0]
	}
	return &PhoneBook{hashtable: NewHashTable(cap)}
}

func trimCsv(s string) string {
	return strings.Trim(s, " \t\n\r\x0b\x0c")
}

// parseCsvLine splits one CSV line into (name, phone).
// Mirrors C++ parseCsvLine: quoted name with "" escapes, else split on LAST comma.
func parseCsvLine(line string) (bool, string, string) {
	l := trimCsv(line)
	if l == "" {
		return false, "", ""
	}
	var name, phone string
	if l[0] == '"' {
		var parsed strings.Builder
		i := 1
		closed := false
		for i < len(l) {
			if l[i] == '"' {
				if i+1 < len(l) && l[i+1] == '"' {
					parsed.WriteByte('"')
					i += 2
				} else {
					closed = true
					i++
					break
				}
			} else {
				parsed.WriteByte(l[i])
				i++
			}
		}
		if !closed {
			return false, "", ""
		}
		name = parsed.String()
		comma := strings.Index(l[i:], ",")
		if comma == -1 {
			return false, "", ""
		}
		comma += i
		phone = l[comma+1:]
	} else {
		comma := strings.LastIndex(l, ",")
		if comma == -1 {
			return false, "", ""
		}
		name = l[:comma]
		phone = l[comma+1:]
	}
	name = trimCsv(name)
	phone = trimCsv(phone)
	if len(phone) >= 2 && phone[0] == '"' && phone[len(phone)-1] == '"' {
		phone = trimCsv(phone[1 : len(phone)-1])
	}
	if name == "" && phone == "" {
		return false, "", ""
	}
	return true, name, phone
}

// IsAllDigits mirrors PhoneBook::isAllDigits (ASCII digits only).
func IsAllDigits(s string) bool {
	if s == "" {
		return false
	}
	for i := 0; i < len(s); i++ {
		if s[i] < '0' || s[i] > '9' {
			return false
		}
	}
	return true
}

// ToLower mirrors PhoneBook::toLower (ASCII only).
func ToLower(s string) string {
	b := []byte(s)
	for i, c := range b {
		if c >= 'A' && c <= 'Z' {
			b[i] = c + 32
		}
	}
	return string(b)
}

// CapitalizeFirst mirrors PhoneBook::capitalizeFirst.
func CapitalizeFirst(s string) string {
	b := []byte(s)
	newWord := true
	for i := 0; i < len(b); i++ {
		if b[i] == ' ' {
			newWord = true
		} else {
			if newWord && b[i] >= 'a' && b[i] <= 'z' {
				b[i] = b[i] - 32
			}
			newWord = false
		}
	}
	return string(b)
}

// InsertContact validates, normalizes the name, appends + indexes.
func (p *PhoneBook) InsertContact(name, phone string) bool {
	if name == "" || phone == "" {
		fmt.Println("Missing name or phone number")
		return false
	}
	if !IsAllDigits(phone) {
		fmt.Println("Invalid phone number")
		return false
	}
	if p.SearchHashByPhone(phone) != -1 {
		fmt.Println("Phone number is already exist")
		return false
	}
	normalized := CapitalizeFirst(ToLower(name))
	// add contact to slice + hashtable (O(1))
	tHash := TimeIt(func() {
		p.contacts = append(p.contacts, Contact{Name: normalized, Phone: phone})
		p.hashtable.HashInsert(phone, len(p.contacts)-1)
	})

	// keep the sorted index sorted: binary-search the spot (O(log n)) + shift (O(n))
	tIndex := TimeIt(func() {
		pos := lowerBound(p.sortedPhones, phone)
		p.sortedPhones = append(p.sortedPhones, "")
		copy(p.sortedPhones[pos+1:], p.sortedPhones[pos:])
		p.sortedPhones[pos] = phone
	})

	fmt.Printf("  hash insert: %vms, sorted-index insert: %vms (the cost of keeping binary search possible)\n", tHash, tIndex)
	return true
}

// DeleteContactByPhone erases + rebuilds the hash index. O(n), preserves order.
func (p *PhoneBook) DeleteContactByPhone(phone string) bool {
	idx := p.SearchHashByPhone(phone)
	if idx == -1 {
		fmt.Println("Phone number not found")
		return false
	}
	p.contacts = append(p.contacts[:idx], p.contacts[idx+1:]...)
	p.hashtable.Clear()
	for i, c := range p.contacts {
		p.hashtable.HashInsert(c.Phone, i)
	}
	p.BuildSortedIndex() // delete is O(n) anyway; rebuild the sorted index
	return true
}

// SearchLinearByPhone linear scan by phone.
func (p *PhoneBook) SearchLinearByPhone(phone string) int {
	for i, c := range p.contacts {
		if c.Phone == phone {
			return i
		}
	}
	return -1
}

// SearchHashByPhone hash lookup by phone.
func (p *PhoneBook) SearchHashByPhone(phone string) int {
	return p.hashtable.HashSearch(phone)
}

// SearchBinaryByPhone binary search by phone on the sorted index (O(log n)).
// Returns the contact index via the hash table, or -1 if not found.
func (p *PhoneBook) SearchBinaryByPhone(phone string) int {
	lo, hi := 0, len(p.sortedPhones)
	for lo < hi {
		mid := lo + (hi-lo)/2
		if p.sortedPhones[mid] == phone {
			return p.hashtable.HashSearch(phone)
		}
		if p.sortedPhones[mid] < phone {
			lo = mid + 1
		} else {
			hi = mid
		}
	}
	return -1
}

// SearchLinearByName case-insensitive linear scan by name.
func (p *PhoneBook) SearchLinearByName(name string) int {
	target := ToLower(name)
	for i, c := range p.contacts {
		if ToLower(c.Name) == target {
			return i
		}
	}
	return -1
}

// SearchPrefixByPhone prefix search using lower_bound + linear for first k results.
func (p *PhoneBook) SearchPrefixByPhone(phone string, k int) []int {
	result := []int{}
	pos := lowerBound(p.sortedPhones, phone)
	for i := pos; i < pos+k && i < len(p.sortedPhones); i++ {
		// stop as soon as the sorted phone no longer starts with the prefix
		if !strings.HasPrefix(p.sortedPhones[i], phone) {
			break
		}
		index := p.hashtable.HashSearch(p.sortedPhones[i])
		if index != -1 {
			result = append(result, index)
		}
	}
	return result
}

// PrintAll prints "i. Name - Phone" per line.
func (p *PhoneBook) PrintAll() {
	for i, c := range p.contacts {
		fmt.Printf("%d. %s - %s\n", i, c.Name, c.Phone)
	}
}

// PrintContact prints one contact; false if index invalid.
func (p *PhoneBook) PrintContact(index int) bool {
	if index < 0 || index >= len(p.contacts) {
		return false
	}
	fmt.Printf("Name: %s\n", p.contacts[index].Name)
	fmt.Printf("Phone: %s\n", p.contacts[index].Phone)
	return true
}

// Size returns the number of contacts.
func (p *PhoneBook) Size() int {
	return len(p.contacts)
}

// GetPhoneAt returns the phone at index or "" if invalid.
func (p *PhoneBook) GetPhoneAt(index int) string {
	if index < 0 || index >= len(p.contacts) {
		return ""
	}
	return p.contacts[index].Phone
}

// LoadFromCSV loads contacts; -1 if the file cannot be opened.
func (p *PhoneBook) LoadFromCSV(path string) int {
	f, err := os.Open(path)
	if err != nil {
		return -1
	}
	defer f.Close()
	count := 0
	sc := bufio.NewScanner(f)
	// Allow long lines (names are short, but be safe).
	sc.Buffer(make([]byte, 64*1024), 1024*1024)
	for sc.Scan() {
		line := sc.Text()
		if strings.HasSuffix(line, "\r") {
			line = line[:len(line)-1]
		}
		if trimCsv(line) == "" {
			continue
		}
		ok, name, phone := parseCsvLine(line)
		if !ok {
			continue
		}
		if name == "" || !IsAllDigits(phone) {
			continue
		}
		if p.SearchHashByPhone(phone) != -1 {
			continue
		}
		// Push directly (no per-row index maintenance — that would be O(n^2));
		// the sorted index is built once below.
		normalized := CapitalizeFirst(ToLower(name))
		p.contacts = append(p.contacts, Contact{Name: normalized, Phone: phone})
		p.hashtable.HashInsert(phone, len(p.contacts)-1)
		count++
	}
	p.BuildSortedIndex()
	return count
}

// SaveToCSV writes contacts; false if the file cannot be opened.
func (p *PhoneBook) SaveToCSV(path string) bool {
	f, err := os.Create(path)
	if err != nil {
		return false
	}
	defer f.Close()
	w := bufio.NewWriter(f)
	defer w.Flush()
	for _, c := range p.contacts {
		needsQuote := strings.Contains(c.Name, ",") || strings.Contains(c.Name, "\"")
		if needsQuote {
			w.WriteByte('"')
			for i := 0; i < len(c.Name); i++ {
				if c.Name[i] == '"' {
					w.WriteString("\"\"")
				} else {
					w.WriteByte(c.Name[i])
				}
			}
			w.WriteByte('"')
		} else {
			w.WriteString(c.Name)
		}
		w.WriteString("," + c.Phone + "\n")
	}
	return true
}
