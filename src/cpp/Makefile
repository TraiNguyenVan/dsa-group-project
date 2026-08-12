CXX      := g++
CXXFLAGS := -std=c++17 -Wall -O2
BUILDDIR := build
TARGET   := $(BUILDDIR)/demo
SRC      := main.cpp

$(TARGET): $(SRC) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)

# Edge cases: empty input, duplicate keys, out-of-range access
edge: $(TARGET)
	./$(TARGET) --empty
	./$(TARGET) --dups
	./$(TARGET) --oob

clean:
	rm -rf $(BUILDDIR)

.PHONY: run edge clean