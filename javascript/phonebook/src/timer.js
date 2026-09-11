'use strict';

// Faithful port of include/timer.hpp.
const { performance } = require('node:perf_hooks');

function timeIt(work) {
  const t0 = performance.now();
  work();
  const t1 = performance.now();
  return t1 - t0;
}

function benchmark(work, repeats = 5) {
  if (repeats <= 0) return 0.0;
  let best = Number.POSITIVE_INFINITY;
  for (let r = 0; r < repeats; r++) {
    const t = timeIt(work);
    if (t < best) best = t;
  }
  return best;
}

function printTaskDuration(work) {
  console.log(`\nTook: ${timeIt(work)}ms.`);
}

module.exports = { timeIt, benchmark, printTaskDuration };
