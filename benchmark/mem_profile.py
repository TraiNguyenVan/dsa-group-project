#!/usr/bin/env python3
"""Peak-memory profiling orchestrator (report D.5).

Runs each language's own profiler headlessly on the real phonebook port,
plus a uniform kernel-measured peak-RSS pass of the same batch program:

  C++      Valgrind/Massif on the existing batch mode (no code changes)
  Python   tracemalloc via benchmark/mem/profile_python.py
  Go       pprof heap via go/phonebook/memprofile_test.go (env-gated test)
  JS       node --heap-prof + V8 heapUsed via benchmark/mem/profile_js.js
  Java     polled peak heap + JFR (VisualVM-readable) via MemProfile.java
  RSS      os.wait4 -> ru_maxrss of the same batch command (all languages)

Writes benchmark/mem/results.csv:
    language,dataset,n,metric,mb,tool
    metric in {profiler_heap, peak_rss}; 60 rows (5 langs x 6 sizes x 2).

Every tool's raw output is kept under benchmark/mem/ as evidence:
  massif-<n>.out + massif-<n>-msprint.txt, tracemalloc-<n>.txt,
  go-<n>.pprof + go-<n>-pprof-top.txt, js-<n>.heapprofile + js-<n>.txt,
  java-<n>.jfr + java-<n>-jfr-summary.txt, rss-<lang>-<n>.txt.

Stdlib only. Usage:
    python3 benchmark/mem_profile.py [--sizes 50,10k,100k,200k,500k,1m] [--skip-rss]
"""

import argparse
import csv
import glob
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))   # benchmark/
ROOT = os.path.normpath(os.path.join(HERE, ".."))   # repo root
MEM = os.path.join(HERE, "mem")                     # benchmark/mem/
RESULTS = os.path.join(MEM, "results.csv")

LANGS = ["cpp", "python", "go", "js", "java"]
SIZES = {"50": 50, "10k": 10_000, "100k": 100_000,
         "200k": 200_000, "500k": 500_000, "1m": 1_000_000}
HEADER = "language,dataset,n,metric,mb,tool"


def run(cmd, cwd=None, env=None, timeout=None):
    """Run a command; return (rc, stdout, stderr)."""
    r = subprocess.run(cmd, cwd=cwd, env=env, timeout=timeout,
                       capture_output=True, text=True)
    return r.returncode, r.stdout, r.stderr


def parse_last(stdout, prefix):
    """Parse '<prefix> <number>' from the last matching stdout line."""
    for line in reversed(stdout.splitlines()):
        line = line.strip()
        if line.startswith(prefix):
            try:
                return int(float(line[len(prefix):].split()[0]))
            except ValueError:
                continue
    return None


# ---------------------------------------------------------------------------
# Panel 1: per-language profiler heap
# ---------------------------------------------------------------------------

def profile_cpp(n_label, csv_path, evidence):
    """Massif on the existing batch mode. Returns (bytes, tool) or None."""
    massif_out = os.path.join(MEM, f"massif-{n_label}.out")
    scratch = os.path.join(MEM, "scratch.csv")
    cmd = ["valgrind", "--tool=massif",
           f"--massif-out-file={massif_out}",
           os.path.join(ROOT, "build", "cpp", "demo"),
           "--benchmark-csv", scratch, csv_path]
    rc, out, err = run(cmd, cwd=ROOT, timeout=1800)
    if rc != 0 or not os.path.exists(massif_out):
        print(f"warn: massif failed for n={n_label}: {err[-400:]}", file=sys.stderr)
        return None
    # Peak = max over snapshots of heap + heap_extra (+ stacks if present).
    # massif-out format: "#-----------\nsnapshot=N\n#-----------\n" then the
    # snapshot's key=value lines. Track per-snapshot values and fold on each
    # new snapshot marker (and at EOF for the final one).
    peak = 0
    heap = extra = stacks = 0.0
    with open(massif_out, "r", encoding="utf-8") as f:
        for line in f:
            if line.startswith("snapshot="):
                peak = max(peak, int(heap + extra + stacks))
            elif line.startswith("mem_heap_B="):
                heap = float(line.split("=")[1])
            elif line.startswith("mem_heap_extra_B="):
                extra = float(line.split("=")[1])
            elif line.startswith("mem_stacks_B="):
                stacks = float(line.split("=")[1])
    peak = max(peak, int(heap + extra + stacks))
    # Headless text evidence: ms_print rendering of the massif graph.
    rc2, msprint, _ = run(["ms_print", massif_out], timeout=120)
    if rc2 == 0:
        with open(os.path.join(MEM, f"massif-{n_label}-msprint.txt"), "w",
                  encoding="utf-8") as f:
            f.write(msprint)
    else:
        with open(os.path.join(MEM, f"massif-{n_label}-msprint.txt"), "w",
                  encoding="utf-8") as f:
            f.write(f"(ms_print unavailable; raw massif-out kept)\n{err}")
    evidence.append(f"massif peak = {peak} bytes (heap+overhead+stacks)")
    return peak


