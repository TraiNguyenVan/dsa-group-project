package com.phonebook;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.time.Instant;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicReference;

// Faithful port of src/main.cpp - Phone Book CLI.
public class Main {

    static void runSearchBenchmark(PhoneBook phonebook, String csvInput) {
        if (phonebook.size() == 0) {
            System.out.println("Phonebook empty, auto-loading from: " + csvInput);
            int loaded = phonebook.loadfromCSV(csvInput);
            if (loaded == -1) {
                System.out.println("Cannot open file.");
                return;
            }
            if (phonebook.size() == 0) {
                System.out.println("No contacts to benchmark. Load contacts first (option 1).");
                return;
            }
            System.out.println("Auto-loaded " + loaded + " contacts.");
        }

        int n = phonebook.size();
        String[] labels = {
            "first (linear best case)",
            "middle (linear avg / binary best)",
            "last (linear worst case)",
            "miss (linear worst / hash & binary worst)"
        };
        int[] indices = new int[4];
        indices[0] = 0;
        indices[1] = n / 2;
        indices[2] = n - 1;
        indices[3] = -1;

        System.out.println("\nBenchmarking phone search (linear: first=best / middle / last,miss=worst; "
                + "hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary)"
                + " (" + n + " contacts, 5 runs each, best reported).");

        for (int k = 0; k < 4; k++) {
            int targetIdx = indices[k];
            String phone;
            if (k == 3) {
                phone = "0000000000";
            } else {
                phone = phonebook.getPhoneAt(targetIdx);
                if (phone.isEmpty()) {
                    System.out.println("[" + labels[k] + "] index " + targetIdx + ": cannot pick target.");
                    continue;
                }
            }
            final int[] idxLin = {-1};
            final int[] idxHash = {-1};
            final int[] idxBin = {-1};
            final String fPhone = phone;
            double linBest = Timer.benchmark(() -> idxLin[0] = phonebook.searchLinearByPhone(fPhone));
            double hashBest = Timer.benchmark(() -> idxHash[0] = phonebook.searchHashByPhone(fPhone));
            double binBest = Timer.benchmark(() -> idxBin[0] = phonebook.searchBinaryByPhone(fPhone));

            System.out.println("[" + labels[k] + " index " + targetIdx + " phone " + phone + "]");
            System.out.println("  Linear best of 5: " + linBest + "ms. (index " + idxLin[0] + ")");
            System.out.println("  Hash best of 5: " + hashBest + "ms. (index " + idxHash[0] + ", position-independent)");
            System.out.println("  Binary best of 5: " + binBest + "ms. (index " + idxBin[0] + ", sorted index, position-independent)");
        }
        System.out.println("Linear: first=best, last/miss=worst. Hash/binary: ~constant; miss is worst (full chain / log n probes).");
    }

    static String csvEscape(String v) {
        if (v.contains(",") || v.contains("\"") || v.contains("\n")) {
            return "\"" + v.replace("\"", "\"\"") + "\"";
        }
        return v;
    }

