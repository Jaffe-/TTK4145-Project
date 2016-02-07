TARGET=elevator
CC = g++
CFLAGS =-Wall -pedantic -std=c++11
LDFLAGS =
MODULES = ./ $(shell ls -d */)
CPPSRC = $(foreach m, $(MODULES), $(wildcard $(m)*.cpp))
HEADERS = $(foreach m, $(MODULES), $(wildcard $(m)*.hpp))
OBJ = $(CPPSRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
