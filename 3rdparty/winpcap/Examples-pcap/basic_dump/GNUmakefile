# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = basic_dump.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o basic_dump.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} basic_dump.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