def profile_python(n_label, csv_path, evidence):
    out_txt = os.path.join(MEM, f"tracemalloc-{n_label}.txt")
    rc, out, err = run([sys.executable, os.path.join(MEM, "profile_python.py"),
                        csv_path, out_txt], cwd=ROOT, timeout=600)
    peak = parse_last(out, "peak_tracemalloc_bytes")
    if rc != 0 or peak is None:
        print(f"warn: tracemalloc failed for n={n_label}: {err[-400:]}", file=sys.stderr)
        return None
    evidence.append(f"tracemalloc peak = {peak} bytes (Python-side allocations)")
    return peak


def profile_go(n_label, csv_path, evidence, go_tool):
    out_pprof = os.path.join(MEM, f"go-{n_label}.pprof")
    env = dict(os.environ, MEMPROFILE_CSV=csv_path, MEMPROFILE_OUT=out_pprof)
    rc, out, err = run(["go", "test", "-v", "-run", "TestMemProfile"],
                       cwd=os.path.join(ROOT, "go", "phonebook"),
                       env=env, timeout=600)
    peak = parse_last(out, "peak_go_heap_bytes")
    if rc != 0 or peak is None:
        print(f"warn: go memprofile failed for n={n_label}: {err[-400:]}", file=sys.stderr)
        return None
    # Human-readable pprof top as evidence.
    rc2, top, _ = run(["go", "tool", "pprof", "-top", "-nodecount=10", out_pprof])
    with open(os.path.join(MEM, f"go-{n_label}-pprof-top.txt"), "w",
              encoding="utf-8") as f:
        f.write(out)
        f.write("\n--- go tool pprof -top ---\n")
        f.write(top if rc2 == 0 else "(pprof -top failed)")
    evidence.append(f"go HeapAlloc live after GC = {peak} bytes")
    return peak


def profile_js(n_label, csv_path, evidence):
    out_txt = os.path.join(MEM, f"js-{n_label}.txt")
    heapprofile = os.path.join(MEM, f"js-{n_label}.heapprofile")
    rc, out, err = run(["node", "--heap-prof",
                        f"--heap-prof-dir={MEM}",
                        f"--heap-prof-name=js-{n_label}.heapprofile",
                        os.path.join(MEM, "profile_js.js"), csv_path, out_txt],
                       cwd=ROOT, timeout=600)
    peak = parse_last(out, "peak_js_heap_bytes")
    if rc != 0 or peak is None:
        print(f"warn: js memprofile failed for n={n_label}: {err[-400:]}", file=sys.stderr)
        return None
    evidence.append(f"V8 heapUsed after forced GC = {peak} bytes "
                    f"(DevTools .heapprofile: js-{n_label}.heapprofile)")
    return peak


