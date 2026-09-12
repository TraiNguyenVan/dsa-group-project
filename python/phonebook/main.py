"""Faithful port of src/main.cpp - Phone Book CLI."""
import csv
import os
import sys
from datetime import datetime, timezone

try:
    from phonebook import PhoneBook
    from timer import benchmark, print_task_duration, time_it
except ImportError:  # package-style run
    from python.phonebook.phonebook import PhoneBook
    from python.phonebook.timer import benchmark, print_task_duration, time_it


HEADER = "language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone"


def run_search_benchmark_batch(phonebook: PhoneBook, csv_input: str,
                               out_csv: str, append: bool) -> int:
    """Batch benchmark: every run to CSV (60 rows)."""
    loaded = phonebook.loadfrom_csv(csv_input)
    if loaded == -1:
        print(f"Cannot open file: {csv_input}", file=sys.stderr)
        return 1
    n = phonebook.size()
    if n == 0:
        print("No contacts to benchmark.", file=sys.stderr)
        return 1
    cases = ["first", "middle", "last", "miss"]
    indices = [0, n // 2, n - 1, -1]
    indices[0] = 0
    indices[1] = n // 2
    indices[2] = n - 1
    indices[3] = -1

    need_header = True
    if append and os.path.exists(out_csv) and os.path.getsize(out_csv) > 0:
        with open(out_csv, "r", encoding="utf-8") as f:
            first = f.readline().strip()
            need_header = not first
    timestamp = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    toolchain = f"python3 {sys.version.split()[0]}"
    try:
        f = open(out_csv, "a" if append else "w", encoding="utf-8", newline="")
    except OSError:
        print(f"Cannot open output file: {out_csv}", file=sys.stderr)
        return 1
    with f:
        w = csv.writer(f)
        if need_header:
            f.write(HEADER + "\n")
        for k in range(4):
            target_idx = indices[k]
            if k == 3:
                phone = "0000000000"
            else:
                phone = phonebook.get_phone_at(target_idx)
                if not phone:
                    continue
            for algo in ("linear", "hash", "binary"):
                for r in range(1, 6):
                    if algo == "linear":
                        t = time_it(lambda: phonebook.search_linear_by_phone(phone))
                    elif algo == "hash":
                        t = time_it(lambda: phonebook.search_hash_by_phone(phone))
                    else:
                        t = time_it(lambda: phonebook.search_binary_by_phone(phone))
                    w.writerow(["python", csv_input, n, cases[k], algo, r, repr(t),
                                timestamp, toolchain, target_idx, phone])
    print(f"Wrote 60 rows -> {out_csv} ({n} contacts).")
    return 0


def run_search_benchmark(phonebook: PhoneBook, csv_input: str) -> None:
    if phonebook.size() == 0:
        print(f"Phonebook empty, auto-loading from: {csv_input}")
        loaded = phonebook.loadfrom_csv(csv_input)
        if loaded == -1:
            print("Cannot open file.")
            return
        if phonebook.size() == 0:
            print("No contacts to benchmark. Load contacts first (option 1).")
            return
        print(f"Auto-loaded {loaded} contacts.")

    n = phonebook.size()
    labels = [
        "first (linear best case)",
        "middle (linear avg / binary best)",
        "last (linear worst case)",
        "miss (linear worst / hash & binary worst)",
    ]
    indices = [0, n // 2, n - 1, -1]
    indices[0] = 0
    indices[1] = n // 2
    indices[2] = n - 1
    indices[3] = -1

    print(
        "\nBenchmarking phone search (linear: first=best / middle / last,miss=worst; "
        "hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary)"
        f" ({n} contacts, 5 runs each, best reported)."
    )

    for k in range(4):
        target_idx = indices[k]
        if k == 3:
            phone = "0000000000"
        else:
            phone = phonebook.get_phone_at(target_idx)
            if not phone:
                print(f"[{labels[k]}] index {target_idx}: cannot pick target.")
                continue
        holder = {"lin": -1, "h": -1, "b": -1}

        def do_lin(p=phone):
            holder["lin"] = phonebook.search_linear_by_phone(p)

        def do_hash(p=phone):
            holder["h"] = phonebook.search_hash_by_phone(p)

        def do_bin(p=phone):
            holder["b"] = phonebook.search_binary_by_phone(p)

        lin_best = benchmark(do_lin)
        hash_best = benchmark(do_hash)
        bin_best = benchmark(do_bin)
        print(f"[{labels[k]} index {target_idx} phone {phone}]")
        print(f"  Linear best of 5: {lin_best}ms. (index {holder['lin']})")
        print(f"  Hash best of 5: {hash_best}ms. (index {holder['h']}, position-independent)")
        print(f"  Binary best of 5: {bin_best}ms. (index {holder['b']}, sorted index, position-independent)")
    print("Linear: first=best, last/miss=worst. Hash/binary: ~constant; miss is worst (full chain / log n probes).")


def print_menu() -> None:
    print("")
    print("========================================")
    print("             PHONE BOOK CLI")
    print("========================================")
    print("0. Benchmark phone search (5 runs, best)")
    print("1. Load contacts from CSV")
    print("2. Save contacts to CSV")
    print("3. Insert contact")
    print("4. Search contact")
    print("5. Print all contacts")
    print("6. Print contact by index")
    print("7. Show number of contacts")
    print("8. Delete contact by phone")
    print("9. Exit")
    print("========================================")
    print("Enter your choice: ", end="")


def _read_choice_line() -> str:
    try:
        return input()
    except EOFError:
        raise
    except KeyboardInterrupt:
        raise EOFError


def main() -> int:
    bench_csv = ""
    bench_append = False
    positionals = []
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--benchmark-csv" and i + 1 < len(args):
            bench_csv = args[i + 1]
            i += 2
        elif args[i] == "--append":
            bench_append = True
            i += 1
        else:
            positionals.append(args[i])
            i += 1
    csv_input = positionals[0] if len(positionals) > 0 else "data/contacts_100k.csv"
    csv_output = positionals[1] if len(positionals) > 1 else csv_input
    phonebook = PhoneBook()

    if bench_csv:
        return run_search_benchmark_batch(phonebook, csv_input, bench_csv, bench_append)

    while True:
        print_menu()
        try:
            raw = _read_choice_line()
        except EOFError:
            print("\nGoodbye")
            return 0
        raw = raw.strip()
        try:
            choice = int(raw)
        except ValueError:
            print("Invalid input.")
            continue

        if choice == 0:
            run_search_benchmark(phonebook, csv_input)
        elif choice == 1:
            print(f"Loading contacts from: {csv_input}")
            result = {}

            def work():
                result["v"] = phonebook.loadfrom_csv(csv_input)

            print_task_duration(work)
            if result["v"] == -1:
                print("Cannot open file.")
            else:
                print(f"Successfully loaded {result['v']} contacts.")
        elif choice == 2:
            print(f"\nSaving contacts to: {csv_output}")
            result = {}

            def work2():
                result["v"] = phonebook.saveto_csv(csv_output)

            print_task_duration(work2)
            if result["v"]:
                print("Contacts saved successfully.")
            else:
                print("Cannot open output file.")
        elif choice == 3:
            try:
                print("\nEnter name: ", end="")
                name = input()
                print("Enter phone: ", end="")
                phone = input()
            except EOFError:
                print("\nGoodbye")
                return 0
            result = {}

            def work3():
                result["v"] = phonebook.insert_contact(name, phone)

            print_task_duration(work3)
            if result["v"]:
                print("Contact inserted successfully.")
            else:
                print("Failed to insert contact.")
        elif choice == 4:
            print("")
            print("========== Search ==========")
            print("1. Search phone - Linear Search")
            print("2. Search phone - Hash Search")
            print("3. Search phone - Binary Search (sorted index)")
            print("4. Search name - Linear Search")
            print("5. Back")
            print("============================")
            print("Enter your choice: ", end="")
            try:
                sraw = input()
            except EOFError:
                print("\nGoodbye")
                return 0
            try:
                search_choice = int(sraw.strip())
            except ValueError:
                print("Invalid search choice.")
                continue
            if search_choice == 1:
                try:
                    print("Enter phone number: ", end="")
                    phone = input()
                except EOFError:
                    print("\nGoodbye")
                    return 0
                result = {}

                def w1():
                    result["v"] = phonebook.search_linear_by_phone(phone)

                print_task_duration(w1)
                if result["v"] == -1:
                    print("Phone number not found.")
                else:
                    print("Phone number found.")
                    print(f"Contact index: {result['v']}")
                    phonebook.print_contact(result["v"])
            elif search_choice == 2:
                try:
                    print("Enter phone number: ", end="")
                    phone = input()
                except EOFError:
                    print("\nGoodbye")
                    return 0
                result = {}

                def w2():
                    result["v"] = phonebook.search_hash_by_phone(phone)

                print_task_duration(w2)
                if result["v"] == -1:
                    print("Phone number not found.")
                else:
                    print("Phone number found using Hash Table.")
                    print(f"Contact index: {result['v']}")
                    phonebook.print_contact(result["v"])
            elif search_choice == 3:
                try:
                    print("Enter phone number: ", end="")
                    phone = input()
                except EOFError:
                    print("\nGoodbye")
                    return 0
                result = {}

                def wbin():
                    result["v"] = phonebook.search_binary_by_phone(phone)

                print_task_duration(wbin)
                if result["v"] == -1:
                    print("Phone number not found.")
                else:
                    print("Phone number found using Binary Search (sorted index).")
                    print(f"Contact index: {result['v']}")
                    phonebook.print_contact(result["v"])
            elif search_choice == 4:
                try:
                    print("Enter name: ", end="")
                    name = input()
                except EOFError:
                    print("\nGoodbye")
                    return 0
                result = {}

                def w3():
                    result["v"] = phonebook.search_linear_by_name(name)

                print_task_duration(w3)
                if result["v"] == -1:
                    print("Name not found.")
                else:
                    print("Name found.")
                    print(f"Contact index: {result['v']}")
                    phonebook.print_contact(result["v"])
            elif search_choice == 5:
                print("Back to main menu.")
            else:
                print("Invalid search choice.")
        elif choice == 5:
            print("")
            print("========== Contacts ==========")

            def work5():
                phonebook.print_all()

            print_task_duration(work5)
            print("==============================")
        elif choice == 6:
            print("\nEnter contact index: ", end="")
            try:
                iraw = input()
            except EOFError:
                print("\nGoodbye")
                return 0
            try:
                index = int(iraw.strip())
            except ValueError:
                print("Invalid index.")
                continue
            result = {}

            def work6():
                result["v"] = phonebook.print_contact(index)

            print_task_duration(work6)
            if not result["v"]:
                print("Invalid contact index.")
        elif choice == 7:
            print(f"\nNumber of contacts: {phonebook.size()}")
        elif choice == 8:
            try:
                print("\nEnter phone number to delete: ", end="")
                phone = input()
            except EOFError:
                print("\nGoodbye")
                return 0
            result = {}

            def work8():
                result["v"] = phonebook.delete_contact_by_phone(phone)

            print_task_duration(work8)
            if result["v"]:
                print("Contact deleted successfully.")
            else:
                print("Failed to delete contact.")
        elif choice == 9:
            print("Goodbye")
            return 0
        else:
            print("Invalid choice. Please choose from 0 to 9.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
