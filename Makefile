CC := gcc
CFLAGS := -std=c89 -pedantic -Wall -Wextra -g3 -O0
LDFLAGS :=

all: g_unpack

g_unpack: g_unpack.c
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS)

clean:
	rm -f g_unpack *.o

.PHONY: clean