def profile_java(n_label, csv_path, evidence, java_cmd):
    out_txt = os.path.join(MEM, f"java-{n_label}.txt")
    jfr = os.path.join(MEM, f"java-{n_label}.jfr")
    # java_cmd = ["java", "-cp", <out>] — JVM flags must precede -cp/class.
    cmd = java_cmd[:1] + [f"-XX:StartFlightRecording=filename={jfr},dumponexit=true"] \
        + java_cmd[1:] + ["com.phonebook.MemProfile", csv_path]
    rc, out, err = run(cmd, cwd=ROOT, timeout=600)
    peak = parse_last(out, "peak_java_heap_bytes")
    if rc != 0 or peak is None:
        print(f"warn: java memprofile failed for n={n_label}: {err[-400:]}", file=sys.stderr)
        return None
    # JFR summary as headless VisualVM evidence.
    rc2, summary, _ = run(["jfr", "summary", jfr])
    with open(os.path.join(MEM, f"java-{n_label}-jfr-summary.txt"), "w",
              encoding="utf-8") as f:
        f.write(out)
        f.write("\n--- jfr summary ---\n")
        f.write(summary if rc2 == 0 else "(jfr tool unavailable)")
    evidence.append(f"polled peak used heap = {peak} bytes "
                    f"(VisualVM-readable JFR: java-{n_label}.jfr)")
    return peak


# ---------------------------------------------------------------------------
# Panel 2: kernel-measured peak RSS of the same batch program
# ---------------------------------------------------------------------------

RSS_BATCH = {
    "cpp": [os.path.join(ROOT, "build", "cpp", "demo"), "--benchmark-csv"],
    "python": [sys.executable, os.path.join(ROOT, "python", "phonebook", "main.py"),
               "--benchmark-csv"],
    "go": None,  # filled with prebuilt binary path at call time
    "js": ["node", os.path.join(ROOT, "javascript", "phonebook", "src", "main.js"),
           "--benchmark-csv"],
    "java": None,  # filled with java -cp out + com.phonebook.Main at call time
}


def measure_rss(lang, n_label, csv_path, go_bin, java_cmd):
    """Run the language's batch mode; return peak RSS bytes via wait4."""
    scratch = os.path.join(MEM, "scratch.csv")
    if lang == "go":
        cmd = [go_bin, "--benchmark-csv", scratch, csv_path]
    elif lang == "java":
        # java_cmd is the JVM prefix ["java", "-cp", out]; the main class
        # for batch mode is com.phonebook.Main.
        cmd = java_cmd + ["com.phonebook.Main", "--benchmark-csv", scratch, csv_path]
    else:
        cmd = RSS_BATCH[lang] + [scratch, csv_path]

    pid = os.fork()
    if pid == 0:
        devnull = os.open(os.devnull, os.O_WRONLY)
        os.dup2(devnull, 1)
        os.dup2(devnull, 2)
        try:
            os.execvp(cmd[0], cmd)
        except Exception:
            os._exit(127)
    _, status, usage = os.wait4(pid, 0)
    if not (os.WIFEXITED(status) and os.WEXITSTATUS(status) == 0):
        print(f"warn: rss batch run failed for {lang} n={n_label} "
              f"(status {status})", file=sys.stderr)
        return None
    rss_kib = usage.ru_maxrss  # KiB on Linux
    with open(os.path.join(MEM, f"rss-{lang}-{n_label}.txt"), "w",
              encoding="utf-8") as f:
        f.write(f"peak RSS (ru_maxrss) = {rss_kib} KiB = "
                f"{rss_kib / 1024:.2f} MB\ncommand: {' '.join(cmd)}\n")
    return rss_kib * 1024


# ---------------------------------------------------------------------------
# Build steps
# ---------------------------------------------------------------------------

