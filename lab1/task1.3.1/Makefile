CXXC = g++
CXXFLAGS = -Wall -O2
EXECUTABLE = ./task1.3.1
OBJECTS = task1.3.1.cpp
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
 