    // Batch benchmark: every run to CSV (60 rows). `miss` uses phone
    // "0000000000" (not in dataset) for true worst case of hash/binary.
    static int runSearchBenchmarkBatch(PhoneBook phonebook, String csvInput,
                                       String outCsv, boolean append) {
        int loaded = phonebook.loadfromCSV(csvInput);
        if (loaded == -1) {
            System.err.println("Cannot open file: " + csvInput);
            return 1;
        }
        int n = phonebook.size();
        if (n == 0) {
            System.err.println("No contacts to benchmark.");
            return 1;
        }
        String[] cases = {"first", "middle", "last", "miss"};
        int[] indices = {0, n / 2, n - 1, -1};
        indices[0] = 0;
        indices[1] = n / 2;
        indices[2] = n - 1;
        indices[3] = -1;

        boolean needHeader = true;
        Path outPath = Paths.get(outCsv);
        if (append && Files.exists(outPath)) {
            try {
                if (Files.size(outPath) > 0) {
                    List<String> lines = Files.readAllLines(outPath, StandardCharsets.UTF_8);
                    if (!lines.isEmpty() && !lines.get(0).trim().isEmpty()) {
                        needHeader = false;
                    }
                }
            } catch (IOException e) {
                // fall through, rewrite header
            }
        }
        List<String> rows = new ArrayList<>();
        if (needHeader) {
            rows.add("language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone");
        }
        String timestamp = Instant.now().truncatedTo(ChronoUnit.SECONDS).toString();
        String toolchain = "javac " + System.getProperty("java.version", "?");
        for (int k = 0; k < 4; k++) {
            String phone;
            if (k == 3) {
                phone = "0000000000";
            } else {
                phone = phonebook.getPhoneAt(indices[k]);
                if (phone.isEmpty()) {
                    continue;
                }
            }
            final String fPhone = phone;
            for (int a = 0; a < 3; a++) {
                String algo = (a == 0) ? "linear" : (a == 1) ? "hash" : "binary";
                final int which = a;
                for (int r = 1; r <= 5; r++) {
                    double t;
                    if (which == 0) {
                        t = Timer.timeIt(() -> phonebook.searchLinearByPhone(fPhone));
                    } else if (which == 1) {
                        t = Timer.timeIt(() -> phonebook.searchHashByPhone(fPhone));
                    } else {
                        t = Timer.timeIt(() -> phonebook.searchBinaryByPhone(fPhone));
                    }
                    rows.add("java," + csvEscape(csvInput) + "," + n + "," + cases[k] + ","
                            + algo + "," + r + "," + t + "," + timestamp + ","
                            + csvEscape(toolchain) + "," + indices[k] + "," + phone);
                }
            }
        }
        try (BufferedWriter w = Files.newBufferedWriter(outPath, StandardCharsets.UTF_8,
                append ? StandardOpenOption.APPEND : StandardOpenOption.TRUNCATE_EXISTING,
                StandardOpenOption.CREATE, StandardOpenOption.WRITE)) {
            for (String row : rows) {
                w.write(row);
                w.newLine();
            }
        } catch (IOException e) {
            System.err.println("Cannot open output file: " + outCsv);
            return 1;
        }
        System.out.println("Wrote 60 rows -> " + outCsv + " (" + n + " contacts).");
        return 0;
    }

    static void printMenu() {
        System.out.println("");
        System.out.println("========================================");
        System.out.println("             PHONE BOOK CLI");
        System.out.println("========================================");
        System.out.println("0. Benchmark phone search (5 runs, best)");
        System.out.println("1. Load contacts from CSV");
        System.out.println("2. Save contacts to CSV");
        System.out.println("3. Insert contact");
        System.out.println("4. Search contact");
        System.out.println("5. Print all contacts");
        System.out.println("6. Print contact by index");
        System.out.println("7. Show number of contacts");
        System.out.println("8. Delete contact by phone");
        System.out.println("9. Exit");
        System.out.println("========================================");
        System.out.print("Enter your choice: ");
    }

