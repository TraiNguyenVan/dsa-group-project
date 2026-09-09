# Phonebook Logic Mindmap — `tuan-vibecode` (`src`/`include` only)

> Branch: `tuan-vibecode` @ `90187d1`. Build target: `build/cpp/demo` from `src/*.cpp` + `include/*.hpp` (`Makefile:7`). Root `main.cpp:1` (legacy monolith `TABLE_SIZE=101` fixed) **excluded** — this map covers modular `src`/`include` only.

## 1. Visual Mindmap (Mermaid) — GitHub / VSCode native

```mermaid
mindmap
  root((Phonebook CLI<br/>tuan-vibecode<br/>src + include))
    Data Model & Storage
      Contact struct
        name string<br/>phone digits-only<br/>contact.hpp:5 O(1)
      HashNode struct
        phone + contactIndex + next*<br/>contact.hpp:10 O(1)
      PhoneBook class
        vector Contact contacts<br/>phonebook.hpp:10 O(1) access
        vector HashNode* buckets<br/>phonebook.hpp:11 O(TABLE_SIZE)
        tableSize nextPrime<br/>phonebook.hpp:12 O(1)
        numElements counter<br/>phonebook.hpp:13 O(1)
    Hash Table Engine O(1) avg
      hashForSize static
        h = h*31 + (c-'0')<br/>phonebook.cpp:28 O(L) L=phone len
        hash % mod<br/>phonebook.cpp:30 O(1)
      hashFunction wrapper
        hashForSize(phone, tableSize)<br/>phonebook.cpp:22 O(L)
      Prime Sizing
        isPrime trial division<br/>phonebook.cpp:33 O(sqrt n)
        nextPrime odd scan<br/>phonebook.cpp:41 O(sqrt n)
        DEFAULT 101 prime<br/>phonebook.hpp:26 O(1)
      Buckets & Chaining
        Separate chaining<br/>head insert<br/>phonebook.cpp:91 O(1)
        Collision linear chain<br/>O(k) k=bucket len
      Dynamic Rehashing
        maybeRehash >0.75<br/>phonebook.cpp:84 O(1) check
        rehash nextPrime 2x<br/>phonebook.cpp:66 O(n)
        Re-bucket all nodes<br/>O(n) one-time
    Search Duo Contrast
      Linear Search O(n)
        toLower query<br/>phonebook.cpp:9 O(L)
        Scan all contacts<br/>phonebook.cpp:111 O(n)
        nameLower.find(q)<br/>phonebook.cpp:113 O(n*m)
        phone.find(query)<br/>partial match O(n*m)
        Returns vector int<br/>O(n) result set
      Hash Search O(1) avg O(n) worst
        hashFunction idx<br/>phonebook.cpp:98 O(L)
        Traverse chain<br/>phonebook.cpp:99 O(k)
        Exact phone ==<br/>phonebook.cpp:101 O(L)
        Return index or -1<br/>O(1)
    Operations CRUD
      insertContact O(1) avg
        empty check<br/>phonebook.cpp:122 O(1)
        isAllDigits check<br/>phonebook.cpp:15 O(L)
        dup hashSearch<br/>phonebook.cpp:130 O(1) avg
        push_back + hashInsert<br/>phonebook.cpp:135 O(1) amort
      printAll O(n)
        empty guard<br/>phonebook.cpp:142 O(1)
        Loop i 0..n print<br/>phonebook.cpp:147 O(n)
      Accessors O(1)
        getContact idx<br/>phonebook.cpp:153
        size bucketCount<br/>phonebook.cpp:155 O(1)
        loadFactor n/tableSize<br/>phonebook.cpp:159 O(1)
    CSV Persistence O(n)
      Helpers O(L)
        trim whitespace<br/>phonebook.cpp:163 O(L)
        parseCsvLine quoted<br/>phonebook.cpp:171 O(L)
        Handles "Doe, John",phone<br/>rfind comma O(L)
      loadFromCSV O(n*L)
        ifstream open<br/>phonebook.cpp:199 O(1)
        getline loop<br/>phonebook.cpp:204 O(n)
        Strip CR LF<br/>phonebook.cpp:205 O(1)
        parse + validate<br/>phonebook.cpp:209 O(L)
        isAllDigits + dup<br/>skip invalid O(1) avg
        push + hashInsert<br/>phonebook.cpp:214 O(1) avg
      saveToCSV O(n*L)
        ofstream open<br/>phonebook.cpp:225 O(1)
        needsQuote , or "<br/>phonebook.cpp:232 O(L)
        Escape "" doubling<br/>phonebook.cpp:237 O(L)
        Write name,phone \n<br/>phonebook.cpp:244 O(L)
    CLI Control Flow
      main O(n) load + loop
        PhoneBook book<br/>main.cpp:64 O(1)
        csvPath argv or 200k.csv<br/>main.cpp:66 O(1)
        timed loadFromCSV<br/>main.cpp:67 O(n*L)
        print Loaded + elapsed<br/>O(1)
        while true menu<br/>main.cpp:76 O(1) per iter
        1 Search doSearch<br/>main.cpp:98 O(n) or O(1)
        2 Insert read+measure<br/>main.cpp:100 O(1) avg
        3 PrintAll timed<br/>main.cpp:112 O(n)
        4 Save timed<br/>main.cpp:115 O(n*L)
        5 Exit break<br/>O(1)
      doSearch branching
        Choose 1 Linear 2 Hash<br/>main.cpp:9 O(1)
        Method 1 linearSearch<br/>main.cpp:29 O(n)
        Print results loop<br/>main.cpp:34 O(k)
        timing printElapsed<br/>O(1)
        Method 2 digit guard<br/>main.cpp:42 O(L)
        hashSearch exact<br/>main.cpp:48 O(1) avg
        Invalid method guard<br/>O(1)
    Timing Layer O(1)
      Timed struct<br/>timer.hpp:17 value+us
      measure generic<br/>timer.hpp:32 Clock.now O(1)
      is_void specialization<br/>timer.hpp:35 O(1)
      duration_cast us<br/>timer.hpp:45 O(1)
      printElapsed ms<br/>timer.hpp:49 O(1)
      ms = us / 1000 fixed 3<br/>timer.hpp:50 O(1)
      timed wrapper<br/>timer.hpp:58 auto print
    Build & Datasets
      Makefile
        CXX g++ -std=c++17 -O2<br/>Makefile:3 O(1)
        SRC wildcard src/*.cpp<br/>Makefile:9
        TARGET build/cpp/demo<br/>Makefile:8
        run targets 50/100k/200k/1m<br/>Makefile:21 O(n) dataset
      Data Generation
        generate.py<br/>vietnamese-like names
        contacts_50.csv<br/>contacts_100k.csv<br/>contacts_200k.csv default<br/>contacts_1m.csv
```

