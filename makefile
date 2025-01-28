CC=g++
CCFLAGS=--std=c++20 -O2 -Wall

bas2uef: src/main.cpp src/tokeniser.cpp
	$(CC) $(CCFLAGS) -o bas2uef src/*.cpp

clean:
	rm bas2uef
