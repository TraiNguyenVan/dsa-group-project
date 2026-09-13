'use strict';

// Peak-memory wrapper for the JS phonebook (report D.5, Chrome DevTools).
// Loads one CSV with the real PhoneBook port and prints the V8 live heap
// after load. Run under `node --heap-prof --heap-prof-dir <dir>` so a
// DevTools-compatible .heapprofile is written as evidence.
//
// Usage: node benchmark/mem/profile_js.js <csv> <evidence-out.txt>
// stdout (last line machine-parsed by benchmark/mem_profile.py):
//   peak_js_heap_bytes <N>

const fs = require('node:fs');
const path = require('node:path');
const v8 = require('node:v8');

const { PhoneBook } = require(path.join(__dirname, '..', '..',
    'javascript', 'phonebook', 'src', 'phonebook.js'));

function main() {
    const [csvPath, evidencePath] = process.argv.slice(2);
    if (!csvPath || !evidencePath) {
        console.error('usage: profile_js.js <csv> <evidence-out.txt>');
        process.exit(2);
    }
    const pb = new PhoneBook();
    const loaded = pb.loadfromCSV(csvPath);
    if (loaded === -1) {
        console.error(`Cannot open file: ${csvPath}`);
        process.exit(1);
    }
    // Force a full GC so heapUsed reflects live data, not transient garbage.
    v8.setFlagsFromString('--expose-gc');
    if (global.gc) global.gc();

    const mu = process.memoryUsage();
    const lines = [
        `JS phonebook memory (n=${loaded} contacts, ${csvPath})`,
        `heapUsed  = ${(mu.heapUsed / 1048576).toFixed(2)} MB (live V8 heap after GC)`,
        `heapTotal = ${(mu.heapTotal / 1048576).toFixed(2)} MB (allocated V8 heap)`,
        `rss       = ${(mu.rss / 1048576).toFixed(2)} MB (node process)`,
        `external  = ${(mu.external / 1048576).toFixed(2)} MB`,
        'Evidence: .heapprofile written by `node --heap-prof` (open in Chrome DevTools > Memory).',
    ];
    fs.writeFileSync(evidencePath, lines.join('\n') + '\n');
    console.log(`loaded ${loaded} contacts`);
    console.log(`peak_js_heap_bytes ${mu.heapUsed}`);
}

main();
