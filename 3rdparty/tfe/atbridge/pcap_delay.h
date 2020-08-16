/*
GSport - an Apple //gs Emulator
Copyright (C) 2014 by Peter Neubauer

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
/*
This interface provides a thin, delay-loaded wrapper around the PCAP library so that
you may start GSport without intalling PCAP.  Of course, some features that require 
PCAP won't be available.  

This wrapper provides a subset of the available PCAP APIs necessary for ATBridge.
Feel free to extend the wrapper.
*/

#ifdef WIN32
#include "../arch/win32/pcap.h"
#elif __linux__
#include <pcap.h>
#endif

bool pcapdelay_load(void);
bool pcapdelay_is_loaded(void);
void pcapdelay_unload(void);

void pcapdelay_freealldevs(pcap_if_t *);
pcap_t* pcapdelay_open_live(const char *, int, int, int, char *);
void pcapdelay_close(pcap_t *);
int	pcapdelay_findalldevs(pcap_if_t **, char *);
int	pcapdelay_datalink(pcap_t *);
int	pcapdelay_setnonblock(pcap_t *, int, char *);
int pcapdelay_sendpacket(pcap_t *p, u_char *buf, int size);
const u_char* pcapdelay_next(pcap_t *, struct pcap_pkthdr *);
int	pcapdelay_dispatch(pcap_t *, int, pcap_handler, u_char *);
