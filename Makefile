CC=g++

#CFLAGS+=-Wall -Werror
CFLAGS+=-g

CFLAGS+=$(shell pkg-config --cflags libpulse)
LDFLAGS+=$(shell pkg-config --libs libpulse)

LDFLAGS+=-lpopt 

EXECUTABLES=pavol

default: $(EXECUTABLES)

pavol: pavol.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-@rm $(EXECUTABLES)

