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
	./$(TARGET) data/contacts_200k.csv

run-50: $(TARGET)
	./$(TARGET) data/contacts_50.csv

run-100k: $(TARGET)
	./$(TARGET) data/contacts_100k.csv

run-200k: $(TARGET)
	./$(TARGET) data/contacts_200k.csv

run-1m: $(TARGET)
	./$(TARGET) data/contacts_1m.csv

clean:
	rm -rf $(BUILDDIR)

.PHONY: all run run-50 run-100k run-200k run-1m clean


