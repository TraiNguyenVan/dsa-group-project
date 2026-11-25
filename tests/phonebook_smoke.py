"""Run the actual phonebook CLI and benchmark for each implementation."""

import argparse
import csv
import math
import re
import shutil
import subprocess
import sys
from collections import Counter
from pathlib import Path
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parents[1]
SAMPLE = ROOT / "data" / "contacts_50.csv"
LANGUAGES = ("cpp", "python", "go", "js", "java")
HEADER = ("language", "dataset", "n", "case", "algo", "run", "ms",
          "timestamp", "toolchain", "target_index", "phone")


def run(command, cwd=ROOT, inputs=None):
    result = subprocess.run(
        [str(arg) for arg in command], cwd=cwd, input=inputs,
        capture_output=True, text=True, timeout=120,
    )
    if result.returncode:
        raise RuntimeError(
            f"{command!r} exited with {result.returncode}\n"
            f"{result.stdout}\n{result.stderr}"
        )
    return result.stdout


def cli(language, tmp, cpp_binary):
    if language == "cpp":
        return [cpp_binary], ROOT
    if language == "python":
        return [sys.executable, ROOT / "python/phonebook/main.py"], ROOT
    if language == "go":
        cwd = ROOT / "go/phonebook"
        binary = tmp / "phonebook-go"
        run(["go", "build", "-o", binary, "."], cwd)
        return [binary], cwd
    if language == "js":
        cwd = ROOT / "javascript/phonebook"
        return ["node", "src/main.js"], cwd
    cwd = ROOT / "java/phonebook"
    classes = tmp / "java-classes"
    classes.mkdir()
    run(["javac", "-d", classes, *sorted((cwd / "src").rglob("*.java"))], cwd)
    return ["java", "-cp", classes, "com.phonebook.Main"], cwd


def check_cli(command, cwd, dataset, saved, contacts):
    phone = contacts[0][1]
    # Load, count, exercise each search, miss, insert/delete, then save and exit.
    inputs = ["1", "7"]
    for algorithm in ("1", "2", "3"):
        inputs.extend(["4", algorithm, phone])
    inputs.extend([
        "4", "2", "0000000000", "3", "Smoke Test Contact", "0999999999",
        "7", "8", "0999999999", "7", "2", "9", "",
    ])
    output = run([*command, dataset, saved], cwd, "\n".join(inputs))
    required = (
        "Successfully loaded 50 contacts.", "Phone number not found.",
        "Contact inserted successfully.", "Contact deleted successfully.",
        "Contacts saved successfully.", "Goodbye",
    )
    for message in required:
        if message not in output:
            raise ValueError(f"Missing {message!r} in CLI output:\n{output}")
    if re.findall(r"Contact index: (\d+)", output) != ["0"] * 3:
        raise ValueError(f"Not all three search algorithms found index 0:\n{output}")
    if re.findall(r"Number of contacts: (\d+)", output) != ["50", "51", "50"]:
        raise ValueError(f"Unexpected contact counts:\n{output}")
    with saved.open(newline="", encoding="utf-8") as handle:
        if list(csv.reader(handle)) != contacts:
            raise ValueError("Saved contacts differ from the original fixture")


def check_benchmark(command, cwd, dataset, out, contacts, language):
    run([*command, "--benchmark-csv", out, dataset], cwd)
    run([*command, "--benchmark-csv", out, "--append", dataset], cwd)
    cases = {
        "first": (0, contacts[0][1]),
        "middle": (len(contacts) // 2, contacts[len(contacts) // 2][1]),
        "last": (len(contacts) - 1, contacts[-1][1]),
        "miss": (-1, "0000000000"),
    }
    interactive = run([*command, dataset], cwd, "0\n9\n")
    expected_indexes = [index for index, _ in cases.values() for _ in range(3)]
    indexes = [int(index) for index in re.findall(r"ms\. \(index (-?\d+)", interactive)]
    if indexes != expected_indexes:
        raise ValueError(f"Unexpected interactive benchmark indexes:\n{interactive}")

    counts = Counter()
    with out.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if tuple(reader.fieldnames or ()) != HEADER:
            raise ValueError(f"Unexpected benchmark columns: {reader.fieldnames}")
        for row in reader:
            index, phone = cases[row["case"]]
            if (None in row or row["dataset"] != str(dataset) or
                    row["language"] != language or int(row["n"]) != len(contacts) or
                    int(row["target_index"]) != index or row["phone"] != phone or
                    not math.isfinite(float(row["ms"])) or float(row["ms"]) < 0):
                raise ValueError(f"Malformed benchmark row: {row}")
            counts[row["case"], row["algo"], int(row["run"])] += 1
    expected = Counter({
        (case, algo, repeat): 2
        for case in cases
        for algo in ("linear", "hash", "binary")
        for repeat in range(1, 6)
    })
    if counts != expected:
        raise ValueError(f"Expected 60 benchmark rows plus the same 60 appended: {counts}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("languages", nargs="*", metavar="LANG",
                        help="cpp, python, go, js, java (default: all)")
    parser.add_argument("--cpp-binary", type=Path, default=ROOT / "build/cpp/demo")
    args = parser.parse_args()
    languages = args.languages or LANGUAGES
    if set(languages) - set(LANGUAGES):
        parser.error("languages must be chosen from " + ", ".join(LANGUAGES))
    with SAMPLE.open(newline="", encoding="utf-8") as handle:
        contacts = list(csv.reader(handle))
    with TemporaryDirectory(prefix="phonebook-smoke-") as directory:
        tmp = Path(directory)
        dataset = tmp / "contacts, sample.csv"
        shutil.copyfile(SAMPLE, dataset)
        for language in dict.fromkeys(languages):
            command, cwd = cli(language, tmp, args.cpp_binary.resolve())
            check_cli(command, cwd, dataset, tmp / f"{language}-saved.csv", contacts)
            check_benchmark(command, cwd, dataset, tmp / f"{language}-bench.csv",
                            contacts, language)
            print(f"{language}: CLI and benchmark passed", flush=True)


if __name__ == "__main__":
    main()
