TARGET=elevator
CC = gcc
COMMONFLAGS += -Wall -Wextra -pedantic -MD -MP -g
CPPFLAGS = $(COMMONFLAGS) -std=c++14
CFLAGS = $(COMMONFLAGS) -std=gnu99
LDFLAGS = -lstdc++ -lcomedi -lpthread -lm
MODULES = network driver driver/hw_interface dispatch_logic util
CPPSRC = main.cpp $(foreach m, $(MODULES), $(wildcard $(m)/*.cpp))
CSRC = $(foreach m, $(MODULES), $(wildcard $(m)/*.c))
OBJ = $(CPPSRC:%.cpp=%.o) $(CSRC:%.c=%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(CPPSRC:%.cpp=%.d) $(CSRC:%.c=%.d)

clean:
	rm -f $(OBJ) $(CPPSRC:%.cpp=%.d) $(CSRC:%.c=%.d)

.PHONY: all clean
