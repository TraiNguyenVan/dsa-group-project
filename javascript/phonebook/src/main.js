'use strict';

// Faithful port of src/main.cpp - Phone Book CLI (Node.js).
const readline = require('node:readline');
const { stdin: input, stdout: output } = require('node:process');
const { PhoneBook } = require('./phonebook');
const { benchmark, printTaskDuration } = require('./timer');

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
    'random (linear average case)',
    'last (linear worst case)',
  ];
  const indices = [0, 0, n - 1];
  indices[0] = 0;
  indices[2] = n - 1;
  if (n <= 2) {
    indices[1] = n - 1;
  } else {
    indices[1] = Math.floor(Math.random() * n);
  }

  console.log(
    `\nBenchmarking phone search (linear cases: first=best / random=average / last=worst; hash is ~O(1) in all cases) (${n} contacts, 5 runs each, best reported).`
  );

  for (let k = 0; k < 3; k++) {
    const targetIdx = indices[k];
    const phone = phonebook.getPhoneAt(targetIdx);
    if (!phone) {
      console.log(`[${labels[k]}] index ${targetIdx}: cannot pick target.`);
      continue;
    }
    const holder = { lin: -1, h: -1 };
    const linBest = benchmark(() => { holder.lin = phonebook.searchLinearByPhone(phone); });
    const hashBest = benchmark(() => { holder.h = phonebook.searchHashByPhone(phone); });
    console.log(`[${labels[k]} index ${targetIdx} phone ${phone}]`);
    console.log(`  Linear best of 5: ${linBest}ms. (index ${holder.lin})`);
    console.log(`  Hash best of 5: ${hashBest}ms. (index ${holder.h}, position-independent)`);
  }
  console.log('Linear: first=best, last=worst. Hash: ~constant regardless of position.');
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
  const csvInput = args.length > 0 ? args[0] : 'data/contacts_100k.csv';
  const csvOutput = args.length > 1 ? args[1] : csvInput;
  const phonebook = new PhoneBook();

  // Line queue via async iterator: works for both TTY and piped stdin+piped
  // stdout (rl.question breaks on 2nd read when stdout is piped).
  const rl = readline.createInterface({ input, terminal: false });
  const lineIter = rl[Symbol.asyncIterator]();
  let eof = false;
  async function ask(prompt) {
    if (eof) return null;
    try {
      output.write(prompt);
    } catch (_) {}
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
      try { rl.close(); } catch (_) {}
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
      if (name === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
      const phone = await ask('Enter phone: ');
      if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
      const result = {};
      printTaskDuration(() => { result.v = phonebook.insertContact(name, phone); });
      if (result.v) console.log('Contact inserted successfully.');
      else console.log('Failed to insert contact.');
    } else if (choice === 4) {
      console.log('');
      console.log('========== Search ==========');
      console.log('1. Search phone - Linear Search');
      console.log('2. Search phone - Hash Search');
      console.log('3. Search name - Linear Search');
      console.log('4. Back');
      console.log('============================');
      const sraw = await ask('Enter your choice: ');
      if (sraw === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
      const searchChoice = parseChoice(sraw);
      if (Number.isNaN(searchChoice)) {
        console.log('Invalid search choice.');
        continue;
      }
      if (searchChoice === 1) {
        const phone = await ask('Enter phone number: ');
        if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
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
        if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
        const result = {};
        printTaskDuration(() => { result.v = phonebook.searchHashByPhone(phone); });
        if (result.v === -1) console.log('Phone number not found.');
        else {
          console.log('Phone number found using Hash Table.');
          console.log(`Contact index: ${result.v}`);
          phonebook.printContact(result.v);
        }
      } else if (searchChoice === 3) {
        const name = await ask('Enter name: ');
        if (name === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
        const result = {};
        printTaskDuration(() => { result.v = phonebook.searchLinearByName(name); });
        if (result.v === -1) console.log('Name not found.');
        else {
          console.log('Name found.');
          console.log(`Contact index: ${result.v}`);
          phonebook.printContact(result.v);
        }
      } else if (searchChoice === 4) {
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
      if (iraw === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
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
      if (phone === null) { console.log('\nGoodbye'); try { rl.close(); } catch (_) {} return 0; }
      const result = {};
      printTaskDuration(() => { result.v = phonebook.deleteContactByPhone(phone); });
      if (result.v) console.log('Contact deleted successfully.');
      else console.log('Failed to delete contact.');
    } else if (choice === 9) {
      console.log('Goodbye');
      try { rl.close(); } catch (_) {}
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
