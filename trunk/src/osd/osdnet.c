#include "emu.h"
#include "osdnet.h"

static class simple_list<netdev_entry_t> netdev_list;

void add_netdev(const char *name, const char *description, create_netdev func)
{
	netdev_entry_t *entry = global_alloc_clear(netdev_entry_t);
	entry->id = netdev_list.count();
	strncpy(entry->name, name, 255);
	entry->name[255] = '\0';
	strncpy(entry->description, description, 255);
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
	while((len = recv_dev(&buf)))
	{
		if(buf[0] & 1)
		{
			if(memcmp("\xff\xff\xff\xff\xff\xff", buf, 6) && !m_dev->mcast_chk(buf, len)) continue;
		}
		else
			if(memcmp(get_mac(), buf, 6) && !get_promisc()) continue;

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
