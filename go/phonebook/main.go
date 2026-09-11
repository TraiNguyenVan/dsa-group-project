package main

import (
	"bufio"
	"encoding/csv"
	"fmt"
	"io"
	"math/rand"
	"os"
	"runtime"
	"strconv"
	"strings"
	"time"
)

const benchHeader = "language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone"

// runSearchBenchmarkBatch writes EVERY run (5 cases x 3 algos x 5 = 75 rows)
// to CSV. Seeded RNG (42) for a reproducible `random` target. `miss` uses
// phone "0000000000" (not in dataset) for true worst case of hash/binary.
func runSearchBenchmarkBatch(phonebook *PhoneBook, csvInput, outCsv string, append bool) int {
	loaded := phonebook.LoadFromCSV(csvInput)
	if loaded == -1 {
		fmt.Fprintf(os.Stderr, "Cannot open file: %s\n", csvInput)
		return 1
	}
	n := phonebook.Size()
	if n == 0 {
		fmt.Fprintln(os.Stderr, "No contacts to benchmark.")
		return 1
	}
	cases := [5]string{"first", "middle", "random", "last", "miss"}
	indices := [5]int{0, n / 2, n - 1, n - 1, -1}
	indices[0] = 0
	indices[1] = n / 2
	indices[3] = n - 1
	indices[4] = -1
	if n > 2 {
		indices[2] = rand.New(rand.NewSource(42)).Intn(n)
	}

	needHeader := true
	if append {
		if fi, err := os.Stat(outCsv); err == nil && fi.Size() > 0 {
			f, err := os.Open(outCsv)
			if err == nil {
				r := bufio.NewReader(f)
				line, _ := r.ReadString('\n')
				f.Close()
				if strings.TrimSpace(line) != "" {
					needHeader = false
				}
			}
		}
	}
	var f *os.File
	var err error
	if append {
		f, err = os.OpenFile(outCsv, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
	} else {
		f, err = os.OpenFile(outCsv, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, 0644)
	}
	if err != nil {
		fmt.Fprintf(os.Stderr, "Cannot open output file: %s\n", outCsv)
		return 1
	}
	defer f.Close()
	w := csv.NewWriter(f)
	defer w.Flush()
	if needHeader {
		fmt.Fprintln(f, benchHeader)
	}
	timestamp := time.Now().UTC().Format("2006-01-02T15:04:05Z")
	toolchain := "go " + runtime.Version()
	for k := 0; k < 5; k++ {
		var phone string
		if k == 4 {
			phone = "0000000000"
		} else {
			phone = phonebook.GetPhoneAt(indices[k])
			if phone == "" {
				continue
			}
		}
		for a := 0; a < 3; a++ {
			algo := "linear"
			if a == 1 {
				algo = "hash"
			} else if a == 2 {
				algo = "binary"
			}
			for r := 1; r <= 5; r++ {
				var t float64
				if a == 0 {
					t = TimeIt(func() { phonebook.SearchLinearByPhone(phone) })
				} else if a == 1 {
					t = TimeIt(func() { phonebook.SearchHashByPhone(phone) })
				} else {
					t = TimeIt(func() { phonebook.SearchBinaryByPhone(phone) })
				}
				w.Write([]string{"go", csvInput, strconv.Itoa(n), cases[k], algo,
					strconv.Itoa(r), strconv.FormatFloat(t, 'g', -1, 64),
					timestamp, toolchain, strconv.Itoa(indices[k]), phone})
			}
		}
	}
	w.Flush()
	if err := w.Error(); err != nil {
		fmt.Fprintf(os.Stderr, "Write failed: %v\n", err)
		return 1
	}
	fmt.Printf("Wrote 75 rows -> %s (%d contacts).\n", outCsv, n)
	return 0
}

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
	labels := [5]string{"first (linear best case)", "middle (linear avg / binary best)", "random (linear average case)", "last (linear worst case)", "miss (linear worst / hash & binary worst)"}
	indices := [5]int{0, n / 2, 0, n - 1, -1}
	indices[0] = 0
	indices[1] = n / 2
	indices[3] = n - 1
	indices[4] = -1
	if n <= 2 {
		indices[2] = n - 1
	} else {
		r := rand.New(rand.NewSource(time.Now().UnixNano()))
		indices[2] = r.Intn(n)
	}

	fmt.Printf("\nBenchmarking phone search (linear: first=best / middle,random=avg / last,miss=worst; hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary) (%d contacts, 5 runs each, best reported).\n", n)

	for k := 0; k < 5; k++ {
		targetIdx := indices[k]
		var phone string
		if k == 4 {
			phone = "0000000000"
		} else {
			phone = phonebook.GetPhoneAt(targetIdx)
			if phone == "" {
				fmt.Printf("[%s] index %d: cannot pick target.\n", labels[k], targetIdx)
				continue
			}
		}
		idxLin := -1
		idxHash := -1
		idxBin := -1
		linBest := Benchmark(func() { idxLin = phonebook.SearchLinearByPhone(phone) })
		hashBest := Benchmark(func() { idxHash = phonebook.SearchHashByPhone(phone) })
		binBest := Benchmark(func() { idxBin = phonebook.SearchBinaryByPhone(phone) })
		fmt.Printf("[%s index %d phone %s]\n", labels[k], targetIdx, phone)
		fmt.Printf("  Linear best of 5: %vms. (index %d)\n", linBest, idxLin)
		fmt.Printf("  Hash best of 5: %vms. (index %d, position-independent)\n", hashBest, idxHash)
		fmt.Printf("  Binary best of 5: %vms. (index %d, sorted index, position-independent)\n", binBest, idxBin)
	}
	fmt.Println("Linear: first=best, last/miss=worst. Hash/binary: ~constant; miss is worst (full chain / log n probes).")
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
	fmt.Println("8. Delete contact by phone")
	fmt.Println("9. Exit")
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
	benchCsv := ""
	benchAppend := false
	var positionals []string
	args := os.Args[1:]
	for i := 0; i < len(args); i++ {
		if args[i] == "--benchmark-csv" && i+1 < len(args) {
			benchCsv = args[i+1]
			i++
		} else if args[i] == "--append" {
			benchAppend = true
		} else {
			positionals = append(positionals, args[i])
		}
	}
	if len(positionals) > 0 {
		csvInput = positionals[0]
	}
	if len(positionals) > 1 {
		csvOutput = positionals[1]
	} else {
		csvOutput = csvInput
	}
	phonebook := NewPhoneBook()

	if benchCsv != "" {
		os.Exit(runSearchBenchmarkBatch(phonebook, csvInput, benchCsv, benchAppend))
	}
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
			fmt.Println("3. Search phone - Binary Search (sorted index)")
			fmt.Println("4. Search name - Linear Search")
			fmt.Println("5. Back")
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
				fmt.Print("Enter phone number: ")
				phone, ok := readRawLine(reader)
				if !ok {
					fmt.Println("\nGoodbye")
					return
				}
				index := -1
				PrintTaskDuration(func() { index = phonebook.SearchBinaryByPhone(phone) })
				if index == -1 {
					fmt.Println("Phone number not found.")
				} else {
					fmt.Println("Phone number found using Binary Search (sorted index).")
					fmt.Printf("Contact index: %d\n", index)
					phonebook.PrintContact(index)
				}
			} else if searchChoice == 4 {
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
			} else if searchChoice == 5 {
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
			fmt.Print("\nEnter phone number to delete: ")
			phone, ok := readRawLine(reader)
			if !ok {
				fmt.Println("\nGoodbye")
				return
			}
			result := false
			PrintTaskDuration(func() { result = phonebook.DeleteContactByPhone(phone) })
			if result {
				fmt.Println("Contact deleted successfully.")
			} else {
				fmt.Println("Failed to delete contact.")
			}
		case 9:
			fmt.Println("Goodbye")
			return
		default:
			fmt.Println("Invalid choice. Please choose from 0 to 9.")
		}
	}
}
