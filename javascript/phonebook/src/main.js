'use strict';

// Faithful port of src/main.cpp - Phone Book CLI (Node.js).
const fs = require('node:fs');
const readline = require('node:readline');
const { stdin: input, stdout: output } = require('node:process');
const { PhoneBook } = require('./phonebook');
const { benchmark, printTaskDuration, timeIt } = require('./timer');

const BENCH_HEADER = 'language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone';

function csvEscape(v) {
  v = String(v);
  if (v.includes(',') || v.includes('"') || v.includes('\n')) {
    return '"' + v.replace(/"/g, '""') + '"';
  }
  return v;
}

// Batch benchmark: every run to CSV (60 rows). `miss` uses phone
// "0000000000" (not in dataset) for true worst case of hash/binary.
function runSearchBenchmarkBatch(phonebook, csvInput, outCsv, append) {
  const loaded = phonebook.loadfromCSV(csvInput);
  if (loaded === -1) {
    console.error(`Cannot open file: ${csvInput}`);
    return 1;
  }
  const n = phonebook.size();
  if (n === 0) {
    console.error('No contacts to benchmark.');
    return 1;
  }
  const cases = ['first', 'middle', 'last', 'miss'];
  const indices = [0, Math.floor(n / 2), n - 1, -1];
  indices[0] = 0;
  indices[1] = Math.floor(n / 2);
  indices[2] = n - 1;
  indices[3] = -1;

  let needHeader = true;
  if (append && fs.existsSync(outCsv) && fs.statSync(outCsv).size > 0) {
    const first = fs.readFileSync(outCsv, 'utf8').split('\n')[0].trim();
    needHeader = !first;
  }
  let fd;
  try {
    fd = fs.openSync(outCsv, append ? 'a' : 'w');
  } catch (e) {
    console.error(`Cannot open output file: ${outCsv}`);
    return 1;
  }
  const lines = [];
  if (needHeader) lines.push(BENCH_HEADER);
  const timestamp = new Date().toISOString().replace(/\.\d+Z$/, 'Z');
  const toolchain = `node ${process.version}`;
  for (let k = 0; k < 4; k++) {
    let phone;
    if (k === 3) phone = '0000000000';
    else {
      phone = phonebook.getPhoneAt(indices[k]);
      if (!phone) continue;
    }
    for (let a = 0; a < 3; a++) {
      const algo = a === 0 ? 'linear' : a === 1 ? 'hash' : 'binary';
      for (let r = 1; r <= 5; r++) {
        let t;
        if (a === 0) t = timeIt(() => phonebook.searchLinearByPhone(phone));
        else if (a === 1) t = timeIt(() => phonebook.searchHashByPhone(phone));
        else t = timeIt(() => phonebook.searchBinaryByPhone(phone));
        lines.push(['js', csvInput, n, cases[k], algo, r, t, timestamp,
          toolchain, indices[k], phone].map(csvEscape).join(','));
      }
    }
  }
  fs.writeSync(fd, lines.join('\n') + '\n');
  fs.closeSync(fd);
  console.log(`Wrote 60 rows -> ${outCsv} (${n} contacts).`);
  return 0;
}

function runSearchBenchmark(phonebook, csvInput) {
  if (phonebook.size() === 0) {
    console.log(`Phonebook empty, auto-loading from: ${csvInput}`);
    const loaded = phonebook.loadfromCSV(csvInput);
    if (loaded === -1) {
      console.log('Cannot open file.');
      return;
    }
    if (phonebook.size() === 0) {
      console.log('No contacts to benchmark. Load contacts first (option 1).');
      return;
    }
    console.log(`Auto-loaded ${loaded} contacts.`);
  }

  const n = phonebook.size();
  const labels = [
    'first (linear best case)',
    'middle (linear avg / binary best)',
    'last (linear worst case)',
    'miss (linear worst / hash & binary worst)',
  ];
  const indices = [0, Math.floor(n / 2), n - 1, -1];
  indices[0] = 0;
  indices[1] = Math.floor(n / 2);
  indices[2] = n - 1;
  indices[3] = -1;

  console.log(
    `\nBenchmarking phone search (linear: first=best / middle / last,miss=worst; hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary) (${n} contacts, 5 runs each, best reported).`
  );

  for (let k = 0; k < 4; k++) {
    const targetIdx = indices[k];
    let phone;
    if (k === 3) phone = '0000000000';
    else {
      phone = phonebook.getPhoneAt(targetIdx);
      if (!phone) {
        console.log(`[${labels[k]}] index ${targetIdx}: cannot pick target.`);
        continue;
      }
    }
    const holder = { lin: -1, h: -1, b: -1 };
    const linBest = benchmark(() => { holder.lin = phonebook.searchLinearByPhone(phone); });
    const hashBest = benchmark(() => { holder.h = phonebook.searchHashByPhone(phone); });
    const binBest = benchmark(() => { holder.b = phonebook.searchBinaryByPhone(phone); });
    console.log(`[${labels[k]} index ${targetIdx} phone ${phone}]`);
    console.log(`  Linear best of 5: ${linBest}ms. (index ${holder.lin})`);
    console.log(`  Hash best of 5: ${hashBest}ms. (index ${holder.h}, position-independent)`);
    console.log(`  Binary best of 5: ${binBest}ms. (index ${holder.b}, sorted index, position-independent)`);
  }
  console.log('Linear: first=best, last/miss=worst. Hash/binary: ~constant; miss is worst (full chain / log n probes).');
}

function printMenu() {
  console.log('');
  console.log('========================================');
  console.log('             PHONE BOOK CLI');
  console.log('========================================');
  console.log('0. Benchmark phone search (5 runs, best)');
  console.log('1. Load contacts from CSV');
  console.log('2. Save contacts to CSV');
  console.log('3. Insert contact');
  console.log('4. Search contact');
  console.log('5. Print all contacts');
  console.log('6. Print contact by index');
  console.log('7. Show number of contacts');
  console.log('8. Delete contact by phone');
  console.log('9. Exit');
  console.log('========================================');
}

function parseChoice(raw) {
  if (raw === undefined || raw === null) return null; // EOF
  const trimmed = raw.trim();
  if (trimmed === '') return NaN;
  const token = trimmed.split(/\s+/)[0];
  // Mirror cin >> int: token must be a plain integer.
  if (!/^[+-]?\d+$/.test(token)) return NaN;
  return parseInt(token, 10);
}

async function main() {
  const args = process.argv.slice(2);
  let benchCsv = '';
  let benchAppend = false;
  const positionals = [];
  for (let i = 0; i < args.length; i++) {
    if (args[i] === '--benchmark-csv' && i + 1 < args.length) {
      benchCsv = args[++i];
    } else if (args[i] === '--append') {
      benchAppend = true;
    } else {
      positionals.push(args[i]);
    }
  }
  const csvInput = positionals.length > 0 ? positionals[0] : 'data/contacts_100k.csv';
  const csvOutput = positionals.length > 1 ? positionals[1] : csvInput;
  const phonebook = new PhoneBook();

  if (benchCsv) {
    process.exitCode = runSearchBenchmarkBatch(phonebook, csvInput, benchCsv, benchAppend);
    return process.exitCode;
  }

  // Line queue via async iterator: works for both TTY and piped stdin+piped
  // stdout (rl.question breaks on 2nd read when stdout is piped).
  const rl = readline.createInterface({ input, terminal: false });
  const lineIter = rl[Symbol.asyncIterator]();
  let eof = false;
  async function ask(prompt) {
    if (eof) return null;
    try {
      output.write(prompt);
    } catch (_) { }
    try {
      const { value, done } = await lineIter.next();
      if (done) {
        eof = true;
        return null;
      }
      if (value === undefined) return null;
      // Strip one trailing \r (Windows pipe); choice parsing trims anyway.
      if (value.endsWith('\r')) return value.slice(0, -1);
      return value;
    } catch (e) {
      eof = true;
      return null;
    }
  }
  // If stdin ends, readline closes; detect it.
  let closed = false;
  rl.on('close', () => { closed = true; });

  while (true) {
    printMenu();
    const raw = await ask('Enter your choice: ');
    if (raw === null || closed) {
      console.log('\nGoodbye');
      try { rl.close(); } catch (_) { }
      return 0;
    }
    const choice = parseChoice(raw);
    if (Number.isNaN(choice)) {
      console.log('Invalid input.');
      continue;
    }

    if (choice === 0) {
      runSearchBenchmark(phonebook, csvInput);
    } else if (choice === 1) {
      console.log(`Loading contacts from: ${csvInput}`);
      const result = {};
      printTaskDuration(() => { result.v = phonebook.loadfromCSV(csvInput); });
      if (result.v === -1) console.log('Cannot open file.');
      else console.log(`Successfully loaded ${result.v} contacts.`);
    } else if (choice === 2) {
      console.log(`\nSaving contacts to: ${csvOutput}`);
      const result = {};
      printTaskDuration(() => { result.v = phonebook.savetoCSV(csvOutput); });
      if (result.v) console.log('Contacts saved successfully.');
      else console.log('Cannot open output file.');
    } else if (choice === 3) {
      const name = await ask('\nEnter name: ');
      if (name === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
      const phone = await ask('Enter phone: ');
      if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
      const result = {};
      printTaskDuration(() => { result.v = phonebook.insertContact(name, phone); });
      if (result.v) console.log('Contact inserted successfully.');
      else console.log('Failed to insert contact.');
    } else if (choice === 4) {
      console.log('');
      console.log('========== Search ==========');
      console.log('1. Search phone - Linear Search');
      console.log('2. Search phone - Hash Search');
      console.log('3. Search phone - Binary Search (sorted index)');
      console.log('4. Search name - Linear Search');
      console.log('5. Back');
      console.log('============================');
      const sraw = await ask('Enter your choice: ');
      if (sraw === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
      const searchChoice = parseChoice(sraw);
      if (Number.isNaN(searchChoice)) {
        console.log('Invalid search choice.');
        continue;
      }
      if (searchChoice === 1) {
        const phone = await ask('Enter phone number: ');
        if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
        const result = {};
        printTaskDuration(() => { result.v = phonebook.searchLinearByPhone(phone); });
        if (result.v === -1) console.log('Phone number not found.');
        else {
          console.log('Phone number found.');
          console.log(`Contact index: ${result.v}`);
          phonebook.printContact(result.v);
        }
      } else if (searchChoice === 2) {
        const phone = await ask('Enter phone number: ');
        if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
        const result = {};
        printTaskDuration(() => { result.v = phonebook.searchHashByPhone(phone); });
        if (result.v === -1) console.log('Phone number not found.');
        else {
          console.log('Phone number found using Hash Table.');
          console.log(`Contact index: ${result.v}`);
          phonebook.printContact(result.v);
        }
      } else if (searchChoice === 3) {
        const phone = await ask('Enter phone number: ');
        if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
        const result = {};
        printTaskDuration(() => { result.v = phonebook.searchBinaryByPhone(phone); });
        if (result.v === -1) console.log('Phone number not found.');
        else {
          console.log('Phone number found using Binary Search (sorted index).');
          console.log(`Contact index: ${result.v}`);
          phonebook.printContact(result.v);
        }
      } else if (searchChoice === 4) {
        const name = await ask('Enter name: ');
        if (name === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
        const result = {};
        printTaskDuration(() => { result.v = phonebook.searchLinearByName(name); });
        if (result.v === -1) console.log('Name not found.');
        else {
          console.log('Name found.');
          console.log(`Contact index: ${result.v}`);
          phonebook.printContact(result.v);
        }
      } else if (searchChoice === 5) {
        console.log('Back to main menu.');
      } else {
        console.log('Invalid search choice.');
      }
    } else if (choice === 5) {
      console.log('');
      console.log('========== Contacts ==========');
      printTaskDuration(() => { phonebook.printAll(); });
      console.log('==============================');
    } else if (choice === 6) {
      const iraw = await ask('\nEnter contact index: ');
      if (iraw === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
      const idx = parseChoice(iraw);
      if (Number.isNaN(idx)) {
        console.log('Invalid index.');
        continue;
      }
      const result = {};
      printTaskDuration(() => { result.v = phonebook.printContact(idx); });
      if (!result.v) console.log('Invalid contact index.');
    } else if (choice === 7) {
      console.log(`\nNumber of contacts: ${phonebook.size()}`);
    } else if (choice === 8) {
      const phone = await ask('\nEnter phone number to delete: ');
      if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) { } return 0; }
      const result = {};
      printTaskDuration(() => { result.v = phonebook.deleteContactByPhone(phone); });
      if (result.v) console.log('Contact deleted successfully.');
      else console.log('Failed to delete contact.');
    } else if (choice === 9) {
      console.log('Goodbye');
      try { rl.close(); } catch (_) { }
      return 0;
    } else {
      console.log('Invalid choice. Please choose from 0 to 9.');
    }
  }
}

if (require.main === module) {
  main().then((code) => process.exit(code));
}

module.exports = { runSearchBenchmark, printMenu };
