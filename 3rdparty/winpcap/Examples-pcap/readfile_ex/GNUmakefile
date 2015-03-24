# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = readfile_ex.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o readfile_ex.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} readfile_ex.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
