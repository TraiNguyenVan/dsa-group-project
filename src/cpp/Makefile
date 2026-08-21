CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Build output lives at project root, not inside src/
BUILDDIR := ../../build/cpp
TARGET   := $(BUILDDIR)/demo
TEST     := $(BUILDDIR)/test_hash
SRC      := main.cpp

.PHONY: all run edge clean test

all: $(TARGET)

$(TARGET): $(SRC) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(TEST): test_hash.cpp hashing.hpp linear_probing.hpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) test_hash.cpp -o $(TEST)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

test: $(TEST)
	./$(TEST)

# Edge cases — kept for backwards compatibility with the original skeleton
edge: $(TARGET)
	./$(TARGET) --empty
	./$(TARGET) --dups
	./$(TARGET) --oob

clean:
	rm -rf $(BUILDDIR)
