CXXC = g++
CXXFLAGS = -Wall -O2 -g
EXECUTABLE = ./task5.1.1
OBJECTS = task5.1.1.cpp
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