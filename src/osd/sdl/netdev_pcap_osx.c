#ifdef SDLMAME_WIN32
#include "../windows/netdev_pcap.c"
#else

#include <pcap.h>
#include "emu.h"
#include "osdnet.h"
#include <pthread.h>
#include <libkern/OSAtomic.h>

struct netdev_pcap_context {
	UINT8 *pkt;
	int len;
	pcap_t *p;

	UINT8 packets[32][1600];
	int packetlens[32];
	int head;
	int tail;
};

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
	struct netdev_pcap_context m_ctx;
	pthread_t m_thread;
};

static void netdev_pcap_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes) {
	struct netdev_pcap_context *ctx = (struct netdev_pcap_context*)user;

	if(OSAtomicCompareAndSwapInt((ctx->head+1) & 0x1F, ctx->tail, &ctx->tail)) {
		printf("buffer full, dropping packet\n");
		return;
	}
	memcpy(ctx->packets[ctx->head], bytes, h->len);
	ctx->packetlens[ctx->head] = h->len;
	OSAtomicCompareAndSwapInt(ctx->head, (ctx->head+1) & 0x1F, &ctx->head);
}

static void *netdev_pcap_blocker(void *arg) {
	struct netdev_pcap_context *ctx = (struct netdev_pcap_context*)arg;

	while(1) {
		pcap_dispatch(ctx->p, 1, netdev_pcap_handler, (u_char*)ctx);
	}

	return 0;
}

netdev_pcap::netdev_pcap(const char *name, class device_network_interface *ifdev, int rate)
	: netdev(ifdev, rate)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	m_p = pcap_open_live(name, 65535, 1, 1, errbuf);
	if(!m_p)
	{
		mame_printf_verbose("Unable to open %s: %s\n", name, errbuf);
		return;
	}
	if(pcap_set_datalink(m_p, DLT_EN10MB) == -1)
	{
		mame_printf_verbose("Unable to set %s to ethernet\n", name);
		pcap_close(m_p);
		m_p = NULL;
		return;
	}
	set_mac(get_mac());

	m_ctx.head = 0;
	m_ctx.tail = 0;
	m_ctx.p = m_p;
	pthread_create(&m_thread, NULL, netdev_pcap_blocker, &m_ctx);
}

void netdev_pcap::set_mac(const char *mac)
{
	char filter[256];
	struct bpf_program fp;
	if(!m_p) return;
	sprintf(filter, "not ether src %.2X:%.2X:%.2X:%.2X:%.2X:%.2X and (ether dst %.2X:%.2X:%.2X:%.2X:%.2X:%.2X or ether multicast or ether broadcast or ether dst 09:00:07:ff:ff:ff)", (unsigned char)mac[0], (unsigned char)mac[1], (unsigned char)mac[2],(unsigned char)mac[3], (unsigned char)mac[4], (unsigned char)mac[5], (unsigned char)mac[0], (unsigned char)mac[1], (unsigned char)mac[2],(unsigned char)mac[3], (unsigned char)mac[4], (unsigned char)mac[5]);
	if(pcap_compile(m_p, &fp, filter, 1, 0) == -1) {
		mame_printf_verbose("Error with pcap_compile\n");
	}
	if(pcap_setfilter(m_p, &fp) == -1) {
		mame_printf_verbose("Error with pcap_setfilter\n");
	}
}

int netdev_pcap::send(UINT8 *buf, int len)
{
	if(!m_p) return 0;
	return (!pcap_sendpacket(m_p, buf, len))?len:0;
}

int netdev_pcap::recv_dev(UINT8 **buf)
{
	UINT8 pktbuf[2048];
	int ret;

	// Empty
	if(OSAtomicCompareAndSwapInt(m_ctx.head, m_ctx.tail, &m_ctx.tail)) {
		return 0;
	}

	memcpy(pktbuf, m_ctx.packets[m_ctx.tail], m_ctx.packetlens[m_ctx.tail]);
	ret = m_ctx.packetlens[m_ctx.tail];
	OSAtomicCompareAndSwapInt(m_ctx.tail, (m_ctx.tail+1) & 0x1F, &m_ctx.tail);
	*buf = pktbuf;
	return ret;
}

netdev_pcap::~netdev_pcap()
{
	if(m_p) pcap_close(m_p);
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
	if(pcap_findalldevs(&devs, errbuf) == -1)
	{
		mame_printf_verbose("Unable to get network devices: %s\n", errbuf);
		return;
	}

	if (devs)
	{
		while(devs->next)
		{
			add_netdev(devs->name, devs->description ? devs->description : devs->name, create_pcap);
			devs = devs->next;
		}
	}
}

void deinit_pcap()
{
	clear_netdev();
}

#endif  // SDLMAME_WIN32
