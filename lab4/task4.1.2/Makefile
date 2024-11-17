CXXC = g++
CXXFLAGS = -Wall -O2 -g
EXECUTABLE = ./task4.1.2
OBJECTS = task4.1.2.cpp
OUTFILE = output.txt

.PHONY: all, build, clean, run, op

$(EXECUTABLE):
	$(CXXC) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)

all: clean build run

build: clean $(EXECUTABLE)

clean:
	rm -rf $(EXECUTABLE)

run:
	$(EXECUTABLE)
op:
	cat output.txt