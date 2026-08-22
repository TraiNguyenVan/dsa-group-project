# Makefile now is less vibecoded and clean :)

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -O2 -I.

BUILDDIR := build/cpp
TARGET   := $(BUILDDIR)/demo
SRC      := main.cpp
HEADERS  := $(wildcard include/*.hpp ) 

all: $(TARGET) 

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: $(TARGET)
	./$(TARGET)
	
clean:
	rm -rf $(BUILDDIR)

.PHONY: all run clean