# Makefile for cygwin gcc
# Loris Degioanni

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = udpdump.o
LIBS = -L ${PCAP_PATH} -lwpcap -lwsock32

all: ${OBJS}
	${CC} ${CFLAGS} -o udpdump.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} udpdump.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
