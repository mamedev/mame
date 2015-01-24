#ifdef SDLMAME_WIN32
#include "../windows/netdev_pcap.c"
#else

#include <pcap.h>
#include "emu.h"
#include "osdnet.h"

#define pcap_compile_dl pcap_compile
#define pcap_findalldevs_dl pcap_findalldevs
#define pcap_open_live_dl pcap_open_live
#define pcap_next_ex_dl pcap_next_ex
#define pcap_close_dl pcap_close
#define pcap_setfilter_dl pcap_setfilter
#define pcap_sendpacket_dl pcap_sendpacket
#define pcap_set_datalink_dl pcap_set_datalink


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
		osd_printf_verbose("Unable to open %s: %s\n", name, errbuf);
		return;
	}
	if(pcap_set_datalink_dl(m_p, DLT_EN10MB) == -1)
	{
		osd_printf_verbose("Unable to set %s to ethernet\n", name);
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
	if(pcap_compile_dl(m_p, &fp, filter, 1, 0) == -1) {
		osd_printf_verbose("Error with pcap_compile\n");
	}
	if(pcap_setfilter_dl(m_p, &fp) == -1) {
		osd_printf_verbose("Error with pcap_setfilter\n");
	}
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

void init_pcap()
{
	pcap_if_t *devs;
	char errbuf[PCAP_ERRBUF_SIZE];
	if(pcap_findalldevs_dl(&devs, errbuf) == -1)
	{
		osd_printf_verbose("Unable to get network devices: %s\n", errbuf);
		return;
	}

    while(devs)
    {
        add_netdev(devs->name, devs->description, create_pcap);
        devs = devs->next;
    }
}

void deinit_pcap()
{
	clear_netdev();
}

#endif  // SDLMAME_WIN32
