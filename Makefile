TARGET=elevator
CC = g++
CFLAGS +=-Wall -Wextra -pedantic -std=c++11 -MD -MP
LDFLAGS = -Ldriver/hw_interface -linterface -lcomedi -lpthread
MODULES = network driver util
CPPSRC = main.cpp $(foreach m, $(MODULES), $(wildcard $(m)/*.cpp))
HEADERS = $(foreach m, $(MODULES), $(wildcard $(m)/*.hpp))
OBJ = $(CPPSRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

-include $(CPPSRC:%.cpp=%.d)

clean:
	rm -f $(OBJ) $(CPPSRC:%.cpp=%.d)

.PHONY: all clean
