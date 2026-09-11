package main

import (
	"bufio"
	"fmt"
	"io"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"time"
)

// Faithful port of src/main.cpp.

func runSearchBenchmark(phonebook *PhoneBook, csvInput string) {
	if phonebook.Size() == 0 {
		fmt.Printf("Phonebook empty, auto-loading from: %s\n", csvInput)
		loaded := phonebook.LoadFromCSV(csvInput)
		if loaded == -1 {
			fmt.Println("Cannot open file.")
			return
		}
		if phonebook.Size() == 0 {
			fmt.Println("No contacts to benchmark. Load contacts first (option 1).")
			return
		}
		fmt.Printf("Auto-loaded %d contacts.\n", loaded)
	}

	n := phonebook.Size()
	labels := [3]string{"first (linear best case)", "random (linear average case)", "last (linear worst case)"}
	indices := [3]int{0, 0, n - 1}
	indices[0] = 0
	indices[2] = n - 1
	if n <= 2 {
		indices[1] = n - 1
	} else {
		r := rand.New(rand.NewSource(time.Now().UnixNano()))
		indices[1] = r.Intn(n)
	}

	fmt.Printf("\nBenchmarking phone search (linear cases: first=best / random=average / last=worst; hash is ~O(1) in all cases) (%d contacts, 5 runs each, best reported).\n", n)

	for k := 0; k < 3; k++ {
		targetIdx := indices[k]
		phone := phonebook.GetPhoneAt(targetIdx)
		if phone == "" {
			fmt.Printf("[%s] index %d: cannot pick target.\n", labels[k], targetIdx)
			continue
		}
		idxLin := -1
		idxHash := -1
		linBest := Benchmark(func() { idxLin = phonebook.SearchLinearByPhone(phone) })
		hashBest := Benchmark(func() { idxHash = phonebook.SearchHashByPhone(phone) })
		fmt.Printf("[%s index %d phone %s]\n", labels[k], targetIdx, phone)
		fmt.Printf("  Linear best of 5: %vms. (index %d)\n", linBest, idxLin)
		fmt.Printf("  Hash best of 5: %vms. (index %d, position-independent)\n", hashBest, idxHash)
	}
	fmt.Println("Linear: first=best, last=worst. Hash: ~constant regardless of position.")
}

func printMenu() {
	fmt.Println("")
	fmt.Println("========================================")
	fmt.Println("             PHONE BOOK CLI")
	fmt.Println("========================================")
	fmt.Println("0. Benchmark phone search (5 runs, best)")
	fmt.Println("1. Load contacts from CSV")
	fmt.Println("2. Save contacts to CSV")
	fmt.Println("3. Insert contact")
	fmt.Println("4. Search contact")
	fmt.Println("5. Print all contacts")
	fmt.Println("6. Print contact by index")
	fmt.Println("7. Show number of contacts")
	fmt.Println("8. Exit")
	fmt.Println("========================================")
	fmt.Print("Enter your choice: ")
}

// readChoiceLine reads a full line; ok=false on EOF.
func readChoiceLine(r *bufio.Reader) (string, bool) {
	line, err := r.ReadString('\n')
	if err != nil {
		if err == io.EOF {
			// If some text precedes EOF, treat it as a line; else signal EOF.
			if len(line) == 0 {
				return "", false
			}
			return line, true
		}
		return "", false
	}
	return line, true
}

// readRawLine preserves leading/trailing spaces (strips only trailing \r\n).
func readRawLine(r *bufio.Reader) (string, bool) {
	line, err := r.ReadString('\n')
	if err != nil && err != io.EOF {
		return "", false
	}
	if err == io.EOF && len(line) == 0 {
		return "", false
	}
	line = strings.TrimSuffix(line, "\n")
	line = strings.TrimSuffix(line, "\r")
	return line, true
}