> Source for PNG: `docs/mindmap.mmd`. Render: `npx @mermaid-js/mermaid-cli -i docs/mindmap.mmd -o docs/mindmap.png`

---

## 2. Logic Flow (Text Tree)

```
main(argc,argv) [src/main.cpp:63] O(n) startup
├─ PhoneBook book [phonebook.hpp:8] -> buckets assigned nextPrime(101) [phonebook.cpp:48]
├─ csvPath = argv[1] ? argv[1] : "data/contacts_200k.csv" [main.cpp:66]
├─ timing::measure( loadFromCSV ) [main.cpp:67] -> O(n*L) [phonebook.cpp:198]
│   └─ for each line: trim -> parseCsvLine -> isAllDigits -> hashSearch dup -> push + hashInsert
├─ timing::printElapsed("Load") [timer.hpp:49]
└─ while(true) [main.cpp:76] O(1) per iteration
    ├─ 1: doSearch(book) [main.cpp:9]
    │   ├─ 1 Linear: timing::measure( linearSearch(query) ) [main.cpp:29] O(n*m)
    │   └─ 2 Hash: digit guard [main.cpp:42] O(L) -> timing::measure( hashSearch ) [main.cpp:48] O(1) avg
    ├─ 2: insertContact(name,phone) [phonebook.cpp:121] O(1) avg -> hashInsert maybeRehash
    ├─ 3: printAll() [phonebook.cpp:141] O(n) -> timing wrapper
    ├─ 4: saveToCSV(csvPath) [phonebook.cpp:224] O(n*L)
    └─ 5: break -> Goodbye
```

