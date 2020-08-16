PCAPOBJ = atbridge/pcap_delay.o
TFEOBJ = tfe/tfe.o tfe/tfearch.o tfe/tfesupp.o

include vars

.SUFFIXES: .dep .proto

AS = $(CC)

all: $(TARGET)

clean:
	- rm -f $(OBJECTS)
	- rm -f $(TARGET)

# Linux for X builds:
libtfe.a: $(OBJECTS)
	$(LD) r libtfe.a $(OBJECTS)
	echo $(OBJECTS)

.c.o:
	$(CC) $(CCOPTS) $(XOPTS) -c $(OPTS) -I. -o $*.o $*.c

.cpp.o:
	$(CC) $(CPPOPTS) $(XOPTS) -c $(OPTS) -I. $*.cpp

.cpp.O:
	$(CC) $(CCOPTS) $(XOPTS) -c $(OPTS) -I. $*.c

# dependency stuff
atbridge/pcap_delay.o: atbridge/pcap_delay.h
tfe.o: tfe/tfe.c tfe/tfe.h tfe/tfe_protos.h
tfearch.o: tfe/tfearch.c tfe/tfearch.h tfe/tfe_protos.h
tfesupp.o: tfe/tfesupp.c tfe/tfesupp.h tfe/tfe_protos.h