func main() {
	csvInput := "data/contacts_100k.csv"
	csvOutput := ""
	if len(os.Args) > 1 {
		csvInput = os.Args[1]
	}
	if len(os.Args) > 2 {
		csvOutput = os.Args[2]
	} else {
		csvOutput = csvInput
	}
	phonebook := NewPhoneBook()
	reader := bufio.NewReader(os.Stdin)

	for {
		printMenu()
		line, ok := readChoiceLine(reader)
		if !ok {
			fmt.Println("\nGoodbye")
			return
		}
		trimmed := strings.TrimSpace(line)
		// Mirror `cin >> choice`: first whitespace-separated token must be an int.
		fields := strings.Fields(trimmed)
		if len(fields) == 0 {
			fmt.Println("Invalid input.")
			continue
		}
		choice, err := strconv.Atoi(fields[0])
		if err != nil {
			fmt.Println("Invalid input.")
			continue
		}
		// If token has trailing garbage like "3abc", Atoi fails -> Invalid input (close enough).

		switch choice {
		case 0:
			runSearchBenchmark(phonebook, csvInput)
		case 1:
			fmt.Printf("Loading contacts from: %s\n", csvInput)
			result := 0
			PrintTaskDuration(func() { result = phonebook.LoadFromCSV(csvInput) })
			if result == -1 {
				fmt.Println("Cannot open file.")
			} else {
				fmt.Printf("Successfully loaded %d contacts.\n", result)
			}
		case 2:
			fmt.Printf("\nSaving contacts to: %s\n", csvOutput)
			result := false
			PrintTaskDuration(func() { result = phonebook.SaveToCSV(csvOutput) })
			if result {
				fmt.Println("Contacts saved successfully.")
			} else {
				fmt.Println("Cannot open output file.")
			}
		case 3:
			fmt.Print("\nEnter name: ")
			name, ok1 := readRawLine(reader)
			if !ok1 {
				fmt.Println("\nGoodbye")
				return
			}
			fmt.Print("Enter phone: ")
			phone, ok2 := readRawLine(reader)
			if !ok2 {
				fmt.Println("\nGoodbye")
				return
			}
			result := false
			PrintTaskDuration(func() { result = phonebook.InsertContact(name, phone) })
			if result {
				fmt.Println("Contact inserted successfully.")
			} else {
				fmt.Println("Failed to insert contact.")
			}
		case 4:
			fmt.Println("")
			fmt.Println("========== Search ==========")
			fmt.Println("1. Search phone - Linear Search")
			fmt.Println("2. Search phone - Hash Search")
			fmt.Println("3. Search name - Linear Search")
			fmt.Println("4. Back")
			fmt.Println("============================")
			fmt.Print("Enter your choice: ")
			sline, ok := readChoiceLine(reader)
			if !ok {
				fmt.Println("\nGoodbye")
				return
			}
			sfields := strings.Fields(strings.TrimSpace(sline))
			if len(sfields) == 0 {
				fmt.Println("Invalid search choice.")
				continue
			}
			searchChoice, err := strconv.Atoi(sfields[0])
			if err != nil {
				fmt.Println("Invalid search choice.")
				continue
			}
			if searchChoice == 1 {
				fmt.Print("Enter phone number: ")
				phone, ok := readRawLine(reader)
				if !ok {
					fmt.Println("\nGoodbye")
					return
				}
				index := -1
				PrintTaskDuration(func() { index = phonebook.SearchLinearByPhone(phone) })
				if index == -1 {
					fmt.Println("Phone number not found.")
				} else {
					fmt.Println("Phone number found.")
					fmt.Printf("Contact index: %d\n", index)
					phonebook.PrintContact(index)
				}
			} else if searchChoice == 2 {
				fmt.Print("Enter phone number: ")
				phone, ok := readRawLine(reader)
				if !ok {
					fmt.Println("\nGoodbye")
					return
				}
				index := -1
				PrintTaskDuration(func() { index = phonebook.SearchHashByPhone(phone) })
				if index == -1 {
					fmt.Println("Phone number not found.")
				} else {
					fmt.Println("Phone number found using Hash Table.")
					fmt.Printf("Contact index: %d\n", index)
					phonebook.PrintContact(index)
				}
			} else if searchChoice == 3 {
				fmt.Print("Enter name: ")
				name, ok := readRawLine(reader)
				if !ok {
					fmt.Println("\nGoodbye")
					return
				}
				index := -1
				PrintTaskDuration(func() { index = phonebook.SearchLinearByName(name) })
				if index == -1 {
					fmt.Println("Name not found.")
				} else {
					fmt.Println("Name found.")
					fmt.Printf("Contact index: %d\n", index)
					phonebook.PrintContact(index)
				}
			} else if searchChoice == 4 {
				fmt.Println("Back to main menu.")
			} else {
				fmt.Println("Invalid search choice.")
			}
		case 5:
			fmt.Println("")
			fmt.Println("========== Contacts ==========")
			PrintTaskDuration(func() { phonebook.PrintAll() })
			fmt.Println("==============================")
		case 6:
			fmt.Print("\nEnter contact index: ")
			iline, ok := readChoiceLine(reader)
			if !ok {
				fmt.Println("\nGoodbye")
				return
			}
			ifields := strings.Fields(strings.TrimSpace(iline))
			if len(ifields) == 0 {
				fmt.Println("Invalid index.")
				continue
			}
			index, err := strconv.Atoi(ifields[0])
			if err != nil || index < 0 {
				// C++ `size_t` parse of "-1"/garbage fails or wraps huge -> invalid.
				fmt.Println("Invalid index.")
				continue
			}
			result := false
			PrintTaskDuration(func() { result = phonebook.PrintContact(index) })
			if !result {
				fmt.Println("Invalid contact index.")
			}
		case 7:
			fmt.Printf("\nNumber of contacts: %d\n", phonebook.Size())
		case 8:
			fmt.Println("Goodbye")
			return
		default:
			fmt.Println("Invalid choice. Please choose from 0 to 8.")
		}
	}
}
