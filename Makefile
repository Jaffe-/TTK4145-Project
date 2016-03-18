TARGET=elevator
CC = gcc
COMMONFLAGS += -Wall -Wextra -pedantic -MD -MP
CPPFLAGS = $(COMMONFLAGS) -std=c++11
CFLAGS = $(COMMONFLAGS) -std=gnu99
LDFLAGS = -lstdc++ -lcomedi -lpthread -lm
MODULES = network driver driver/hw_interface logic util
SRC = main.cpp $(foreach m, $(MODULES), $(wildcard $(m)/*.cpp)) $(foreach m, $(MODULES), $(wildcard $(m)/*.c))
OBJ = $(SRC:.cpp=.o) $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(CPPSRC:%.cpp=%.d) $(CSRC:%.c=%.d)

clean:
	rm -f $(OBJ) $(SRC:%.cpp=%.d)

.PHONY: all clean
