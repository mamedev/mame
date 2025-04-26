// license:BSD-3-Clause
// copyright-holders:Carl

#include "netdev_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_NET_USE_PCAP)

#include "netdev_common.h"

#include "modules/lib/osdlib.h"

#include "osdcore.h" // osd_printf_*

#include "util/strformat.h" // string_format

#include <memory>
#include <vector>

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS)
#include <windows.h>
#undef interface
#define LIB_NAME    "wpcap.dll"

#elif defined(SDLMAME_MACOSX)
#include <atomic>
#include <pthread.h>
#define LIB_NAME    "libpcap.dylib"

#else
#define LIB_NAME    "libpcap.so"
#endif

#include <pcap.h>

namespace osd {

namespace {

// Typedefs for dynamically loaded functions
typedef int (*pcap_findalldevs_fn)(pcap_if_t **, char *);
typedef pcap_t *(*pcap_open_live_fn)(const char *, int, int, int, char *);
typedef int (*pcap_next_ex_fn)(pcap_t *, struct pcap_pkthdr **, const u_char **);
typedef int (*pcap_compile_fn)(pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32);
typedef void (*pcap_close_fn)(pcap_t *);
typedef int (*pcap_setfilter_fn)(pcap_t *, struct bpf_program *);
typedef int (*pcap_sendpacket_fn)(pcap_t *, const u_char *, int);
typedef int (*pcap_set_datalink_fn)(pcap_t *, int);
typedef int (*pcap_dispatch_fn)(pcap_t *, int, pcap_handler, u_char *);

class pcap_module : public osd_module, public netdev_module
{
public:
	pcap_module() :
		osd_module(OSD_NETDEV_PROVIDER, "pcap"), netdev_module(),
		pcap_findalldevs_dl(nullptr), pcap_open_live_dl(nullptr), pcap_next_ex_dl(nullptr), pcap_compile_dl(nullptr),
		pcap_close_dl(nullptr), pcap_setfilter_dl(nullptr), pcap_sendpacket_dl(nullptr), pcap_set_datalink_dl(nullptr), pcap_dispatch_dl(nullptr)
	{
	}

	virtual ~pcap_module() { }

	virtual int init(osd_interface &osd, const osd_options &options) override;
	virtual void exit() override;

	virtual bool probe() override
	{
		pcap_dll = osd::dynamic_module::open({ LIB_NAME });

		pcap_findalldevs_dl = pcap_dll->bind<pcap_findalldevs_fn>("pcap_findalldevs");
		pcap_open_live_dl = pcap_dll->bind<pcap_open_live_fn>("pcap_open_live");
		pcap_next_ex_dl = pcap_dll->bind<pcap_next_ex_fn>("pcap_next_ex");
		pcap_compile_dl = pcap_dll->bind<pcap_compile_fn>("pcap_compile");
		pcap_close_dl = pcap_dll->bind<pcap_close_fn>("pcap_close");
		pcap_setfilter_dl = pcap_dll->bind<pcap_setfilter_fn>("pcap_setfilter");
		pcap_sendpacket_dl = pcap_dll->bind<pcap_sendpacket_fn>("pcap_sendpacket");
		pcap_set_datalink_dl = pcap_dll->bind<pcap_set_datalink_fn>("pcap_set_datalink");
		pcap_dispatch_dl = pcap_dll->bind<pcap_dispatch_fn>("pcap_dispatch");

		if (!pcap_findalldevs_dl || !pcap_open_live_dl    || !pcap_next_ex_dl   ||
			!pcap_compile_dl     || !pcap_close_dl        || !pcap_setfilter_dl ||
			!pcap_sendpacket_dl  || !pcap_set_datalink_dl || !pcap_dispatch_dl)
		{
			osd_printf_verbose("Unable to load the PCAP library\n");
			return false;
		}

		return true;
	}

	virtual std::unique_ptr<network_device> open_device(int id, network_handler &handler) override;
	virtual std::vector<network_device_info> list_devices() override;

private:
	struct device_info
	{
		std::string name;
		std::string description;
	};

	class netdev_pcap;

	std::vector<device_info> m_devices;

	osd::dynamic_module::ptr pcap_dll;

	pcap_findalldevs_fn  pcap_findalldevs_dl;
	pcap_open_live_fn    pcap_open_live_dl;
	pcap_next_ex_fn      pcap_next_ex_dl;
	pcap_compile_fn      pcap_compile_dl;
	pcap_close_fn        pcap_close_dl;
	pcap_setfilter_fn    pcap_setfilter_dl;
	pcap_sendpacket_fn   pcap_sendpacket_dl;
	pcap_set_datalink_fn pcap_set_datalink_dl;
	pcap_dispatch_fn     pcap_dispatch_dl;
};

class pcap_module::netdev_pcap : public network_device_base
{
public:
	netdev_pcap(pcap_module &module, const char *name, network_handler &handler);
	~netdev_pcap();

	virtual int send(void const *buf, int len) override;

protected:
	virtual int recv_dev(uint8_t **buf) override;

private:
	pcap_module &m_module;
	pcap_t *m_p;
#ifdef SDLMAME_MACOSX
	pthread_t m_thread;
	uint8_t *pkt;
	int len;
	pcap_t *p;
	uint8_t m_pktbuf[2048];

	uint8_t m_packets[32][1600];
	int m_packetlens[32];
	std::atomic<int> m_head;
	std::atomic<int> m_tail;

