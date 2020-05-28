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
	m_loopback_control = false;
}

device_network_interface::~device_network_interface()
{
}

void device_network_interface::interface_pre_start()
{
	m_send_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_network_interface::send_complete), this));
	m_recv_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_network_interface::recv_complete), this));
}

void device_network_interface::interface_post_start()
{
	device().save_item(NAME(m_loopback_control));
}

int device_network_interface::send(u8 *buf, int len)
{
	// TODO: enable this check when other devices implement delayed transmit
	//if (m_send_timer->enabled())
		//throw emu_fatalerror("%s(%s): attempted to transmit while transmit already in progress", device().shortname(), device().tag());

	int result = 0;

	if (m_loopback_control)
	{
		// loop data back to receiver
		result = recv_start_cb(buf, len);

		if (result)
		{
			// schedule receive complete callback
			m_recv_timer->adjust(attotime::from_ticks(len, m_bandwidth * 1'000'000 / 8), result);
		}
	}
	else if (m_dev)
	{
		// send the data
		result = m_dev->send(buf, len);
	}

	// schedule transmit complete callback
	m_send_timer->adjust(attotime::from_ticks(len, m_bandwidth * 1'000'000 / 8), result);

	return result;
}

TIMER_CALLBACK_MEMBER(device_network_interface::send_complete)
{
	send_complete_cb(param);
}

void device_network_interface::recv_cb(u8 *buf, int len)
{
	if (m_recv_timer->enabled())
		throw emu_fatalerror("%s(%s): attempted to receive while receive already in progress", device().shortname(), device().tag());

	int result = 0;

	// process the received data
	if (!m_loopback_control)
		result = recv_start_cb(buf, len);

	if (result)
	{
		// stop receiving more data from the network
		if (m_dev)
			m_dev->stop();

		// schedule receive complete callback
		m_recv_timer->adjust(attotime::from_ticks(len, m_bandwidth * 1'000'000 / 8), result);
	}
}

TIMER_CALLBACK_MEMBER(device_network_interface::recv_complete)
{
	recv_complete_cb(param);

	// start receiving data from the network again
	if (m_dev && !m_loopback_control)
		m_dev->start();
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
	if(m_dev)
		m_dev->stop();
	m_dev.reset(open_netdev(id, this, (int)(m_bandwidth*1000000/8.0f/1500)));
	if(!m_dev) {
		device().logerror("Network interface %d not found\n", id);
		id = -1;
	}
	m_intf = id;
}

void device_network_interface::set_loopback(bool loopback)
{
	if (m_loopback_control == loopback)
		return;

	m_loopback_control = loopback;

	if (m_dev)
	{
		if (loopback)
			m_dev->stop();
		else if (!m_recv_timer->enabled())
			m_dev->start();
	}
}
