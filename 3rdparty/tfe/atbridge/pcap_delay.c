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

#include <stdbool.h>
#include "pcap_delay.h"

#ifdef WIN32
#include <Windows.h>
static HMODULE module = NULL;
#elif __linux__
#include <dlfcn.h>
static void* module = 0;
#endif


bool pcapdelay_load(void)
{
	if (!pcapdelay_is_loaded())
	{
#ifdef WIN32
		module = LoadLibrary("wpcap.dll");
#elif __linux__
		module = dlopen("libpcap.so", RTLD_LAZY);
#endif
	}
	return pcapdelay_is_loaded();
}

bool pcapdelay_is_loaded(void)
{
#ifdef WIN32
	return module != NULL;
#elif __linux__
	return module != 0;
#endif
}

void pcapdelay_unload(void)
{
	if (pcapdelay_is_loaded())
	{
#ifdef WIN32
		FreeLibrary(module);
		module = NULL;
#elif __linux__
		dlclose(module);
		module = 0;
#endif
	}
}

typedef void (*PFNVOID)(void);

static PFNVOID delay_load(const char* proc, PFNVOID* ppfn)
{
	if (pcapdelay_load() && proc && ppfn && !*ppfn)
	{
#ifdef WIN32
		*ppfn = (PFNVOID)GetProcAddress(module, proc);
#elif __linux__
		*ppfn = (PFNVOID)dlsym(module, proc);
#endif
	}
	if (ppfn)
		return *ppfn;
	else
		return 0;
}

void pcapdelay_freealldevs(pcap_if_t* a0)
{
	typedef void (*PFN)(pcap_if_t*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_freealldevs", (PFNVOID*)&pfn)))
		(*pfn)(a0);
}

pcap_t* pcapdelay_open_live(const char* a0, int a1, int a2, int a3, char* a4)
{
	typedef pcap_t* (*PFN)(const char*, int, int, int, char*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_open_live", (PFNVOID*)&pfn)))
		return (*pfn)(a0, a1, a2, a3, a4);
	else
		return 0;
}

void pcapdelay_close(pcap_t* a0)
{
	typedef void (*PFN)(pcap_t*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_close", (PFNVOID*)&pfn)))
		(*pfn)(a0);
}

int	pcapdelay_findalldevs(pcap_if_t** a0, char* a1)
{
	typedef int (*PFN)(pcap_if_t**, char*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_findalldevs", (PFNVOID*)&pfn)))
		return (*pfn)(a0, a1);
	else
		return 0;
}

int	pcapdelay_datalink(pcap_t* a0)
{
	typedef int(*PFN)(pcap_t*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_datalink", (PFNVOID*)&pfn)))
		return (*pfn)(a0);
	else
		return 0;
}

int	pcapdelay_setnonblock(pcap_t* a0, int a1, char* a2)
{
	typedef int(*PFN)(pcap_t*, int, char*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_setnonblock", (PFNVOID*)&pfn)))
		return (*pfn)(a0, a1, a2);
	else
		return 0;
}

int pcapdelay_sendpacket(pcap_t* a0, u_char* a1, int a2)
{
	typedef int(*PFN)(pcap_t*, u_char*, int);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_sendpacket", (PFNVOID*)&pfn)))
		return (*pfn)(a0, a1, a2);
	else
		return 0;
}

const u_char* pcapdelay_next(pcap_t* a0, struct pcap_pkthdr* a1)
{
	typedef const u_char*(*PFN)(pcap_t*, struct pcap_pkthdr*);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_next", (PFNVOID*)&pfn)))
		return (*pfn)(a0, a1);
	else
		return 0;
}

int	pcapdelay_dispatch(pcap_t* a0, int a1, pcap_handler a2, u_char* a3)
{
	typedef const int(*PFN)(pcap_t *, int, pcap_handler, u_char *);
	static PFN pfn = 0;
	if ((pfn = (PFN)delay_load("pcap_dispatch", (PFNVOID*)&pfn)))
		return (*pfn)(a0, a1, a2, a3);
	else
		return 0;
}
