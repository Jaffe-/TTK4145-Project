CC = gcc
CFLAGS = -std=gnu99 -g -Wall -Wextra
AR = ar
TARGET = libinterface.a
SRC = elev.c io.c

all: $(TARGET) $(SIMTARGET)

OBJS = $(SRC:.c=.o)

$(TARGET) : $(OBJS)
	$(AR) rs $(TARGET) $(OBJS)

%.o: %.c io.h elev.h channels.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

rebuild:	clean all

.PHONY: all rebuild clean
