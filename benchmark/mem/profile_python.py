#!/usr/bin/env python3
"""Peak-memory wrapper for the Python phonebook (report D.5, tracemalloc).

Loads one CSV with the real PhoneBook port (python/phonebook/phonebook.py)
under tracemalloc and prints the exact peak of Python-side allocations.

Usage:
    python3 benchmark/mem/profile_python.py <csv> <evidence-out.txt>

stdout (last line is machine-parsed by benchmark/mem_profile.py):
    peak_tracemalloc_bytes <N>

Evidence file: top allocation sites (lineno stats) filtered to the port.
Stdlib only.
"""

import os
import sys
import tracemalloc

HERE = os.path.dirname(os.path.abspath(__file__))
PORT_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "python", "phonebook"))

sys.path.insert(0, PORT_DIR)
from phonebook import PhoneBook  # noqa: E402


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: profile_python.py <csv> <evidence-out.txt>", file=sys.stderr)
        return 2
    csv_path, evidence_path = sys.argv[1], sys.argv[2]

    tracemalloc.start()
    pb = PhoneBook()
    loaded = pb.loadfrom_csv(csv_path)
    if loaded == -1:
        print(f"Cannot open file: {csv_path}", file=sys.stderr)
        return 1
    current, peak = tracemalloc.get_traced_memory()
    tracemalloc.stop()

    # Evidence: top allocation sites, filtered to the port's own modules.
    lines = [f"tracemalloc top sites (n={loaded} contacts, {csv_path})"]
    try:
        tracemalloc.start()
        pb2 = PhoneBook()
        pb2.loadfrom_csv(csv_path)
        snap = tracemalloc.take_snapshot()
        tracemalloc.stop()
        stats = snap.statistics("lineno")
        shown = 0
        for stat in stats:
            frame = stat.traceback[0]
            if "python" + os.sep + "phonebook" in frame.filename or "/phonebook/" in frame.filename:
                lines.append(f"{stat.size / 1024 / 1024:10.2f} MB  {frame.filename}:{frame.lineno}")
                shown += 1
            if shown >= 10:
                break
        if shown == 0:
            lines.append("(no allocation sites inside python/phonebook — runtime-dominated)")
    except Exception as e:  # evidence is best-effort; peak number already printed
        lines.append(f"(snapshot failed: {e})")

    with open(evidence_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print(f"loaded {loaded} contacts")
    print(f"peak_tracemalloc_bytes {peak}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
