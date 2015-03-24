PCAP_PATH = ../../lib
CFLAGS = -g -O -mno-cygwin -I ../../include

OBJS = pktdump_ex.o
LIBS = -L ${PCAP_PATH} -lwpcap

all: ${OBJS}
	${CC} ${CFLAGS} -o pktdump_ex.exe ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} pktdump_ex.exe

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
