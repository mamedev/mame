#include "emu.h"
#include "osdnet.h"

static class simple_list<netdev_entry_t> netdev_list;

void add_netdev(const char *name, const char *description, create_netdev func)
{
	netdev_entry_t *entry = global_alloc_clear(netdev_entry_t);
	entry->id = netdev_list.count();
	strncpy(entry->name, name, 255);
	entry->name[255] = '\0';
	strncpy(entry->description, (description != NULL) ? description : "(no name)", 255);
	entry->description[255] = '\0';
	entry->func = func;
	netdev_list.append(*entry);
}

void clear_netdev()
{
	netdev_list.reset();
}

const netdev_entry_t *netdev_first() {
	return netdev_list.first();
}

class netdev *open_netdev(int id, class device_network_interface *ifdev, int rate)
{
	netdev_entry_t *entry = netdev_list.first();
	while(entry) {
		if(entry->id==id)
			return entry->func(entry->name, ifdev, rate);
		entry = entry->m_next;
	}

	return NULL;
}

netdev::netdev(class device_network_interface *ifdev, int rate)
{
	m_dev = ifdev;
	ifdev->device().machine().scheduler().timer_pulse(attotime::from_hz(rate), timer_expired_delegate(FUNC(netdev::recv), this));
}

netdev::~netdev()
{
}

int netdev::send(UINT8 *buf, int len)
{
	return 0;
}

void netdev::recv(void *ptr, int param)
{
	UINT8 *buf;
	int len;
	//const char atalkmac[] = { 0x09, 0x00, 0x07, 0xff, 0xff, 0xff };
	while((len = recv_dev(&buf)))
	{
#if 0
		if(buf[0] & 1)
		{
			if(memcmp("\xff\xff\xff\xff\xff\xff", buf, 6) && memcmp(atalkmac, buf, 6) && !m_dev->mcast_chk(buf, len)) continue;
		}
		else {
			//const unsigned char *ourmac = (const unsigned char *)get_mac();
			//printf("our mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X dst mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", ourmac[0], ourmac[1], ourmac[2], ourmac[3], ourmac[4], ourmac[5], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
			if(memcmp(get_mac(), buf, 6) && !get_promisc()) continue;
		}
#endif

		m_dev->recv_cb(buf, len);
	}
}

int netdev::recv_dev(UINT8 **buf)
{
	return 0;
}

void netdev::set_mac(const char *mac)
{
}

void netdev::set_promisc(bool promisc)
{
}

bool netdev::get_promisc()
{
	if(m_dev)
		return m_dev->get_promisc();
	return false;
}

const char *netdev::get_mac()
{
	if(m_dev)
		return m_dev->get_mac();
	return "\0\0\0\0\0\0";
}

int netdev_count()
{
	return netdev_list.count();
}

void osd_list_network_adapters(void)
{
	#ifdef USE_NETWORK
	int num_devs = netdev_list.count();

	if (num_devs == 0)
	{
		printf("No network adapters were found\n");
		return;
	}

	printf("Available network adapters:\n");
	const netdev_entry_t *entry = netdev_first();
	while(entry) {
		printf("    %s\n", entry->description);
		entry = entry->m_next;
	}

	#else
	printf("Network is not supported in this build\n");
	#endif
}