def build_all():
    """Build C++ demo, Go binary, Java classes. Returns (go_bin, java_cmd)."""
    rc, _, err = run(["make"], cwd=ROOT, timeout=600)
    if rc != 0:
        print(f"warn: make failed: {err[-400:]}", file=sys.stderr)

    go_bin = os.path.join(MEM, "phonebook-go")
    os.makedirs(MEM, exist_ok=True)
    rc, _, err = run(["go", "build", "-o", go_bin, "."],
                     cwd=os.path.join(ROOT, "go", "phonebook"), timeout=600)
    if rc != 0:
        print(f"warn: go build failed: {err[-400:]}", file=sys.stderr)
        go_bin = None

    java_out = os.path.join(ROOT, "java", "phonebook", "out")
    sources = sorted(glob.glob(os.path.join(ROOT, "java", "phonebook", "src",
                                            "com", "phonebook", "*.java")))
    rc, _, err = run(["javac", "-d", java_out] + sources, timeout=600)
    if rc != 0:
        print(f"warn: javac failed: {err[-400:]}", file=sys.stderr)
    # Base command only; callers append their own main class
    # (com.phonebook.Main for batch RSS, com.phonebook.MemProfile for JFR).
    java_cmd = ["java", "-cp", java_out]
    return go_bin, java_cmd


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sizes", default="50,10k,100k,200k,500k,1m",
                    help="comma list of dataset labels (default: all 6)")
    ap.add_argument("--skip-rss", action="store_true",
                    help="skip the kernel peak-RSS panel")
    args = ap.parse_args()

    labels = [s.strip() for s in args.sizes.split(",") if s.strip()]
    for label in labels:
        if label not in SIZES:
            print(f"error: unknown size label {label!r} (known: {', '.join(SIZES)})",
                  file=sys.stderr)
            return 2

    # Datasets must exist (seeded generator, same as run-benchmark-sizes).
    rc, _, _ = run([sys.executable, os.path.join(ROOT, "data", "generate.py"),
                    "--seed", "42"], cwd=ROOT, timeout=600)
    if rc != 0:
        print("error: dataset generation failed", file=sys.stderr)
        return 1

    go_bin, java_cmd = build_all()
    if not go_bin:
        print("error: go binary missing; cannot profile go", file=sys.stderr)
        return 1
    os.makedirs(MEM, exist_ok=True)

    rows = []
    t0 = time.time()
    for label in labels:
        n = SIZES[label]
        csv_path = os.path.join(ROOT, "data", f"contacts_{label}.csv")
        print(f"== n={label} ({n} rows) ==", flush=True)
        for lang in LANGS:
            evidence = []
            if lang == "cpp":
                peak = profile_cpp(label, csv_path, evidence)
                tool = "valgrind/massif"
            elif lang == "python":
                peak = profile_python(label, csv_path, evidence)
                tool = "tracemalloc"
            elif lang == "go":
                peak = profile_go(label, csv_path, evidence, "go")
                tool = "pprof (HeapAlloc live, GC on)"
            elif lang == "js":
                peak = profile_js(label, csv_path, evidence)
                tool = "node --heap-prof (V8 heapUsed)"
            else:
                peak = profile_java(label, csv_path, evidence, java_cmd)
                tool = "JFR + polled heap"
            if peak is not None:
                rows.append([lang, f"data/contacts_{label}.csv", n,
                             "profiler_heap", f"{peak / 1048576:.3f}", tool])
            for line in evidence:
                print(f"  [{lang}] {line}", flush=True)

            if not args.skip_rss:
                rss = measure_rss(lang, label, csv_path, go_bin, java_cmd)
                if rss is not None:
                    rows.append([lang, f"data/contacts_{label}.csv", n,
                                 "peak_rss", f"{rss / 1048576:.3f}", "kernel wait4/ru_maxrss"])
                    print(f"  [{lang}] peak RSS = {rss / 1048576:.2f} MB", flush=True)

    with open(RESULTS, "w", encoding="utf-8", newline="") as f:
        f.write(HEADER + "\n")
        w = csv.writer(f)
        w.writerows(rows)

    print(f"\nWrote {len(rows)} rows -> {RESULTS} "
          f"({time.time() - t0:.0f}s total).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
