TARGET=elevator
CC = g++
CFLAGS =-Wall -pedantic -std=c++11
LDFLAGS = -Ldriver/hw_interface -linterface -lcomedi -lm -lsimelev -lphobos2 -lpthread
MODULES = network logger cmdopts driver
CPPSRC = main.cpp $(foreach m, $(MODULES), $(wildcard $(m)/*.cpp))
HEADERS = $(foreach m, $(MODULES), $(wildcard $(m)/*.hpp))
OBJ = $(CPPSRC:.cpp=.o)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
