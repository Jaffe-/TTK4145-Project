CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra
AR = ar
TARGET = libinterface.a
SIMTARGET = libsimelev.a
SRC = elev.c io.c

all: $(TARGET) $(SIMTARGET)

OBJS = $(SRC:.c=.o)

$(TARGET) : $(OBJS)
	$(AR) rs $(TARGET) $(OBJS)

$(SIMTARGET) : sim_backend.d timer_event.d
	dmd sim_backend.d timer_event.d -lib -of$(SIMTARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS)

rebuild:	clean all

.PHONY: all rebuild clean
