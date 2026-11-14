# Build with:  make        Run app:  make run        Run tests:  make test
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: kpfl kpfl_tests

kpfl: src/main.cpp src/*.h
	$(CXX) $(CXXFLAGS) -o kpfl src/main.cpp

kpfl_tests: src/tests.cpp src/*.h
	$(CXX) $(CXXFLAGS) -o kpfl_tests src/tests.cpp

run: kpfl
	./kpfl

test: kpfl_tests
	./kpfl_tests

clean:
	rm -f kpfl kpfl_tests kpfl_data.dat standings.csv
