CC=g++
CCFLAGS=--std=c++20 -O2

bas2uef: main.cpp tokeniser.cpp
	$(CC) $(CCFLAGS) -o bas2uef main.cpp tokeniser.cpp -I. -Wall
