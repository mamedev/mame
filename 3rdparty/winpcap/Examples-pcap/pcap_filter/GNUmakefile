# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = pcap_filter.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o pf.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} pf.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