---

## 3. Complexity & Code Reference Table

| Layer | Function / Block | Location | Code Snippet | Big-O | Notes |
|-------|------------------|----------|--------------|-------|-------|
| **Model** | `Contact` | `include/contact.hpp:5` | `struct Contact { string name; string phone; };` | `O(1)` storage per contact | phone digits-only invariant |
| | `HashNode` | `include/contact.hpp:10` | `struct HashNode { string phone; int contactIndex; HashNode* next; };` | `O(1)` per node | separate chaining node |
| | `PhoneBook` fields | `include/phonebook.hpp:10` | `vector<Contact> contacts; vector<HashNode*> buckets; size_t tableSize, numElements;` | `O(n + TABLE_SIZE)` memory | `contacts` owns data, `buckets` indexes it |
| **Hash** | `hashForSize` | `src/phonebook.cpp:26` | `hash = hash*31 + (c-'0'); return hash % mod;` | `O(L)` `L≈10-11` | djb2-variant over digits, prime mod |
| | `hashFunction` | `src/phonebook.cpp:22` | `return hashForSize(phone, tableSize);` | `O(L)` | wrapper with current tableSize |
| | `isPrime` | `src/phonebook.cpp:33` | `for(i=3;i*i<=n;i+=2) if(n%i==0) return false;` | `O(√n)` | trial division odd only |
| | `nextPrime` | `src/phonebook.cpp:41` | `if(n%2==0) ++n; while(!isPrime(n)) n+=2;` | `O(√n)` amort | finds next prime ≥ n |
| | `hashInsert` | `src/phonebook.cpp:89` | `buckets[idx]=new HashNode{phone,idx,buckets[idx]}; ++numElements; maybeRehash();` | `O(1)` avg, `O(n)` if rehash | head insert, triggers rehash |
| | `rehash` | `src/phonebook.cpp:66` | `newSize=nextPrime(newSize*2); re-bucket all nodes; buckets.swap(newBuckets);` | `O(n)` | only grows, prime 2×, re-hashes `hash % newSize` |
| | `maybeRehash` | `src/phonebook.cpp:84` | `if(numElements/tableSize > 0.75) rehash(tableSize*2);` | `O(1)` check, `O(n)` when fired | `MAX_LOAD_FACTOR=0.75` `phonebook.hpp:27` |
| **Search** | `toLower` | `src/phonebook.cpp:9` | `transform(s.begin(),s.end(), ::tolower);` | `O(L)` | for case-insensitive name match |
| | `isAllDigits` | `src/phonebook.cpp:15` | `for(c:s) if(!isdigit(c)) return false;` | `O(L)` | validation gate |
| | `linearSearch` | `src/phonebook.cpp:107` | `for(i) if(toLower(contacts[i].name).find(q)!=npos \|\| phone.find(query)!=npos) results.push_back(i);` | `O(n·m)` `m=query len` | partial match name+phone, returns `vector<int>` |
| | `hashSearch` | `src/phonebook.cpp:97` | `idx=hashFunction(phone); for(node=buckets[idx]; node; node=node->next) if(node->phone==phone) return idx; return -1;` | `O(1)` avg, `O(n)` worst | exact full-phone only, chain length `k` |
| **CRUD** | `insertContact` | `src/phonebook.cpp:121` | `if(empty\|!isAllDigits\|hashSearch!=-1) return false; contacts.push_back({name,phone}); hashInsert(phone, size-1);` | `O(1)` avg | dup via hashSearch, validates before insert |
| | `printAll` | `src/phonebook.cpp:141` | `for(i) cout << i+1 << "\t" << contacts[i].name << "\t" << contacts[i].phone;` | `O(n)` | `measure` wrapper in `main.cpp:113` prints timing after |
| | `getContact/size/bucketCount/loadFactor` | `src/phonebook.cpp:153` | `contacts[idx]; contacts.size(); tableSize; n/tableSize` | `O(1)` | accessors |
| **CSV** | `trim` | `src/phonebook.cpp:163` | `while(isspace(s[start])) start++; while(isspace(s[end-1])) end--;` | `O(L)` | strips spaces |
| | `parseCsvLine` | `src/phonebook.cpp:171` | `if(l[0]=='"') {close=l.find('"',1); comma=l.find(',',close+1);} else comma=l.rfind(',');` | `O(L)` | handles `"Doe, John",0123` + `rfind` for last comma |
| | `loadFromCSV` | `src/phonebook.cpp:198` | `while(getline) { if(trim empty) continue; parseCsvLine -> validate -> hashSearch dup -> push+hashInsert; }` | `O(n·L)` | `n=rows`, skips invalid/dup quietly, `\r` strip |
| | `saveToCSV` | `src/phonebook.cpp:224` | `if(name.find(',')\|\|'"') { file<<'"'; for(c:name) if(c=='"') file<<'""'; } file<<','<<phone;` | `O(n·L)` | re-quotes `,`/`"` by doubling `""`, round-trip safe |
| **CLI** | `main` | `src/main.cpp:63` | `PhoneBook book; load=measure(loadFromCSV); while(true) switch(choice) {1..5}` | `O(n)` startup + `O(1)` loop | `cin >> choice` with `numeric_limits::max` ignore `main.cpp:86` + `eof` goodbye |
| | `doSearch` | `src/main.cpp:9` | `cin>>method; getline query; if(1) measure(linearSearch) else if(2) { digit guard; measure(hashSearch); }` | `O(n)` or `O(1)` | user picks algorithm to benchmark side-by-side, `printElapsed` last |
| **Timing** | `measure` | `include/timer.hpp:31` | `auto start=Clock::now(); func(); auto end=Clock::now(); return {result, us};` | `O(1)` overhead + `O(func)` | generic `Timed<T>`, `is_void_v` branch, `high_resolution_clock` |
| | `printElapsed` | `include/timer.hpp:49` | `ms=us/1000.0; oss<<"["<<label<<"] elapsed: "<<fixed<<setprecision(3)<<ms<<" ms";` | `O(1)` | caller prints after domain output |
| **Build** | `Makefile` | `Makefile:1` | `CXX=g++ -std=c++17 -O2 -Iinclude; SRC=$(wildcard src/*.cpp); TARGET=build/cpp/demo` | — | `make run` defaults `data/contacts_100k.csv:21`, `run-200k:29` |

