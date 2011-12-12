#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <pcap.h>

static int (*pcap_compile_dl)(pcap_t *, struct bpf_program *, char *, int, bpf_u_int32) = NULL;
static int (*pcap_findalldevs_dl)(pcap_if_t **, char *) = NULL;
static pcap_t *(*pcap_open_live_dl)(const char *name, int, int, int, char *) = NULL;
static int (*pcap_next_ex_dl)(pcap_t *, struct pcap_pkthdr **, const u_char **) = NULL;
static void (*pcap_close_dl)(pcap_t *) = NULL;
static int (*pcap_setfilter_dl)(pcap_t *, struct bpf_program *) = NULL;
static int (*pcap_sendpacket_dl)(pcap_t *, u_char *, int) = NULL;
static int (*pcap_set_datalink_dl)(pcap_t *, int) = NULL;

#include "emu.h"
#include "osdnet.h"

class netdev_pcap : public netdev
{
public:
	netdev_pcap(const char *name, class device_network_interface *ifdev, int rate);
	~netdev_pcap();

	int send(UINT8 *buf, int len);
	void set_mac(const char *mac);
protected:
	int recv_dev(UINT8 **buf);
private:
	pcap_t *m_p;
};

netdev_pcap::netdev_pcap(const char *name, class device_network_interface *ifdev, int rate)
	: netdev(ifdev, rate)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	m_p = pcap_open_live_dl(name, 65535, 1, 1, errbuf);
	if(!m_p)
	{
		logerror("Unable to open %s: %s\n", name, errbuf);
		return;
	}
	if(pcap_set_datalink_dl(m_p, DLT_EN10MB) == -1)
	{
		logerror("Unable to set %s to ethernet", name);
		pcap_close_dl(m_p);
		m_p = NULL;
		return;
	}
	set_mac(get_mac());
}

void netdev_pcap::set_mac(const char *mac)
{
	char filter[256];
	struct bpf_program fp;
	if(!m_p) return;
	sprintf(filter, "ether dst %.2X:%.2X:%.2X:%.2X:%.2X:%.2X or ether multicast or ether broadcast", (unsigned char)mac[0], (unsigned char)mac[1], (unsigned char)mac[2],(unsigned char)mac[3], (unsigned char)mac[4], (unsigned char)mac[5]);
	pcap_compile_dl(m_p, &fp, filter, 1, 0);
	pcap_setfilter_dl(m_p, &fp);
}

int netdev_pcap::send(UINT8 *buf, int len)
{
	if(!m_p) return 0;
	return (!pcap_sendpacket_dl(m_p, buf, len))?len:0;
}

int netdev_pcap::recv_dev(UINT8 **buf)
{
	struct pcap_pkthdr *header;
	if(!m_p) return 0;
	return (pcap_next_ex_dl(m_p, &header, (const u_char **)buf) == 1)?header->len:0;
}

netdev_pcap::~netdev_pcap()
{
	if(m_p) pcap_close_dl(m_p);
}

static CREATE_NETDEV(create_pcap)
{
	class netdev_pcap *dev = global_alloc(netdev_pcap(ifname, ifdev, rate));
	return dynamic_cast<netdev *>(dev);
}

static HMODULE handle = NULL;

void init_pcap()
{
	pcap_if_t *devs;
	char errbuf[PCAP_ERRBUF_SIZE];
	handle = NULL;

	try
	{
		if(!(handle = LoadLibrary(L"wpcap.dll"))) throw GetLastError();
		if(!(pcap_findalldevs_dl = (int (*)(pcap_if_t **, char *))GetProcAddress(handle, "pcap_findalldevs")))
			throw GetLastError();
		if(!(pcap_open_live_dl = (pcap_t* (*)(const char *, int, int, int, char *))GetProcAddress(handle, "pcap_open_live")))
			throw GetLastError();
		if(!(pcap_next_ex_dl = (int (*)(pcap_t *, struct pcap_pkthdr **, const u_char **))GetProcAddress(handle, "pcap_next_ex")))
			throw GetLastError();
		if(!(pcap_compile_dl = (int (*)(pcap_t *, struct bpf_program *, char *, int, bpf_u_int32))GetProcAddress(handle, "pcap_compile")))
			throw GetLastError();
		if(!(pcap_close_dl = (void (*)(pcap_t *))GetProcAddress(handle, "pcap_close")))
			throw GetLastError();
		if(!(pcap_setfilter_dl = (int (*)(pcap_t *, struct bpf_program *))GetProcAddress(handle, "pcap_setfilter")))
			throw GetLastError();
		if(!(pcap_sendpacket_dl = (int (*)(pcap_t *, u_char *, int))GetProcAddress(handle, "pcap_sendpacket")))
			throw GetLastError();
		if(!(pcap_set_datalink_dl = (int (*)(pcap_t *, int))GetProcAddress(handle, "pcap_set_datalink")))
			throw GetLastError();
	}
	catch (DWORD e)
	{
		FreeLibrary(handle);
		mame_printf_verbose("Unable to load winpcap: %lx\n", e);
		return;
	}
	if(pcap_findalldevs_dl(&devs, errbuf) == -1)
	{
		FreeLibrary(handle);
		mame_printf_verbose("Unable to get network devices: %s\n", errbuf);
		return;
	}

	if (devs)
	{
		while(devs->next)
		{
			add_netdev(devs->name, devs->description, create_pcap);
			devs = devs->next;
		}
	}
}

void deinit_pcap()
{
	clear_netdev();
	FreeLibrary(handle);
}

