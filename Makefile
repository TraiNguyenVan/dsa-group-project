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
BENCHDATA := data/contacts_100k.csv

run-benchmark: $(TARGET)
	mkdir -p benchmark
	rm -f $(BENCHCSV)
	./$(TARGET) --benchmark-csv $(BENCHCSV) $(BENCHDATA)
	python3 python/phonebook/main.py --benchmark-csv $(BENCHCSV) --append $(BENCHDATA)
	cd go/phonebook && go run . --benchmark-csv ../../$(BENCHCSV) --append ../../$(BENCHDATA)
	cd javascript/phonebook && node src/main.js --benchmark-csv ../../$(BENCHCSV) --append ../../$(BENCHDATA)
	cd java/phonebook && javac -d out src/com/phonebook/*.java && java -cp out com.phonebook.Main --benchmark-csv ../../$(BENCHCSV) --append ../../$(BENCHDATA)
	python3 benchmark/plot.py $(BENCHCSV) benchmark/plot.png || echo "plot skipped: pip install matplotlib"

clean:
	rm -rf $(BUILDDIR)

.PHONY: all run run-50 run-100k run-200k run-1m run-benchmark clean