---

## 4. Architecture Notes

- **Two-level storage invariant:** `contacts: vector<Contact>` is source of truth; `buckets: vector<HashNode*>` stores `contactIndex` not copy. `loadFromCSV` and `insertContact` both `push_back` then `hashInsert` atomically.
- **Rehash invariant:** `tableSize` always prime (`nextPrime`), `loadFactor ≤0.75` guaranteed post-insert. `rehash` reuses existing `HashNode*` (no re-alloc) and only grows.
- **Search semantics split:** Linear = user-friendly (partial, case-insensitive name OR substring phone), Hash = strict (exact full digits). CLI forces choice (`main.cpp:9`) for benchmark comparison.
- **CSV robustness:** `trim` + `rfind(',')` + quoted `"Name, With Comma"` + `""` escape + `\r` strip handles Windows files, Vietnamese names (`data/generate.py`).
- **Timing discipline:** `measure` does NOT print (`timer.hpp:29`), `printElapsed` called *after* domain output so `[Label] elapsed: X ms` is always last line per feature parity.
- **Complexity summary:** Build `O(n)` load, Search `O(n)` vs `O(1)`, Insert `O(1)` amort, Save/Print `O(n)`, Memory `O(n + TABLE_SIZE)`.

---

## 5. Render PNG

```bash
# install once (needs Node >=18)
npm install -g @mermaid-js/mermaid-cli
# or npx without install:
npx --yes @mermaid-js/mermaid-cli -i docs/mindmap.mmd -o docs/mindmap.png -b white -w 2400 -H 1600
# alternative SVG (crisp):
npx mmdc -i docs/mindmap.mmd -o docs/mindmap.svg -b white
```

PNG output: `docs/mindmap.png` (2400×1600, white bg). If CLI unavailable, open `docs/mindmap.md` in VSCode + `Markdown Preview Mermaid Support` → export.

---

*Generated: docs/mindmap.md + docs/mindmap.mmd. Next: `npx mmdc` to produce `docs/mindmap.png`.*
