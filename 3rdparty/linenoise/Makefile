CFLAGS += -Wall -W -Os -g
CC ?= gcc

all:  linenoise_example linenoise_utf8_example

linenoise_example: linenoise.h linenoise-ship.c linenoise-win32.c example.c
	$(CC) $(CFLAGS) -o $@ linenoise-ship.c example.c

linenoise_utf8_example: linenoise.h linenoise-ship.c linenoise-win32.c
	$(CC) $(CFLAGS) -DUSE_UTF8 -o $@ linenoise-ship.c example.c

clean:
	rm -f linenoise_example linenoise_utf8_example linenoise-ship.c *.o

ship: linenoise-ship.c

# linenoise-ship.c simplifies delivery of linenoise support
# simple copy linenoise-ship.c to linenoise.c in your application, and also linenoise.h
# - If you want win32 support, also copy linenoise-win32.c
# - If you never want to support utf-8, you can omit utf8.h and utf8.c

linenoise-ship.c: utf8.h utf8.c stringbuf.h stringbuf.c linenoise.c
	cat $^ >$@
