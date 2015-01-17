# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = savedump.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o savedump.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} savedump.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
