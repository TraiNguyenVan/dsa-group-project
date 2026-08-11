CXX      := g++
CXXFLAGS := -std=c++17 -Wall -O2
TARGET   := demo
SRC      := main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Edge cases: empty input, duplicate keys, out-of-range access
edge: $(TARGET)
	./$(TARGET) --empty
	./$(TARGET) --dups
	./$(TARGET) --oob

clean:
	rm -f $(TARGET) *.o

.PHONY: run edge clean
