"""Faithful port of src/main.cpp - Phone Book CLI."""
import random
import sys

try:
    from phonebook import PhoneBook
    from timer import benchmark, print_task_duration
except ImportError:  # package-style run
    from python.phonebook.phonebook import PhoneBook
    from python.phonebook.timer import benchmark, print_task_duration


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
        "random (linear average case)",
        "last (linear worst case)",
    ]
    indices = [0, 0, n - 1]
    indices[0] = 0
    indices[2] = n - 1
    if n <= 2:
        indices[1] = n - 1
    else:
        indices[1] = random.randint(0, n - 1)

    print(
        "\nBenchmarking phone search (linear cases: first=best / random=average / last=worst; "
        "hash is ~O(1) in all cases)"
        f" ({n} contacts, 5 runs each, best reported)."
    )

    for k in range(3):
        target_idx = indices[k]
        phone = phonebook.get_phone_at(target_idx)
        if not phone:
            print(f"[{labels[k]}] index {target_idx}: cannot pick target.")
            continue
        holder = {"lin": -1, "h": -1}

        def do_lin(p=phone):
            holder["lin"] = phonebook.search_linear_by_phone(p)

        def do_hash(p=phone):
            holder["h"] = phonebook.search_hash_by_phone(p)

        lin_best = benchmark(do_lin)
        hash_best = benchmark(do_hash)
        print(f"[{labels[k]} index {target_idx} phone {phone}]")
        print(f"  Linear best of 5: {lin_best}ms. (index {holder['lin']})")
        print(f"  Hash best of 5: {hash_best}ms. (index {holder['h']}, position-independent)")
    print("Linear: first=best, last=worst. Hash: ~constant regardless of position.")


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
    print("8. Exit")
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
    csv_input = sys.argv[1] if len(sys.argv) > 1 else "data/contacts_100k.csv"
    csv_output = sys.argv[2] if len(sys.argv) > 2 else csv_input
    phonebook = PhoneBook()

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
            print("3. Search name - Linear Search")
            print("4. Back")
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
            elif search_choice == 4:
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
            print("Goodbye")
            return 0
        else:
            print("Invalid choice. Please choose from 0 to 8.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
