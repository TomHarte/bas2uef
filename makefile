CC=g++
CCFLAGS=--std=c++20 -g

bas2uef: main.cpp tokeniser.cpp
	$(CC) $(CCFLAGS) -o bas2uef main.cpp tokeniser.cpp -I. -Wall