    public static void main(String[] args) {
        String benchCsv = "";
        boolean benchAppend = false;
        List<String> positionals = new ArrayList<>();
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("--benchmark-csv") && i + 1 < args.length) {
                benchCsv = args[++i];
            } else if (args[i].equals("--append")) {
                benchAppend = true;
            } else {
                positionals.add(args[i]);
            }
        }
        String csvInput = positionals.size() > 0 ? positionals.get(0) : "data/contacts_100k.csv";
        String csvOutput = positionals.size() > 1 ? positionals.get(1) : csvInput;
        PhoneBook phonebook = new PhoneBook();

        if (!benchCsv.isEmpty()) {
            System.exit(runSearchBenchmarkBatch(phonebook, csvInput, benchCsv, benchAppend));
            return;
        }
        Scanner sc = new Scanner(System.in);

        while (true) {
            printMenu();
            int choice;
            if (!sc.hasNext()) {
                System.out.println("\nGoodbye");
                return;
            }
            if (!sc.hasNextInt()) {
                sc.nextLine(); // discard bad input
                System.out.println("Invalid input.");
                continue;
            }
            choice = sc.nextInt();
            sc.nextLine(); // discard rest of line (mirrors cin.ignore)

            if (choice == 0) {
                runSearchBenchmark(phonebook, csvInput);
            } else if (choice == 1) {
                System.out.println("Loading contacts from: " + csvInput);
                final int[] result = {0};
                Timer.printTaskDuration(() -> result[0] = phonebook.loadfromCSV(csvInput));
                if (result[0] == -1) {
                    System.out.println("Cannot open file.");
                } else {
                    System.out.println("Successfully loaded " + result[0] + " contacts.");
                }
            } else if (choice == 2) {
                System.out.println("\nSaving contacts to: " + csvOutput);
                final boolean[] result = {false};
                Timer.printTaskDuration(() -> result[0] = phonebook.savetoCSV(csvOutput));
                if (result[0]) {
                    System.out.println("Contacts saved successfully.");
                } else {
                    System.out.println("Cannot open output file.");
                }
            } else if (choice == 3) {
                String name;
                String phone;
                try {
                    System.out.print("\nEnter name: ");
                    name = sc.nextLine();
                    System.out.print("Enter phone: ");
                    phone = sc.nextLine();
                } catch (NoSuchElementException e) {
                    System.out.println("\nGoodbye");
                    return;
                }
                final boolean[] result = {false};
                final String fName = name;
                final String fPhone = phone;
                Timer.printTaskDuration(() -> result[0] = phonebook.insertContact(fName, fPhone));
                if (result[0]) {
                    System.out.println("Contact inserted successfully.");
                } else {
                    System.out.println("Failed to insert contact.");
                }
            } else if (choice == 4) {
                System.out.println("");
                System.out.println("========== Search ==========");
                System.out.println("1. Search phone - Linear Search");
                System.out.println("2. Search phone - Hash Search");
                System.out.println("3. Search phone - Binary Search (sorted index)");
                System.out.println("4. Search name - Linear Search");
                System.out.println("5. Prefix phone search");
                System.out.println("6. Back");
                System.out.println("============================");
                System.out.print("Enter your choice: ");
                if (!sc.hasNext()) {
                    System.out.println("\nGoodbye");
                    return;
                }
                if (!sc.hasNextInt()) {
                    try {
                        sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    System.out.println("Invalid search choice.");
                    continue;
                }
                int searchChoice = sc.nextInt();
                try {
                    sc.nextLine();
                } catch (NoSuchElementException e) {
                    System.out.println("\nGoodbye");
                    return;
                }
                if (searchChoice == 1) {
                    System.out.print("Enter phone number: ");
                    String phone;
                    try {
                        phone = sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    final String fPhone = phone;
                    final int[] index = {-1};
                    Timer.printTaskDuration(() -> index[0] = phonebook.searchLinearByPhone(fPhone));
                    if (index[0] == -1) {
                        System.out.println("Phone number not found.");
                    } else {
                        System.out.println("Phone number found.");
                        System.out.println("Contact index: " + index[0]);
                        phonebook.printContact(index[0]);
                    }
                } else if (searchChoice == 2) {
                    System.out.print("Enter phone number: ");
                    String phone;
                    try {
                        phone = sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    final String fPhone = phone;
                    final int[] index = {-1};
                    Timer.printTaskDuration(() -> index[0] = phonebook.searchHashByPhone(fPhone));
                    if (index[0] == -1) {
                        System.out.println("Phone number not found.");
                    } else {
                        System.out.println("Phone number found using Hash Table.");
                        System.out.println("Contact index: " + index[0]);
                        phonebook.printContact(index[0]);
                    }
                } else if (searchChoice == 3) {
                    System.out.print("Enter phone number: ");
                    String phone;
                    try {
                        phone = sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    final String fPhone3 = phone;
                    final int[] index = {-1};
                    Timer.printTaskDuration(() -> index[0] = phonebook.searchBinaryByPhone(fPhone3));
                    if (index[0] == -1) {
                        System.out.println("Phone number not found.");
                    } else {
                        System.out.println("Phone number found using Binary Search (sorted index).");
                        System.out.println("Contact index: " + index[0]);
                        phonebook.printContact(index[0]);
                    }
                } else if (searchChoice == 4) {
                    System.out.print("Enter name: ");
                    String name;
                    try {
                        name = sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    final String fName = name;
                    final int[] index = {-1};
                    Timer.printTaskDuration(() -> index[0] = phonebook.searchLinearByName(fName));
                    if (index[0] == -1) {
                        System.out.println("Name not found.");
                    } else {
                        System.out.println("Name found.");
                        System.out.println("Contact index: " + index[0]);
                        phonebook.printContact(index[0]);
                    }
                } else if (searchChoice == 5) {
                    System.out.print("Enter phone number: ");
                    String phone;
                    try {
                        phone = sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    final String fPhone5 = phone;
                    final AtomicReference<List<Integer>> results = new AtomicReference<>();
                    Timer.printTaskDuration(() -> results.set(phonebook.searchPrefixByPhone(fPhone5, 10)));
                    if (results.get().isEmpty()) {
                        System.out.println("This does not match any phone prefix.");
                    } else {
                        System.out.println("Found, here is the first 10 results (sorted index). ");
                        for (int idx : results.get()) {
                            System.out.println("Contact index: " + idx);
                            phonebook.printContact(idx);
                        }
                    }
                } else if (searchChoice == 6) {
                    System.out.println("Back to main menu.");
                } else {
                    System.out.println("Invalid search choice.");
                }
            } else if (choice == 5) {
                System.out.println("");
                System.out.println("========== Contacts ==========");
                Timer.printTaskDuration(phonebook::printAll);
                System.out.println("==============================");
            } else if (choice == 6) {
                System.out.print("\nEnter contact index: ");
                if (!sc.hasNext()) {
                    System.out.println("\nGoodbye");
                    return;
                }
                if (!sc.hasNextInt()) {
                    try {
                        sc.nextLine();
                    } catch (NoSuchElementException e) {
                        System.out.println("\nGoodbye");
                        return;
                    }
                    System.out.println("Invalid index.");
                    continue;
                }
                int index = sc.nextInt();
                try {
                    sc.nextLine();
                } catch (NoSuchElementException e) {
                    System.out.println("\nGoodbye");
                    return;
                }
                final int fIndex = index;
                final boolean[] result = {false};
                Timer.printTaskDuration(() -> result[0] = phonebook.printContact(fIndex));
                if (!result[0]) {
                    System.out.println("Invalid contact index.");
                }
            } else if (choice == 7) {
                System.out.println("\nNumber of contacts: " + phonebook.size());
            } else if (choice == 8) {
                System.out.print("\nEnter phone number to delete: ");
                String phone;
                try {
                    phone = sc.nextLine();
                } catch (NoSuchElementException e) {
                    System.out.println("\nGoodbye");
                    return;
                }
                final String fPhone = phone;
                final boolean[] result = {false};
                Timer.printTaskDuration(() -> result[0] = phonebook.deleteContactByPhone(fPhone));
                if (result[0]) {
                    System.out.println("Contact deleted successfully.");
                } else {
                    System.out.println("Failed to delete contact.");
                }
            } else if (choice == 9) {
                System.out.println("Goodbye");
                return;
            } else {
                System.out.println("Invalid choice. Please choose from 0 to 9.");
            }
        }
    }
}
