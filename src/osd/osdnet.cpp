// license:BSD-3-Clause
// copyright-holders:Carl

#include "osdnet.h"

#include "interface/nethandler.h"


static std::vector<std::unique_ptr<osd_network_device::entry_t>> netdev_list;

void add_netdev(const char *name, const char *description, create_netdev func)
{
	auto entry = std::make_unique<osd_network_device::entry_t>();
	entry->id = netdev_list.size();
	strncpy(entry->name, name, 255);
	entry->name[255] = '\0';
	strncpy(entry->description, (description != nullptr) ? description : "(no name)", 255);
	entry->description[255] = '\0';
	entry->func = func;
	netdev_list.push_back(std::move(entry));
}

void clear_netdev()
{
	netdev_list.clear();
}

const std::vector<std::unique_ptr<osd_network_device::entry_t>>& get_netdev_list()
{
	return netdev_list;
}

osd_network_device *open_netdev(int id, osd::network_handler &ifdev)
{
	for(auto &entry : netdev_list)
		if(entry->id==id)
			return entry->func(entry->name, ifdev);
	return nullptr;
}

osd_network_device::osd_network_device(osd::network_handler &ifdev)
	: m_dev(ifdev)
	, m_stopped(true)
{
}

osd_network_device::~osd_network_device()
{
}

void osd_network_device::start()
{
	m_stopped = false;
}

void osd_network_device::stop()
{
	m_stopped = true;
}

void osd_network_device::poll()
{
	uint8_t *buf;
	int len;
	//const char atalkmac[] = { 0x09, 0x00, 0x07, 0xff, 0xff, 0xff };
	while(!m_stopped && (len = recv_dev(&buf)))
	{
#if 0
		if(buf[0] & 1)
		{
			if(memcmp("\xff\xff\xff\xff\xff\xff", buf, 6) && memcmp(atalkmac, buf, 6) && !m_dev.mcast_chk(buf, len)) continue;
		}
		else {
			//const unsigned char *ourmac = (const unsigned char *)get_mac();
			//printf("our mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X dst mac: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", ourmac[0], ourmac[1], ourmac[2], ourmac[3], ourmac[4], ourmac[5], buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
			if(memcmp(get_mac(), buf, 6) && !get_promisc()) continue;
		}
#endif

		m_dev.recv_cb(buf, len);
	}
}

int osd_network_device::send(uint8_t *buf, int len)
{
	return 0;
}

int osd_network_device::recv_dev(uint8_t **buf)
{
	return 0;
}

void osd_network_device::set_mac(const uint8_t *mac)
{
}

void osd_network_device::set_promisc(bool promisc)
{
}

bool osd_network_device::get_promisc()
{
	return m_dev.get_promisc();
}

const std::array<uint8_t, 6> &osd_network_device::get_mac()
{
	return m_dev.get_mac();
}