	static void pcap_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes);
	static void *pcap_blocker(void *arg);
#endif
};

#ifdef SDLMAME_MACOSX
void pcap_module::netdev_pcap::pcap_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
	netdev_pcap *const ctx = reinterpret_cast<netdev_pcap *>(user);

	if (!ctx->p)
		return;

	int const curr = ctx->m_head.load(std::memory_order_relaxed);
	int const next = (curr + 1) & 0x1f;
	if (ctx->m_tail.load(std::memory_order_acquire) == next)
	{
		printf("buffer full, dropping packet\n");
		return;
	}
	memcpy(ctx->m_packets[curr], bytes, h->len);
	ctx->m_packetlens[curr] = h->len;
	ctx->m_head.store(next, std::memory_order_release);
}

void *pcap_module::netdev_pcap::pcap_blocker(void *arg)
{
	netdev_pcap *const ctx = reinterpret_cast<netdev_pcap *>(arg);

	while (ctx && ctx->p)
		(*ctx->m_module.pcap_dispatch_dl)(ctx->p, 1, &netdev_pcap::pcap_handler, reinterpret_cast<u_char *>(ctx));

	return nullptr;
}
#endif

pcap_module::netdev_pcap::netdev_pcap(pcap_module &module, const char *name, network_handler &handler) :
	network_device_base(handler),
	m_module(module),
	m_p(nullptr)
{
	char errbuf[PCAP_ERRBUF_SIZE];
#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS)
	m_p = (*m_module.pcap_open_live_dl)(name, 65535, 1, -1, errbuf);
#else
	m_p = (*m_module.pcap_open_live_dl)(name, 65535, 1, 1, errbuf);
#endif
	if(!m_p)
	{
		osd_printf_error("Unable to open %s: %s\n", name, errbuf);
		return;
	}
	if ((*m_module.pcap_set_datalink_dl)(m_p, DLT_EN10MB) == -1)
	{
		osd_printf_error("Unable to set %s to ethernet", name);
		(*m_module.pcap_close_dl)(m_p);
		m_p = nullptr;
		return;
	}

#ifdef SDLMAME_MACOSX
	m_head = 0;
	m_tail = 0;
	p = m_p;
	pthread_create(&m_thread, nullptr, &netdev_pcap::pcap_blocker, this);
#endif
}

int pcap_module::netdev_pcap::send(void const *buf, int len)
{
	if (!m_p)
	{
		printf("send invoked, but no pcap context\n");
		return 0;
	}
	int ret = (*m_module.pcap_sendpacket_dl)(m_p, reinterpret_cast<const u_char *>(buf), len);
	printf("sent packet length %d, returned %d\n", len, ret);
	return ret ? len : 0;
	//return (!pcap_sendpacket_dl(m_p, reinterpret_cast<const u_char *>(buf), len)) ? len : 0;
}

int pcap_module::netdev_pcap::recv_dev(uint8_t **buf)
{
	// no device open?
	if (!m_p)
		return 0;

#ifdef SDLMAME_MACOSX
	// Empty
	int const curr = m_tail.load(std::memory_order_relaxed);
	if (m_head.load(std::memory_order_acquire) == curr)
		return 0;

	memcpy(m_pktbuf, m_packets[curr], m_packetlens[curr]);
	int ret = m_packetlens[curr];
	m_tail.store((curr + 1) & 0x1f, std::memory_order_release);
	*buf = m_pktbuf;
	return ret;
#else
	struct pcap_pkthdr *header;
	return ((*m_module.pcap_next_ex_dl)(m_p, &header, (const u_char **)buf) == 1)?header->len:0;
#endif
}

pcap_module::netdev_pcap::~netdev_pcap()
{
#ifdef SDLMAME_MACOSX
	p = nullptr;
	pthread_cancel(m_thread);
	pthread_join(m_thread, nullptr);
#endif
	if(m_p) (*m_module.pcap_close_dl)(m_p);
	m_p = nullptr;
}

int pcap_module::init(osd_interface &osd, const osd_options &options)
{
	pcap_if_t *devs;
	char errbuf[PCAP_ERRBUF_SIZE];

	if ((*pcap_findalldevs_dl)(&devs, errbuf) == -1)
	{
		osd_printf_error("Unable to get network devices: %s\n", errbuf);
		return 1;
	}

	while (devs)
	{
		m_devices.emplace_back(device_info{ devs->name, devs->description ? devs->description : devs->name });
		devs = devs->next;
	}
	return 0;
}

void pcap_module::exit()
{
	m_devices.clear();
}

std::unique_ptr<network_device> pcap_module::open_device(int id, network_handler &handler)
{
	if ((0 > id) || (m_devices.size() <= id))
		return nullptr;

	return std::make_unique<netdev_pcap>(*this, m_devices[id].name.c_str(), handler);
}

std::vector<network_device_info> pcap_module::list_devices()
{
	std::vector<network_device_info> result;
	result.reserve(m_devices.size());
	for (int id = 0; m_devices.size() > id; ++id)
		result.emplace_back(network_device_info{ id, m_devices[id].description });
	return result;
}

} // anonymous namespace

} // namespace osd

#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(pcap_module, OSD_NETDEV_PROVIDER, "pcap") } }

#endif


MODULE_DEFINITION(NETDEV_PCAP, osd::pcap_module)
