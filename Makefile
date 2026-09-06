# Makefile now is less vibecoded and clean :)

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -O2 -I.

BUILDDIR := build/cpp
TARGET   := $(BUILDDIR)/main
SRC      := main.cpp

# No headers now yet
# HEADERS  := $(wildcard include/*.hpp ) 

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)
	
clean:
	rm -rf $(BUILDDIR)

.PHONY: run clean