# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = iflist.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o iflist.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} iflist.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
