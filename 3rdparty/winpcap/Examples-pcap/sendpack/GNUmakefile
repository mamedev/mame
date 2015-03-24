# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = sendpack.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o sendpack.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} sendpack.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
