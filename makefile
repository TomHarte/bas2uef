CC=g++
CCFLAGS=--std=c++20 -O2 -Wall

bas2uef: main.cpp tokeniser.cpp
	$(CC) $(CCFLAGS) -o bas2uef main.cpp tokeniser.cpp -I. -Wall
