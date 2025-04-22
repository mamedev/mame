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
	while(!m_stopped && (len = recv_dev(&buf)))
	{
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
