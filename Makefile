CXX      := g++

CXXFLAGS := -std=c++17 -Wall -O2 -Iinclude


SRCDIR   := ./src
BUILDDIR := build/cpp
TARGET   := $(BUILDDIR)/demo
SRC      := $(wildcard $(SRCDIR)/*.cpp)
HEADERS  := $(wildcard include/*.hpp)

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(SRC) $(HEADERS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) data/contacts_100k.csv

run-50: $(TARGET)
	./$(TARGET) data/contacts_50.csv

run-100k: $(TARGET)
	./$(TARGET) data/contacts_100k.csv

run-200k: $(TARGET)
	./$(TARGET) data/contacts_200k.csv

run-1m: $(TARGET)
	./$(TARGET) data/contacts_1m.csv

# Full 5-language benchmark matrix: every run (not just best) -> benchmark/results.csv
# Option 0 in the CLI is untouched; each port's --benchmark-csv batch mode is
# seeded (42) so the `random` target is reproducible. C++ writes header, rest append.
BENCHCSV  := benchmark/results.csv
BENCHDATA := data/contacts_200k.csv
# Multi-size scaling run: nested-prefix datasets, appended into one CSV.
BENCHFILES := data/contacts_50.csv data/contacts_10k.csv data/contacts_100k.csv data/contacts_200k.csv data/contacts_500k.csv data/contacts_1m.csv

run-benchmark: $(TARGET)
	mkdir -p benchmark
	rm -f $(BENCHCSV)
	./$(TARGET) --benchmark-csv $(BENCHCSV) $(BENCHDATA)
	python3 python/phonebook/main.py --benchmark-csv $(BENCHCSV) --append $(BENCHDATA)
	cd go/phonebook && go run . --benchmark-csv ../../$(BENCHCSV) --append ../../$(BENCHDATA)
	cd javascript/phonebook && node src/main.js --benchmark-csv ../../$(BENCHCSV) --append ../../$(BENCHDATA)
	cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main --benchmark-csv ../../$(BENCHCSV) --append ../../$(BENCHDATA)
	python3 benchmark/plot.py $(BENCHCSV) benchmark/plot.png || echo "plot skipped: pip install matplotlib"

# Multi-size scaling benchmark (guide Part D): every size x every language,
# appended into one results.csv so plot.py --line can draw runtime vs n.
run-benchmark-sizes: $(TARGET)
	mkdir -p benchmark
	python3 data/generate.py --seed 42
	rm -f $(BENCHCSV)
	@for f in $(BENCHFILES); do \
		echo "== $$f =="; \
		./$(TARGET) --benchmark-csv $(BENCHCSV) --append $$f; \
		python3 python/phonebook/main.py --benchmark-csv $(BENCHCSV) --append $$f; \
		(cd go/phonebook && go run . --benchmark-csv ../../$(BENCHCSV) --append ../../$$f); \
		(cd javascript/phonebook && node src/main.js --benchmark-csv ../../$(BENCHCSV) --append ../../$$f); \
		(cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main --benchmark-csv ../../$(BENCHCSV) --append ../../$$f); \
	done
	python3 benchmark/plot.py --line $(BENCHCSV) benchmark/plot-runtime-vs-n.png || echo "plot skipped: pip install matplotlib"
	python3 benchmark/plot.py --unified $(BENCHCSV) benchmark/plot-unified-cpp.png || echo "plot skipped: pip install matplotlib"
	python3 benchmark/plot.py --per-algo $(BENCHCSV) benchmark/plot-per-algo-cpp.png || echo "plot skipped: pip install matplotlib"

clean:
	rm -rf $(BUILDDIR)

# Export report/report.md -> report/report.pdf (headless Chromium, no deps)
pdf:
	python3 report/export_pdf.py

.PHONY: all run run-50 run-100k run-200k run-1m run-benchmark run-benchmark-sizes pdf clean


