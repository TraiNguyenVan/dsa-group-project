package com.phonebook;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

// Faithful port of include/phonebook.hpp + src/phonebook.cpp.
public class PhoneBook {
    private final List<Contact> contacts = new ArrayList<>();
    private final HashTable hashtable;

    public PhoneBook() {
        this(HashTable.DEFAULT_TABLE_SIZE);
    }

    public PhoneBook(int initialCapacity) {
        this.hashtable = new HashTable(initialCapacity);
    }

    private static String trimCsv(String s) {
        int start = 0;
        int end = s.length();
        while (start < end && isCsvSpace(s.charAt(start))) {
            start++;
        }
        while (end > start && isCsvSpace(s.charAt(end - 1))) {
            end--;
        }
        return s.substring(start, end);
    }

    private static boolean isCsvSpace(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\u000B' || c == '\u000C';
    }

    // Returns {ok, name, phone} via out array. Mirrors C++ parseCsvLine.
    static String[] parseCsvLine(String line) {
        String l = trimCsv(line);
        if (l.isEmpty()) {
            return null;
        }
        String name;
        String phone;
        if (l.charAt(0) == '"') {
            StringBuilder parsed = new StringBuilder();
            int i = 1;
            boolean closed = false;
            while (i < l.length()) {
                char c = l.charAt(i);
                if (c == '"') {
                    if (i + 1 < l.length() && l.charAt(i + 1) == '"') {
                        parsed.append('"');
                        i += 2;
                    } else {
                        closed = true;
                        i++;
                        break;
                    }
                } else {
                    parsed.append(c);
                    i++;
                }
            }
            if (!closed) {
                return null;
            }
            name = parsed.toString();
            int comma = l.indexOf(',', i);
            if (comma == -1) {
                return null;
            }
            phone = l.substring(comma + 1);
        } else {
            int comma = l.lastIndexOf(',');
            if (comma == -1) {
                return null;
            }
            name = l.substring(0, comma);
            phone = l.substring(comma + 1);
        }
        name = trimCsv(name);
        phone = trimCsv(phone);
        if (phone.length() >= 2 && phone.charAt(0) == '"' && phone.charAt(phone.length() - 1) == '"') {
            phone = trimCsv(phone.substring(1, phone.length() - 1));
        }
        if (name.isEmpty() && phone.isEmpty()) {
            return null;
        }
        return new String[]{name, phone};
    }

    public static boolean isAllDigits(String s) {
        if (s == null || s.isEmpty()) {
            return false;
        }
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c < '0' || c > '9') {
                return false;
            }
        }
        return true;
    }

    public static String toLower(String s) {
        StringBuilder res = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c >= 'A' && c <= 'Z') {
                res.append((char) (c + 32));
            } else {
                res.append(c);
            }
        }
        return res.toString();
    }

    public static String capitalizeFirst(String s) {
        StringBuilder res = new StringBuilder(s);
        boolean newWord = true;
        for (int i = 0; i < res.length(); i++) {
            char c = res.charAt(i);
            if (c == ' ') {
                newWord = true;
            } else {
                if (newWord && c >= 'a' && c <= 'z') {
                    res.setCharAt(i, (char) (c - 32));
                }
                newWord = false;
            }
        }
        return res.toString();
    }

    public boolean insertContact(String name, String phone) {
        if (name == null || phone == null || name.isEmpty() || phone.isEmpty()) {
            System.out.println("Missing name or phone number");
            return false;
        }
        if (!isAllDigits(phone)) {
            System.out.println("Invalid phone number");
            return false;
        }
        if (searchHashByPhone(phone) != -1) {
            System.out.println("Phone number is already exist");
            return false;
        }
        String normalized = capitalizeFirst(toLower(name));
        contacts.add(new Contact(normalized, phone));
        hashtable.hashInsert(phone, contacts.size() - 1);
        return true;
    }

    public int searchLinearByPhone(String phone) {
        for (int i = 0; i < contacts.size(); i++) {
            if (contacts.get(i).phone.equals(phone)) {
                return i;
            }
        }
        return -1;
    }

    public int searchHashByPhone(String phone) {
        return hashtable.hashSearch(phone);
    }

    public boolean deleteContactByPhone(String phone) {
        // O(n): list erase shifts tail + full hash rebuild preserves order.
        int idx = searchHashByPhone(phone);
        if (idx == -1) {
            System.out.println("Phone number not found");
            return false;
        }
        contacts.remove(idx);
        hashtable.clear();
        for (int i = 0; i < contacts.size(); i++) {
            hashtable.hashInsert(contacts.get(i).phone, i);
        }
        return true;
    }

    public int searchLinearByName(String name) {
        String target = toLower(name);
        for (int i = 0; i < contacts.size(); i++) {
            if (toLower(contacts.get(i).name).equals(target)) {
                return i;
            }
        }
        return -1;
    }

    public void printAll() {
        for (int i = 0; i < contacts.size(); i++) {
            System.out.println(i + ". " + contacts.get(i).name + " - " + contacts.get(i).phone);
        }
    }

    public boolean printContact(int index) {
        if (index < 0 || index >= contacts.size()) {
            return false;
        }
        System.out.println("Name: " + contacts.get(index).name);
        System.out.println("Phone: " + contacts.get(index).phone);
        return true;
    }

    public int size() {
        return contacts.size();
    }

    public String getPhoneAt(int index) {
        if (index < 0 || index >= contacts.size()) {
            return "";
        }
        return contacts.get(index).phone;
    }

    public int loadfromCSV(String path) {
        BufferedReader br;
        try {
            br = Files.newBufferedReader(Paths.get(path), StandardCharsets.UTF_8);
        } catch (IOException e) {
            return -1;
        }
        int count = 0;
        try (BufferedReader r = br) {
            String line;
            while ((line = r.readLine()) != null) {
                // readLine strips \n and trailing \r already; keep parity:
                // (no extra \r handling needed)
                if (trimCsv(line).isEmpty()) {
                    continue;
                }
                String[] parsed = parseCsvLine(line);
                if (parsed == null) {
                    continue;
                }
                String name = parsed[0];
                String phone = parsed[1];
                if (name.isEmpty() || !isAllDigits(phone)) {
                    continue;
                }
                if (insertContact(name, phone)) {
                    count++;
                }
            }
        } catch (IOException e) {
            // fall through, return what we have
        }
        return count;
    }

    public boolean savetoCSV(String path) {
        try (BufferedWriter w = Files.newBufferedWriter(Paths.get(path), StandardCharsets.UTF_8)) {
            for (Contact c : contacts) {
                boolean needsQuote = c.name.contains(",") || c.name.contains("\"");
                if (needsQuote) {
                    w.write('"');
                    for (int i = 0; i < c.name.length(); i++) {
                        char ch = c.name.charAt(i);
                        if (ch == '"') {
                            w.write("\"\"");
                        } else {
                            w.write(ch);
                        }
                    }
                    w.write('"');
                } else {
                    w.write(c.name);
                }
                w.write("," + c.phone + "\n");
            }
            return true;
        } catch (IOException e) {
            return false;
        }
    }
}
