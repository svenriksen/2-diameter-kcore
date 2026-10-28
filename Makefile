CXX = g++
CXXFLAGS = -Wall -Werror -std=c++17
PYTHON ?= python3

.PHONY: cpp test clean

cpp:
	$(CXX) main.cpp -o main $(CXXFLAGS)
	./main < inp.txt > out.txt
	rm main

build:
	mkdir -p $@

build/main: main.cpp | build
	$(CXX) $< -o $@ $(CXXFLAGS)

build/main-greedy: main-greedy.cpp | build
	$(CXX) $< -o $@ $(CXXFLAGS)

test: build/main build/main-greedy
	$(PYTHON) tests/run_tests.py build/main build/main-greedy

clean:
	$(RM) main out.txt build/main build/main-greedy
