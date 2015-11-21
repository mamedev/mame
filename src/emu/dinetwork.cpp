// license:BSD-3-Clause
// copyright-holders:Carl, Miodrag Milanovic
#include "emu.h"
#include "osdnet.h"

device_network_interface::device_network_interface(const machine_config &mconfig, device_t &device, float bandwidth)
	: device_interface(device, "network")
{
	m_promisc = false;
	m_bandwidth = bandwidth;
	set_mac("\0\0\0\0\0\0");
	m_intf = 0;
}

device_network_interface::~device_network_interface()
{
}

int device_network_interface::send(UINT8 *buf, int len) const
{
	if(!m_dev) return 0;
	return m_dev->send(buf, len);
}

void device_network_interface::recv_cb(UINT8 *buf, int len)
{
}

void device_network_interface::set_promisc(bool promisc)
{
	m_promisc = promisc;
	if(m_dev) m_dev->set_promisc(promisc);
}

void device_network_interface::set_mac(const char *mac)
{
	memcpy(m_mac, mac, 6);
	if(m_dev) m_dev->set_mac(m_mac);
}

void device_network_interface::set_interface(int id)
{
	m_dev.reset(open_netdev(id, this, (int)(m_bandwidth*1000000/8.0f/1500)));
	if(!m_dev) {
		device().logerror("Network interface %d not found\n", id);
		id = -1;
	}
	m_intf = id;
}
