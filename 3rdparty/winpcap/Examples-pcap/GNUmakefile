# Makefile for cygwin gcc
# Nate Lawson <nate@rootlabs.com>

SUBDIRS = basic_dump basic_dump_ex iflist pcap_filter pktdump_ex readfile readfile_ex savedump sendpack UDPdump

all clean install uninstall: ${SUBDIRS}
	for subdir in ${SUBDIRS}; do \
		echo "Entering $$subdir"; \
		(cd $$subdir && ${MAKE} $@) \
	done;
